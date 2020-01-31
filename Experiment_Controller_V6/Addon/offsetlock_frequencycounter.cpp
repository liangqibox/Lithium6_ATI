#include "offsetlock_frequencycounter.h"
#include "ui_offsetlock_frequencycounter.h"

const QHostAddress hostIP = QHostAddress("192.168.0.4");

OffsetLock_FrequencyCounter::OffsetLock_FrequencyCounter(QWidget *parent, ControllerDataGroup *dataGroup) :
    QWidget(parent),
    ui(new Ui::OffsetLock_FrequencyCounter)
{
    ui->setupUi(this);

    QPixmap state = QPixmap(ui->Connection_Indicator_Counter->size());
    state.fill(Qt::lightGray);
    ui->Connection_Indicator_Counter->setPixmap(state);
    ui->Connection_Indicator_Oscillator->setPixmap(state);

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(hostIP,8005);
    connect(udpSocket,SIGNAL(readyRead()),this,SLOT(readUdpMessage()));
    QiEngine = dataGroup->QiEngine;

    oscillatorConnected = false;
    signalGenerator = new DSG_SignalGenerator(NULL);
    QThread *signalGeneratorThread = new QThread;
    signalGenerator->moveToThread(signalGeneratorThread);
    connect(this,SIGNAL(RampGeneratorFrequency(double)),signalGenerator,SLOT(rampToFrequency(double)));
    connect(signalGenerator,SIGNAL(reportFrequency(double)),ui->Display_Oscillator,SLOT(display(double)));
    signalGeneratorThread->start();
    if(signalGenerator->HandShake()){
        QPixmap state1 = QPixmap(ui->Connection_Indicator_Counter->size());
        state1.fill(Qt::darkGreen);
        ui->Connection_Indicator_Oscillator->setPixmap(state1);
        oscillatorConnected = true;
    }

    Check = new QTimer(this);
    counter = 0;
    connect(Check,SIGNAL(timeout()),this,SLOT(checkConnection()));
    Check->start(1000);
}

OffsetLock_FrequencyCounter::~OffsetLock_FrequencyCounter()
{
    udpSocket->disconnect();
    udpSocket->disconnectFromHost();
    delete ui;
}

void OffsetLock_FrequencyCounter::reloadSetting(ControllerDataGroup *dataGroup){
    QiEngine = dataGroup->QiEngine;
}

void OffsetLock_FrequencyCounter::newCycle(QString msg){
    double feq = QiEngine->evaluate("OffsetLoackOscillatorFeq("+msg.split(":").at(2)+")").toNumber();
    emit RampGeneratorFrequency(feq);
}

void OffsetLock_FrequencyCounter::readUdpMessage(){
    while(udpSocket->hasPendingDatagrams()){
        QByteArray buffer;
        buffer.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(buffer.data(),buffer.size());
        QString Frequency = QString::fromLatin1(buffer);
        ui->Display_Counter->display(Frequency.toDouble());
        QScriptValue feq = QiEngine->evaluate(Frequency);
        QiEngine->globalObject().setProperty("offsetLock_Frequency",feq);
        counter = 0;
    }
    //if(oscillatorConnected)ui->Display_Oscillator->display(signalGenerator->);
}

void OffsetLock_FrequencyCounter::checkConnection(){
    if(counter>2){
        QPixmap state = QPixmap(ui->Connection_Indicator_Counter->size());
        state.fill(Qt::lightGray);
        ui->Connection_Indicator_Counter->setPixmap(state);
    }
    else{
        QPixmap state = QPixmap(ui->Connection_Indicator_Counter->size());
        state.fill(Qt::darkGreen);
        ui->Connection_Indicator_Counter->setPixmap(state);
    }
    counter++;
}
