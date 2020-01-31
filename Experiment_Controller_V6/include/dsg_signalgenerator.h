#ifndef DSG_SIGNALGENERATOR_H
#define DSG_SIGNALGENERATOR_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include "include/IVI/visa.h"
#include "include/IVI/visatype.h"
#include "cstring"

class DSG_SignalGenerator : public QObject
{
    Q_OBJECT

public:
    explicit DSG_SignalGenerator(QObject *parent = 0);
    ~DSG_SignalGenerator();

public slots:
    bool HandShake();
    void changeFrequency(double feq);
    void rampToFrequency(double tofeq); //MHz

signals:
    reportFrequency(double);

private:
    ViSession defaultRM,instr;
    ViStatus status;
    ViUInt32 retCount;
};

#endif // DSG_SIGNALGENERATOR_H
