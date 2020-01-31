#ifndef CAMERACONTROLQE_H
#define CAMERACONTROLQE_H

#include <QWidget>
#include <QThread>
#include <QFileDialog>
#include <QImage>
#include <QTimer>
#include <QDateTime>
#include "camera_screen.h"
#include "pixelfly_cam.h"
#include "pixelflyqe_cam.h"
#include "include/PCO_PCI540/pccamdef.h"
#include "include/image_save.h"
#include "include/png++/png.hpp"

namespace Ui {
class CameraControlQE;
}

class CameraControlQE : public QWidget
{
    Q_OBJECT

public:
    explicit CameraControlQE(QWidget *parent = 0);
    ~CameraControlQE();

signals:
    void start_acquire(int);
    void camera_force_trigger();
    void set_trigger(int);
    void set_exposure(int);
    void recording_start();
    void recording_stop();
    void save_image(QString);

public slots:
    void Reload(QString msg);

private slots:
    void Load_default_setting();
    void on_Preview_clicked();
    void update_preview();
    void preview_close();

    void on_Connect_clicked();
    void on_Reset_clicked();
    void on_Arm_clicked();

    void on_Path_clicked();
    void on_Exposure_returnPressed();

    void acquire_next_image();
    void save_images(int i);

    void on_Trigger_clicked();
    void image_time_out();
    void receive_image(QList<int> img_dat);

private:
    Ui::CameraControlQE *ui;

    int milisecondtowait;
    int number_of_run;
    int number_of_scan;
    bool isArm;
    QList<QList<int> > images_data;
    Camera_Screen *preview;
    QThread *Cam_thread;
    //Pixelfly_Cam *cam;
    PixelflyQE_Cam *cam;
    Image_Save *Image_Saver;
};

#endif // CAMERACONTROLQE_H
