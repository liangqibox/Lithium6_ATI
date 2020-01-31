#include "lenscontrol_optotune.h"
#include "ui_lenscontrol_optotune.h"

const int Initial_Focus = 100;
const int Max_Focus = 160;
const int Min_Focus = 90;

LensControl_Optotune::LensControl_Optotune(QWidget *parent, bool isAdwin, QList<Variable *> *vari) :
    QWidget(parent),
    ui(new Ui::LensControl_Optotune)
{
    ui->setupUi(this);

    setAcceptDrops(true);
    external = false;
    continous = false;
    counter = 0;
    number_of_run = 0;
    number_of_scan = 0;
    current_focus = Initial_Focus;
    file_load.clear();
    polynomial = 0xA001;
    table = new ushort[256];

    if(!isAdwin){
        ui->Trigger_adwin->setDisabled(true);
    }
    variables = vari;

    ui->Focus_Slider->setDisabled(true);
    connect(this,SIGNAL(Focus_change(int)),ui->Focus_Slider,SLOT(setValue(int)));
    connect(this,SIGNAL(Cycle_start(bool)),ui->Focus_Slider,SLOT(setDisabled(bool)));
    connect(this,SIGNAL(Cycle_end(bool)),ui->Focus_Slider,SLOT(setEnabled(bool)));
    connect(this,SIGNAL(Focus_change(int)),this,SLOT(Show_current_Focus(int)));

    run_timer = new QTimer(this);
    run_timer->setSingleShot(true);
    connect(run_timer,SIGNAL(timeout()),this,SLOT(Next()));

    T_timer = new QTimer(this);
    connect(T_timer,SIGNAL(timeout()),this,SLOT(Read_temperature()));

    Progress_timer = new QTimer(this);
    connect(Progress_timer,SIGNAL(timeout()),this,SLOT(Cycle_progress()));

    connect(ui->Connect,SIGNAL(clicked()),this,SLOT(Hand_shake()));
    connect(ui->Reset,SIGNAL(clicked()),this,SLOT(Reset()));

    ushort value;
    ushort temp;
    for(ushort i = 0; i < 256; ++i) {
        value = 0;
        temp = i;
        for(byte j = 0; j < 8; ++j) {
            if(((value ^ temp) & 0x0001) != 0) {
                value = (ushort)((value >> 1) ^ polynomial);
            }
            else {
                value >>= 1;
            }
            temp >>= 1;
        }
        table[i] = value;
    }
}

LensControl_Optotune::~LensControl_Optotune()
{
    if(serial.isOpen()){
        Send_command(1000,true);
        serial.close();
    }
    delete ui;
}

void LensControl_Optotune::dragEnterEvent(QDragEnterEvent *event){
    if (event -> mimeData()->hasUrls()){
        event -> acceptProposedAction();
    }
}

void LensControl_Optotune::dropEvent(QDropEvent *event){
    QStringList files;
    foreach (QUrl url, event->mimeData()->urls()) {
        files.append(url.path().remove(0,1));
    }
    QString filename = files.first();
    if(filename.isEmpty()){
        ui->File_display->setText("None");
        file_load.clear();
    }
    else{
        ui->File_display->setText(filename);
        file_load = filename;
        if(Load_file(filename))ui->Status_text->append("File Loaded");
        else ui->Status_text->append("Fail to Load File");
    }
}

bool LensControl_Optotune::Hand_shake(){
    bool connected = false;
    bool error = 0;
    char buffer[256];
    serial.setBaudRate(QSerialPort::Baud115200);
    serial.setDataBits(QSerialPort::Data8);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setParity(QSerialPort::NoParity);
    serial.setPort(QSerialPortInfo("COM3"));
    error = serial.open(QIODevice::ReadWrite);

    if(error == true){
        ui->Status_label->setText("Connected");
        connected = true;
        ui->Focus_Slider->setEnabled(true);
        T_timer->start(1000);
    }
    else if(error == false){
        int err = serial.error();
        if(err == QSerialPort::DeviceNotFoundError)ui->Status_label->setText("No Device");
        else if(err == QSerialPort::PermissionError)ui->Status_label->setText("Permission Error");
        return connected;
    }

    //Handshaking, say "Start", controller should return "Ready"
    char msg[] = {'S','t','a','r','t','\0'};
    serial.write(msg,5);
    if(!serial.waitForBytesWritten(500)){
        ui->Status_text->append("Send fail");
    }
    else if(!serial.waitForReadyRead(500)){
        ui->Status_text->append("Device no respone");
    }
    else{
        serial.read(buffer,10);
        if(buffer[0]=='R')ui->Status_text->append("Lens Ready");
    }

    /*char ini[] = {'P','w','D','A',0x02,0x80,0x01,0x40,0,0,'\0'};
    ushort crci = CRC_Checksum(ini,8);
    ini[8] = (byte)(crci&0xFF);
    ini[9] = (byte)(crci >> 8);
    serial.write(ini,10);
    if(!serial.waitForBytesWritten(500)){
        ui->Status_text->append("Send fail");
    }*/

    char mode[] = {'M','w','C','A',0x56,0x76}; // Controlled mode
    //char mode[] = {'M','w','D','A',0x56,0x76}; // DC current mode
    ushort crc = CRC_Checksum(mode,4);
    mode[4] = (byte)(crc&0xFF);
    mode[5] = (byte)(crc >> 8);
    serial.write(mode,6);
    if(!serial.waitForBytesWritten(500)){
        ui->Status_text->append("Send fail");
    }
    else if(!serial.waitForReadyRead(500)){
        ui->Status_text->append("Device no respone");
    }
    else{
        serial.read(buffer,6);
        if(buffer[0]=='M' && buffer[1]=='C')ui->Status_text->append("Controller Set");
        Send_command(Initial_Focus,true);
    }

    return connected;
}

void LensControl_Optotune::on_Connect_clicked()
{
    Hand_shake();
}

void LensControl_Optotune::Reset(){
    T_timer->stop();
    serial.close();
    external = false;
    continous = false;
    counter = 0;
    file_load.clear();
    ui->File_display->setText("None");
    ui->Status_text->clear();
}

bool LensControl_Optotune::Load_file(QString filename){
    Data_time.clear();
    Data_focus.clear();
    bool ok = true;

    QFile read(filename);
    if(!read.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream in(&read);
    while (!in.atEnd()) {
       QString temp1;
       QString temp2;
       in >> temp1;
       in >> temp2;
       if((!temp1.isEmpty())&&(!temp2.isEmpty())){
           int time = 10000;
           double focus = 1000;
           if(temp1.at(0)=='['){
               QString name = temp1;
               for(int i=0; i<variables->size(); i++){
                   if(name== '['+variables->at(i)->get_name()+']'){
                       switch(variables->at(i)->get_type()){
                       case 0:
                           time = variables->at(i)->get_value()[0];
                           break;
                       case 1:
                           time = variables->at(i)->get_scan(number_of_scan);
                           break;
                       case 2:
                           time = variables->at(i)->get_calculated(number_of_scan,0,0,0,0);
                           break;
                       default:
                           break;
                       }
                   }
               }
           }
           else time = temp1.toInt(&ok,10);

           if(temp2.at(0)=='['){
               QString name = temp2;
               for(int i=0; i<variables->size(); i++){
                   if(name == '['+variables->at(i)->get_name()+']'){
                       switch(variables->at(i)->get_type()){
                       case 0:
                           focus = variables->at(i)->get_value()[0];
                           break;
                       case 1:
                           focus = variables->at(i)->get_scan(number_of_scan);
                           break;
                       case 2:
                           focus = variables->at(i)->get_calculated(number_of_scan,0,0,0,0);
                           break;
                       default:
                           break;
                       }
                   }
               }
           }
           else focus = temp2.toDouble(&ok);

           Data_time.append(time);
           Data_focus.append(focus);
       }
       if(!ok)break;
    }
    return ok;
}

void LensControl_Optotune::Run_cycle(){
    counter = 0;
    progress_time = 0;
    bool loaded = Load_file(file_load);
    if(!loaded){
        ui->Status_text->append("Error: Fail to load file.");
        if(continous)continous = false;
        Stop_cycle();
        return;
    }

    emit Cycle_start(true);
    ui->Status_label->setText("Running");
    if(Data_time.first() == 0){
        Send_command(Data_focus.first(),true);
        //Send_command(Data_focus.first(),false);
        counter++;
    }

    if(Data_time.size()>1){
        run_timer->start(Data_time.at(1));
        Progress_timer->start(100);
    }
    else Stop_cycle();
}

void LensControl_Optotune::Stop_cycle(){
    if(continous)continous = false;
    else{
        run_timer->stop();
        Progress_timer->stop();
        emit Cycle_end(true);
    }
    ui->Status_label->setText("Stopped");
}

void LensControl_Optotune::ADwin_trigger(QString msg){
    QStringList msglist = msg.split(':');
    number_of_run = msglist.at(1).toInt();
    number_of_scan = msglist.at(2).toInt();
    if(external){
        //Load_file(file_load);
        continous = false;
        Run_cycle();
    }
}

void LensControl_Optotune::Next(){
    Send_command(Data_focus.at(counter),true);
    //Send_command(Data_focus.at(counter),false);
    counter++;
    if(counter<Data_time.size()){
        run_timer->start(Data_time.at(counter)-Data_time.at(counter-1));
    }
    else if(continous)Run_cycle();
    else ui->Status_text->append("Cycle "+QString::number(number_of_run)+" End");
}

void LensControl_Optotune::on_Trigger_adwin_stateChanged(int arg1)
{
    if(arg1>0){
        ui->Singal->setDisabled(true);
        ui->Continous->setDisabled(true);
        external = true;
    }
    else{
        ui->Singal->setEnabled(true);
        ui->Continous->setEnabled(true);
        external = false;
    }
}

void LensControl_Optotune::on_File_load_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,"Load File",0,tr("(*.txt)"));
    if(filename.isEmpty()){
        ui->File_display->setText("None");
        file_load.clear();
    }
    else{
        ui->File_display->setText(filename);
        file_load = filename;
        if(Load_file(filename))ui->Status_text->append("File Loaded");
        else ui->Status_text->append("Fail to Load File");
    }
}

void LensControl_Optotune::on_Singal_clicked()
{
    continous = false;
    ui->Status_text->append("Start Cycle: Signal Shot");
    Run_cycle();
}


void LensControl_Optotune::on_Continous_clicked()
{
    continous = true;
    ui->Status_text->append("Start Cycle: Continous");
    Run_cycle();
}

void LensControl_Optotune::on_Stop_clicked()
{
    Stop_cycle();
}

void LensControl_Optotune::Send_command(double f,bool focus){
    //This function send command to the USB lens controller
    char read[3];
    if(focus){
        char send[] = {'P','w','D','A',0,0,0,0,0,0,'\0'};
        focus_to_command(f,send);
        serial.write(send,10);
        serial.waitForBytesWritten(500);
        if(serial.waitForReadyRead(1)){;
            serial.read(read,3);
            if(read[0]=='N')ui->Status_text->append("Command Error");
        }
        else{
            emit Focus_change(int(f));
        }
    }
    else{
        char send[] = {'A','w',0,0,0,0,'\0'};
        current_to_command(f,send);
        serial.read(send,6);
    }
}

char* LensControl_Optotune::focus_to_command(double f, char* send){
    if(f>Max_Focus)f = Max_Focus;
    else if(f<Min_Focus)f = Min_Focus;
    int x = (1000./f + 5) * 200;
    send[4] = (byte)(x >> 8);
    send[5] = (byte)(x&0xFF);
    send[6] = 0x00;
    send[7] = 0x00;

    ushort crc = CRC_Checksum(send,8);
    send[8] = (byte)(crc&0xFF);
    send[9] = (byte)(crc >> 8);

    return send;
}

char* LensControl_Optotune::current_to_command(int c, char send[]){
    ushort x =(double)c/293.*4096;
    send[2] = (byte)(x >> 8);
    send[3] = (byte)(x&0xFF);

    ushort crc = CRC_Checksum(send,4);
    send[4] = (byte)(crc&0xFF);
    send[5] = (byte)(crc >> 8);
    return send;
}

ushort LensControl_Optotune::CRC_Checksum(char send[], int length){

    ushort crc = 0;
    for (int i = 0; i < length; ++i){
        byte index = (byte)(crc ^ send[i]);
        crc = (ushort)((crc >> 8) ^ table[index]);
    }
    return crc;
}

void LensControl_Optotune::Read_temperature(){
    char send[] = {'T','A',0,0,'\0'};
    send[2] = 0xFE;
    send[3] = 0xF0;
    char buffer[20];
    serial.write(send,4);
    serial.waitForBytesWritten(500);
    serial.waitForReadyRead(500);
    serial.read(buffer,20);
    double T = (quint8(buffer[3])*256+quint8(buffer[4]))*0.0625;
    if(T > 100)ui->Temperature->setText("T: NA");
    else ui->Temperature->setText("T: "+QString::number(T));
}

void LensControl_Optotune::Cycle_progress(){
    progress_time += 100;
    int x = (double)progress_time/(double)Data_time.last()*100;
    ui->Progress->setValue(x);
}

void LensControl_Optotune::Show_current_Focus(int focus){
    ui->Focus->setText(QString::number(focus)+" mm");
}

void LensControl_Optotune::on_Focus_Slider_actionTriggered(int action)
{
    Send_command(double(ui->Focus_Slider->value()),true);
}
