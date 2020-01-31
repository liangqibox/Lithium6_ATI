#ifndef OFFSETLOCK_FREQUENCYCOUNTER_H
#define OFFSETLOCK_FREQUENCYCOUNTER_H

#include <QWidget>
#include <QUdpSocket>
#include <QTimer>
#include <QScriptEngine>
#include <QScriptValue>
#include <QThread>
#include <QDebug>
#include "include/dsg_signalgenerator.h"
#include "include/controllerdatagroup.h"

namespace Ui {
class OffsetLock_FrequencyCounter;
}

class OffsetLock_FrequencyCounter : public QWidget
{
    Q_OBJECT

signals:
    RampGeneratorFrequency(double);

public:
    explicit OffsetLock_FrequencyCounter(QWidget *parent = 0, ControllerDataGroup *dataGroup = NULL);
    ~OffsetLock_FrequencyCounter();

public slots:
    void newCycle(QString msg);
    void reloadSetting(ControllerDataGroup *dataGroup);

private slots:
    void readUdpMessage();
    void checkConnection();

private:
    Ui::OffsetLock_FrequencyCounter *ui;

    QTimer *Check;
    QScriptEngine *QiEngine;
    int counter;
    QUdpSocket *udpSocket;
    DSG_SignalGenerator *signalGenerator;
    bool oscillatorConnected;
};

#endif // OFFSETLOCK_FREQUENCYCOUNTER_H
