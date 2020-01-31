#ifndef CAMERACONTROL_LUCAM_H
#define CAMERACONTROL_LUCAM_H

#include <QWidget>
#include <QDialog>
#include <QFileDialog>
#include <QPixmap>
#include <QImage>
#include <QTimer>
#include <QDateTime>
#include <QThread>
#include "windows.h"
#include "include/Lucam/lucamapi.h"
#include "include/Lucam/callbacktrigger.h"
#include "include/image_save.h"

void __stdcall SnapshotCallback(VOID *pContext, BYTE *pData, ULONG dataLength);

namespace Ui {
class CameraControl_LuCam;
}

class CameraControl_LuCam : public QWidget
{
    Q_OBJECT

public:
    explicit CameraControl_LuCam(QWidget *parent = 0);
    ~CameraControl_LuCam();

public slots:
    void Receive_New_Cycle_Trigger(QStringList msg);

signals:
    void Save_image(QString);

private slots:
    void on_OpenCam_clicked();
    void on_CloseCam_clicked();
    void on_PreView_clicked();
    void PreView_Close();

    void on_Video_Adjust_clicked();
    void on_Image_Adjust_clicked();

    void on_Start_clicked();
    void on_VideoMode_SoftwareStart_clicked();
    void VideoMode_Update();
    void on_ImageMode_SoftwaveStart_clicked();
    void Receive_Callback_Image(QImage img);

    void on_Images_SavePath_clicked();

    void on_Edit_xOffset_editingFinished();
    void on_Edit_yOffset_editingFinished();
    void on_Edit_Width_editingFinished();
    void on_Edit_Height_editingFinished();

private:
    Ui::CameraControl_LuCam *ui;
    HANDLE Camera;
    HWND PreviewWindow;

    BYTE *Data_Raw;
    LUCAM_CONVERSION *Conversion_Struct;
    LUCAM_FRAME_FORMAT *FrameFormat_Struct;
    LUCAM_SNAPSHOT *Snapshot_Struct;
    int Pixel_Width,Pixel_Height,Offset_X,Offset_Y;
    bool connected;
    bool VideoMode,ImageMode;
    bool softwareControl;

    QTimer *VideoMode_Timer;    //ms
    QThread *ImageSaveThread;
    Image_Save *imageSave;
    CallbackTrigger *callbackTrigger;
    int VideoMode_Period;    //ms
    int VideoMode_progressCounter;
    int MOT_Load_Time;
    int Image_Taken;
    QString filePath;
    QDateTime Image_Taken_Time;
};

#endif // CAMERACONTROL_LUCAM_H
