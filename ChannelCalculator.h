#ifndef CHANNELCALCULATOR_H
#define CHANNELCALCULATOR_H

#include <QObject>
#include <QDialog>
#include <QThread>
#include <QList>
#include <QScriptEngine>
#include <QScriptEngineDebugger>
#include <QScriptValueIterator>
#include <QElapsedTimer>
#include <QDebug>
#include "qiscriptenginecluster.h"
#include "DataType/sequence.h"
#include "DataType/variable.h"

class Worker : public QObject
{
    Q_OBJECT
    QList<QList<Sequence*>**> *sequences;
    QList<Variable*> *variables;
    QList<QString> *channel_calibration;
    QList<bool> *section_activity;
    int time_resolution;
    int **transfer_data;
    int *total_number_of_scan;
    int number_of_scan;
    int total_step;
    int Analog_output_channel,Digital_output_channel;
    double *transfer_section_time;
    int channel;

    QScriptEngine *Engine;

public:
    Worker(QScriptEngine *eng,QList<QList<Sequence*>**>*iseq,QList<Variable*> *ivari, QList<QString>*chl_cal, QList<bool>*sec_act,
                      int time_res,int**trans_d,int AO,int DiO,int *total,int scan,int step,int chl){
        Engine = eng;
        sequences = iseq;
        variables = ivari;
        channel_calibration = chl_cal;
        section_activity = sec_act;
        time_resolution = time_res;
        transfer_data = trans_d;
        Analog_output_channel = AO;
        Digital_output_channel = DiO;
        total_number_of_scan = total;
        number_of_scan = scan;
        total_step = step;
        channel = chl;
        const int number_of_section = sequences->size();
        transfer_section_time = new double[number_of_section];
        calculate_transfer_section_time();
    }

    ~Worker(){
        delete transfer_section_time;
    }

signals:
    void done(int);

public slots:
    void doWork(){
        if(channel<Analog_output_channel){
            int last_out=32768;
            for(int i=0; i<total_step; ++i){
                double time = i*time_resolution/1000.;
                int temp = find_output_in_time(channel,time,last_out);
                transfer_data[channel][i] = temp;
                last_out = temp;
            }
        }
        else{
            int chl = (channel - Analog_output_channel)%32;
            int binary = 0;
            //if(chl == 31) binary = -2147483648;
            if(chl == 31) binary = 0x80000000;
            else binary = pow(2,chl);
            int last_out = 0;
            for(int i=0; i<total_step;++i){
                double time = i*time_resolution/1000.;
                last_out = find_output_in_time(channel,time,last_out);
                if(last_out > 0.5)transfer_data[Analog_output_channel][i] += binary;
            }
        }
        emit done(channel);
    }

private slots:
    void calculate_transfer_section_time(){
        for(int i=0; i<sequences->size(); ++i){
            double sect = 0;
            if(section_activity->at(i)){
                for(int j=0; j<Analog_output_channel+Digital_output_channel; ++j){
                    double temp =0;
                    for(int k=0; k<sequences->at(i)[j]->size(); ++k){
                        temp += sequences->at(i)[j]->at(k)->get_time(Engine,number_of_scan);
                    }
                    if(temp>sect)sect=temp;
                }
            }
            transfer_section_time[i] = sect;
        }
    }

    int find_output_in_time(int chl, double time, int last_out){
        double time1 = 0;
        double time2 = 0;
        double time3 = 0;
        int ii = 0;

        for(int i=0; i<sequences->size(); i++){
            if(time3<=time && time<=(time3+transfer_section_time[i])){
                ii = i;
                time2 = time3;
                break;
            }
            else{
                time3 += transfer_section_time[i];
            }
        }
        for(int i=0; i<sequences->at(ii)[chl]->size();i++){
            if(sequences->at(ii)[chl]->at(i)->get_type()!=0){
                time1 = time2;
                time2 += sequences->at(ii)[chl]->at(i)->get_time(Engine,number_of_scan);;
            }
            if((time1<=time) && (time<time2)){
                QString cal = "Not calibrated";
                if(chl < Analog_output_channel)cal = channel_calibration->at(chl);
                double voltage = sequences->at(ii)[chl]->at(i)->get_output(Engine,number_of_scan,time-time1,time-time3,time,cal);
                if(chl>(Analog_output_channel-1))return int(voltage);
                else return int((voltage+10)*3276.75);
            }
            if(i==sequences->at(ii)[channel]->size()-1 && time>time2){
                return last_out;
            }
        }
        return last_out;
    }
};

class ChannelCalculator : public QObject
{
    Q_OBJECT
    QList<QList<Sequence*>**> *sequences;
    QList<Variable*> *variables;
    QList<QString> *channel_calibration;
    QList<QString> *sections_activity;
    QList<bool> *sections_activity_bool;
    int time_resolution;
    int **transfer_data;
    int *total_number_of_scan;
    int number_of_scan;
    int total_step;
    int Analog_output_channel,Digital_output_channel;
    QList<bool> progress;
    bool cycle_wrong;

    //QScriptEngine *MainEngine;
    QiScriptEngineCluster *MainEngine;
    QThread *workerThread;

public:
    ChannelCalculator(QiScriptEngineCluster *eng, QList<QList<Sequence*>**>*iseq,QList<Variable*> *ivari, QList<QString>*chl_cal, QList<QString> *sec_act,
                      int time_res,int**trans_d,int AO,int DiO,int *total,int scan,int step){
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
        progress.clear();
        //progress = 0;

        workerThread = new QThread[Analog_output_channel+Digital_output_channel];
    }

    ~ChannelCalculator() {
        for(int i=0; i<Analog_output_channel+Digital_output_channel; i++){
            workerThread[i].quit();
            workerThread[i].wait();
            workerThread[i].deleteLater();
        }
        delete sections_activity_bool;
        //delete workerThread;
    }

    void run(){
        calculate_section_activity();
        for(int i=0; i<Analog_output_channel+Digital_output_channel; i++){
            QScriptEngine *SubEngine = new QScriptEngine;
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
            Worker *worker = new Worker(SubEngine,sequences,variables,channel_calibration,sections_activity_bool,time_resolution,transfer_data,
                                        Analog_output_channel,Digital_output_channel,total_number_of_scan,number_of_scan,total_step,i);
            worker->moveToThread(&workerThread[i]);
            connect(this,SIGNAL(start()),worker,SLOT(doWork()));
            connect(this,SIGNAL(start()),&workerThread[i],SLOT(start()));
            connect(worker,SIGNAL(done(int)),this,SLOT(cycle(int)));
            connect(worker,SIGNAL(done(int)),SubEngine,SLOT(deleteLater()));
            connect(worker,SIGNAL(done(int)),worker,SLOT(deleteLater()));
            //connect(worker,SIGNAL(done(int)),&workerThread[i],SLOT(quit()));
            //connect(worker,SIGNAL(done(int)),&workerThread[i],SLOT(deleteLater()));
        }
        emit start();
        /*for(int i=0; i<Analog_output_channel+Digital_output_channel; i++){
            workerThread[i].start();
            //qDebug() << "Thread" << i << "Started";
            //workerThread[i].msleep(5);
        }*/
    }

    double Progress(){
        double p = double(progress.size())/double(Analog_output_channel+Digital_output_channel);
        return p;
    }

private slots:
    void cycle(int){
        progress.append(true);
        //qDebug() << "Progress:" << progress.size() << "out of" << Analog_output_channel+Digital_output_channel;
        if(progress.size()<(Analog_output_channel+Digital_output_channel)){
            double p = double(progress.size())*100/double(Analog_output_channel+Digital_output_channel);
            emit finishing(p);
        }
        else emit finished();
    }

    void calculate_section_activity(){
        for(int i=0; i<sections_activity->size(); i++){
            double act = 0;
            bool ok = false;
            act = sections_activity->at(i).toDouble(&ok);

            if(!ok){
                for(int j=0; j<variables->size(); j++){
                    if(sections_activity->at(i)==variables->at(i)->get_name()){
                        switch(variables->at(i)->get_type()){
                        case 0:
                            act = variables->at(i)->get_value()[0];
                            break;
                        case 1:
                            act = variables->at(i)->get_scan(number_of_scan);
                            break;
                        case 2:
                            act = variables->at(i)->get_calculated(number_of_scan,0,0,0,0);
                            break;
                        default:
                            break;
                        }
                        break;
                    }
                }
            }

            if(act>0) sections_activity_bool->append(true);
            else sections_activity_bool->append(false);
        }
    }

signals:
    void start();
    void finishing(double);
    void finished();
};

#endif // CHANNELCALCULATOR_H
