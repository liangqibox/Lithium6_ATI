#ifndef TEMPERATURECONTROLLER_H
#define TEMPERATURECONTROLLER_H
#include <QObject>
#include <QTimer>

#include "ATMCD32D.H"
#include "ixoncamera.h"
#include "overheatmonitor.h"

class OverheatMonitor;
class TemperatureController : public QObject
{
    Q_OBJECT

public:
    TemperatureController();
    ~TemperatureController();

    typedef enum {FAN_HIGH = 0, FAN_LOW = 1, FAN_OFF = 2} fanSpeed;

    inline bool isMonitoring() {return m_isMonitoring;}
    inline int maxTemp() {return m_maxTemp;}
    inline int minTemp() {return m_minTemp;}
    inline int targetTemp() {return m_targetTemp;}
    inline bool coolerEnabled() {return m_coolerEnabled;}
    inline bool coolerAfterShutdownEnabled() {return m_coolerAfterShutdownEnabled;}

    inline OverheatMonitor *TECMonitor() {return m_TECMonitor;}


private:
    QTimer *timer;
    bool m_isMonitoring;
    OverheatMonitor *m_TECMonitor;
    HANDLE m_quitEvent;


    fanSpeed curFanSpeed;

    int m_maxTemp;
    int m_minTemp;
    int m_targetTemp;
    bool m_coolerEnabled;
    bool m_coolerAfterShutdownEnabled;


public slots:
    int setTemperature(int temp);
    int setCooler(bool enabled);
    int setCoolerAfterShutdown(bool enabled);
    int currentTemperature(int *status);
    int getTempRange();
    void startTempMonitor(int interval_ms);
    void stopTempMonitor();
    void monitorTemp();

    int setFanSpeed(int speed);

    void initTemperatureController();

    void setOverheatQuitEvent();

signals:
    void targetTempChanged(int temp);
    void curTempChanged(int temp, int status);
    void tempRangeChanged(int min, int max);
    void coolerEnabledChanged(bool enabled);
    void coolerAfterShutdownEnabledChanged(bool enabled);
    void temperatureError(int status);

    void fanSpeedChanged(int speed);

    void startOverheatMonitor();

};

#endif // TEMPERATURECONTROLLER_H
