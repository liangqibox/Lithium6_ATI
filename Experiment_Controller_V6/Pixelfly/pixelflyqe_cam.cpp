#include "pixelflyqe_cam.h"
#include <QDebug>

PixelflyQE_Cam::PixelflyQE_Cam(QObject *parent) : QObject(parent){
    CamNum = 0;
    hCam = NULL;
    TriggerMode = HW_TRIGGER;
    CameraMode = ASYNC_SHUTTER;
    ExposureTime = 100;

    XRes = 1392;
    YRes = 1040;

    BinH = 0;
    BinV = 0;
    Gain = 0;
    BitPerPixel = 12;

    BUFFER_MAXIMUM = 2;
    bufferCounter = 0;
}

PixelflyQE_Cam::~PixelflyQE_Cam()
{
    Close_Camera();
}

int PixelflyQE_Cam::Open_Camera(){
    int xsize,ysize;
    int mode = TriggerMode|CameraMode;
    int err = INITBOARD(CamNum,&hCam);
    SETMODE(hCam,mode,0,ExposureTime,BinH,BinV,Gain,0,BitPerPixel,0);
    GETSIZES(hCam,&XRes,&YRes,&xsize,&ysize,&BitPerPixel);
    XRes = xsize;
    YRes = ysize;
    return err;
}

int PixelflyQE_Cam::Close_Camera(){
    STOP_CAMERA(hCam);
    Free_Buffer();
    return CLOSEBOARD(&hCam);
}

int PixelflyQE_Cam::Free_Buffer(){
    REMOVE_ALL_BUFFERS_FROM_LIST(hCam);
    for(int i=0 ; i<sBufNr.size(); i++){
        FREE_BUFFER(hCam,sBufNr.at(i));
    }
    /*for(int i=0; i<wBuf.size(); i++){
        delete wBuf.at(i);
    }*/
    images.clear();
    bufferCounter=0;
    BufEvent.clear();
    wBuf.clear();
    sBufNr.clear();
    return 0;
}

int PixelflyQE_Cam::Set_Exposure(int usec){
    ExposureTime = usec;
    return 0;
}

int PixelflyQE_Cam::Set_Trigger(int mode){
    TriggerMode = mode;
    return 0;
}

int PixelflyQE_Cam::Allocate_Buffer(){
    HANDLE BE = NULL;
    int Number = -1;
    int size = XRes*YRes*((BitPerPixel+7)/8);
    void *Buffer = NULL;
    ALLOCATE_BUFFER_EX(hCam,&Number,size,&BE,&Buffer);
    BufEvent.append(BE);
    sBufNr.append(Number);
    wBuf.append((uint*)Buffer);
    return int(Number);
}

int PixelflyQE_Cam::Start_Recording(int cammode){
    for(int i=0; i<BUFFER_MAXIMUM; i++)Allocate_Buffer();
    bufferCounter = 0;
    int mode = TriggerMode|cammode;
    SETMODE(hCam,mode,0,ExposureTime,BinH,BinV,Gain,0,BitPerPixel,0);
    return START_CAMERA(hCam);
}

int PixelflyQE_Cam::Stop_Recording(){
    int err = STOP_CAMERA(hCam);
    Free_Buffer();
    return err;
}

int PixelflyQE_Cam::Wait_for_Image(int msec){
    int size = XRes*YRes*((BitPerPixel+7)/8);
    ADD_BUFFER_TO_LIST(hCam,sBufNr.at(bufferCounter),size,0,0);
    uint error = WaitForSingleObject(BufEvent.at(bufferCounter),msec);
    if(error == WAIT_TIMEOUT){
        ResetEvent(BufEvent.at(bufferCounter));
        bufferCounter++;
        if(bufferCounter>=BUFFER_MAXIMUM)bufferCounter = 0;
        emit Wait_time_out();
        qDebug() << "Image" << bufferCounter << "Wait Timeout";
    }
    else if(error == WAIT_FAILED){
        ResetEvent(BufEvent.at(bufferCounter));
        bufferCounter++;
        if(bufferCounter>=BUFFER_MAXIMUM)bufferCounter = 0;
        emit Wait_time_out();
        qDebug() << "Image" << bufferCounter << "Wait Failed";
    }
    else{
        //const int size = XRes*YRes;
        //int image_data_8[size];
        QList<int> image_data;
        image_data.append(XRes);
        image_data.append(YRes);
        for(int i=0; i<XRes*YRes/2; i++){
            //image_data_8[i*2] = wBuf.at(bufferCounter)[i]%65536/16;
            //image_data_8[i*2+1] = wBuf.at(bufferCounter)[i]/65536/16;
            image_data.append(uint(wBuf.at(bufferCounter)[i])%65536);
            image_data.append(uint(wBuf.at(bufferCounter)[i])/65536);
        }
        //QImage image = QImage((uchar*)image_data_8,int(XRes),int(YRes),QImage::Format_Indexed8);
        ResetEvent(BufEvent.at(bufferCounter));
        bufferCounter++;
        if(bufferCounter>=BUFFER_MAXIMUM)bufferCounter = 0;
        //emit Image_receive(image);
        emit Image_data_receive(image_data);
        //images_data.append(image_data);
    }
    //images.append(image);
    if(images.size()>=BUFFER_MAXIMUM)images.removeFirst();
    if(images_data.size()>=BUFFER_MAXIMUM)images_data.removeFirst();
    return 0;
}

int PixelflyQE_Cam::Force_Trigger(){
    return TRIGGER_CAMERA(hCam);
}

QImage PixelflyQE_Cam::Read_Image(){
    QImage image = QImage(XRes,YRes,QImage::Format_Indexed8);
    image.fill(Qt::blue);
    if(!images.isEmpty())return images.last();
    return image;
}

int PixelflyQE_Cam::Set_Buffer_Number(int n){
    BUFFER_MAXIMUM = n;
    return BUFFER_MAXIMUM;
}
