#ifndef SEQUENCECALCULATOR
#define SEQUENCECALCULATOR

#include <QObject>
#include <QDialog>
#include <QThreadPool>
#include <QRunnable>
#include <QList>
#include <QScriptEngine>
#include <QScriptEngineDebugger>
#include <QScriptValueIterator>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QTime>
#include <QDebug>
#include "definition.h"
#include "qiscriptenginecluster.h"
#include "DataType/sequence.h"
#include "DataType/variable.h"
#include "DataType/sectionactivity.h"

const int Maximum_number_of_running_thread = 100;

class SeqWorker :  public QObject, public QRunnable
{
    Q_OBJECT

private:
    QScriptEngine *Engine;
    int *channel_data;
    double cyc_time,sec_time,seq_time;
    int time_resolution;
    int number_of_scan;
    double single_time_step;
    Sequence *sequence;
    QString calibration;

public:
    SeqWorker(QScriptEngine *eng, Sequence* seq, double sect, double cyct, int t_res, int num_of_scan ,int *chl_dat, QString cal){
        Engine = eng;
        sequence = seq;
        sec_time = sect;
        cyc_time = cyct+sect;
        seq_time = 0;
        number_of_scan = num_of_scan;
        time_resolution = t_res;
        channel_data = chl_dat;
        calibration = cal;

        single_time_step = double(time_resolution)/1000.;
    }

    ~SeqWorker(){
    }

signals:
    void done();

public slots:
    void run(){
        int total_step = sequence->get_time(Engine,number_of_scan)*1000./time_resolution;
        double start_point = cyc_time*1000/time_resolution;
        int type = sequence->get_type();
        if(total_step>0){
            for(int i=0; i<total_step; i++){
                double output = sequence->get_output(Engine,number_of_scan,seq_time,sec_time,cyc_time,calibration);
                if(type==7)channel_data[int(start_point)+i] = -1;
                else if(type==-1)channel_data[int(start_point)+i] = output;
                else channel_data[int(start_point)+i] = int((output+10)*3276.75);

                seq_time += single_time_step;
                sec_time += single_time_step;
                cyc_time += single_time_step;
            }
        }
        else{
            if(abs(total_step)>start_point)total_step = start_point*-1;
            for(int i=-1; i>=total_step; i--){
                double output = sequence->get_output(Engine,number_of_scan,seq_time,sec_time,cyc_time,calibration);
                if(type==7)channel_data[int(start_point)+i] = -1;
                else if(type==-1)channel_data[int(start_point)+i] = output;
                else channel_data[int(start_point)+i] = int((output+10)*3276.75);

                seq_time -= single_time_step;
                sec_time -= single_time_step;
                cyc_time -= single_time_step;
            }
        }
        emit done();
    }
};


class SequenceCalculator : public QObject
{
    Q_OBJECT

private:
    QList<QList<Sequence*>**> *sequences;
    QList<Variable*> *variables;
    QList<QString> *channel_calibration;
    QList<SectionActivity*> *sections_activity;
    QList<bool> *sections_activity_bool;
    int time_resolution;
    int **transfer_data;
    int *total_number_of_scan;
    int number_of_scan;
    int total_step;
    int Analog_output_channel,Digital_output_channel;
    QList<bool> progress;
    int total_number_of_sequence;
    bool cycle_wrong;
    bool fuse_digital;

    //QiScriptEngineCluster *MainEngine;
    QScriptEngine *MainEngine;
    QThreadPool *threadPool;
    QThreadPool *threadPool_negative;
    QList<SeqWorker*> negative_sequences;
    QList<QScriptEngine*> SubEngine;

public:
    SequenceCalculator(QScriptEngine *eng, QList<QList<Sequence*>**>*iseq,QList<Variable*> *ivari, QList<QString>*chl_cal, QList<SectionActivity*> *sec_act,
                      int time_res,int**trans_d,int AO,int DiO,int *total,int scan,int step,bool fuse){
        this->setParent(NULL);
        MainEngine = eng;
        sequences = iseq;
        variables = ivari;
        channel_calibration = chl_cal;
        sections_activity = sec_act;
        sections_activity_bool = new QList<bool>;
        time_resolution = time_res;
        transfer_data = trans_d;
        Analog_output_channel = AO;
        Digital_output_channel = DiO;
        total_number_of_scan = total;
        number_of_scan = scan;
        total_step = step;
        total_number_of_sequence = 0;

        fuse_digital = fuse;
        progress.clear();
        SubEngine.clear();
        threadPool = new QThreadPool;
        threadPool->setMaxThreadCount(Maximum_number_of_running_thread);
        threadPool->setExpiryTimeout(-1);
        threadPool_negative = new QThreadPool;
        threadPool_negative->setMaxThreadCount(Maximum_number_of_running_thread);
        threadPool_negative->setExpiryTimeout(-1);
    }

    ~SequenceCalculator(){
        threadPool->waitForDone(1000);
        threadPool_negative->waitForDone(1000);
        threadPool->deleteLater();
        threadPool_negative->deleteLater();
        delete sections_activity_bool;
        for(int i=0; i<SubEngine.size(); i++){
            delete SubEngine.at(i);
        }
        SubEngine.clear();
    }

    bool run(){
        calculate_section_activity();
        double total_time = 0;
        int counter = 0;
        for(int sec=0; sec<sequences->size(); sec++){
            double section_time = 0;
            if(sections_activity_bool->at(sec)){
            for(int chl=0; chl<Analog_output_channel+Digital_output_channel; chl++){
                double channel_time = 0;
                for(int seq=0; seq<sequences->at(sec)[chl]->size(); seq++){
                    Sequence *sequence = sequences->at(sec)[chl]->at(seq);
                    if(sequence->get_type()!=0){
                        QScriptEngine *engine = copyEngine();
                        SubEngine.append(engine);
                        double seq_time = sequence->get_time(engine,number_of_scan);
                        QString cal = "only_system_used_null";
                        cal = channel_calibration->at(chl);
                        SeqWorker *seqWorker = new SeqWorker(engine,sequence,channel_time,total_time,time_resolution,number_of_scan,transfer_data[chl],cal);
                        seqWorker->setAutoDelete(true);
                        connect(seqWorker,SIGNAL(done()),this,SLOT(cycle()));
                        if(seq_time>0){
                            channel_time += seq_time;
                            //seqWorker->run(); // Testing code without multithread.
                            threadPool->start(seqWorker,QThread::NormalPriority);
                        }
                        else if(seq_time<0){
                            negative_sequences.append(seqWorker);
                            //seqWorker->run();
                        }
                        counter++;
                        //qDebug() << "Sequence" << counter << "of" << total_number_of_sequence << "Started. Section" << sec << "Channel" << chl << "Sequence" << seq;
                        Delay(1);
                    }
                }
                if(channel_time>section_time)section_time = channel_time;
            }
            }
            total_time += section_time;
        }
        if(threadPool->waitForDone()){
            if(negative_sequences.size()==0){
                fill_Transfer_Data();
                return true;
            }
            else for(int i=0; i<negative_sequences.size(); i++)threadPool_negative->start(negative_sequences.at(i),QThread::NormalPriority);
        }
        if(threadPool_negative->waitForDone()){
            fill_Transfer_Data();
            return true;
        }
        return false;
    }

    double Progress(){
        double p = double(progress.size())*100/double(total_number_of_sequence);
        return p;
    }

private slots:
    void cycle(){
        progress.append(true);
        if(progress.size()<total_number_of_sequence){
            double p = double(progress.size())*100/double(total_number_of_sequence);
            emit finishing(p);
        }
        //else fill_Transfer_Data();
    }

    void calculate_section_activity(){
        for(int i=0; i<sections_activity->size(); i++){
            sections_activity_bool->append(sections_activity->at(i)->get_activity(number_of_scan));
        }

        for(int i=0; i<sequences->size(); i++){
            if(sections_activity_bool->at(i)){
                for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
                    for(int k=0; k<sequences->at(i)[j]->size(); k++){
                        if(sequences->at(i)[j]->at(k)->get_type()!=0)total_number_of_sequence++;
                    }
                }
            }
        }
    }

    QScriptEngine *copyEngine(QObject *parent = 0){
        QScriptEngine *SubEngine = new QScriptEngine(parent);
        QScriptValueIterator it(MainEngine->globalObject());
        while(it.hasNext()){
            it.next();
            QString name = it.name();
            QScriptValue value = it.value();
            if(value.isNumber())SubEngine->globalObject().setProperty(name,SubEngine->evaluate(value.toString()));
            else if(value.isFunction()){
                QString code = value.toString();
                if(!code.contains("native"))SubEngine->globalObject().setProperty(name,SubEngine->evaluate("("+code+")"));
            }
            else if(value.isArray()){
                QString array = value.toString();
                array.remove('"');
                array.prepend('[');
                array.append(']');
                SubEngine->globalObject().setProperty(name,SubEngine->evaluate(array));
            }
        }
        return SubEngine;
    }

    void fill_Transfer_Data(){
        qDebug() << "All Sequences Calculation Complete.";
        for(int chl=0; chl<Analog_output_channel+Digital_output_channel; chl++){
            int last_out = 0;
            if(chl<Analog_output_channel)last_out = 32767;
            for(int i=0; i<total_step; i++){
                if(transfer_data[chl][i]<0)transfer_data[chl][i] = last_out;
                else last_out = transfer_data[chl][i];
            }
        }
        qDebug() << "Fill Un-define Data.";
        if(fuse_digital)fuse_Digital_Data();
        else emit finished();
    }

    void fuse_Digital_Data(){
        int base[32];
        for(int i=0; i<31; i++)base[i] = pow(2,i);
        //base[31] = -2147483648;
        base[31] = 0x80000000;

        const int number_of_digital_card = Digital_output_channel/32;

        for(int i=0; i<total_step; i++){
            for(int n=0; n<number_of_digital_card; n++){
                int temp = 0;
                for(int chl=0; chl<32; chl++){
                    if(transfer_data[chl+Analog_output_channel+n*32][i]>0)temp += base[chl];
                }
                transfer_data[Analog_output_channel+n][i] = temp;
            }
        }
        qDebug() << "Fuse Digital Data into Signal Channel";
        emit finished();
    }

    void Delay(int millisecondsToWait){
        QTime dieTime = QTime::currentTime().addMSecs(millisecondsToWait);
        while(QTime::currentTime() < dieTime){
            QCoreApplication::processEvents(QEventLoop::AllEvents,100);
        }
    }

signals:
    void start();
    void finishing(double);
    void finished();
};

#endif // SEQUENCECALCULATOR

