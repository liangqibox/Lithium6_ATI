#ifndef PIXELFLYQE_CAM_H
#define PIXELFLYQE_CAM_H

#include <QObject>
#include <QPixmap>
#include <QImage>
#include <QTime>
#include <QList>
#include <windows.h>
//#include "include/PCO_PCI540/Pccam.h"
#include "include/PCO_PCI540/PfcamExport.h"
#include "include/PCO_PCI540/pccamdef.h"

class PixelflyQE_Cam : public QObject
{
    Q_OBJECT
public:
    explicit PixelflyQE_Cam(QObject *parent = 0);
    ~PixelflyQE_Cam();

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
    int Start_Recording(int cammode);
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
    int CamNum;
    int TriggerMode;
    int CameraMode;

    int ExposureTime;
    int BinH;
    int BinV;
    int Gain;
    int XRes;
    int YRes;
    int BitPerPixel;

    QList<uint*> wBuf;
    QList<int> sBufNr;

    int BUFFER_MAXIMUM;
    int bufferCounter;
};

#endif // PIXELFLYQE_CAM_H
