#ifndef ACQUISITIONMONITOR_H
#define ACQUISITIONMONITOR_H

#include "ixoncamera.h"
#include <QDebug>
#include <QThread>

class AcquisitionMonitor : public QObject
{
    Q_OBJECT

public slots:
    int monitorAcqStatus(ushort watchdogTime_ms/*, iXonCamera::acquisitionMode acqMode*/)
    {
        HANDLE driverEvent;
        driverEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        SetDriverEvent(driverEvent);
        DWORD result;
        while(true)
        {
            result = WaitForSingleObject(driverEvent, watchdogTime_ms);
            ResetEvent(driverEvent);

            if(result == WAIT_FAILED)
            {
                qDebug() << "wait failed";
                emit acqError();
                break;
            }
            else if(result == WAIT_TIMEOUT)
            {
                qDebug() << "wait timed out";
                emit acqError();
                break;
            }
            else if(result == WAIT_ABANDONED)
            {
                qDebug() << "wait abandoned";
                emit acqError();
                break;
            }
            else if(result == WAIT_OBJECT_0)
            {
                if(currentAcqStatus() == DRV_IDLE)
                {
                    emit frameCaptured();
                    emit acqSuccess();
                    break;
                }
                else if(currentAcqStatus() == DRV_ACQUIRING)
                {
                    emit frameCaptured();

//                    long imgfs, imgls;
//                    GetNumberNewImages(&imgfs, &imgls);
//                    qDebug() << "valid image range" << imgfs << imgls;
                }
                else break;
            }
        }
        SetDriverEvent(NULL);
        CloseHandle(driverEvent);
        return result;
    }

    int currentAcqStatus()
    {
        int status;
        int err_check = GetStatus(&status);
        if(err_check != DRV_SUCCESS)
        {
            qDebug() << "warining: getstatus error" << err_check;
            return err_check;
        }

        return status;
    }

signals:
    void acqSuccess();
    void acqError();
    void frameCaptured();

};



#endif // ACQUISITIONMONITOR_H
