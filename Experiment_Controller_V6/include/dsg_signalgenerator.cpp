#include "dsg_signalgenerator.h"

char SendAddr[] = "TCPIP::192.168.0.12::INSTR";
char feqQuary[] = ":FREQ?";
const double frequencyStep = 2;  //2MHz
const uint dwell = 2;       //2ms

DSG_SignalGenerator::DSG_SignalGenerator(QObject *parent) :
    QObject(parent)
{
}

DSG_SignalGenerator::~DSG_SignalGenerator()
{
    status = viClose(instr);
    status = viClose(defaultRM);
}

bool DSG_SignalGenerator::HandShake(){
    status = viOpenDefaultRM(&defaultRM);
    if (status < VI_SUCCESS){
        qDebug() <<"No VISA instrument was opened !";
        return false;
    }
    status = viOpen(defaultRM, SendAddr, VI_NULL, VI_NULL, &instr);
    if (status < VI_SUCCESS)return false;
    qDebug() << "OffsetLock SignalGenerator Connected";
    status = viClose(instr);
    return true;
}

void DSG_SignalGenerator::changeFrequency(double feq){
    status = viOpen(defaultRM, SendAddr, VI_NULL, VI_NULL, &instr);
    QString msg = ":FREQ "+QString::number(feq);
    char *send = msg.toLatin1().data();
    status = viWrite(instr,(unsigned char *)send,strlen(send),&retCount);
    status = viClose(instr);
}

void DSG_SignalGenerator::rampToFrequency(double tofeq){
    unsigned char RecBuf[256];
    memset(RecBuf,0,256);
    status = viOpen(defaultRM, SendAddr, VI_NULL, VI_NULL, &instr);
    status = viWrite(instr,(unsigned char *)feqQuary,strlen(feqQuary),&retCount);
    status = viRead(instr, RecBuf, 256, &retCount);
    QString result;
    for(uint i=0; i<retCount; i++)result.append(RecBuf[i]);
    double startFeq = result.split('.').at(0).toDouble();
    tofeq = tofeq * 1000000;
    double Step = frequencyStep*1000000;

    int totalStep = abs(tofeq-startFeq)/Step;
    if(tofeq<startFeq)Step = -1*Step;
    for(int i=0; i<totalStep; i++){
        QString msg = ":FREQ "+QString::number(startFeq+Step*i);
        char *send = msg.toLatin1().data();
        status = viWrite(instr,(unsigned char *)send,strlen(send),&retCount);
        QThread::msleep(dwell);
    }
    QString msg = ":FREQ "+QString::number(tofeq);
    char *send = msg.toLatin1().data();
    status = viWrite(instr,(unsigned char *)send,strlen(send),&retCount);

    status = viWrite(instr,(unsigned char *)feqQuary,strlen(feqQuary),&retCount);
    status = viRead(instr, RecBuf, 256, &retCount);
    QString reportS;
    for(uint i=0; i<retCount; i++)reportS.append(RecBuf[i]);
    double report = result.split('.').at(0).toDouble()/1000000;
    emit reportFrequency(report);

    status = viClose(instr);
}
