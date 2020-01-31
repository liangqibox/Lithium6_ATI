#ifndef OVERHEATMONITOR_H
#define OVERHEATMONITOR_H

#include "ATMCD32D.H"
#include <QDebug>


class OverheatMonitor : public QObject
{
    Q_OBJECT

public:
    OverheatMonitor(HANDLE qE)
    {
        quitEvent = qE;
    }

public slots:
    void monitorOverheating()
    {
        HANDLE TECEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        HANDLE hdl[2] = {TECEvent, quitEvent};
        SetTECEvent(TECEvent);

        while(true)
        {
            int result = WaitForMultipleObjects(2, hdl, false, INFINITE);
            ResetEvent(TECEvent);
            if(result == WAIT_OBJECT_0)
            {
                int status;
                int err_check = GetTECStatus(&status);
                if(status == 1 || err_check != DRV_SUCCESS) emit overheatWarning(); //emits the warning if status cannont be obtained as well
            }
            else
            {
                break;
            }
        }
        SetTECEvent(NULL);
        CloseHandle(TECEvent);
        return;
    }

signals:
    void overheatWarning();

private:
    HANDLE quitEvent;



};


#endif // OVERHEATMONITOR_H
