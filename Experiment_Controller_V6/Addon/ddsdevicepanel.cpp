#include "ddsdevicepanel.h"
#include "ui_ddsdevicepanel.h"
#include <QDebug>

DDSDevicePanel::DDSDevicePanel(QWidget *parent, QString p, QList<QString> *chlname, int num_of_analog, int num_of_digi) :
    QWidget(parent),
    ui(new Ui::DDSDevicePanel)
{
    ui->setupUi(this);

    amplitude = 1;
    phase = 0;
    port = p;
    channel_names = chlname;
    Analog_output_channel = num_of_analog;
    Digital_output_channel = num_of_digi;
    QStringList temp;
    for(int i=0; i<Digital_output_channel; i++)temp.append(channel_names->at(i+Analog_output_channel));
    temp.prepend("None");
    QiCombobox *combobox = new QiCombobox(ui->Panel);
    combobox->setGeometry(280,45,69,25);
    combobox->addItems(temp);
    connect(combobox,SIGNAL(currentIndexChanged(int)),this,SLOT(Merge_Channel_Change(int)));
    combobox->show();
    ui->Panel->setTitle(port);
    ui->Amplitude->installEventFilter(this);
    ui->Phase->installEventFilter(this);
    device = new DDSController(port,NULL);
    if(device->isOpen()){
        ChangeIndicator(1);
    }
    else ChangeIndicator(2);
}

DDSDevicePanel::~DDSDevicePanel()
{
    device->Close();
    delete ui;
}

QString DDSDevicePanel::Port(){
    return port;
}

void DDSDevicePanel::Reset(){
    uint8_t error = device->stop_cycle();
    if(error){
        emit ErrorReturn(QString::number(error));
        ChangeIndicator(2);
    }

    else ChangeIndicator(1);
}

void DDSDevicePanel::on_Reset_clicked()
{
    Reset();
}

void DDSDevicePanel::on_Softtrigger_clicked()
{
    uint error = device->send_trigger();
    if(error>0)ChangeIndicator(2);
    else ChangeIndicator(1);
}

void DDSDevicePanel::on_Amplitude_returnPressed()
{
    bool ok = false;
    float temp = ui->Amplitude->text().toFloat(&ok);
    if(ok){
        amplitude = temp;
        uint error = 0;
        DDSCycle cycle;
        cycle.setAmplitude(amplitude);
        error += device->program_cycle(cycle);
        error += device->start_cycle();
        if(error>0)ChangeIndicator(2);
        else ChangeIndicator(1);
    }
    else{
        ui->Amplitude->setText(QString::number(amplitude));
    }
}

void DDSDevicePanel::on_Phase_returnPressed()
{
    bool ok = false;
    float temp = ui->Phase->text().toFloat(&ok);
    if(ok){
        phase = temp;
        uint error = 0;
        DDSCycle cycle;
        cycle.setPhase(phase);
        error += device->program_cycle(cycle);
        error += device->start_cycle();
        if(error>0)ChangeIndicator(2);
        else ChangeIndicator(1);
    }
    else{
        ui->Phase->setText(QString::number(phase));
    }
}

void DDSDevicePanel::ChangeIndicator(int i){
    QPixmap indecate = QPixmap(ui->Indicator->size());
    indecate.fill(QColor(240,240,240));
    QPainter painter(&indecate);
    switch(i){
    case 0: //Standby
        painter.setBrush(Qt::darkGray);
        break;
    case 1: //Normal
        painter.setBrush(Qt::darkGreen);
        break;
    case 2: //Error
        painter.setBrush(Qt::red);
        break;
    }
    painter.drawEllipse(2,2,ui->Indicator->width()-3,ui->Indicator->height()-3);
    painter.end();
    ui->Indicator->setPixmap(indecate);
}

void DDSDevicePanel::update_Device(DDSCycle &cyc){
    uint error = 0;

    uint8_t error_code;

    error_code = device->program_cycle(cyc);
    if(error_code){
        error++;
        emit ErrorReturn(QString("Program Cycle Fail. %1").arg(error_code));
    }
    if(error_code = device->start_cycle()){
        error++;
        emit ErrorReturn(QString("Cycle Start Fail. %1").arg(error_code));
    }
    if(error>0)ChangeIndicator(2);
    else ChangeIndicator(1);
}

void DDSDevicePanel::update_Device(){
    DDSCycle newCycle;
    double AMP = amplitude;
    uint error = 0;
    /*for(int i=0; i<sequences->size(); i++){
        for(int j=0; j<sequences->at(i)->size(); j++){
            double amp = sequences->at(i)->at(j)->get_value(3,Number_of_Scan,0,0,0);
            double time = sequences->at(i)->at(j)->get_time(Number_of_Scan);
            double fromFeq = sequences->at(i)->at(j)->get_output(Number_of_Scan,0,0,0,"NULL");
            double toFeq = sequences->at(i)->at(j)->get_output(Number_of_Scan,time,0,0,"NULL");
            if(fromFeq==toFeq){
                newCycle.singleTone(float(fromFeq));
            }
            else{
                newCycle.linearSweep((float)fromFeq,(float)toFeq,(float)time);
            }
            if(amp!=AMP){
                AMP = amp;
                newCycle.setAmplitude(float(amp));
            }
        }
    }*/
    error += device->program_cycle(newCycle);
    error += device->start_cycle();
    if(error>0)ChangeIndicator(2);
    else ChangeIndicator(1);
}

void DDSDevicePanel::Merge_Channel_Change(int index){
    emit Merge(index-1+Analog_output_channel);
}
