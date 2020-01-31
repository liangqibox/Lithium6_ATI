#include "analogrecording.h"
#include "ui_analogrecording.h"
#include <QDebug>

const int Number_of_Analog_Input_Channels = 8;

AnalogRecording::AnalogRecording(QWidget *parent, ControllerDataGroup *dg) :
    QWidget(parent),
    ui(new Ui::AnalogRecording)
{
    ui->setupUi(this);

    AIn_Sampling_Rate = 50;
    number_of_scan = 0;
    number_of_run = 0;
    DataGroup = dg;
    vStart = NULL;
    vEnd = NULL;
    vSamplingRate = NULL;

    variables = DataGroup->variables;
    variableClass = DataGroup->variableClass;

    for(int i=0; i<Number_of_Analog_Input_Channels; i++){
        QCheckBox *checkbox = new QCheckBox(ui->Saved_channels);
        //checkbox->setGeometry(30+(i%4)*85,35+(i/4)*35,70,20);
        checkbox->setGeometry(20+(i%2)*60,35+(i/2)*35,60,20);
        checkbox->setText("AIn"+QString::number(i+1));
        checkbox->show();
        checkbox->setChecked(true);
        Channels_active.append(checkbox);
    }

    ui->Time_Start->installEventFilter(this);
    ui->Time_End->installEventFilter(this);
    ui->Ain_samlpling_rate->installEventFilter(this);
    status_check = new QTimer;
    connect(status_check,SIGNAL(timeout()),this,SLOT(Status_Check()));
}

AnalogRecording::~AnalogRecording()
{
    status_check->deleteLater();
    delete ui;
}

bool AnalogRecording::eventFilter(QObject *obj, QEvent *event){
    if(obj==ui->Time_Start || obj==ui->Time_End || obj==ui->Ain_samlpling_rate){
        QMouseEvent *a = static_cast<QMouseEvent*>(event);
        if(event->type()==QEvent::MouseButtonPress && a->button()==Qt::RightButton){
            QLineEdit *lineEdit = static_cast<QLineEdit*>(obj);
            show_variable_menu(lineEdit);
            return true;
        }
        return false;
    }
    return false;
}

void AnalogRecording::Reload(QString msg){
    QStringList msglist = msg.split(':');
    number_of_run = msglist.at(1).toInt();
    number_of_scan = msglist.at(2).toInt();

    if(ui->Ain_acquire_enable->isChecked()){
        long tStart = 0;
        long tEnd = 0;
        long samplingRate = 100;
        if(vStart!=NULL)tStart = vStart->get_output(number_of_scan,0,0,0,0);
        else tStart = ui->Time_Start->text().toDouble();
        if(vEnd!=NULL)tEnd = vEnd->get_output(number_of_scan,0,0,0,0);
        else tEnd = ui->Time_End->text().toDouble();
        if(vSamplingRate!=NULL)samplingRate = vSamplingRate->get_output(number_of_scan,0,0,0,0);
        else samplingRate = ui->Ain_samlpling_rate->text().toInt();

        long length = (tEnd-tStart)*1000/samplingRate;
        if(length>ANALOG_RECORD_MAXIMUM)length = ANALOG_RECORD_MAXIMUM;
        Set_Processdelay(2,1000000);
        Set_Par(ANALOGINPUT_START,tStart);
        Set_Par(ANALOGINPUT_LENGTH,length);
        Set_Par(ANALOGINTPUT_SAMPLING_RATE,samplingRate);
        Set_Par(ANALOGINPUT_SWITCH,1);
        status_check->start(100);
    }
}

void AnalogRecording::Controller_Setting_Load(ControllerDataGroup *){
    AIn_Sampling_Rate = 50;
    number_of_scan = 0;
    number_of_run = 0;
    vStart = NULL;
    vEnd = NULL;
    vSamplingRate = NULL;
    ui->Ain_samlpling_rate->setEnabled(true);
    ui->Time_Start->setEnabled(true);
    ui->Time_End->setEnabled(true);
    ui->Ain_samlpling_rate->setText("50");
    ui->Time_Start->setText("0");
    ui->Time_End->setText("0");
    variables = DataGroup->variables;
    variableClass = DataGroup->variableClass;
}

void AnalogRecording::Status_Check(){
    int swich = Get_Par(ANALOGINPUT_SWITCH);
    if(swich == 0){
        status_check->stop();
        accquire_Ain_data(Get_Par(ANALOGINPUT_LENGTH));
    }
}

void AnalogRecording::on_Ain_acquire_enable_clicked()
{
}


void AnalogRecording::on_Ain_file_path_button_clicked()
{
    QString folder =  QFileDialog::getExistingDirectory(this,"Select save folder");
    if(folder.isEmpty()){
        ui->Ain_file_path->setText("None");
        return;
    }
    else ui->Ain_file_path->setText(folder);
}

void AnalogRecording::on_Ain_samlpling_rate_editingFinished()
{
    bool ok;
    QString temp = ui->Ain_samlpling_rate->text();
    double temp_number = temp.toInt(&ok,10);
    if (!ok&&vSamplingRate==NULL){
        ui->Ain_samlpling_rate->setText(QString::number(AIn_Sampling_Rate));
        QMessageBox msgBox;
        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Invaild time resolution.");
        msgBox.exec();
    }
    else if(temp_number<0.01){
        ui->Ain_samlpling_rate->setText(QString::number(AIn_Sampling_Rate));
        QMessageBox msgBox;
        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Time resolution too high.");
        msgBox.exec();
    }
    else AIn_Sampling_Rate = temp_number;
}

void AnalogRecording::on_Time_Start_editingFinished()
{
    bool ok;
    double temp_number = ui->Time_Start->text().toDouble(&ok);
    if(!ok&&vStart==NULL){
        ui->Time_Start->setText("0");
        QMessageBox msgBox;
        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Invaild starting time.");
        msgBox.exec();
    }
    else Start = temp_number;
}

void AnalogRecording::on_Time_End_editingFinished()
{
    bool ok;
    double temp_number = ui->Time_End->text().toDouble(&ok);
    if(!ok&&vEnd==NULL){
        ui->Time_Start->setText("0");
        QMessageBox msgBox;
        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Invaild ending time.");
        msgBox.exec();
    }
    else End = temp_number;
}

void AnalogRecording::save_Ain_data(int length){
    //if(AIn_Data[0].isEmpty())return;

    QString path = ui->Ain_file_path->text();
    QString filename = ui->Ain_file_description->text();
    if(path=="None")path.clear();
    filename.replace("[scan]",QString::number(number_of_scan));
    filename.replace("[run]",QString::number(number_of_run));
    filename.replace("YYYY",QDateTime::currentDateTime().toString("yyyy"));
    filename.replace("MM",QDateTime::currentDateTime().toString("MM"));
    filename.replace("DD",QDateTime::currentDateTime().toString("dd"));
    filename.replace("hh",QDateTime::currentDateTime().toString("HH"));
    filename.replace("mm",QDateTime::currentDateTime().toString("mm"));
    filename.replace("ss",QDateTime::currentDateTime().toString("ss"));
    for(int i=0; i<variables->size(); i++)
        filename.replace("["+variables->at(i)->get_name()+"]",QString::number(variables->at(i)->get_output(number_of_scan,0,0,0,0)));
    QFile file(path+"/"+filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))return;

    bool Channels[Number_of_Analog_Input_Channels];
    for(int i=0; i<Number_of_Analog_Input_Channels; i++)Channels[i] = Channels_active.at(i)->isChecked();

    QTextStream out(&file);
    out << "Time(ms)";
    for(int i=0; i<Number_of_Analog_Input_Channels; i++){
        if(Channels[i])out << ",AIn" << QString::number(i+1);
    }
    out << endl;
    for(int i=0; i<length; i++){
        out << QString::number(double(Start)+double(AIn_Sampling_Rate)/1000.*i,'g',10);
        for(int chl=0; chl<Number_of_Analog_Input_Channels; chl++){
            if(Channels[chl])out << "," << QString::number(data_to_voltage(Data[i*8+chl]));
        }
        out << endl;
    }
    file.close();
    delete Data;
    qDebug() << "Analog input recorded: " << filename;
    //for(int i=0; i<8; i++)AIn_Data[i].clear();
}

void AnalogRecording::accquire_Ain_data(int length){
    qDebug() << "Analog recording finish, Data length: " << length;
    //long Data[length*8];
    Data = new long[length*8];
    GetData_Long(ANALOG_RECORD_ARRAY,Data,1,length*8);
    /*for(int i=0; i<length; i++){
        for(int chl=0; chl<8; chl++){
            AIn_Data[chl].append(data_to_voltage(Data[i*8+chl]));
        }
    }*/
    qDebug() << "Extration completed.";
    save_Ain_data(length);
}

void AnalogRecording::next_recording(QString msg){
    QStringList msglist = msg.split(':');
    number_of_run = msglist.at(1).toInt();
    number_of_scan = msglist.at(2).toInt();
}

double AnalogRecording::data_to_voltage(int in){
    return double (in)/65535. * 20 - 10;
}


void AnalogRecording::show_variable_menu(QLineEdit *lineEdit){
    QMenu *menu = new QMenu(this);
    int index = 0;
    Variable *used = NULL;
    if(lineEdit == ui->Ain_samlpling_rate){
        index = 0;
        used = vSamplingRate;
    }
    else if(lineEdit == ui->Time_Start){
        index = 1;
        used = vStart;
    }
    else if(lineEdit == ui->Time_End){
        index = 2;
        used = vEnd;
    }

    if(used!=NULL){
        QAction *action = new QAction(menu);
        QSignalMapper *signalmap = new QSignalMapper(menu);
        action->setText("Delete Variable");
        menu->addAction(action);
        connect(action,SIGNAL(triggered()),signalmap,SLOT(map()));
        signalmap->setMapping(action,QString::number(index)+":only_system_used_null");
        connect(signalmap,SIGNAL(mapped(QString)),this,SLOT(set_variable(QString)));
        menu->addSeparator();
    }
    VariableClass *vc = variableClass;
    QMenu *m = menu;
    do{
        for(int j=0; j<vc->get_VariableSize(); j++){
            QAction *action = new QAction(vc->get_Variable(j)->get_name(),m);
            QSignalMapper *signalmap = new QSignalMapper(m);
            if(vc->get_Variable(j)==used){
                action->setCheckable(true);
                action->setChecked(true);
            }
            m->addAction(action);
            connect(action,SIGNAL(triggered()),signalmap,SLOT(map()));
            QString msg = QString::number(index)+":"+vc->get_Variable(j)->get_name();
            signalmap->setMapping(action,msg);
            connect(signalmap,SIGNAL(mapped(QString)),this,SLOT(set_variable(QString)));
        }
        if(vc->get_VariableSize()==0){
            QAction *empty = new QAction(m);
            empty->setText("Empty Class");
            empty->setDisabled(true);
            m->addAction(empty);
        }
        m->addSeparator();
        if(vc->childCount()>0){
            vc = (VariableClass*)vc->child(0);
            QMenu *temp = new QMenu(vc->text(0),m);
            m->addMenu(temp);
            m = temp;
        }
        else if(vc!=variableClass){
            if((vc->parent()->indexOfChild(vc)+1)<vc->parent()->childCount()){
                vc = (VariableClass*)vc->parent()->child(vc->parent()->indexOfChild(vc)+1);
                QMenu *upper = (QMenu*)m->parent();
                QMenu *temp = new QMenu(vc->text(0),upper);
                upper->addMenu(temp);
                m = temp;
                }
            else{
                while((vc->parent()->indexOfChild(vc)+1)==vc->parent()->childCount()){
                    vc = static_cast<VariableClass*>(vc->parent());
                    m = (QMenu*)m->parent();
                    if(vc==variableClass)break;
                }
                if(vc!=variableClass){
                    vc = (VariableClass*)vc->parent()->child(vc->parent()->indexOfChild(vc)+1);
                    QMenu *upper = (QMenu*)m->parent();
                    QMenu *temp = new QMenu(vc->text(0),upper);
                    upper->addMenu(temp);
                    m = temp;
                }
            }
        }
    }while(vc!=variableClass);
    menu->popup(QCursor::pos());
}

void AnalogRecording::set_variable(QString msg){
    QStringList temp_msg = msg.split(':');
    int index = temp_msg.at(0).toInt();
    QString name = temp_msg.at(1);
    QLineEdit *lineEdit = NULL;
    int value = 0;
    switch (index) {
    case 0:
        lineEdit = ui->Ain_samlpling_rate;
        value = AIn_Sampling_Rate;
        break;
    case 1:
        lineEdit = ui->Time_Start;
        value = Start;
        break;
    case 2:
        lineEdit = ui->Time_End;
        value = End;
        break;
    default:
        break;
    }

    if(name!="only_system_used_null"){
        for(int i=0; i<variables->size(); i++){
            if(name==variables->at(i)->get_name()){
                if(index==0)vSamplingRate = variables->at(i);
                else if(index==1)vStart = variables->at(i);
                else if(index==2)vEnd = variables->at(i);
            }
            lineEdit->setText(name);
        }
    }
    else{
        if(index==0)vSamplingRate = NULL;
        else if(index==1)vStart = NULL;
        else if(index==2)vEnd = NULL;
        lineEdit->setText(QString::number(value));
    }
    if(lineEdit->text()!=name)lineEdit->setEnabled(true);
    else lineEdit->setDisabled(true);
}
