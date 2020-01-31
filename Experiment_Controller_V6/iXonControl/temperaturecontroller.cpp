#include "temperaturecontroller.h"

#define MONITORINTERVAL 2000

TemperatureController::TemperatureController() : QObject(0)
{
    timer = NULL;
    m_isMonitoring = false;
    m_quitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_TECMonitor = new OverheatMonitor(m_quitEvent);
    connect(this, SIGNAL(startOverheatMonitor()), m_TECMonitor, SLOT(monitorOverheating()));
    connect(QThread::currentThread(), SIGNAL(finished()), this, SLOT(setOverheatQuitEvent()));
}

TemperatureController::~TemperatureController()
{
    if(timer != NULL) timer->deleteLater();
    setOverheatQuitEvent();
    CloseHandle(m_quitEvent);
    setCooler(false);
}

void TemperatureController::initTemperatureController()
{
    startTempMonitor(MONITORINTERVAL);
    setCooler(false);
    setTemperature(20);

    return;
}

void TemperatureController::setOverheatQuitEvent()
{
    SetEvent(m_quitEvent);
    return;
}

int TemperatureController::getTempRange()
{
    int minTemp, maxTemp;

    int err_check = GetTemperatureRange(&minTemp, &maxTemp);
    if(err_check == DRV_SUCCESS)
    {
        m_minTemp = minTemp;
        m_maxTemp = maxTemp;
    }
    emit tempRangeChanged(m_minTemp, m_maxTemp);

    return err_check;
}

int TemperatureController::setTemperature(int temp)
{
    getTempRange();
    if(temp > m_maxTemp) temp = m_maxTemp;
    else if(temp < m_minTemp) temp = m_minTemp;

    int err_check = SetTemperature(temp);
    if(err_check == DRV_SUCCESS) m_targetTemp = temp;
    emit targetTempChanged(m_targetTemp);

    return err_check;
}

int TemperatureController::setCooler(bool enabled)
{
    int err_check;

    if(enabled) err_check = CoolerON();
    else err_check = CoolerOFF();

    if(err_check == DRV_SUCCESS) m_coolerEnabled = enabled;
    emit coolerEnabledChanged(m_coolerEnabled);
    return err_check;
}

int TemperatureController::setCoolerAfterShutdown(bool enabled)
{
    int err_check = SetCoolerMode(enabled);
    if(err_check == DRV_SUCCESS) m_coolerAfterShutdownEnabled = enabled;

    emit coolerAfterShutdownEnabledChanged(enabled);
    return err_check;
}

int TemperatureController::currentTemperature(int *status)
{
    int temp;
    int err_check  = GetTemperature(&temp);
    /*
     * POSSIBLE STATUS:
     *
     * DRV_NOT_INITIALIZED      System not initialized.
     * DRV_ACQUIRING            Acquisition in progress.
     * DRV_ERROR_ACK            Unable to communicate with card.
     * DRV_TEMP_OFF             Temperature is OFF.
     * DRV_TEMP_STABILIZED      Temperature has stabilized at set point.
     * DRV_TEMP_NOT_REACHED     Temperature has not reached set point.
     * DRV_TEMP_DRIFT           Temperature had stabilized but has since drifted
     * DRV_TEMP_NOT_STABILIZED  Temperature reached but not stabilized
     */

    *status = err_check;

    return temp;
}

void TemperatureController::startTempMonitor(int interval_ms)
{
    if(timer == NULL)
    {
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(monitorTemp()));
    }

    timer->start(interval_ms);
    m_isMonitoring = true;
}

void TemperatureController::stopTempMonitor()
{
    if(timer != NULL)
    {
        timer->stop();
        m_isMonitoring = false;
    }
}

void TemperatureController::monitorTemp()
{
    int status;
    int temp = currentTemperature(&status);
    if(status != DRV_NOT_INITIALIZED && status != DRV_ERROR_ACK) emit curTempChanged(temp, status);
    else emit temperatureError(status);
}

int TemperatureController::setFanSpeed(int speed)
{
    if(speed > 2 || speed < 0) return DRV_P1INVALID;

    int err_check = SetFanMode(speed);
    if(err_check == DRV_SUCCESS)
    {
        curFanSpeed = static_cast<fanSpeed>(speed);
    }
    emit fanSpeedChanged(curFanSpeed);
    return err_check;
}

