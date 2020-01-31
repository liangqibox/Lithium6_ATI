#include "maincontroller.h"
#include "ui_maincontroller.h"
#include <QDebug>

char T9[25] = "c:\\ADwin\\ADwin9.btl";
char T10[25] = "c:\\ADwin\\ADwin10.btl";
char T11[25] = "c:\\ADwin\\ADwin11.btl";
char T12[25] = "c:\\ADwin\\ADwin12.btl";
char AIN_BIN_File_T12[25] = "ADwin_Bin\\AIN_v2.TC2";
char AIN_BIN_File_T11[25] = "ADwin_Bin\\AIN_v2.TB2";
char AIN_BIN_File_T10[25] = "ADwin_Bin\\AIN_v2.TA2";
char AOUT_Bin_File_T10[25] = "ADwin_Bin\\T10_DAC_v2.TA1";
char AOUT_Bin_File_T11[25] = "ADwin_Bin\\T11_DAC_v2.TB1";
//char AOUT_Bin_File_T12[25] = "ADwin_Bin\\T12_DAC_v3.TC1";
char AOUT_Bin_File_T12[50] = "ADwin_Bin\\T12_DAC_64DigiOut_20200102.TC1";
char Safty_T10[25] = "ADwin_Bin\\T10_Safty.TA3";
char Safty_T11[25] = "ADwin_Bin\\T11_Safty.TB3";
char Safty_T12[25] = "ADwin_Bin\\T12_Safty.TC3";
char Manual_T12[30] = "ADwin_Bin\\T12_Manual.TC4";
char Safty_CPU_T12[30] = "ADwin_Bin\\T12_Safty_CPU.TC5";

const int CYCLE_CHECK_TIMER_PREIOD = 100;
const int MANUAL_SWITCH_MONITOR = 300;

MainController::MainController(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainController)
{
    ui->setupUi(this);
    ui->ErrorDisplay->clear();
    ui->Section_editer->hide(); //No longer used
    QElapsedTimer timer;
    timer.start();

    Adwin_state = new Indicator(this);
    Adwin_state->move(20,500);
    Adwin_state->show();
    auto_save = new AutoSave(this);
    auto_save->move(240,310);
    connect(this,SIGNAL(new_cycle(QString)),auto_save,SLOT(Update(QString)));
    connect(auto_save,SIGNAL(save_setting(QString)),this,SLOT(save_setting(QString)));

    /*Indicator below "Load Parameter Button"*/
    QPixmap pixmap = QPixmap(ui->already_Load->width(),ui->already_Load->height());
    pixmap.fill(Qt::darkGray);
    ui->already_Load->setPixmap(pixmap);

    system_log(0,0);
    Load_Default_Setting();

    //Menu Buttons
    connect(ui->actionLoad_Setting,SIGNAL(triggered()),this,SLOT(load_setting_clicked()));
    connect(ui->actionSave_Setting,SIGNAL(triggered()),this,SLOT(save_setting_clicked()));
    connect(ui->actionExit,SIGNAL(triggered()),this,SLOT(close()));
    connect(ui->actionSystem_Log,SIGNAL(triggered()),this,SLOT(System_log_open()));
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(program_infomation()));
    connect(ui->actionScript_Engine,SIGNAL(triggered()),this,SLOT(ScriptEngineDebug()));
    connect(ui->actionRemote_Message_Log,SIGNAL(triggered()),this,SLOT(RemoteServer_Log()));
    connect(this,SIGNAL(reloading(int)),ui->cycle_progress,SLOT(reset()));

    refresh_timer = new QTimer(this);
    connect(refresh_timer,SIGNAL(timeout()),this,SLOT(Refresh()));
    remoteControl = new RemoteControlServer;
    connect(remoteControl,SIGNAL(RemoteCommand(QString)),this,SLOT(Receive_Remote_Command(QString)));

    cycle_check_timer = new QTimer(this);
    cycle_check_timer->setTimerType(Qt::PreciseTimer);
    connect(cycle_check_timer,SIGNAL(timeout()),this,SLOT(check_cycle_state()));
    global_timer = new QTimer(this);
    connect(global_timer,SIGNAL(timeout()),this,SLOT(global_timer_move()));
    hardwareControl_check_timer = new QTimer(this);
    connect(hardwareControl_check_timer,SIGNAL(timeout()),this,SLOT(check_hardware_state()));

    DataGroup = new ControllerDataGroup(this);
    program_initial();
    qDebug() << "Program Initialize " << timer.elapsed();
    ui->ErrorDisplay->append("Program Initialize "+QString::number(timer.elapsed()));

    Setting *SettingWindow;
    SettingWindow = new Setting(this,program_setting,remoteControl);
    SettingWindow->move(376,50);
    SettingWindow->hide();
    connect(SettingWindow,SIGNAL(Setting_file_changed(QString)),this,SLOT(load_setting(QString)));
    connect(SettingWindow,SIGNAL(System_setting_apply(QStringList)),this,SLOT(system_setting_changed(QStringList)));
    connect(this,SIGNAL(program_setting_change(QStringList)),SettingWindow,SLOT(Setting_refresh(QStringList)));
    connect(this,SIGNAL(Mission_used_state_updata(int)),SettingWindow,SLOT(cycle_mission_check(int)));
    connect(SettingWindow,SIGNAL(Run_Clicked()),this,SLOT(on_Process_start_clicked()));
    connect(SettingWindow,SIGNAL(Scan_Clicked()),this,SLOT(on_Scan_clicked()));
    connect(SettingWindow,SIGNAL(Load_Clicked()),this,SLOT(on_Process_load_clicked()));
    connect(SettingWindow,SIGNAL(Remote_Control_Stop()),this,SLOT(on_Emergency_stop_clicked()));

    External *FiletransWindow;
    External_File_Transfer = new QList<QStringList>;
    FiletransWindow = new External(this,variables,External_File_Transfer,total_number_of_scan,number_of_run,number_of_scan);
    FiletransWindow->move(376,50);
    FiletransWindow->hide();
    connect(this,SIGNAL(new_cycle(QString)),FiletransWindow,SLOT(reload(QString)));

    Safety *safety = new Safety(this);
    safety->move(376,50);
    safety->hide();
    connect(this,SIGNAL(new_cycle(QString)),safety,SLOT(Reload(QString)));

    CameraControl *Pixelfly = new CameraControl(this);
    Pixelfly->move(376,50);
    Pixelfly->hide();
    connect(this,SIGNAL(new_cycle(QString)),Pixelfly,SLOT(Reload(QString)));

    /*CameraControlQE *PixelflyQE = new CameraControlQE(this);
    PixelflyQE->move(376,50);
    PixelflyQE->hide();
    connect(this,SIGNAL(new_cycle(QString)),PixelflyQE,SLOT(Reload(QString)));*/

    AnalogRecording *AIn = new AnalogRecording(this,DataGroup);
    AIn->move(376,50);
    AIn->hide();
    connect(this,SIGNAL(new_cycle(QString)),AIn,SLOT(Reload(QString)));
    connect(this,SIGNAL(setting_load(ControllerDataGroup*)),AIn,SLOT(Controller_Setting_Load(ControllerDataGroup*)));

    /*LensControl_Optotune *lencontrol = new LensControl_Optotune(this,true);
    lencontrol->move(376,50);
    lencontrol->hide();
    connect(this,SIGNAL(new_cycle(QString)),lencontrol,SLOT(ADwin_trigger(QString)));*/

    /*DDSControl *ddscontrol = new DDSControl(this,variables,channel_name,QiEngine,Analog_output_channel,Digital_output_channel);
    connect(this,SIGNAL(new_cycle(QString)),ddscontrol,SLOT(Receive_new_cycle(QString)));
    ddscontrol->move(376,50);
    ddscontrol->hide();*/

    /*iXonControl *iXon = new iXonControl(this);
    connect(this,SIGNAL(new_cycle(QString)),iXon,SLOT(processExperimentControllerMessage(QString)));
    iXon->move(376,50);
    iXon->show();*/

    OffsetLock_FrequencyCounter *offsetLock_Frequency = new OffsetLock_FrequencyCounter(this,DataGroup);
    connect(this,SIGNAL(setting_load(ControllerDataGroup*)),offsetLock_Frequency,SLOT(reloadSetting(ControllerDataGroup*)));
    connect(this,SIGNAL(new_cycle(QString)),offsetLock_Frequency,SLOT(newCycle(QString)));
    offsetLock_Frequency->move(376,50);

    Widget_Addon.append((QWidget*)SettingWindow);
    Widget_Addon.append((QWidget*)safety);
    Widget_Addon.append((QWidget*)AIn);
    Widget_Addon.append((QWidget*)FiletransWindow);
    Widget_Addon.append((QWidget*)Pixelfly);
    Widget_Addon.append((QWidget*)offsetLock_Frequency);
    //Widget_Addon.append((QWidget*)iXon);
    //Widget_Addon.append((QWidget*)PixelflyQE);
    //Widget_Addon.append((QWidget*)lencontrol);
    //Widget_Addon.append((QWidget*)ddscontrol);

    addons_names << "Setting" << "Safety" << "Analog Input" << "File Transfer" << "Pixelfly Camera" << "Offset Lock" << "iXonCamera" << "DDS Control";

    //Auto config saving every 30s, (in case of random crash)
    /*QString filename = "D://PROG//Experiment Controller//Cycle_setting//System_Auto_Save.adcs";
    QTimer *savetimer = new QTimer;
    QSignalMapper *mapper = new QSignalMapper(savetimer);
    mapper->setMapping(savetimer,filename);
    connect(savetimer,SIGNAL(timeout()),mapper,SLOT(map()));
    connect(mapper,SIGNAL(mapped(QString)),this,SLOT(save_setting(QString)));
    savetimer->start(30000);*/

    qDebug() << "Widget Initialize " << timer.elapsed();
    ui->ErrorDisplay->append("Widget Initialize "+QString::number(timer.elapsed()));
}

MainController::~MainController()
{
    if(connected){
        Stop_Process(1);
        Stop_Process(2);
        Stop_Process(3);
        Stop_Process(4);
        Stop_Process(5);
    }
    system_log(8,0);
    emit program_terminate();
    delete ui;
}

void MainController::Load_Default_Setting(){
    QFile file("Default.txt");
    bool read = true;
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))read = false;
    QTextStream in(&file);

    voltage_resolution = 0;     //In 16-bit 1=0.305mV (0~65535)
    int CPU = CPUTYPE_T12;
    int number_of_analog_output_card = 3;
    int number_of_digital_output_card = 1;
    QString status = "online";
    QString precalculate = "preoff";
    QString TCP = "tcpoff";
    QString TCPort = "0";
    QString StopAfterScan = "ContinueAfterScan";
    QString CpuSafety = "off";

    if(read){
    while(!in.atEnd()){
        QString temp = in.readLine();
        if(temp.contains("[CPU Type]")){
            QString cpu = in.readLine();
            if(cpu=="T12")CPU = CPUTYPE_T12;
            else if(cpu=="T11")CPU = CPUTYPE_T11;
            else if(cpu=="T10")CPU = CPUTYPE_T10;
            else if(cpu=="T9")CPU = CPUTYPE_T9;
        }
        else if(temp.contains("[Number of Analog Output Cards]")){
            QString ao = in.readLine();
            number_of_analog_output_card = ao.toInt();
        }
        else if(temp.contains("[Number of Digital Output Cards]")){
            QString dio = in.readLine();
            number_of_digital_output_card = dio.toInt();
        }
        else if(temp.contains("[Online Status]")){
            QString s = in.readLine();
            if(s.contains("online"))status = "online";
            else status = "offline";
        }
        else if(temp.contains("[Pre-Calculation]")){
            QString s = in.readLine();
            if(s.contains("on"))precalculate = "preon";
            else precalculate = "preoff";
        }
        else if(temp.contains("[TCP Switch]")){
            QString s = in.readLine();
            if(s.contains("on"))TCP = "tcpon";
            else TCP = "tcpoff";
        }
        else if(temp.contains("[TCP Port NO.]")){
            QString s = in.readLine();
            TCPort = s;
        }
        else if(temp.contains("[Voltage Resolution]")){
            QString vr = in.readLine();
            voltage_resolution = vr.toInt();
        }
        else if(temp.contains("[Stop Cycle After Scan]")){
            QString s = in.readLine();
            if(s.contains("yes",Qt::CaseInsensitive))StopAfterScan = "StopAfterScan";
        }
        else if(temp.contains("[CPU Safety Function]")){
            QString s = in.readLine();
            if(s.contains("on",Qt::CaseInsensitive))CpuSafety = "on";
        }
    }
    file.close();
    }
    Analog_output_channel = 8*number_of_analog_output_card;
    Digital_output_channel = 32*number_of_digital_output_card;
    program_setting.append(QString::number(CPU));
    program_setting.append(QString::number(number_of_analog_output_card));
    program_setting.append(QString::number(number_of_digital_output_card));
    program_setting.append(status);
    program_setting.append(precalculate);
    program_setting.append(TCP);
    program_setting.append(TCPort);
    program_setting.append(StopAfterScan);
    program_setting.append(CpuSafety);
}

void MainController::link_to_DataGroup(){
    DataGroup->sequences = sequences;
    DataGroup->variables = variables;
    DataGroup->variableClass = variableClass;
    DataGroup->DisplayClass = DisplayClass;
    DataGroup->sections_activity = sections_activity;
    DataGroup->sections_name = sections_name;
    DataGroup->section_array = section_array;
    DataGroup->sections = sections;
    DataGroup->channel_name = channel_name;
    DataGroup->channel_color = channel_color;
    DataGroup->channel_calibration = channel_calibration;
    DataGroup->External_File_Transfer = External_File_Transfer;
    DataGroup->QiEngine = QiEngine;
}

// Initialize all control parameters
void MainController::program_initial(){
    cycle_time = 0;
    currentRunning_cycle_time = 0;
    progress_time = 0;
    total_time = 0;
    run_time = 0;
    number_of_run = 0;
    number_of_scan = 0;
    time_resolution = 10;
    ui->Time_resolution->setText("10");
    seq_wrong = false;
    connected = false;
    running = false;
    scanning = false;
    remote_start = false;
    remote_load = false;
    isOpen_ChannelEditor = false;
    isOpen_SectionEditor = false;
    isOpen_VariableMonitor = false;
    for(int i=0; i<10; i++)total_number_of_scan[i]=0;
    QiEngine = new QScriptEngine();
    ScriptEngineSetup();

    variables = new QList<Variable*>;
    variableClass = new VariableClass;
    variableClass->setText(0,"All Variable");
    DisplayClass = new QList<VariableClass*>;
    sections = new QList<QList<Sequence*>**>;
    sections_name = new QList<QString>;
    section_array = new QList<QString>;
    sequences = new QList<QList<Sequence*>**>;
    sections_activity = new QList<SectionActivity*>;
    channel_name = new QList<QString>;
    channel_color = new QList<int>;
    channel_calibration = new QList<QString>;
    for(int i=0;i<Analog_output_channel;i++){
        channel_color->append(i%2);
        channel_name->append("AO "+QString::number(i+1));
        channel_calibration->append("Not calibrated");
    }
    for(int i=0;i<Digital_output_channel;i++){
        channel_color->append(i%2);
        channel_name->append("DiO "+QString::number(i+1));
        channel_calibration->append("Normal");
    }
    link_to_DataGroup();
    variableExpleror = new VariableExpleror(0,variables,variableClass,DisplayClass,QiEngine); //!!!!!!!!!!!!

    auto_save->init_variable(variables);
    emit program_reset();
    refresh_timer->start(300);
    hardwareControl_check_timer->start(300);
}
/*
QScriptValue ScanScriptFunction(QScriptContext *context, QScriptEngine *engine){
    int total[10],current;
    double value[4];
    for(int i=0; i<10; i++)total[i] = context->argument(0).property(i).toInt32();
    current = context->argument(1).toInt32();
    for(int i=0; i<4; i++)value[i] = context->argument(2).property(i).toNumber();
    int temp = 1;
    int step = 0;
    if(value[0]==0 || current==0)return value[1];
    for (int j=value[0]; j<10; j++){
        if(total[j]!=0)temp = temp * total[j];
    }
    if (temp==1)step = (current-1)%(total[int(value[0])-1]);
    else step = (current-1)/temp;
    double v = value[1]+value[3]*step;
    if(value[3]>0 && v>value[2])v = value[2];
    else if(value[3]<0 &&v <value[2])v = value[2];
    return v;
}*/
/*
QScriptValue SplineScriptFunction(QScriptContext *context, QScriptEngine *engine){
    std::vector<double> m[5];
    for(int j=0; j<5; j++){
        int size = context->argument(j).property("length").toInt32();
        m[j].resize(size);
        for(int i=0; i<size;i++){
            m[j][i] = context->argument(j).property(i).toNumber();
        }
    }

    size_t n = m[0].size();
    std::vector<double>::const_iterator it;
    it=std::lower_bound(m[0].begin(),m[0].end(),context->argument(7).toNumber());
    int idx=std::max( int(it-m[0].begin())-1, 0);

    double h=context->argument(7).toNumber()-m[0][idx];
    double interpol;
    if(context->argument(7).toNumber()<m[0][0]) {
        interpol=(context->argument(5).toNumber()*h + context->argument(6).toNumber())*h + m[1][0];
    } else if(context->argument(7).toNumber()>m[0][n-1]) {
        interpol=(m[3][n-1]*h + m[4][n-1])*h + m[1][n-1];
    } else {
        interpol=((m[2][idx]*h + m[3][idx])*h + m[4][idx])*h + m[1][idx];
    }
    return interpol;
}*/

void MainController::ScriptEngineSetup(){
    QScriptValue pi = QiEngine->evaluate("Math.PI");
    QScriptValue E = QiEngine->evaluate("Math.E");
    QScriptValue random = QiEngine->evaluate("(function(){return Math.random();})");
    QScriptValue abs = QiEngine->evaluate("(function(x){return Math.abs(x);})");
    QScriptValue sin = QiEngine->evaluate("(function(x){return Math.sin(x);})");
    QScriptValue cos = QiEngine->evaluate("(function(x){return Math.cos(x);})");
    QScriptValue tan = QiEngine->evaluate("(function(x){return Math.tan(x);})");
    QScriptValue asin = QiEngine->evaluate("(function(x){return Math.asin(x);})");
    QScriptValue acos = QiEngine->evaluate("(function(x){return Math.acos(x);})");
    QScriptValue atan = QiEngine->evaluate("(function(x){return Math.atan(x);})");
    QScriptValue ln = QiEngine->evaluate("(function(x){return Math.log(x);})");
    QScriptValue floor = QiEngine->evaluate("(function(x){return Math.floor(x);})");
    QScriptValue ceil = QiEngine->evaluate("(function(x){return Math.ceil(x);})");
    QScriptValue exp = QiEngine->evaluate("(function(x){return Math.exp(x);})");
    QScriptValue pow = QiEngine->evaluate("(function(x,y){return Math.pow(x,y);})");
    QScriptValue log = QiEngine->evaluate("(function(x){return Math.log(x)/Math.LN10;})");
    QiEngine->globalObject().setProperty("pi",pi);
    QiEngine->globalObject().setProperty("random",random);
    QiEngine->globalObject().setProperty("E",E);
    QiEngine->globalObject().setProperty("abs",abs);
    QiEngine->globalObject().setProperty("sin",sin);
    QiEngine->globalObject().setProperty("cos",cos);
    QiEngine->globalObject().setProperty("tan",tan);
    QiEngine->globalObject().setProperty("asin",asin);
    QiEngine->globalObject().setProperty("acos",acos);
    QiEngine->globalObject().setProperty("atan",atan);
    QiEngine->globalObject().setProperty("exp",exp);
    QiEngine->globalObject().setProperty("pow",pow);
    QiEngine->globalObject().setProperty("ln",ln);
    QiEngine->globalObject().setProperty("log",log);
    QiEngine->globalObject().setProperty("floor",floor);
    QiEngine->globalObject().setProperty("ceil",ceil);

    QScriptValue total = QiEngine->newArray(10);
    QiEngine->globalObject().setProperty("Total_Number_of_Scan",total);
    QString cur = "(function Current_Step(c,v){"
                  "var temp = 1;"
                  "if(v==0 || c==0)return 0;"
                  "for (var j=v; j<10; j++){"
                  "if(Total_Number_of_Scan[j]!=0)temp = temp * Total_Number_of_Scan[j];}"
                  "if (temp==1)return parseInt((c-1)%(Total_Number_of_Scan[v-1]));"
                  "else return parseInt((c-1)/temp%(Total_Number_of_Scan[v-1]));})";
    QScriptValue scan = QiEngine->evaluate(cur);
    QiEngine->globalObject().setProperty("Current_Step",scan);

    //Debug for ScriptEngine setup
    /*QScriptValueIterator it(QiEngine->globalObject());
    while(it.hasNext()){
        it.next();
        QString name = it.name();
        QScriptValue value = it.value();
        if(value.isNumber())qDebug()<<name <<"    " << value.toNumber();
        else if(value.isFunction()){
            QString code = value.toString();
            if(!code.contains("native"))qDebug()<<name << "    "<<code;
        }
    }
    qDebug()<<QiEngine->evaluate("sin(pi/4)").toNumber();*/

    //Native C function (no longer used)
    //QScriptValue scan = QiEngine->newFunction(ScanScriptFunction,3);
    //QScriptValue spline = QiEngine->newFunction(SplineScriptFunction,8);
    //QiEngine->globalObject().setProperty("Scan",scan);
    //QiEngine->globalObject().setProperty("Spline",spline);
}

void MainController::Clear(){
    if(isOpen_ChannelEditor){
        channelEditer->close();
        delete channelEditer;
        isOpen_ChannelEditor = false;
    }
    if(isOpen_VariableMonitor){
        variableExpleror->Close();
        delete variableExpleror;
        isOpen_VariableMonitor = false;
    }
    channel_name->clear();
    channel_color->clear();
    channel_calibration->clear();

    for(int i=0; i<sequences->size(); i++){
        for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
            for(int k=0; k<sequences->at(i)[j]->size(); k++){
                delete sequences->at(i)[j]->at(k);
            }
            sequences->at(i)[j]->clear();
            delete sequences->at(i)[j];
        }
        delete sequences->at(i);
    }
    sequences->clear();
    delete sequences;

    for(int i=0; i<sections->size(); i++){
        for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
            for(int k=0; k<sections->at(i)[j]->size(); k++){
                delete sections->at(i)[j]->at(k);
            }
            sections->at(i)[j]->clear();
            delete sections->at(i)[j];
        }
        delete sections->at(i);
    }
    sections->clear();
    delete sections;

    DisplayClass->clear();
    delete DisplayClass;
    for(int i=0; i<variables->size(); i++){
        delete variables->at(i);
    }
    variables->clear();
    delete variables;

    for(int i=0; i<sections_activity->size(); i++)delete sections_activity->at(i);
    delete sections_activity;

    delete section_array;
    delete sections_name;
    delete channel_name;
    delete channel_color;
    delete channel_calibration;
    //delete External_File_Transfer;
    delete QiEngine;
}

// Refresh the display of control parameters
void MainController::Refresh(){
    int sum = 1;
    for(int i=0; i<10; i++){
        QiEngine->globalObject().property("Total_Number_of_Scan").setProperty(i,QiEngine->evaluate(QString::number(total_number_of_scan[i])));
        if(total_number_of_scan[i]!=0)sum = sum * total_number_of_scan[i];
    }

    ui->Current_cyc_time->display(cycle_time/1000);
    ui->Total_time->display(total_time/1000);
    ui->Total_Scan->display(sum);
    ui->Cycle_to_scan->display(sum-number_of_scan);
    ui->Number_of_run->display(number_of_run);
    if(connected)ui->Workload->setValue(Workload(0));
}

void MainController::global_timer_move(){
    ui->Global_timer->display(ui->Global_timer->value()+1);
}

// Connect to ADwin
void MainController::on_Boot_clicked()
{
    if(program_setting.at(0)=="0")Boot(T9,0);
    else if(program_setting.at(0)=="1")Boot(T10,0);
    else if(program_setting.at(0)=="2")Boot(T11,0);
    else if(program_setting.at(0)=="3")Boot(T12,0);
    if(!Test_Version()){
        Adwin_state->S_connected();
        if(program_setting.at(0)=="1"){
            Load_Process(AOUT_Bin_File_T10);
            //Load_Process(AIN_BIN_File_T10);
            //Load_Process(Safty_T10);
        }
        else if(program_setting.at(0)=="2"){
            Load_Process(AOUT_Bin_File_T11);
            Load_Process(Safty_T11);
        }
        else if(program_setting.at(0)=="3"){
            Load_Process(AOUT_Bin_File_T12);
            Load_Process(AIN_BIN_File_T12);
            Load_Process(Safty_T12);
            Load_Process(Manual_T12);
            Load_Process(Safty_CPU_T12);
        }
        Start_Process(1);
        Start_Process(2);
        Start_Process(3);
        Start_Process(4);
        if(program_setting.at(8)=="on")Start_Process(5);
        Set_Par(ANALOG_LOWER,0);
        Set_Par(ANALOG_UPPER,65535);
        connected = true;
        ui->Process_load->setEnabled(true);
        ui->Process_start->setEnabled(true);
        ui->Scan->setEnabled(true);
        ui->Emergency_stop->setEnabled(true);
        ui->Process_start->setText("Start");
        running = false;
        scanning = false;
    }
    else{
        Adwin_state->S_BootErr();
    }

}

// Reboot ADwin, deleting process and re-initialize program
void MainController::on_Reset_clicked()
{
    refresh_timer->stop();
    cycle_check_timer->stop();
    global_timer->stop();
    if(connected){
        ui->Process_load->setDisabled(true);
        ui->Process_start->setDisabled(true);
        ui->Scan->setDisabled(true);
        ui->Emergency_stop->setDisabled(true);

        Stop_Process(1);
        Stop_Process(2);
        Stop_Process(3);
        Adwin_state->S_Standby();
    }
    emit program_terminate();
    Clear();
    Analog_output_channel = 8*program_setting.at(1).toInt();
    Digital_output_channel = 32*program_setting.at(2).toInt();
    program_initial();
    emit setting_load(DataGroup);
}

//Load current current cycle into Adwin system
void MainController::on_Process_load_clicked(){
    ui->Process_load->setDisabled(true);
    ui->Process_start->setDisabled(true);
    ui->Scan->setDisabled(true);

    if(connected)Set_Par(51,0);
    refresh_timer->stop();
    hardwareControl_check_timer->stop();
    int cpu_time = 1000;
    int maximum = 4999999;
    if(program_setting.at(0)=="1"){
        cpu_time = 40;
        maximum = 799999;
    }
    else if(program_setting.at(0)=="2"){
        cpu_time = 300;
        maximum = 999999;
    }
    else if(program_setting.at(0)=="3"){
        cpu_time = 1000;
        maximum = 4999999;
    }
    cycle_change = false;
    seq_wrong = false;
    for(int i=0; i<sequences->size(); i++){
        for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
            for(int k=0; k<sequences->at(i)[j]->size(); k++){
                sequences->at(i)[j]->at(k)->set_wrong(false);
                sequences->at(i)[j]->at(k)->set_change(false);
            }
        }
    }
    for(int i=0; i<variables->size(); i++){
        variables->at(i)->Reload();
    }
    calculate_cycle_time();
    Refresh();
    currentRunning_cycle_time = cycle_time;
    bool data_loaded = false;
    long int **transfer_data;
    long int *transfer_processdelay;
    transfer_data = new long*[Analog_output_channel+Digital_output_channel/32];
    const int total_size = cycle_time*1000./time_resolution;
    int transfer_size = total_size;
    qDebug() << "Cycle time:" << cycle_time << "    Total size: " << total_size;
    ui->ErrorDisplay->append("Cycle time:"+QString::number(cycle_time)+" Total size:"+QString::number(total_size));
    const int total_channel = Analog_output_channel+Digital_output_channel;
    QString md5 = MD5_of_current_cycle();
    QDir dir;
    dir.root();
    qDebug()<< "Current Cycle MD5: " << md5;
    ui->ErrorDisplay->append("Current Cycle MD5:"+md5);
    if(dir.exists("Cycle_data/"+md5+".data") && program_setting.at(4)=="preon"){
        qDebug() << "Precalculation exist   Load from file";
        ui->ErrorDisplay->append("Precalculation exist. Load from file");
        transfer_size = Load_Tranfer_Data_from_File(transfer_data,transfer_processdelay,md5);
        if(transfer_size>0)data_loaded = true;
    }
    if(!data_loaded){
        qDebug() << "No precalculate data   Setup Calculator";
        ui->ErrorDisplay->append("No precalculate data. Setup Calculator");
        int **calculate_data;
        calculate_data = new int*[total_channel];
        for(int i=0; i<total_channel; i++){
            calculate_data[i] = new int[total_size+1];
            for(int j=0; j<total_size+1; j++)calculate_data[i][j]=-1;
        }
        SequenceCalculator *sequenceCalculator = new SequenceCalculator(QiEngine,sequences,variables,channel_calibration,sections_activity,time_resolution,calculate_data,
                                                                Analog_output_channel,Digital_output_channel,total_number_of_scan,number_of_scan,total_size,true);
        connect(sequenceCalculator,SIGNAL(finishing(double)),Adwin_state,SLOT(S_Calculating(double)));
        sequenceCalculator->run();
        delete sequenceCalculator;
        qDebug() << "Calculation Finished.    Data Length:" << transfer_size;
        ui->ErrorDisplay->append("Calculation Finished. Data Length:"+QString::number(transfer_size));

        QList<int> calculate_processdelay;
        int processdelay = time_resolution*cpu_time;
        int delay_count = 1;

        for(int i=0; i<total_size; ++i){
            bool reduce = true;
            for(int j=0; j<Analog_output_channel+(Digital_output_channel/32); ++j){
                if(abs(calculate_data[j][i]-calculate_data[j][i+1])>voltage_resolution){
                    reduce = false;
                    break;
                }
            }
            if(((delay_count+1)*time_resolution)>10000)reduce = false; //ADwin doesn't take processdelay longer than 2s.
            if(reduce){
                delay_count++;
                transfer_size--;
            }
            else{
                calculate_processdelay.append(delay_count*processdelay);
                delay_count = 1;
            }
        }
        calculate_processdelay.append(5000);
        qDebug() << "Reduce Algorithm Completed     Data Length:" << transfer_size;
        ui->ErrorDisplay->append("Reduce Algorithm Completed. Data Length:"+QString::number(transfer_size));

        for(int channel=0; channel<Analog_output_channel; channel++){
            transfer_data[channel] = new long[transfer_size];
            int pointer = 0;
            for(int i=0; i<transfer_size; ++i){
                transfer_data[channel][i]=calculate_data[channel][pointer];
                pointer += (calculate_processdelay.at(i)/processdelay);
            }
        }
        for(int j=0; j<Digital_output_channel/32; j++){
            transfer_data[j+Analog_output_channel] = new long[transfer_size];
            int pointer = 0;
            for(int i=0; i<transfer_size; ++i){
                transfer_data[j+Analog_output_channel][i]=calculate_data[Analog_output_channel+j][pointer];
                pointer += (calculate_processdelay.at(i)/processdelay);
            }
        }
        transfer_processdelay = new long[transfer_size];
        calculate_processdelay.prepend(500);
        for(int i=0; i<transfer_size; i++)transfer_processdelay[i] = calculate_processdelay.at(i);

        for(int i=0; i<sequences->size(); i++){
            for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
                for(int k=0; k<sequences->at(i)[j]->size(); k++){
                    if(sequences->at(i)[j]->at(k)->get_wrong())seq_wrong = true;
                }
            }
        }
        for(int i=0; i<total_channel; i++)delete []calculate_data[i];
        delete []calculate_data;
        qDebug() << "Translated to Transfer Data";
        ui->ErrorDisplay->append("Translated to Transfer Data");

        if(program_setting.at(4)=="preon"){ //Saving transfer data?
            Save_Tranfer_Data_to_File(transfer_data,transfer_processdelay,transfer_size,md5);
        }
        cycle_changed(false);

        //Test code for channel output
        /*int testing_pointer = 0;
        double time = 0;
        for(int p=0; p<transfer_size; p++){
            qDebug()<<transfer_processdelay[p]<<"  "<<transfer_data[0][p];
            time += calculate_processdelay.at(p)/cpu_time/1000.;
            testing_pointer +=calculate_processdelay.at(p)/processdelay;
        }*/
    }

    if(transfer_size > maximum){
        Adwin_state->S_TransErr();
        for(int i=0; i<Analog_output_channel+Digital_output_channel/32; i++)delete transfer_data[i];
        delete transfer_data;
        delete transfer_processdelay;

        QMessageBox msgBox;
        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Cycle too long.");
        msgBox.exec();
        return;
    }

    emit reloading(number_of_scan);
    if(program_setting.at(3)=="online"){
        double progress = 0;
        for(int channel=0; channel<Analog_output_channel; channel++){
            SetData_Long(channel+1,&transfer_data[channel][0],1,transfer_size);
            progress += 100./double(Analog_output_channel+Digital_output_channel/32);
            Adwin_state->S_Uploading(progress);
        }
        for(int j=0; j<Digital_output_channel/32; j++){
            SetData_Long(41+j,&transfer_data[Analog_output_channel+j][0],1,transfer_size);
            progress += 100./double(Analog_output_channel+Digital_output_channel/32);
            Adwin_state->S_Uploading(progress);
        }
        SetData_Long(51,&transfer_processdelay[0],1,transfer_size);

        Set_Par(ANALOG_LOWER,0);
        Set_Par(ANALOG_UPPER,65535);
        Set_Par(28,transfer_size);
        Set_Processdelay(1,cpu_time*time_resolution);
    }
    qDebug() << "Data Transfer Finished.";
    ui->ErrorDisplay->append("Data Transfer Finished.");

    progress_time = 0;
    for(int i=0; i<Analog_output_channel+Digital_output_channel/32; i++)delete []transfer_data[i];
    delete []transfer_data;
    delete []transfer_processdelay;
    refresh_timer->start(300);
    hardwareControl_check_timer->start(300);
    emit Mission_used_state_updata(2);
    MD5_Last_Cycle = md5;
    qDebug() << "Raw Data Deleted";
    ui->ErrorDisplay->append("Raw Data Deleted.");

    if(seq_wrong){
        Adwin_state->S_SeqErr();
        scanning = false;
        running = false;
    }
    else{
        Adwin_state->S_Ready();
    }
    ui->Process_load->setEnabled(true);
    ui->Process_start->setEnabled(true);
    ui->Scan->setEnabled(true);
}

//Reduce size of the transfer data matrix, moved to SequenceCalculator
void MainController::Reduce_Algorithm(int **){
    /*int cpu_time = 1000;
    int maximum = 4999999;
    if(program_setting.at(0)=="1"){
        cpu_time = 40;
        maximum = 799999;
    }
    else if(program_setting.at(0)=="2"){
        cpu_time = 300;
        maximum = 999999;
    }
    else if(program_setting.at(0)=="3"){
        cpu_time = 1000;
        maximum = 4999999;
    }
    qDebug() << "Calculation Finished.    Data Length:" << transfer_size;

    QList<int> calculate_processdelay;
    int processdelay = time_resolution*cpu_time;
    int delay_count = 1;

    for(int i=0; i<total_size; ++i){
        bool reduce = true;
        for(int j=0; j<Analog_output_channel+(Digital_output_channel/32); ++j){
            if(abs(calculate_data[j][i]-calculate_data[j][i+1])>voltage_resolution){
                reduce = false;
                break;
            }
        }
        if(((delay_count+1)*time_resolution)>10000)reduce = false; //ADwin doesn't take processdelay longer than 2s.
        if(reduce){
            delay_count++;
            transfer_size--;
        }
        else{
            calculate_processdelay.append(delay_count*processdelay);
            delay_count = 1;
        }
    }
    calculate_processdelay.append(5000);
    qDebug() << "Reduce Algorithm Completed     Data Length:" << transfer_size;

    //transfer_data = new long*[Analog_output_channel+Digital_output_channel/32];
    for(int channel=0; channel<Analog_output_channel; channel++){
        transfer_data[channel] = new long[transfer_size];
        int pointer = 0;
        for(int i=0; i<transfer_size; ++i){
            transfer_data[channel][i]=calculate_data[channel][pointer];
            pointer += (calculate_processdelay.at(i)/processdelay);
        }
    }
    for(int j=0; j<Digital_output_channel/32; j++){
        transfer_data[j+Analog_output_channel] = new long[transfer_size];
        int pointer = 0;
        for(int i=0; i<transfer_size; ++i){
            transfer_data[j+Analog_output_channel][i]=calculate_data[Analog_output_channel+j][pointer];
            pointer += (calculate_processdelay.at(i)/processdelay);
        }
    }
    transfer_processdelay = new long[transfer_size];
    for(int i=0; i<transfer_size; i++)transfer_processdelay[i] = calculate_processdelay.at(i);

    for(int i=0; i<sequences->size(); i++){
        for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
            for(int k=0; k<sequences->at(i)[j]->size(); k++){
                if(sequences->at(i)[j]->at(k)->get_wrong())seq_wrong = true;
            }
        }
    }
    for(int i=0; i<total_channel; i++)delete calculate_data[i];
    delete calculate_data;
    qDebug() << "Translated to Transfer Data";

    if(true){ //Saving transfer data?
        Save_Tranfer_Data_to_File(transfer_data,transfer_processdelay,transfer_size,md5);
    }

    if(transfer_size > maximum){
        Adwin_state->S_TransErr();
        for(int i=0; i<Analog_output_channel+Digital_output_channel/32; i++)delete transfer_data[i];
        delete transfer_data;
        delete transfer_processdelay;

        QMessageBox msgBox;
        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Cycle too long.");
        msgBox.exec();
        return;
    }*/
}

//Transfer data matrix to Adwin, merge into on_Process_load_clicked.
void MainController::Transfer_Data(long int **transfer_data, long int *transfer_processdelay, int transfer_size, int cpu_time){
    double progress = 0;
    for(int channel=0; channel<Analog_output_channel; channel++){
        SetData_Long(channel+1,&transfer_data[channel][0],1,transfer_size);
        progress += 100./double(Analog_output_channel+Digital_output_channel/32);
        Adwin_state->S_Uploading(progress);
    }
    for(int j=0; j<Digital_output_channel/32; j++){
        SetData_Long(41+j,&transfer_data[Analog_output_channel+j][0],1,transfer_size);
        progress += 100./double(Analog_output_channel+Digital_output_channel/32);
        Adwin_state->S_Uploading(progress);
    }
    SetData_Long(51,&transfer_processdelay[0],1,transfer_size);

    Set_Par(ANALOG_LOWER,0);
    Set_Par(ANALOG_UPPER,65535);
    Set_Par(28,transfer_size);
    Set_Processdelay(1,cpu_time*time_resolution);

    qDebug() << "Data Transfer Finished.";
}

//Display the transfer progress of data matrix, discarded
void MainController::show_progress_window(QMessageBox *msgbox){
    int dots = 1;
    QString message = "Transmitting ";
    while(!msgbox->isHidden()){
        message += ".";
        msgbox->setText(message);
        dots++;
        if(dots>6){
            dots = 1;
            message.chop(6);
        }
    }
    return;
}

void MainController::on_Process_start_clicked()
{
    bool offline = false;
    if(program_setting.at(3)=="offline")offline = true;
    if(offline){
        number_of_run++;
        system_log(11,number_of_run);
        return;
    }
    if(!running){
        system_log(1,0);
        number_of_run = 0;
        number_of_scan = 0;
        ui->Process_load->setDisabled(true);
        ui->Process_start->setText("Stop");
        Adwin_state->S_Running();
        running = true;
        cycle_check_timer->start(CYCLE_CHECK_TIMER_PREIOD);
        global_timer->start(1000);
    }
    else if(running){
        Set_Par(41,0);
        ui->Process_start->setDisabled(true);
        Adwin_state->S_Stopping();
        running = false;
        scanning = false;
        ui->Scan->setText("Start\nScan");
    }
}

void MainController::on_Scan_clicked()
{
    bool offline = false;
    if(program_setting.at(3)=="offline")offline = true;
    if(offline){
        int sum = 1;
        for(int i=0; i<10; i++)if(total_number_of_scan[i]!=0)sum = sum * total_number_of_scan[i];
        system_log(12,sum);
        for(int i=1; i<sum+1; i++){
            number_of_run++;
            number_of_scan++;
            if(MD5_of_current_cycle()!=MD5_Last_Cycle){
                on_Process_load_clicked();
            }
            emit_new_cycle();
        }
        number_of_scan = 0;
        return;
    }
    if(scanning){
        scanning = false;
        system_log(4,number_of_scan);
        ui->Scan->setText("Start\nScan");
        Adwin_state->S_ScanFin();
    }
    else{
        /*int sum = 1;
        for(int i=0; i<10; i++)if(total_number_of_scan[i]!=0)sum = sum * total_number_of_scan[i];
        system_log(3,sum);*/
        scanning = true;
        ui->Scan->setText("Stop\nScan");
        Adwin_state->S_Scanning();
    }
    if(!running)on_Process_start_clicked();
}

void MainController::on_Time_resolution_returnPressed()
{
    bool ok;
    QString temp = ui->Time_resolution->text();
    int temp_number = temp.toInt(&ok,10);
    if (!ok){
        ui->Time_resolution->setText(QString::number(time_resolution));
        QMessageBox msgBox;

        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Invaild time resolution.");
        msgBox.exec();
    }
    else time_resolution = temp_number;
}

void MainController::calculate_cycle_time(){
    double max_time = 0;   //ms
    double time[Analog_output_channel+Digital_output_channel];       //ms
    int sum = 1;
    int step[10];
    for(int i=0; i<10; i++)step[i]=0;

    for(int i=0; i<variables->size();i++){
        if(variables->at(i)->get_type()==1&&variables->at(i)->get_value()[0]!=0){
            int temp_step = ((variables->at(i)->get_value()[2]-variables->at(i)->get_value()[1])/variables->at(i)->get_value()[3])+1;
            if(step[int(variables->at(i)->get_value()[0])-1]<temp_step)step[int(variables->at(i)->get_value()[0])-1] = temp_step;
        }
    }

    for(int i=0; i<10;i++){
        if(step[i]!=0){
            sum = sum * step[i];
        }
        total_number_of_scan[i] = step[i];
    }

    for(int i=0; i<Analog_output_channel; i++)time[i]=0;
    for(int i=0; i<sequences->size();i++){
        double t_sec=0;
        if(sections_activity->at(i)->get_activity(number_of_scan)){
            for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
                double temp=0;
                for(int k=0; k<sequences->at(i)[j]->size(); k++){
                    double seq_time = sequences->at(i)[j]->at(k)->get_time(number_of_scan);
                    if(seq_time>0)temp = temp + seq_time;
                }
                time[j] += temp;
                if(temp>t_sec)t_sec = temp;
            }
        }
        max_time += t_sec;
    }
    total_time = max_time*(sum);
    cycle_time = max_time;
}

void MainController::cycle_changed(bool c){
    QPixmap pixmap = QPixmap(ui->already_Load->width(),ui->already_Load->height());
    if(c){
        cycle_change = true;
        calculate_cycle_time();
        if(isOpen_ChannelEditor)channelEditer->calcualte_times();
        pixmap.fill(Qt::yellow);
    }
    else{
        cycle_change = false;
        pixmap.fill(Qt::darkGreen);
    }
    ui->already_Load->setPixmap(pixmap);
}

void MainController::check_cycle_state(){
    //cycle_check_timer->stop();
    int AIn_lock = Get_Par(ANALOG_SAFETY_LOCK);
    int DIn_lock = Get_Par(DIGITAL_SAFETY_LOCK);
    int mode = Get_Par(MODE_PAR);
    if(AIn_lock){
        Stop_Process(1);
        Stop_Process(2);
        Stop_Process(3);
        global_timer->stop();
        cycle_check_timer->stop();
        ui->Process_start->setText("Start");
        ui->Scan->setText("Start\nScan");
        ui->Process_load->setDisabled(true);
        ui->Process_start->setDisabled(true);
        ui->Scan->setDisabled(true);
        emit Mission_used_state_updata(-2);
        Adwin_state->S_Frozen();
        system_log(7,1);
    }
    else if(DIn_lock){
        Stop_Process(1);
        Stop_Process(2);
        Stop_Process(3);
        global_timer->stop();
        cycle_check_timer->stop();
        ui->Process_start->setText("Start");
        ui->Scan->setText("Start\nScan");
        ui->Process_load->setDisabled(true);
        ui->Process_start->setDisabled(true);
        ui->Scan->setDisabled(true);
        emit Mission_used_state_updata(-2);
        Adwin_state->S_Frozen();
        system_log(7,0);
    }

    if(progress_time<currentRunning_cycle_time){
        progress_time += CYCLE_CHECK_TIMER_PREIOD;
        ui->cycle_progress->setValue(int(progress_time*100/currentRunning_cycle_time));
    }
    int sum = 1;
    for(int i=0; i<10; i++)if(total_number_of_scan[i]!=0)sum = sum * total_number_of_scan[i];
    if(mode==MODE_RUNNING)return;
    else if(mode==MODE_STOPPED){
        if(scanning && number_of_scan < sum){
            number_of_run++;
            number_of_scan++;
            if(MD5_of_current_cycle()!=MD5_Last_Cycle){
                on_Process_load_clicked();
            }
            if(!running){
                SetPar(51,0);
                system_log(6,number_of_scan-1);
                return;
            }
            Adwin_state->S_Scanning();
            ui->cycle_progress->setValue(0);
            progress_time = 0;
            Set_Par(41,1);
            ui->ErrorDisplay->clear();
            ui->ErrorDisplay->append("New Cycle. Run:"+QString::number(number_of_run)+" Scan:"+QString::number(number_of_scan));
            emit_new_cycle();
            //cycle_check_timer->start(CYCLE_CHECK_TIMER_PREIOD);
            emit Mission_used_state_updata(1);
        }
        else if(scanning && number_of_scan>=sum){
            scanning = false;
            system_log(4,number_of_scan);
            ui->Scan->setText("Start\nScan");
            Adwin_state->S_ScanFin();
            if(program_setting.at(7).contains("ContinueAfterScan",Qt::CaseInsensitive)){
                number_of_scan = 0;
                if(MD5_of_current_cycle()!=MD5_Last_Cycle){
                    on_Process_load_clicked();
                }
                ui->cycle_progress->setValue(0);
                progress_time = 0;
                Set_Par(41,1);
                ui->ErrorDisplay->clear();
                ui->ErrorDisplay->append("New Cycle. Run:"+QString::number(number_of_run)+" Scan:"+QString::number(number_of_scan));
                emit_new_cycle();
                //cycle_check_timer->start(CYCLE_CHECK_TIMER_PREIOD);
                Set_Par(51,0);
            }
            else{
                scanning = false;
                running = false;
                SetPar(51,0);
                system_log(2,0);
                ui->Process_start->setEnabled(true);
                ui->Process_load->setEnabled(true);
                ui->Process_start->setText("Start");
                cycle_check_timer->stop();
                global_timer->stop();
            }
        }
        else if(running){
            number_of_run++;
            ui->cycle_progress->setValue(0);
            progress_time = 0;
            if(program_setting.at(5)=="tcpon"){
                cycle_check_timer->stop();
                Adwin_state->S_Waiting();
                while(!remote_start){
                    Delay(100);
                    if(!running){
                        cycle_check_timer->start(CYCLE_CHECK_TIMER_PREIOD);
                        return;
                    }
                }
                if(remote_load)on_Process_load_clicked();
                Adwin_state->S_Running();
            }
            Set_Par(41,1);
            remote_start = false;
            remote_load = false;
            ui->ErrorDisplay->clear();
            ui->ErrorDisplay->append("New Cycle. Run:"+QString::number(number_of_run)+" Scan:"+QString::number(number_of_scan));
            emit_new_cycle();
            cycle_check_timer->start(CYCLE_CHECK_TIMER_PREIOD);
            emit Mission_used_state_updata(0);
        }
        else if(!running){
            SetPar(51,0);
            system_log(2,0);
            number_of_run = 0;
            number_of_scan = 0;
            ui->cycle_progress->reset();
            ui->Process_start->setEnabled(true);
            ui->Process_load->setEnabled(true);
            ui->Process_start->setText("Start");
            if(seq_wrong){
                Adwin_state->S_SeqErr();
            }
            else{
                Adwin_state->S_Ready();
            }
            cycle_check_timer->stop();
            ui->Global_timer->display(0);
            global_timer->stop();
            emit Mission_used_state_updata(3);
        }
        else on_Process_start_clicked();
    }
}

void MainController::check_hardware_state(){
    if(connected){
        int manual_trigger = Get_Par(42);
        if(manual_trigger>0){
            if(ui->Process_start->isEnabled())on_Process_start_clicked();
            Set_Par(42,0);
        }
    }
}

void MainController::save_setting_clicked(){
    QString filename = QFileDialog::getSaveFileName(this,"Save Setting",0,tr("Setting File (*.adcs)"));
    if(!filename.isNull())save_setting(filename);
}

void MainController::save_setting(QString filename){
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))return;
    QTextStream out(&file);
    out << "ADwin ";
    if(program_setting.at(0)=="0")out << "T9" << endl;
    else if(program_setting.at(0)=="1")out << "T10" << endl;
    else if(program_setting.at(0)=="2")out << "T11" << endl;
    else if(program_setting.at(0)=="3")out << "T12" << endl;
    out << "Analog_output_channel:" << Analog_output_channel << endl;
    out << "Digital_output_channel:" << Digital_output_channel << endl;
    out << "Number of Run:" << endl << number_of_run << endl;
    out << "Number of Scan:" << endl << number_of_scan << endl;
    out << "Setting:" << endl;
    out << "cycle_time:" << endl << cycle_time << endl;
    out << "total_time:" << endl<< total_time << endl;
    out << "time_resolution:" << endl << time_resolution << endl;
    out << "Auto_saving:" << endl << auto_save->get_enable() << endl << auto_save->get_path() << endl
        << auto_save->get_path() << endl<< auto_save->get_description() << endl;

    /*out << "Safety setting:" << endl;
    out << ui->Ain_Safty_enable->isChecked() << endl;
    out << ui->Safty_voltage_lower->text() << endl;
    out << ui->Safty_voltage_upper->text() << endl;
    for(int i=0; i<8; i++){
        out << Din_safety_switch[i]->isChecked() << endl;
        out << Din_safety[i]->currentIndex() << endl;
    }
    out << "End of Safety setting" << endl;*/

    out << "AO Channels:" << endl;
    for(int i=0; i<Analog_output_channel; i++){
        out << channel_name->at(i) << endl << channel_calibration->at(i) << endl << QString::number(channel_color->at(i)) << endl;
    }
    out << "End of AO Channel" << endl;

    out << "DiO Channels:" << endl;
    for(int i=0; i<Digital_output_channel; i++){
        out << channel_name->at(i+Analog_output_channel) << endl << channel_calibration->at(i+Analog_output_channel)<< endl << QString::number(channel_color->at(i+Analog_output_channel)) << endl;
    }
    out << "End of DiO Channel" << endl;

    out << "Variables:" << endl;
    for(int i=0; i<variables->size(); i++){
        double *v = variables->at(i)->get_value();
        out<< variables->at(i)->get_name()<< endl << variables->at(i)->get_type() << endl;
        for(int j=0; j<4; j++)out << v[j] << endl;
        out<< variables->at(i)->get_filename()<<endl << variables->at(i)->get_fomular()<< endl;
        out<< variables->at(i)->get_fitting()<< endl;
    }
    out << "End of Variables" << endl;

    out << "VariableClass:" << endl;
    QList<int> classPosition;
    VariableClass *vc = variableClass;
    classPosition.append(0);
    out << "0" << endl << "All Variables";
    for(int i=0; i<variableClass->get_VariableSize(); i++){
        out << ":" << variableClass->get_Variable(i)->get_name()+'!'+QString::number(variableClass->get_VariableColor(i));
    }
    out << endl;
    if(variableClass->childCount()!=0){
        vc = (VariableClass*)variableClass->child(0);
        do{
            for(int j=0; j<classPosition.size(); j++)out << QString::number(classPosition.at(j))+":";
            out << QString::number(vc->parent()->indexOfChild(vc)) << endl << vc->text(0);
            for(int j=0; j<vc->get_VariableSize(); j++)out << ":"+vc->get_Variable(j)->get_name()+'!'+QString::number(vc->get_VariableColor(j));
            out << endl;
            if(vc->childCount()>0){
                classPosition.append(vc->parent()->indexOfChild(vc));
                vc = (VariableClass*)vc->child(0);
            }
            else if((vc->parent()->indexOfChild(vc)+1)<vc->parent()->childCount()){
                vc = (VariableClass*)vc->parent()->child(vc->parent()->indexOfChild(vc)+1);
            }
            else{
                while((vc->parent()->indexOfChild(vc)+1)==vc->parent()->childCount()){
                    vc = static_cast<VariableClass*>(vc->parent());
                    classPosition.removeLast();
                    if(vc==variableClass)break;
                }
                if(vc!=variableClass){
                    vc = (VariableClass*)vc->parent()->child(vc->parent()->indexOfChild(vc)+1);
                    int temp = classPosition.last();
                    classPosition.replace(classPosition.size()-1,temp+1);
                }
            }
        }while(vc!=variableClass);
    }
    out << "End of VariableClass" << endl;

    out << "DisplayClass" << endl;
    for(int i=0; i<DisplayClass->size(); i++){
        out << DisplayClass->at(i)->text(0) << endl;
    }
    out << "End of DisplayClass" << endl;

    out << "Section Array:" << endl;
    for(int i=0; i<section_array->size(); i++)out << section_array->at(i) << endl;
    out << "End of Section Array" << endl;

    out << "Section Activity:" << endl;
    for(int i=0; i<sections_activity->size(); i++){
        if(sections_activity->at(i)->get_variable()==NULL)out << QString::number(sections_activity->at(i)->get_value()) << endl;
        else out << sections_activity->at(i)->get_variable()->get_name() << endl;
    }
    out << "End of Section Activity" << endl;

    out << "Sequences AOut:" << endl;
    for(int i=0; i<sequences->size(); i++){
        for(int j=0; j<Analog_output_channel; j++){
            out << "AO"+QString::number(j+1)+":" << endl;
            for(int k=0; k<sequences->at(i)[j]->size();k++){
                out << QString::number(sequences->at(i)[j]->at(k)->lower_bound())+"!"+QString::number(sequences->at(i)[j]->at(k)->upper_bound()) << endl;
                double *value = sequences->at(i)[j]->at(k)->get_value();
                Variable **vari = sequences->at(i)[j]->at(k)->get_variables();
                for(int l=0; l<5; l++)out << value[l] << endl;
                for(int l=0; l<4; l++){
                    if(vari[l]!=NULL)out << vari[l]->get_name() << endl;
                    else out << "only_system_used_null" << endl;
                }
            }
        }
    }
    out << "End of AO Sequences" << endl;

    out << "Sequences DiO:" << endl;
    for(int i=0; i<sequences->size(); i++){
        for(int j=Analog_output_channel; j<Analog_output_channel+Digital_output_channel; j++){
            out << "DiO"+QString::number(j+1-Analog_output_channel)+":" << endl;
            for(int k=0; k<sequences->at(i)[j]->size();k++){
                double *value = sequences->at(i)[j]->at(k)->get_value();
                Variable **vari = sequences->at(i)[j]->at(k)->get_variables();
                for(int l=1; l<4; l++)out << value[l] << endl;
                for(int l=0; l<3; l++){
                    if(vari[l]!=NULL)out << vari[l]->get_name() << endl;
                    else out << "only_system_used_null" << endl;
                }
            }
        }
    }
    out << "End of DiO Sequences" << endl;

    out << "Sections:" << endl;
    for(int i=0; i<sections->size(); i++){
        out << ":"+sections_name->at(i) << endl;
        for(int j=0; j<Analog_output_channel; j++){
            out << "AO"+QString::number(j+1)+":" << endl;
            for(int k=0; k<sections->at(i)[j]->size(); k++){
                double *value = sections->at(i)[j]->at(k)->get_value();
                Variable **vari = sections->at(i)[j]->at(k)->get_variables();
                for(int l=0; l<5; l++)out << value[l] << endl;
                for(int l=0; l<4; l++){
                    if(vari[l]!=NULL)out << vari[l]->get_name() << endl;
                    else out << "only_system_used_null" << endl;
                }
            }
        }
        for(int j=Analog_output_channel; j<Analog_output_channel+Digital_output_channel; j++){
            out << "DiO"+QString::number(j+1-Analog_output_channel)+":" << endl;
            for(int k=0; k<sections->at(i)[j]->size(); k++){
                double *value = sections->at(i)[j]->at(k)->get_value();
                Variable **vari = sections->at(i)[j]->at(k)->get_variables();
                for(int l=1; l<4; l++)out << value[l] << endl;
                for(int l=0; l<3; l++){
                    if(vari[l]!=NULL)out << vari[l]->get_name() << endl;
                    else out << "only_system_used_null" << endl;
                }
            }
        }
    }
    out << "End of Sections" << endl;

    out << "Transfer Files" << endl;
    for(int i=0; i<External_File_Transfer->size(); i++){
        for(int j=0; j<External_File_Transfer->at(i).size(); j++){
            if(External_File_Transfer->at(i).at(j).isEmpty())out << "0";
            else out << External_File_Transfer->at(i).at(j) << "|";
        }
        out << endl;
    }
    out << "End of Transfer Files" << endl;

    /*out << "DDS Control" << endl;
    out << DDSControl(Widget_Addon.at(5)).current_Setting() << endl;
    out << "End of DDS Control" << endl;*/

    file.close();
}

void MainController::load_setting_clicked(){
    QString filename = QFileDialog::getOpenFileName(this,"Load Setting",0,tr("Setting File (*.adcs)"));
    if(!filename.isNull())load_setting(filename);
}

void MainController::load_setting(QString filename){
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))return;
    QTextStream in(&file);
    QStringList newsetting;
    QString temp;
    temp = in.readLine().remove(0,6);
    if(temp == "T9") newsetting.append("0");
    else if(temp == "T10") newsetting.append("1");
    else if(temp == "T11") newsetting.append("2");
    else if(temp == "T12") newsetting.append("3");
    temp = in.readLine().remove("Analog_output_channel:");
    newsetting.append(QString::number(temp.toInt()/8));
    temp = in.readLine().remove("Digital_output_channel:");
    newsetting.append(QString::number(temp.toInt()/32));
    newsetting.append("online");
    if(newsetting.at(1)!=program_setting.at(1) || newsetting.at(2)!=program_setting.at(2)){
        QMessageBox msgBox;
        msgBox.setText("This setting has different number of channels.");
        msgBox.setInformativeText("Proceed?");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        switch (ret) {
        case QMessageBox::Yes:
            break;
        case QMessageBox::Cancel:
            return;
            break;
        default:
            break;
        }
        QMessageBox msgBox2;

        msgBox2.setText("Change the number of channels as the setting?");
        msgBox2.setIcon(QMessageBox::Question);
        msgBox2.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox2.setDefaultButton(QMessageBox::No);
        int ret2 = msgBox2.exec();
        switch (ret2){
        case QMessageBox::Yes:
            program_setting = newsetting;
            break;
        case QMessageBox::No:
            break;
        case QMessageBox::Cancel:
            return;
            break;
        default:
            break;
        }
    }

    refresh_timer->stop();
    cycle_check_timer->stop();
    global_timer->stop();
    Clear();
    Analog_output_channel = 8*program_setting.at(1).toInt();
    Digital_output_channel = 32*program_setting.at(2).toInt();
    program_initial();

    emit program_setting_change(program_setting);
    channel_name->clear();
    channel_color->clear();
    channel_calibration->clear();
    int counter = 0;
    bool ifvariableclass = false;
    while(!in.atEnd()){
        QString temp = in.readLine();
        if(temp=="cycle_time:"){
            QString n = in.readLine();
            cycle_time = n.toDouble();
        }
        else if(temp=="total_time:"){
            QString n = in.readLine();
            total_time = n.toDouble();
        }
        else if(temp=="time_resolution:"){
            QString n = in.readLine();
            time_resolution = n.toDouble();
            ui->Time_resolution->setText(n);
        }
        else if(temp=="Auto_saving"){
            QString n = in.readLine();
            auto_save->setEnabled(n.toInt());
            n = in.readLine();
            n = in.readLine();
            auto_save->set_path(n);
            n = in.readLine();
            auto_save->set_description(n);
        }
        /*else if(temp=="Safety setting:"){
            QString a = in.readLine();
            ui->Ain_Safty_enable->setChecked(a.toInt());
            a = in.readLine();
            ui->Safty_voltage_lower->setText(a);
            a = in.readLine();
            ui->Safty_voltage_upper->setText(a);
            for(int i=0; i<8; i++){
                a = in.readLine();
                Din_safety_switch[i]->setChecked(a.toInt());
                a = in.readLine();
                Din_safety[i]->setCurrentIndex(a.toInt());
            }
        }*/
        else if(temp=="AO Channels:"){
            counter = 0;
            QString name = in.readLine();
            while(name!="End of AO Channel"){
                if(counter < Analog_output_channel){
                    QString cal = in.readLine();
                    int col = in.readLine().toInt();
                    channel_name->append(name);
                    channel_calibration->append(cal);
                    channel_color->append(col);
                    name = in.readLine();
                    counter++;
                }
            }
            if(counter < Analog_output_channel){
                for(int i=counter; i<Analog_output_channel; i++){
                    channel_name->append("AO "+QString::number(i+1));
                    channel_calibration->append("Not calibrated");
                    channel_color->append(i%2);
                }
            }
            counter = 0;
        }
        else if(temp=="DiO Channels:"){
            counter = 0;
            QString name = in.readLine();
            while(name!="End of DiO Channel"){
                if(counter < Digital_output_channel){
                    QString col = in.readLine();
                    channel_name->append(name);
                    if(col=="Normal" || col=="Inverted"){
                        channel_calibration->append(col);
                        col = in.readLine();
                    }
                    channel_color->append(col.toInt());
                    name = in.readLine();
                    counter++;
                }
            }
            if(counter < Digital_output_channel){
                for(int i=counter; i<Digital_output_channel; i++){
                    channel_name->append("DiO "+QString::number(i+1));
                    channel_color->append(i%2);
                    channel_calibration->append("Normal");
                }
            }
            counter = 0;
        }
        else if(temp=="Variables:"){
            QString name = in.readLine();
            while(name!="End of Variables"){
                int type =in.readLine().toInt();
                QString f;
                Variable *temp = new Variable(this,name,type,QiEngine,variables);
                variables->append(temp);
                for(int i=0; i<4; i++){
                    QString v = in.readLine();
                    if(type<2)temp->set_value(i,v.toDouble());
                }
                f = in.readLine();
                if(!f.isEmpty())temp->set_file(f);
                f = in.readLine();
                if(!f.isEmpty())temp->set_fomular(f);
                f = in.readLine();
                if(type==3)temp->change_fitting_mode(f.toInt());
                name = in.readLine();
            }
        }
        else if(temp=="VariableClass:"){
            ifvariableclass = true;
            QString vc_in = in.readLine();
            while(vc_in!="End of VariableClass"){
                VariableClass* vc = variableClass;
                QStringList classPosition_string = vc_in.split(':');
                QList<int> classPosition;
                for(int i=0; i<classPosition_string.size(); i++)classPosition.append(classPosition_string.at(i).toInt());
                QStringList vc_variable = in.readLine().split(':');
                for(int i=1; i<classPosition.size(); i++){
                    while(classPosition.at(i)>(vc->childCount()-1)){
                        VariableClass* newClass = new VariableClass;
                        vc->addChild((QTreeWidgetItem*)newClass);
                    }
                    vc = (VariableClass*)vc->child(classPosition.at(i));
                }
                vc->setText(0,vc_variable.at(0));
                for(int i=1; i<vc_variable.size(); i++){
                    QStringList v = vc_variable.at(i).split('!');
                    QString name = v.first();
                    int color = 0;
                    if(v.size()>1)color = v.at(1).toInt();
                    for(int j=0; j<variables->size(); j++){
                        if(name==variables->at(j)->get_name()){
                            vc->apend_Variable(variables->at(j),color);
                            break;
                        }
                    }
                }
                vc_in = in.readLine();
            }
        }
        else if(temp=="DisplayClass"){
            QString dc = in.readLine();
            while(dc!="End of DisplayClass"){

            }
        }
        else if(temp=="Section Array:"){
            QString name = in.readLine();
            while(name!="End of Section Array"){
                section_array->append(name);
                name = in.readLine();
            }
        }
        else if(temp=="Section Activity:"){
            QString act = in.readLine();
            while(act!="End of Section Activity"){
                SectionActivity *secact = new SectionActivity;
                bool ok = false;
                double temp = act.toDouble(&ok);
                if(ok)secact->set_value(temp);
                else{
                    for(int i=0; i<variables->size(); i++){
                        if(act==variables->at(i)->get_name()){
                            secact->set_variable(variables->at(i));
                        }
                    }
                }

                sections_activity->append(secact);
                act = in.readLine();
            }
        }
        else if(temp=="Sequences AOut:"){
            QString ao = in.readLine();
            int ii=-1;
            int channel = 0;
            while(ao!="End of AO Sequences"){
                if(ao=="AO1:"){
                    QList<Sequence*> **sec = new QList<Sequence*>*[Analog_output_channel+Digital_output_channel];
                    for(int i=0; i<Analog_output_channel+Digital_output_channel; i++)sec[i]=new QList<Sequence*>;
                    sequences->append(sec);
                    ii++;
                }
                if(ao.contains(':')){
                    ao.remove("AO");
                    ao.remove(':');
                    channel = ao.toInt()-1;
                    ao = in.readLine();
                }
                else{
                    if(channel < Analog_output_channel){
                        Sequence *temp = new Sequence(this,false,variables,variableClass);
                        sequences->at(ii)[channel] -> append(temp);
                        temp->set_boundary(ao);
                        double v[5];
                        v[0]=in.readLine().toDouble();
                        for(int i=1; i<5; i++)v[i]=in.readLine().toDouble();
                        temp->set_value(v);
                        for(int i=0; i<4; i++){
                            QString name = in.readLine();
                            if(name!="only_system_used_null"){
                                for(int j=0; j<variables->size(); j++){
                                    if(name==variables->at(j)->get_name())temp->set_vari(i,variables->at(j));
                                }
                            }
                        }
                    }
                    ao = in.readLine();
                }
            }
        }
        else if(temp=="Sequences DiO:"){
            QString di = in.readLine();
            int ii = -1;
            int channel = 0;
            while(di!="End of DiO Sequences"){
                if(di=="DiO1:")ii++;
                if(di.contains(':')){
                    di.remove("DiO");
                    di.remove(':');
                    channel = di.toInt()-1;
                    di = in.readLine();
                }
                else{
                    if(channel<Digital_output_channel){
                        Sequence *temp = new Sequence(this,true,variables,variableClass);
                        sequences->at(ii)[channel+Analog_output_channel] -> append(temp);
                        temp->set_value(1,di.toDouble());
                        for(int i=2; i<4; i++){
                            double v = in.readLine().toDouble();
                            temp->set_value(i,v);
                        }
                        for(int i=0; i<3; i++){
                            QString name = in.readLine();
                            if(name!="only_system_used_null"){
                                for(int j=0; j<variables->size(); j++){
                                    if(name==variables->at(j)->get_name())temp->set_vari(i,variables->at(j));
                                }
                            }
                        }
                    }
                    di = in.readLine();
                }
            }
        }
        else if(temp=="Sections:"){
            QString ao = in.readLine();
            int ii=-1;
            int channel = 0;
            while(ao!="End of Sections"){
                if(ao.at(0)==':'){
                    QList<Sequence*> **sec = new QList<Sequence*>*[Analog_output_channel+Digital_output_channel];
                    for(int i=0; i<Analog_output_channel+Digital_output_channel; i++)sec[i]=new QList<Sequence*>;
                    sections->append(sec);
                    ao.remove(':');
                    sections_name->append(ao);
                    ii++;
                    ao = in.readLine();
                }
                else if(ao.contains("AO")){
                    ao.remove("AO");
                    ao.remove(':');
                    channel = ao.toInt()-1;
                    ao = in.readLine();
                }
                else if(ao.contains("DiO")){
                    ao.remove("DiO");
                    ao.remove(':');
                    channel = ao.toInt()-1+50+Analog_output_channel;
                    ao = in.readLine();
                }
                else if(channel<Analog_output_channel){
                    Sequence *temp = new Sequence(this,false,variables,variableClass);
                    sections->at(ii)[channel] -> append(temp);
                    double v[5];
                    v[0]=ao.toDouble();
                    for(int i=1; i<5; i++)v[i]=in.readLine().toDouble();
                    temp->set_value(v);
                    for(int i=0; i<4; i++){
                        QString name = in.readLine();
                        if(name!="only_system_used_null"){
                            for(int j=0; j<variables->size(); j++){
                                if(name==variables->at(j)->get_name())temp->set_vari(i,variables->at(j));
                            }
                        }
                    }
                    ao = in.readLine();
                }
                else if(channel>49 && (channel-50-Analog_output_channel)<Digital_output_channel){
                    int chl = channel - 50;
                    Sequence *temp = new Sequence(this,true,variables,variableClass);
                    sections->at(ii)[chl] -> append(temp);
                    double v[5];
                    v[1]=ao.toDouble();
                    for(int i=2; i<4; i++)v[i]=in.readLine().toDouble();
                    v[0] = -1;
                    v[4] = 0;
                    temp->set_value(v);
                    for(int i=0; i<3; i++){
                        QString name = in.readLine();
                        if(name!="only_system_used_null"){
                            for(int j=0; j<variables->size(); j++){
                                if(name==variables->at(j)->get_name())temp->set_vari(i,variables->at(j));
                            }
                        }
                    }
                    ao = in.readLine();
                }
                else ao = in.readLine();
            }
        }
        else if(temp=="Transfer Files"){
            External_File_Transfer->clear();
            QString tra = in.readLine();
            while(tra!="End of Transfer Files"){
                External_File_Transfer->append(tra.split('|'));
                tra = in.readLine();
            }
        }
        /*else if(temp=="DDS Control"){
            //QString filename = in.readLine();
            //DDSControl(Widget_Addon.at(6)).new_CycleSetting(filename);
        }*/
    }
    file.close();
    for(int i=0; i<variables->size(); i++)variables->at(i)->Reload();
    static_cast<External*>(Widget_Addon.at(FILETRANSFER))->Reset(variables);
    if(!ifvariableclass){
        for(int i=0; i<variables->size(); i++)variableClass->apend_Variable(variables->at(i));
    }
    if(sections_activity->size() < sequences->size()){
        int temp = sections_activity->size();
        for(int i=0; i<(sequences->size()-temp); i++){
            SectionActivity *secact = new SectionActivity;
            secact->set_value(1);
            sections_activity->append(secact);
        }
    }
    cycle_changed(true);
    link_to_DataGroup();
    emit setting_load(DataGroup);
}

void MainController::system_log(int event, int scan){
    QFile file("System_log.txt");
    if (!file.open(QIODevice::Append | QIODevice::Text))return;
    QTextStream out(&file);
    QDateTime datetime = QDateTime::currentDateTime();
    QString n = QString::number(scan);

    switch (event) {
    case 0://Program start up
        out << datetime.toString("yyyy.MM.dd -- HH:mm:ss")+" #Program Start" << endl;
        break;
    case 1://Run begin
        out << datetime.toString("yyyy.MM.dd -- HH:mm:ss")+" #Continous Run Begin" << endl;
        break;
    case 2://Run stop
        out << datetime.toString("yyyy.MM.dd -- HH:mm:ss")+" #Continous Run Stop" << endl;
        break;
    case 3://Scan begin
        out << datetime.toString("yyyy.MM.dd -- HH:mm:ss")+" #Scan Begin, cycle to scan " << n << endl;
        break;
    case 4://Scan stop normal
        out << datetime.toString("yyyy.MM.dd -- HH:mm:ss")+" #Scan End, cycle complete " << n << endl;
        break;
    case 5://Emergency stop
        out << datetime.toString("yyyy.MM.dd -- HH:mm:ss")+" #Emergency Stop" << endl;
        break;
    case 6://Scan stop with error
        out << datetime.toString("yyyy.MM.dd -- HH:mm:ss")+" #Scan End, cycle complete " << n << ", error occur" << endl;
        break;
    case 7://Safty trigger stop
        if(scan)out << datetime.toString("yyyy.MM.dd -- HH:mm:ss")+" #Analog Safty Triggered Emergency Stop" << endl;
        else out << datetime.toString("yyyy.MM.dd -- HH:mm:ss")+" #Digital Safty Triggered Emergency Stop" << endl;
        break;
    case 8://Program exit normally
        out << datetime.toString("yyyy.MM.dd -- HH:mm:ss")+" #Program Exit" << endl;
        break;
    case 9://Switch to offline mode
        out << datetime.toString("yyyy.MM.dd -- HH:mm:ss")+" #Switch to Offline Mode" << endl;
        break;
    case 10://Switch to online mode
        out << datetime.toString("yyyy.MM.dd -- HH:mm:ss")+" #Switch to Online Mode" << endl;
        break;
    case 11://Continous run in offline mode
        out << datetime.toString("yyyy.MM.dd -- HH:mm:ss")+" #Offline Continous Run" << endl;
        break;
    case 12://Scan in offline mode
        out << datetime.toString("yyyy.MM.dd -- HH:mm:ss")+" #Offline Scan, total cycle " << n << endl;
        break;
    default:
        break;
    }
}

void MainController::System_log_open(){
    /*Systemlog *a = new Systemlog;
    a->show();
    connect(a,SIGNAL(accepted()),a,SLOT(deleteLater()));
    connect(a,SIGNAL(rejected()),a,SLOT(deleteLater()));*/
}

void MainController::emit_new_cycle(){
    QString msg = QString::number(int(cycle_time))+":"+QString::number(number_of_run)+":"+QString::number(number_of_scan);
    emit new_cycle(msg);
}

void MainController::on_Extra_Addon_Close_clicked()
{
    this->setMinimumWidth(375);
    this->setMaximumWidth(375);
    for(int i=0; i<Widget_Addon.size(); i++){
        Widget_Addon.at(i)->hide();
    }
}

void MainController::on_Extra_Addon_Open_clicked()
{
    QMenu *menu = new QMenu(this);
    for(int i=0; i<Widget_Addon.size(); i++){
        QAction *action = new QAction(menu);
        action->setText(addons_names.at(i));
        menu->addAction(action);
        QSignalMapper *signalmapper = new QSignalMapper(menu);
        signalmapper->setMapping(action,i);
        connect(action,SIGNAL(triggered()),signalmapper,SLOT(map()));
        connect(signalmapper,SIGNAL(mapped(int)),this,SLOT(open_Addon(int)));
    }
    menu->popup(QCursor::pos());
}

void MainController::open_Addon(int index){
    this->setMaximumWidth(850);
    this->setMinimumWidth(850);
    for(int i=0; i<Widget_Addon.size(); i++){
        if(i==index)Widget_Addon.at(i)->show();
        else Widget_Addon.at(i)->hide();
    }
}

void MainController::on_Variable_monitor_clicked()
{
    if(isOpen_VariableMonitor)emit Variable_Monitor_Click();
    else{
        isOpen_VariableMonitor = true;
        variableExpleror->Initialize();
        variableExpleror->show();
        connect(variableExpleror,SIGNAL(accepted()),this,SLOT(Variable_monitor_close()));
        connect(variableExpleror,SIGNAL(rejected()),this,SLOT(Variable_monitor_close()));
        connect(this,SIGNAL(Variable_Monitor_Click()),variableExpleror,SLOT(raise()));
        connect(variableExpleror,SIGNAL(report_variable_Edited()),this,SLOT(cycle_changed()));
        variableExpleror->Setup_report(true);
    }
}

void MainController::on_Channel_editer_clicked()
{
    if(isOpen_ChannelEditor)emit Channel_Editor_Click();
    else{
        isOpen_ChannelEditor = true;
        channelEditer = new Channelediter(0,Analog_output_channel,Digital_output_channel);
        channelEditer->initial(sequences,variables,variableClass,sections,sections_name,section_array,sections_activity,
                   channel_name,channel_calibration,channel_color,total_number_of_scan,&number_of_scan);
        channelEditer->show();
        connect(channelEditer,SIGNAL(accepted()),channelEditer,SLOT(deleteLater()));
        connect(channelEditer,SIGNAL(rejected()),channelEditer,SLOT(deleteLater()));
        connect(channelEditer,SIGNAL(accepted()),this,SLOT(Channel_editor_close()));
        connect(channelEditer,SIGNAL(rejected()),this,SLOT(Channel_editor_close()));
        connect(this,SIGNAL(Channel_Editor_Click()),channelEditer,SLOT(raise()));
        connect(this,SIGNAL(section_change()),channelEditer,SLOT(section_names_import()));
        connect(channelEditer,SIGNAL(report_cycle_Edited()),this,SLOT(cycle_changed()));
    }
}

void MainController::on_Section_editer_clicked()
{
    if(isOpen_SectionEditor)emit Section_Editor_Click();
    else{
        isOpen_SectionEditor = true;
        Sectionediter *a = new Sectionediter(0,Analog_output_channel,Digital_output_channel);
        a->initial(variables,variableClass,sections,sections_name,channel_name,total_number_of_scan);
        a->show();
        connect(a,SIGNAL(section_change()),this,SIGNAL(section_change()));
        connect(a,SIGNAL(accepted()),a,SLOT(deleteLater()));
        connect(a,SIGNAL(rejected()),a,SLOT(deleteLater()));
        connect(a,SIGNAL(accepted()),this,SLOT(Section_editor_close()));
        connect(a,SIGNAL(rejected()),this,SLOT(Section_editor_close()));
        connect(this,SIGNAL(Section_Editor_Click()),a,SLOT(raise()));
    }
}

void MainController::on_Channel_Display_clicked()
{
    CycleDisplayer *a = new CycleDisplayer(0,QiEngine,sequences,variables,sections_activity,channel_calibration,channel_name,Analog_output_channel,Digital_output_channel,true);
    a->show();
}

void MainController::Channel_editor_close(){
    isOpen_ChannelEditor = false;
    disconnect(channelEditer,SIGNAL(report_cycle_Edited()),this,SLOT(cycle_changed()));
}

void MainController::Section_editor_close(){
    isOpen_SectionEditor = false;
}

void MainController::Variable_monitor_close(){
    isOpen_VariableMonitor = false;
    variableExpleror->Setup_report(false);
    disconnect(variableExpleror,SIGNAL(report_variable_Edited()),this,SLOT(cycle_changed()));
}

void MainController::on_Real_Time_clicked()
{
    RealTime *a = new RealTime(0,Analog_output_channel,8,Digital_output_channel,32);
    a->show();
    /*if(connected){
        RealTime *a = new RealTime(0,Analog_output_channel,8,Digital_output_channel,32);
        a->show();
    }
    else{
        QMessageBox msgBox;
        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("ADwin System not connected.");
        msgBox.exec();
        return;
    }*/
}

void MainController::system_setting_changed(QStringList n){
    bool need_reset = false;
    if(program_setting.at(1)!=n.at(1))need_reset = true;
    if(program_setting.at(2)!=n.at(2))need_reset = true;

    program_setting = n;
    if(need_reset)on_Reset_clicked();
    if(program_setting.at(3)=="offline"){
        ui->Boot->setDisabled(true);
        ui->Process_load->setEnabled(true);
        ui->Process_start->setEnabled(true);
        ui->Scan->setEnabled(true);
        Adwin_state->S_Offline();
    }
    else{
        ui->Boot->setEnabled(true);
        ui->Process_load->setDisabled(true);
        ui->Process_start->setDisabled(true);
        ui->Scan->setDisabled(true);
        Adwin_state->S_Standby();
    }
}

void MainController::on_Emergency_stop_clicked()
{
    Stop_Process(1);
    Stop_Process(2);
    Stop_Process(3);
    system_log(5,0);
    running = false;
    scanning = false;
    connected = false;
    ui->Process_load->setDisabled(true);
    ui->Process_start->setDisabled(true);
    ui->Scan->setDisabled(true);
    cycle_check_timer->stop();
    global_timer->stop();
    Adwin_state->S_Frozen();
}

double MainController::input_to_voltage(int in){
    return double (in)/65535. * 20 - 10;
}

int MainController::voltage_to_output(double v){
    return 65535*(v+10)/20;
}

void MainController::Delay(int millisecondsToWait){
    QTime dieTime = QTime::currentTime().addMSecs(millisecondsToWait);
    while(QTime::currentTime() < dieTime){
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
    }
}

QString MainController::MD5_of_current_cycle(){
    MD5_Calculator md5;
    QString cycle_setting;
    cycle_setting.append(QString::number(Analog_output_channel)+QString::number(Digital_output_channel)+QString::number(time_resolution));
    for(int i=0; i<channel_calibration->size(); i++)cycle_setting.append(channel_calibration->at(i));
    for(int i=0; i<sections_activity->size(); i++)cycle_setting.append(sections_activity->at(i)->get_activity(number_of_scan));
    for(int i=0; i<variables->size(); i++){
        if(variables->at(i)->get_name().compare("DUMMY",Qt::CaseInsensitive)!=0){
            cycle_setting.append(variables->at(i)->get_name());
            cycle_setting.append(QString::number(variables->at(i)->get_type()));
            switch (variables->at(i)->get_type()){
            case 0:
                cycle_setting.append(QString::number(variables->at(i)->get_value()[0]));
                break;
            case 1:
                cycle_setting.append(QString::number(variables->at(i)->get_scan(number_of_scan)));
                break;
            case 2:
                cycle_setting.append(variables->at(i)->get_fomular());
                break;
            case 3:
                cycle_setting.append(variables->at(i)->get_filename()+QString::number(variables->at(i)->get_fitting()));
                break;
            default:
                break;
            }
        }
    }
    for(int i=0; i<sequences->size(); i++){
        for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
            for(int k=0; k<sequences->at(i)[j]->size(); k++){
                double *value = sequences->at(i)[j]->at(k)->get_value();
                Variable **vari = sequences->at(i)[j]->at(k)->get_variables();
                for(int v=0; v<4; v++){
                    if(vari[v]==NULL)cycle_setting.append("NULL");
                    else cycle_setting.append(vari[v]->get_name());
                }
                for(int v=0; v<5; v++)cycle_setting.append(QString::number(value[v]));
            }
        }
    }
    return QString::fromStdString(md5.getMD5(cycle_setting.toStdString()));
}

void MainController::Save_Tranfer_Data_to_File(long **transdata, long *transprodelay, int size, QString md5){
    QFile file("Cycle_data/"+md5+".data");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))return;
    QTextStream out(&file);
    for(int i=0; i<Analog_output_channel+Digital_output_channel/32; i++){
        QString chl;
        for(int j=0; j<size; j++){
            chl.append(","+QString::number(transdata[i][j]));
        }
        chl.remove(0,1);
        out << chl << endl;
    }
    QString chl;
    for(int j=0; j<size; j++){
        chl.append(","+QString::number(transprodelay[j]));
    }
    chl.remove(0,1);
    out << chl << endl;
    file.close();
}

int MainController::Load_Tranfer_Data_from_File(long **transdata, long *transprodelay, QString md5){
    //transdata = new long*[Analog_output_channel+Digital_output_channel/32];
    QFile file("Cycle_data/"+md5+".data");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))return 0;
    QTextStream in(&file);
    for(int i=0; i<Analog_output_channel+Digital_output_channel/32; i++){
        QStringList data = in.readLine().split(',',QString::SkipEmptyParts);
        transdata[i] = new long[data.size()];
        for(int j=0; j<data.size(); j++){
            transdata[i][j] = data.at(j).toLong();
        }
    }
    QStringList d = in.readLine().split(',');
    transprodelay = new long[d.size()];
    for(int j=0; j<d.size(); j++)transprodelay[j] = d.at(j).toLong();

    file.close();
    return d.size();
}

void MainController::Receive_Remote_Command(QString com){
    if(com.contains("STOP",Qt::CaseInsensitive)){
        on_Emergency_stop_clicked();
    }
    else if(com.contains("START_WITHOUT_RELOAD",Qt::CaseInsensitive)){
        remote_start = true;
        remote_load = false;
    }
    else if(com.contains("START_WITH_RELOAD",Qt::CaseInsensitive)){
        remote_load = true;
        remote_start = true;
    }
}

void MainController::RemoteServer_Log(){

}

void MainController::ScriptEngineDebug(){
    QiEngineDebug *debuger = new QiEngineDebug(0,QiEngine);
    debuger->show();
}

void MainController::program_infomation(){
    QMessageBox msgBox;
    QFont font;
    font.setBold(true);
    font.setPointSize(12);
    font.setItalic(true);
    msgBox.setFont(font);
    msgBox.setText("Experiment Controller V5.5");
    msgBox.setInformativeText("Last Updata: 2019/04/03");
    msgBox.exec();

    /*Change Log
     * Comming Next  Profile compare, Real time channel adjustment, Profile name display, Graphic zooming on editor(?), Relative Path;
     *
     * 2019/04/03    Sequence copy function in ChannelEditor. Safty move to CPU module.
     * 2018/08/29    PixelflyQE, DDSControl move to external program. Update on VariableExpleror. Section saving in ChannelEditer.
     * 2018/07/27    Adding PixelflyQE Control.
     * 2018/07/16    SubEngines in SequenceClaculator properly deleted. Fix Bug for Scan level larger than 2 (Error in Current_Step() function).
     * 2018/06/28    DDS Control disable, VariableExpleror install.
     * 2018/06/15    DDS Control, Temp version.
     * 2018/03/19    ChannelEditer moving function improve, .
     * 2018/03/02    Adding new data type "SectionActivity" to replace QString type activity.
     * 2018/02/23    Section merge, Sequence editing status, Setting auto save.
     * 2018/01/29    Select Channel for Analog input.
     * 2018/01/24    Remote command for Run and Reload control.
     * 2017/10/31    Add Scan variable "DUMMY" which would not reload during scan.
     * 2017/10/20    Upgrade to V5 version, calculation scheme change. ADwin Bin file upgrade to DAC_V3.
     * 2017/09/21    Pixelfly image accquire MSB alignment change to LSB.
     * 2017/09/05    QScriptEngine "Currernt_Step" function output restrict to Int.
     * 2017/08/20    New Sequence type "SRamp" "Stand", "Linear" sequence change name to "LRamp", Negative time.
     * 2017/08/01    Default setting for Addons, Save Digital Channel invertion.
     * 2017/07/08    Add Inverting function to Digital channel.
     * 2017/07/05    Add "PNG++" package support for saving 16bit/channel png image.
     * 2017/03/20    Transfer_Data precalculate and save.
     * 2017/03/01    Safety interlock installed.
     * 2017/02/28    Saving External_File_Transfer to ADCS. Saving Addon configuration.
     * 2017/01/11    ChannelDisplay update to CycleDisplayer.
     * 2017/01/04    Add size adjustment to VariableMonitor.
     * 2016/12/05    Add control for Sections behavior.
     * 2016/11/16    Modulize all Addon, Making Pixelfly_cam a thread process.
     * 2016/11/02    Reform Program Structure, Adding Pixelfly_Cam Controller.
     * 2016/10/04    Anding UDP Protocol to External File Transfer.
     * 2016/09/30    Trigger TCP/IP File Send at the beginning of every cycle.
     * 2016/09/29    Bug Fix for loading variables into ScriptEngine.
     * 2016/09/22    SubEngine copy for multithread calculation, change script function syntax.
     * 2016/09/15    Bug Fix for DiO Sequence in File Load
     *               ******QScriptEngine is not thread-Safe!*******
     * 2016/09/13    Rewrite Script for "Scan" and "Data File".
     * 2016/09/08    New Version V4: Impliment variable calculation ScriptEngine.
     * 2016/07/12    Bug Fix for "Channel Calibraion" function
     * 2016/07/11    Bug Fix for adding 'Section'.
     * 2016/06/29    'Mission' Function operational.
     * 2016/06/15    Adding TCP/IP Remote Switch.
     * 2016/06/08    Bug Fix for Analog Sequence Type not changable in SectionEditor.
     * 2016/06/07    Bug Fix for Deleting Variables.
     * 2016/05/19    Multiple Bug Fix.
     * 2016/05/14    Program Update to Version 3
     * 2016/05/02    Bug Fix for SectionEditer crash during adding Digital sequence.
     * 2016/04/15    Bug Fix for Channel Calibration can't be remove.
     * 2016/04/12    Remove GPIB Support. Going to replace it with TCP/IP and RS-232.
     * 2016/02/29    Data reduce calculation bug fix.
     * 2016/02/24    Upgrade ADwin bin file for T12(1.5), fix standing output.
     * 2016/02/03    Bug Fix for ChannelEditer
     * V2.2:         Adding Optotune LensController Module
     * V2.1:         Adding GPIB Support
     * V2.0:         Interface rebuild,  Changing Name of the program
    */
}
