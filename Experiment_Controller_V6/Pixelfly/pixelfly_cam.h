#ifndef PIXELFLY_CAM_H
#define PIXELFLY_CAM_H

#include <QObject>
#include <QPixmap>
#include <QImage>
#include <QTime>
#include <QList>
#include <windows.h>
#include "include/PCO/SC2_CamExport.h"
#include "include/PCO/sc2_defs.h"
#include "include/PCO/sc2_SDKStructures.h"
#include "include/PCO/PCO_err.h"
#include "include/PCO/Pco_ConvExport.h"
#include "include/PCO/SC2_DialogExport.h"

class Pixelfly_Cam : public QObject
{
    Q_OBJECT
public:
    explicit Pixelfly_Cam(QObject *parent = 0);
    ~Pixelfly_Cam();

signals:
    void Image_receive(QImage);
    void Image_data_receive(QList<int>);
    void Wait_time_out();

public slots:
    int Open_Camera();
    int Close_Camera();
    int Set_Exposure(int usec);
    int Set_Trigger(int mode);
    int Allocate_Buffer();
    int Start_Recording(int trigger);
    int Stop_Recording();
    int Wait_for_Image(int msec);
    int Force_Trigger();
    QImage Read_Image();
    int Free_Buffer();

    int Set_Buffer_Number(int n);

private:
    QList<QImage> images;
    QList<int> images_data;
    HANDLE hCam;
    QList<HANDLE> BufEvent;
    HANDLE hBWConv;
    WORD wCamNum;
    WORD wTriggerMode;
    WORD ROIx;
    WORD ROIy;
    WORD wBinH;
    WORD wBinV;

    DWORD dwDelay;
    DWORD dwExposure;
    WORD wTimeBaseDelay;
    WORD wTimeBaseExposure;

    WORD XRes;
    WORD YRes;
    WORD XResM;
    WORD YResM;
    WORD wBitPerPixel;
    DWORD dwSize;

    QList<WORD*> wBuf;
    QList<SHORT> sBufNr;

    PCO_General strGeneral;
    PCO_CameraType strCamType;
    PCO_Sensor strSensor;
    PCO_Description strDescription;
    PCO_Timing strTiming;
    PCO_Storage strStorage;
    PCO_Recording strRecording;

    DWORD dwWarn;
    DWORD dwError;
    DWORD dwStatus;

    int BUFFER_MAXIMUM;
    int bufferCounter;
};

#endif // PIXELFLY_CAM_H
