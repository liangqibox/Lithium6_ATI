#include "cameracontrol_lucam.h"
#include "ui_cameracontrol_lucam.h"
#include <QDebug>

const int VIDEO_MODE_UPDATE_RATE = 20;  //FPS
const int NUMBER_OF_IMAGES_TAKEN = 3;
const uint MAX_WIDTH = 2048;
const uint MAX_HEIGHT = 2048;

CameraControl_LuCam::CameraControl_LuCam(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CameraControl_LuCam)
{
    ui->setupUi(this);

    connected = false;
    ImageMode = false;
    VideoMode = false;
    softwareControl = false;
    VideoMode_progressCounter = 0;
    Image_Taken = 0;
    MOT_Load_Time = 2000;
    Offset_X = 0;
    Offset_Y = 0;
    Pixel_Height = 2048;
    Pixel_Width = 2048;
    Conversion_Struct = new LUCAM_CONVERSION;
    FrameFormat_Struct = new LUCAM_FRAME_FORMAT;
    Snapshot_Struct = new LUCAM_SNAPSHOT;

    VideoMode_Timer = new QTimer;
    VideoMode_Timer->setTimerType(Qt::PreciseTimer);
    connect(VideoMode_Timer,SIGNAL(timeout()),this,SLOT(VideoMode_Update()));
    callbackTrigger = new CallbackTrigger;
    connect(callbackTrigger,SIGNAL(Image_Collected(QImage)),this,SLOT(Receive_Callback_Image(QImage)));

    ImageSaveThread = new QThread;
    imageSave = new Image_Save;
    connect(this,SIGNAL(Save_image(QString)),imageSave,SLOT(Save_image_16bit(QString)));
    //connect(this,SIGNAL(Save_image(QString)),imageSave,SLOT(Save_image_8bit(QString)));
    imageSave->moveToThread(ImageSaveThread);
    ImageSaveThread->start();

    QPixmap BG = QPixmap(ui->Display_Load->size());
    BG.fill(Qt::black);
    ui->Display_Load->setPixmap(BG);

    QPixmap BG2 = QPixmap(ui->Display_Record->width()-2,ui->Display_Record->height()-2);
    BG2.fill(Qt::white);
    ui->Display_Record->setPixmap(BG2);

    QPixmap BG3 = QPixmap(ui->Display_Image->size());
    BG3.fill(Qt::black);
    ui->Display_Image1->setPixmap(BG3);
    ui->Display_Image2->setPixmap(BG3);
    ui->Display_Image3->setPixmap(BG3);
}

CameraControl_LuCam::~CameraControl_LuCam()
{
    delete ui;
    if(connected)LucamCameraClose(Camera);
}

void CameraControl_LuCam::Receive_New_Cycle_Trigger(QStringList msg){
    qDebug() << msg;
}

void CameraControl_LuCam::on_OpenCam_clicked()
{
    long camNum = LucamNumCameras();
    Camera = LucamCameraOpen(camNum);
    callbackTrigger->Camera = Camera;
    connected = true;
    ui->PreView->setEnabled(true);
    ui->CloseCam->setEnabled(true);
    ui->VideoMode_SoftwareStart->setEnabled(true);
    ui->ImageMode_SoftwaveStart->setEnabled(true);
    qDebug() << "Camera" << camNum << " Open with: " << Camera;
    qDebug() << "Callback Function Add: " << LucamAddSnapshotCallback(Camera,SnapshotCallback,callbackTrigger);
}

void CameraControl_LuCam::on_CloseCam_clicked()
{

    bool closed = false;
    if(connected){
        LucamRemoveSnapshotCallback(Camera,0);
        closed = LucamCameraClose(Camera);
    }
    if(closed){
        connected = false;
        ui->PreView->setDisabled(true);
        ui->CloseCam->setDisabled(true);
        ui->VideoMode_SoftwareStart->setDisabled(true);
        ui->ImageMode_SoftwaveStart->setDisabled(true);
    }
    qDebug() << "Camera Closed: " << closed;
}

void CameraControl_LuCam::on_PreView_clicked()
{
    QDialog *preview = new QDialog;
    preview->setGeometry(100,100,800,800);
    preview->show();
    LucamStreamVideoControl(Camera,START_DISPLAY,HWND(preview->winId()));
    connect(preview,SIGNAL(rejected()),this,SLOT(PreView_Close()));
}

void CameraControl_LuCam::PreView_Close(){
    LucamStreamVideoControl(Camera,STOP_STREAMING,NULL);
}

void CameraControl_LuCam::on_Video_Adjust_clicked()
{
    LucamDisplayVideoFormatPage(Camera,NULL);
}

void CameraControl_LuCam::on_Image_Adjust_clicked()
{
    LucamDisplayPropertyPage(Camera,NULL);
}

void CameraControl_LuCam::on_Start_clicked()
{

}

void CameraControl_LuCam::on_VideoMode_SoftwareStart_clicked()
{
    if(VideoMode){
        qDebug() << "Software Video Stop";
        VideoMode_Timer->stop();
        LucamStreamVideoControl(Camera,STOP_STREAMING,NULL);
        softwareControl = false;
        VideoMode = false;
        ui->Edit_Height->setEnabled(true);
        ui->Edit_Width->setEnabled(true);
        ui->Edit_xOffset->setEnabled(true);
        ui->Edit_yOffset->setEnabled(true);
        ui->VideoMode_SoftwareStart->setText("Start");
    }
    else{
        qDebug() << "Software Video Start";
        softwareControl = true;
        VideoMode = true;
        ui->Edit_Height->setDisabled(true);
        ui->Edit_Width->setDisabled(true);
        ui->Edit_xOffset->setDisabled(true);
        ui->Edit_yOffset->setDisabled(true);
        ui->VideoMode_SoftwareStart->setText("Stop");
        LucamStreamVideoControl(Camera,START_DISPLAY,HWND(ui->Display_MOT->winId()));
        VideoMode_Timer->start(1000/VIDEO_MODE_UPDATE_RATE);
    }
}

void CameraControl_LuCam::VideoMode_Update(){
    VideoMode_progressCounter += 1000/VIDEO_MODE_UPDATE_RATE;
    Data_Raw = new BYTE[Pixel_Width*Pixel_Height];

    LucamTakeVideo(Camera,1,Data_Raw);

    double SUM = 0;
    for(int i=0; i<Pixel_Width; i++){
        for(int j=0; j<Pixel_Height; j++){
            SUM += double(Data_Raw[i*Pixel_Width+j]);
        }
    }
    ui->Display_CurrentNum->display(SUM);
    int progress = double(VideoMode_progressCounter)/double(MOT_Load_Time)*100.;
    if(progress<100){
        ui->MOT_Load_Progress->setValue(progress);
    }
    else if(softwareControl){
        VideoMode_progressCounter = 0;
    }
    else{

    }
    delete Data_Raw;
}

void CameraControl_LuCam::on_ImageMode_SoftwaveStart_clicked()
{
    if(ui->ImageMode_SoftwaveStart->text()=="Trigger"){
        Data_Raw = new BYTE[MAX_WIDTH*MAX_HEIGHT];
        LucamTakeFastFrame(Camera,Data_Raw);
    }
    else if(ImageMode){
        ImageMode = false;
        ui->Edit_Exposure->setEnabled(true);
        ui->Edit_Gain->setEnabled(true);
        ui->ImageMode_SoftwaveStart->setText("Start");
    }
    else{
        ImageMode = true;
        {
            Snapshot_Struct->exposure = ui->Edit_Exposure->text().toFloat();
            Snapshot_Struct->gain = ui->Edit_Gain->text().toFloat();
            Snapshot_Struct->strobeDelay = 5000;
            Snapshot_Struct->timeout = 5000;
            LUCAM_FRAME_FORMAT frame_format;
            frame_format.xOffset = Offset_X;
            frame_format.yOffset = Offset_Y;
            frame_format.width = Pixel_Width;
            frame_format.height = Pixel_Height;
            //frame_format.pixelFormat = LUCAM_PF_8;
            frame_format.pixelFormat = LUCAM_PF_16;
            frame_format.subSampleX = 1;
            frame_format.subSampleY = 1;
            frame_format.binningX = 1;
            frame_format.binningY = 1;
            frame_format.flagsX = 0;
            frame_format.flagsY = 0;
            Snapshot_Struct->format = frame_format;
            LucamEnableFastFrames(Camera,Snapshot_Struct);
        }
        ui->Edit_Exposure->setDisabled(true);
        ui->Edit_Gain->setDisabled(true);
        callbackTrigger->Height = Pixel_Height;
        callbackTrigger->Width = Pixel_Width;
        if(ui->ImageTrigger->currentIndex()==0){
            LucamSetTriggerMode(Camera,false);
            softwareControl = true;
            Image_Taken++;
            callbackTrigger->setHW(false);
            ui->ImageMode_SoftwaveStart->setText("Trigger");
        }
        else{
            LucamSetTriggerMode(Camera,true);
            callbackTrigger->setHW(true);
            Image_Taken++;
            ui->ImageMode_SoftwaveStart->setText("Stop");
        }
    }
}

void CameraControl_LuCam::Receive_Callback_Image(QImage img){
    QPixmap dimage = QPixmap::fromImage(img);
    if(Image_Taken==1)Image_Taken_Time = QDateTime::currentDateTime();
    if(Image_Taken>0){
        qDebug() << "Expecting image size:" << Pixel_Height*Pixel_Width << " Receive image size:" << callbackTrigger->image_data.size();
        callbackTrigger->image_data.prepend(Pixel_Height);
        callbackTrigger->image_data.prepend(Pixel_Width);
        if(Pixel_Height*Pixel_Width==(callbackTrigger->image_data.size()-2))imageSave->Add_image(callbackTrigger->image_data);
        else{
            QList<int> black_image;
            black_image.append(Pixel_Height);
            black_image.append(Pixel_Width);
            for(int i=0; i<Pixel_Height*Pixel_Width; i++)black_image.append(0);
            imageSave->Add_image(black_image);
        }
        emit Save_image(filePath+"//AtomImage_"+QString::number(Image_Taken)+"_"+Image_Taken_Time.toString("yyyyMMddHHmmss")+".PNG");
    }
    switch(Image_Taken){
    case 0:
        break;
    case 1:
        ui->Display_Image1->setPixmap(dimage.scaled(ui->Display_Image1->size()));
        break;
    case 2:
        ui->Display_Image2->setPixmap(dimage.scaled(ui->Display_Image2->size()));
        break;
    case 3:
        ui->Display_Image3->setPixmap(dimage.scaled(ui->Display_Image3->size()));
        break;
    default:
        break;
    }
    if(!callbackTrigger->is_HW())delete Data_Raw;
    callbackTrigger->image_data.clear();
    Image_Taken++;
    if(Image_Taken>3&&callbackTrigger->is_HW()&&ImageMode){
        Image_Taken = 1;
    }
    else if(Image_Taken>3){
        ImageMode = false;
        softwareControl = false;
        LucamDisableFastFrames(Camera);
        Image_Taken = 0;
        ui->Edit_Exposure->setEnabled(true);
        ui->Edit_Gain->setEnabled(true);
        ui->ImageMode_SoftwaveStart->setText("Start");
    }
}

void __stdcall SnapshotCallback(VOID *pContext, BYTE *pData, ULONG dataLength){
    CallbackTrigger *trig = static_cast<CallbackTrigger*> (pContext);
    QImage image = QImage(trig->Width,trig->Height,QImage::Format_Indexed8);
    for(int i=0; i<trig->Width*trig->Height; i++){
        trig->image_data.append(int(pData[2*i])*256+int(pData[2*i+1]));
        image.setPixel(i%trig->Width,i/trig->Height,pData[2*i+1]);
    }
    //QImage image = QImage((uchar*)pData,MAX_WIDTH,MAX_HEIGHT,QImage::Format_Indexed8);
    //for(uint i=0; i<dataLength; i++)trig->image_data.append(pData[i]);
    trig->Add_Image(image);
    trig->Emit_Singal();
}

void CameraControl_LuCam::on_Images_SavePath_clicked()
{
    filePath = QFileDialog::getExistingDirectory();
}

void CameraControl_LuCam::on_Edit_xOffset_editingFinished()
{
    bool ok = false;
    int X = ui->Edit_xOffset->text().toInt(&ok);
    if(ok && X>=0){
        Offset_X = X;
    }
    else{
        ui->Edit_xOffset->setText(QString::number(Offset_X));
    }
}

void CameraControl_LuCam::on_Edit_yOffset_editingFinished()
{
    bool ok = false;
    int Y = ui->Edit_yOffset->text().toInt(&ok);
    if(ok && Y>=0){
        Offset_Y = Y;
    }
    else{
        ui->Edit_yOffset->setText(QString::number(Offset_Y));
    }
}

void CameraControl_LuCam::on_Edit_Width_editingFinished()
{
    bool ok = false;
    uint W = ui->Edit_Width->text().toInt(&ok);
    if(ok && W<=MAX_WIDTH){
        Pixel_Width = W;
    }
    else{
        ui->Edit_Width->setText(QString::number(Pixel_Width));
    }
}

void CameraControl_LuCam::on_Edit_Height_editingFinished()
{
    bool ok = false;
    uint H = ui->Edit_Height->text().toInt(&ok);
    if(ok && H<=MAX_HEIGHT){
        Pixel_Height = H;
    }
    else{
        ui->Edit_Height->setText(QString::number(Pixel_Height));
    }
}
