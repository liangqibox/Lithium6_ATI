#include "cameracontrolqe.h"
#include "ui_cameracontrolqe.h"
#include <QDebug>

CameraControlQE::CameraControlQE(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CameraControlQE)
{
    //cam = new Pixelfly_Cam;
    cam = new PixelflyQE_Cam;
    Cam_thread = new QThread;
    cam->moveToThread(Cam_thread);
    Cam_thread->start(QThread::NormalPriority);
    connect(this,SIGNAL(start_acquire(int)),cam,SLOT(Wait_for_Image(int)));
    connect(this,SIGNAL(camera_force_trigger()),cam,SLOT(Force_Trigger()));
    connect(this,SIGNAL(set_trigger(int)),cam,SLOT(Set_Trigger(int)));
    connect(this,SIGNAL(set_exposure(int)),cam,SLOT(Set_Exposure(int)));

    Image_Saver = new Image_Save;
    QThread *Saver_thread = new QThread;
    Image_Saver->moveToThread(Saver_thread);
    Saver_thread->start(QThread::NormalPriority);
    connect(this,SIGNAL(save_image(QString)),Image_Saver,SLOT(Save_image_16bit(QString)));

    ui->setupUi(this);

    milisecondtowait = 1000;
    number_of_run = 1;
    number_of_scan = 0;
    isArm = false;
    Load_default_setting();
}

CameraControlQE::~CameraControlQE()
{
    delete ui;
}

void CameraControlQE::Load_default_setting(){
    QFile file("Default.txt");
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))return;
    QTextStream in(&file);

    while(!in.atEnd()){
        QString temp = in.readLine();
        if(temp.contains("[PixelflyQE Camera Image Number]")){
            QString a = in.readLine();
            ui->Image_Nr->setCurrentIndex(a.toInt());
        }
        else if(temp.contains("[PixelflyQE Camera Trigger Mode]")){
            QString a = in.readLine();
            if(a.contains("TTL")){
                ui->Trigger_mode->setCurrentIndex(0);
            }
            else ui->Trigger_mode->setCurrentIndex(1);
        }
        else if(temp.contains("[PixelflyQE Camera Save Path]")){
            QString a = in.readLine();
            ui->Image_Path->setText(a);
        }
        else if(temp.contains("[PixelflyQE Camera File Name]")){
            QString a = in.readLine();
            ui->Image_Name->setText(a);
        }
        else if(temp.contains("[PixelflyQE Camera Exposure Time]")){
            QString a = in.readLine();
            ui->Exposure->setText(a);
        }
    }
    file.close();
}

void CameraControlQE::on_Preview_clicked()
{
    //Doesn't work for Pixelfly QE
    /*preview = new Camera_Screen;
    QTimer *timer = new QTimer;
    preview->show();
    connect(preview,SIGNAL(rejected()),timer,SLOT(stop()));
    connect(preview,SIGNAL(rejected()),timer,SLOT(deleteLater()));
    connect(preview,SIGNAL(rejected()),this,SLOT(preview_close()));
    connect(cam,SIGNAL(Image_receive(QImage)),preview,SLOT(Update(QImage)));
    connect(timer,SIGNAL(timeout()),this,SLOT(update_preview()));
    ui->Arm->setDisabled(true);
    ui->Preview->setDisabled(true);

    cam->Stop_Recording();
    cam->Set_Buffer_Number(2);
    cam->Set_Trigger(SW_TRIGGER);
    //cam->Start_Recording(0x0000);
    cam->Start_Recording(VIDEO_MODE);
    timer->start(1000/Camera_Frame_Rate);*/
}

void CameraControlQE::update_preview(){
    const int Camera_Frame_Rate = 5;
    emit start_acquire(1000/Camera_Frame_Rate);
}

void CameraControlQE::preview_close(){
    preview->deleteLater();
    delete preview;
    cam->Stop_Recording();
    cam->Free_Buffer();
    ui->Arm->setEnabled(true);
    ui->Preview->setEnabled(true);
}

void CameraControlQE::on_Connect_clicked()
{
    if(cam->Open_Camera()==0){
        ui->Preview->setEnabled(true);
        ui->Arm->setEnabled(true);
        ui->Parameters->setEnabled(true);
    }
}

void CameraControlQE::on_Reset_clicked()
{
    cam->Close_Camera();
    cam->disconnect();
    isArm = false;
    ui->Preview->setDisabled(true);
    ui->Arm->setDisabled(true);
    ui->Parameters->setDisabled(true);
}

void CameraControlQE::on_Arm_clicked()
{
    if(isArm){
        disconnect(cam,SIGNAL(Image_data_receive(QList<int>)),this,SLOT(receive_image(QList<int>)));
        disconnect(cam,SIGNAL(Wait_time_out()),this,SLOT(image_time_out()));
        cam->Stop_Recording();
        cam->Free_Buffer();
        ui->Parameters->setEnabled(true);
        ui->Preview->setEnabled(true);
        ui->Trigger->setDisabled(true);
        isArm = false;
        ui->Arm->setText("Arm");
    }
    else{
        ui->Parameters->setDisabled(true);
        ui->Preview->setDisabled(true);
        ui->Trigger->setDisabled(true);
        connect(cam,SIGNAL(Image_data_receive(QList<int>)),this,SLOT(receive_image(QList<int>)));
        connect(cam,SIGNAL(Wait_time_out()),this,SLOT(image_time_out()));
        if(ui->Trigger_mode->currentIndex()==1){
            ui->Trigger->setEnabled(true);
            cam->Stop_Recording();
            cam->Set_Buffer_Number(ui->Image_Nr->currentIndex());
            cam->Set_Trigger(SW_TRIGGER);
            //cam->Start_Recording(0x0002);
            cam->Start_Recording(ASYNC_SHUTTER);
        }
        isArm = true;
        ui->Arm->setText("Release");
    }
}

void CameraControlQE::on_Path_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,"Select save folder");
    if(path.isEmpty())ui->Image_Path->setText("None");
    else ui->Image_Path->setText(path);
}

void CameraControlQE::on_Exposure_returnPressed()
{
    bool ok = false;
    int exposure = ui->Exposure->text().toInt(&ok);
    if(ok)emit set_exposure(exposure);  //cam->Set_Exposure(exposure);
    else{
        ui->Exposure->setText("100");
        cam->Set_Exposure(100);
    }
}

void CameraControlQE::Reload(QString msg){
    QStringList msglist = msg.split(':');
    milisecondtowait = msglist.at(0).toInt();
    number_of_run = msglist.at(1).toInt();
    number_of_scan = msglist.at(2).toInt();

    if(isArm&&ui->Trigger_mode->currentIndex()==0){
        cam->Stop_Recording();
        cam->Set_Buffer_Number(ui->Image_Nr->currentIndex());
        images_data.clear();
        cam->Set_Trigger(HW_TRIGGER);
        //cam->Start_Recording(0x0002);  //0x0002 external trigger & software trigger,  0x0003 external exposure control
        cam->Start_Recording(ASYNC_SHUTTER);
        if(ui->Trigger_mode->currentIndex()==0){
            acquire_next_image();
        }
    }
}

void CameraControlQE::acquire_next_image(){
    if(ui->Trigger_mode->currentIndex()==1){
        emit camera_force_trigger();
    }

    emit start_acquire(milisecondtowait);
}

void CameraControlQE::image_time_out(){
    if(ui->Arm->text()!="Release")return;

    QList<int> img;
    img << 1392 << 1060;
    for(int i=0; i<1392; i++){
        for(int j=0; j<1060; j++){
            if(i<j+10 && i>j-10)img.append(65000);
            else img.append(5);
        }
    }
    images_data.append(img);
    save_images(images_data.size()-1);
    if(images_data.size()>=ui->Image_Nr->currentIndex()){
        images_data.clear();
    }
    else if(ui->Trigger_mode->currentIndex()==0){
        acquire_next_image();
    }
}

void CameraControlQE::receive_image(QList<int> img_dat){
    if(ui->Arm->text()!="Release")return;
    images_data.append(img_dat);
    save_images(images_data.size()-1);
    if(images_data.size()>=ui->Image_Nr->currentIndex()){
        images_data.clear();
    }
    else if(ui->Trigger_mode->currentIndex()==0){
        acquire_next_image();
    }
}

void CameraControlQE::save_images(int i){
    QString filename = ui->Image_Name->text();
    QString path = ui->Image_Path->text();
    if(path=="None")path.clear();
    filename.replace("[scan]",QString::number(number_of_scan));
    filename.replace("[run]",QString::number(number_of_run));
    filename.replace("YYYY",QDateTime::currentDateTime().toString("yyyy"));
    filename.replace("MM",QDateTime::currentDateTime().toString("MM"));
    filename.replace("DD",QDateTime::currentDateTime().toString("dd"));
    filename.replace("hh",QDateTime::currentDateTime().toString("HH"));
    filename.replace("mm",QDateTime::currentDateTime().toString("mm"));
    filename.replace("ss",QDateTime::currentDateTime().toString("ss"));
    QString save = path+ "/" + filename.replace("*",QString::number(i+1));

    Image_Saver->Add_image(images_data.at(i));
    emit save_image(save);

    if(ui->Trigger_mode->currentIndex()==1)ui->Trigger->setEnabled(true);
}

void CameraControlQE::on_Trigger_clicked()
{
    acquire_next_image();
}
