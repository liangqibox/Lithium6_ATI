#include "autosave.h"
#include "ui_autosave.h"

AutoSave::AutoSave(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AutoSave)
{
    ui->setupUi(this);

    number_of_scan = 0;
    number_of_run = 0;
    cycle_time = 0;
    Scan_count = 0;
    Load_default_setting();
    startTime = QDateTime::currentDateTime();
}

AutoSave::~AutoSave()
{
    delete ui;
}

void AutoSave::init_variable(QList<Variable *> *ivar){
    variables = ivar;
}

void AutoSave::Load_default_setting(){
    QFile file("Default.txt");
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))return;
    QTextStream in(&file);

    while(!in.atEnd()){
        QString temp = in.readLine();
        if(temp.contains("[AutoSave Enable]")){
            QString a = in.readLine();
            if(a.contains("Yes")){
                ui->Enable->setChecked(true);
            }
            else{
                ui->Enable->setChecked(false);
            }
        }
        else if(temp.contains("[AutoSave Setting Enable]")){
            QString a = in.readLine();
            if(a.contains("Yes")){
                ui->Setting->setChecked(true);
            }
            else{
                ui->Setting->setChecked(false);
            }
        }
        else if(temp.contains("[AutoSave Description]")){
            QString a = in.readLine();
            ui->Filename->setText(a);
        }
        else if(temp.contains("[AutoSave Path]")){
            QString a = in.readLine();
            save_path = a;
        }
    }
    file.close();
}

void AutoSave::set_enable(bool en){
    ui->Enable->setChecked(en);
}

void AutoSave::set_path(QString path){
    save_path = path;
}

void AutoSave::set_description(QString string){
    ui->Filename->setText(string);
}

bool AutoSave::get_enable(){
    return ui->Enable->isChecked();
}

QString AutoSave::get_path(){
    return save_path;
}

QString AutoSave::get_description(){
    return ui->Filename->text();
}

void AutoSave::on_Path_clicked()
{
    save_path = QFileDialog::getExistingDirectory(this,"Select save folder");
}

void AutoSave::Update(QString msg){
    QString current_time = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    QStringList msglist = msg.split(':');
    cycle_time = msglist.at(0).toDouble();
    number_of_run = msglist.at(1).toInt();
    number_of_scan = msglist.at(2).toInt();

    QString description = ui->Filename->text();
    for(int i=0; i<variables->size(); i++){
        if(description.contains("["+variables->at(i)->get_name()+"]")){
            switch(variables->at(i)->get_type()){
            case 0:
                description.replace("["+variables->at(i)->get_name()+"]",QString::number(variables->at(i)->get_value()[0]));
                break;
            case 1:
                description.replace("["+variables->at(i)->get_name()+"]",QString::number(variables->at(i)->get_scan(number_of_scan)));
                break;
            case 2:
                description.replace("["+variables->at(i)->get_name()+"]",QString::number(variables->at(i)->get_calculated(number_of_scan,0,0,0,0)));
                break;
            default:
                break;
            }
        }
    }

    if(number_of_scan == 1){
        if(QDate::currentDate()>startTime.date()){
            Scan_count = 0;
            //startTime = QDateTime::currentDateTime();
            startTime.setDate(QDate::currentDate());
            startTime.setTime(QTime::currentTime());
        }
        Scan_count++;
        if(ui->Enable->isChecked())save_scan_summary();
        if(ui->Enable->isChecked() && ui->Setting->isChecked()){
            emit save_setting(save_path+"/"+description+"_"+current_time+".adcs");
        }
    }
    else if(number_of_run == 1){
        if(ui->Enable->isChecked() && ui->Setting->isChecked()){
            emit save_setting(save_path+"/"+description+"_"+current_time+".adcs");
        }
    }
    if(ui->Enable->isChecked())save_file(save_Filename());
}

QString AutoSave::save_Filename(){
    QString current_time = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    QString filename;
    if(number_of_scan<1)filename = save_path + "/" + ui->Filename->text() + "_" + current_time + ".txt";
    else{
        QString start = startTime.toString("yyyyMMddHHmmss");
        QString scan = "scan" + QString::number(Scan_count);
        filename = save_path + "/Scan_" + start + "_" + ui->Filename->text() + "_" + scan + "/" + "Cycle_" + current_time + ".txt";
    }
    return filename;
}

void AutoSave::save_scan_summary(){
    QString start = startTime.toString("yyyyMMddHHmmss");
    QString scan = "scan" + QString::number(Scan_count);
    const QString newdir = save_path + "/Scan_" + start + "_" + ui->Filename->text() + "_" + scan + "/";
    QDir dir;
    dir.mkdir(newdir);
    QString filename = save_path + "/Scan_" + start + "_" + ui->Filename->text() + "_" + scan + ".txt";

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))return;
    QTextStream out(&file);
    out << "Lithium,ScanSummaryFile" << endl;
    out << "scan,1" << endl;
    out << "scan_name," << "Scan_"+ start + "_" + ui->Filename->text() + "_" + scan << endl;

    int counter = 0;
    for(int i=0; i<variables->size(); i++){
        if(variables->at(i)->get_type() == 1){
            counter++;
            QString name = "scan_var" + QString::number(counter) + "_";
            out << name+"name," << variables->at(i)->get_name() << endl;
            out << name+"prior," << variables->at(i)->get_value()[0] << endl;
            out << name+"start," << variables->at(i)->get_value()[1] << endl;
            out << name+"step," << variables->at(i)->get_value()[3] << endl;
            out << name+"end," << variables->at(i)->get_value()[2] << endl;
        }
    }
    file.close();
}

void AutoSave::save_file(QString filename){
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))return;
    QTextStream out(&file);
    out << "Cycle,DataFile" << endl;
    if(number_of_scan==0) out << "scan,0" << endl;
    else out << "scan,1" << endl;
    out << "run_cycle_count," << number_of_run << endl;
    out << "scan_cycle_count," << number_of_scan << endl;
    out << "Cycle_time," << cycle_time << endl;

    for(int i=0; i<variables->size(); i++){
        if(variables->at(i)->get_type()<3)out << variables->at(i)->get_name() << ",";
        switch (variables->at(i)->get_type()) {
        case 0:
            out << variables->at(i)->get_value()[0] << endl;
            break;
        case 1:
            out << variables->at(i)->get_scan(number_of_scan) << endl;
            break;
        case 2:
            out << variables->at(i)->get_calculated(number_of_scan,0,0,0,0) << endl;
            break;
        default:
            break;
        }
    }
    file.close();
}
