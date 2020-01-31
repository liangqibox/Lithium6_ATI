#ifndef DDSCONTROLLER_H
#define DDSCONTROLLER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QSerialPort>
#include <QBuffer>
#include <stdint-gcc.h>

#define DDS_ERROR_TIMEOUT       0x01
#define DDS_ERROR_INVALID_CRC   0x02

uint16_t crc(QByteArray data);

class DDSCycle {

public:
    DDSCycle();

    void reset();
    void trigger();
    void wait_for_trigger();

    void singleTone(float freq);
    void setAmplitude(float ampl);
    void setPhase(float phase);

    void linearSweep(float fromFreq, float toFreq, float duration); //Hz,Hz,s
    void linearSweep(float fromFreq, float toFreq, float duration, bool nodwell, float timestep = 0);
    void shortPulse(float freq, float duration);
    void resetSweep();

    QByteArray getCycleData();

private:
    QBuffer buffer;
};


class DDSController : public QObject
{
    Q_OBJECT

public:
    explicit DDSController(QString port, QObject *parent = 0);
    ~DDSController();

    static QList<QString> availablePorts();
    bool isOpen();
    void Close();

    uint8_t program_cycle(DDSCycle & cycle);
    uint8_t start_cycle();
    uint8_t stop_cycle();
    uint8_t send_trigger();

signals:
    void statusReceived(QString message);

private slots:
    void onDataReady();

private:
    QSerialPort serial;
    QString msg;
    bool opened;
};

#endif // DDSCONTROLLER_H
