#include "ddscontrol.h"
#include "ui_ddscontrol.h"
#include <QDebug>

DDSControl::DDSControl(QWidget *parent, QList<Variable *> *vari, QList<QString> *chlname, QiScriptEngineCluster *eng, int num_of_analog, int num_of_digi) :
    QWidget(parent),
    ui(new Ui::DDSControl)
{
    ui->setupUi(this);

    variables = vari;
    Engine = eng;
    channel_names = chlname;
    Analog_output_channel = num_of_analog;
    Digital_output_channel = num_of_digi;

    Initialize();
}

DDSControl::~DDSControl()
{
    for(int i=0; i<DDS_Devices->size(); i++)DDS_Devices->at(i)->deleteLater();
    delete DDS_Devices;
    delete ui;
}

void DDSControl::Initialize(){
    QList<QString> available_device = DDSController::availablePorts();
    DDS_Devices = new QList<DDSDevicePanel*>;
    for(int i=0; i<available_device.size(); i++){
        DDSDevicePanel *panel = new DDSDevicePanel(ui->Device_Panel,available_device.at(i),channel_names,Analog_output_channel,Digital_output_channel);
        DDS_Devices->append(panel);
        connect(panel,SIGNAL(ErrorReturn(QString)),this,SLOT(ErrorReceive(QString)));
        panel->move(10,15+95*i);
        panel->show();
    }
    ui->Manual_Device->addItems(QStringList()<<available_device);
    ui->Device_Panel->setMinimumHeight(20+95*available_device.size());
    ui->Status->setText(QString::number(available_device.size())+" Devices Found");
}

void DDSControl::Receive_new_cycle(QString msg){
    QStringList temp = msg.split(':');
    Number_of_Run = temp.at(1).toInt();
    Number_of_Scan = temp.at(2).toInt();

    //Update_all_device();
    on_Manual_Start_clicked();
}

void DDSControl::new_CycleSetting(QString setting){
    Manual_Filename = setting;
}

QString DDSControl::current_Setting(){
    return Manual_Filename;
}

void DDSControl::on_Refresh_clicked()
{
    QList<QString> available_device = DDSController::availablePorts();
    for(int i=0; i<DDS_Devices->size(); i++){
        bool exist = false;
        for(int j=0; j<available_device.size(); j++){
            if(DDS_Devices->at(i)->Port()==available_device.at(j)){
                exist = true;
                available_device.removeAt(j);
                break;
            }
        }
        if(!exist){
            delete DDS_Devices->at(i);
            DDS_Devices->removeAt(i);
            i--;
        }
    }

    for(int i=0; i<available_device.size(); i++){
        //QList<QList<Sequence*>*> *dds = new QList<QList<Sequence*>*>;
        //DDS_sequences->append(dds);
        DDSDevicePanel *panel = new DDSDevicePanel(ui->Device_Panel,available_device.at(i),channel_names,Analog_output_channel,Digital_output_channel);
        DDS_Devices->append(panel);
        connect(panel,SIGNAL(ErrorReturn(QString)),this,SLOT(ErrorReceive(QString)));
        panel->show();
    }
    QStringList devicelist;
    for(int i=0; i<DDS_Devices->size(); i++){
        DDS_Devices->at(i)->move(10,15+95*i);
        devicelist.append(DDS_Devices->at(i)->Port());
    }
    ui->Manual_Device->clear();
    ui->Manual_Device->addItems(devicelist);
    ui->Device_Panel->setMinimumHeight(20+95*DDS_Devices->size());
    ui->Status->setText(QString::number(DDS_Devices->size())+" Devices Found");
}

void DDSControl::Update_all_device(){
    for(int i=0; i<DDS_Devices->size(); i++){
        DDS_Devices->at(i)->update_Device();
    }
}

void DDSControl::on_Manual_File_clicked()
{
    //Manual_Command.clear();
    Manual_Filename = QFileDialog::getOpenFileName(0,tr("Select File"), "", tr("Text File (*.txt)"));
    /*QFile file(Manual_Filename);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        ui->Manual_Status->setText("File Open Fail");
        return;
    }
    QTextStream in(&file);
    while(!in.atEnd())Manual_Command.append(in.readLine());
    file.close();*/
    if(!Manual_Filename.isEmpty())ui->Manual_Status->setText("File Loaded");
}

void DDSControl::Load_Command_from_File(QString filename){
    Manual_Command.clear();
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        ui->Manual_Status->setText("File Open Fail");
        return;
    }
    QTextStream in(&file);
    while(!in.atEnd())Manual_Command.append(in.readLine());
    file.close();
    ui->Manual_Status->setText("File Loaded");
}

void DDSControl::on_Manual_Start_clicked()
{
    if(DDS_Devices->size()==0)return;
    Load_Command_from_File(Manual_Filename);
    int deviceno = ui->Manual_Device->currentIndex();
    DDS_Devices->at(deviceno)->Reset();
    DDSCycle com;
    int counter = 0;
    for(int i=0; i<Manual_Command.size(); i++){
        QList<float> vari;
        QStringList split = Manual_Command.at(i).split(':');
        for(int j=0; j<split.size(); j++){
            bool ok = false;
            float temp = split.at(j).toFloat(&ok);
            if(ok)vari.append(temp);
            else if(split.at(j).contains('[')){
                QString name = split.at(j);
                name.remove('[');
                name.remove(']');
                for(int k=0; k<variables->size(); k++){
                    if(variables->at(k)->get_name()==name){
                        switch(variables->at(k)->get_type()){
                        case 0:
                            vari.append(variables->at(k)->get_value()[0]);
                            break;
                        case 1:
                            vari.append(variables->at(k)->get_scan(Number_of_Scan));
                            break;
                        case 2:
                            vari.append(variables->at(k)->get_calculated(Number_of_Scan,0,0,0,0));
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }
        if(!vari.isEmpty())counter++;
        switch(vari.size()){
        case 1:
            com.singleTone(vari.at(0)*1000000);
            break;
        case 2:
            com.singleTone(vari.at(0)*1000000);
            break;
        case 3:
            com.linearSweep(vari.at(0)*1000000,vari.at(1)*1000000,vari.at(2)/1000);
            break;
        case 4:
            com.linearSweep(vari.at(0)*1000000,vari.at(1)*1000000,vari.at(2)/1000,bool(vari.at(3)));
            break;
        default:
            break;
        }
    }
    if(counter>0){
        DDS_Devices->at(deviceno)->update_Device(com);
        ui->Manual_Status->setText("Update Completed");
        ui->ErrorReturn->append("Update Completed");
    }
    else{
        ui->Manual_Status->setText("No Vaild Input");
        ui->ErrorReturn->append("No Vaild Input");
    }
}

void DDSControl::ErrorReceive(QString err){
    ui->ErrorReturn->append(err);
}
