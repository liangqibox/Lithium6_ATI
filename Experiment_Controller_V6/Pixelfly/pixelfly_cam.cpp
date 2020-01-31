#include "pixelfly_cam.h"
#include <QDebug>

Pixelfly_Cam::Pixelfly_Cam(QObject *parent) : QObject(parent){
    wCamNum = 0;
    hBWConv = 0;
    wTriggerMode = 0x0000;

    dwDelay = 0;
    dwExposure = 100;
    wTimeBaseDelay = 0;
    wTimeBaseExposure = 0x0001;

    XRes = 1392;
    YRes = 1040;
    wBitPerPixel = 14;
    dwSize = XRes*YRes*2;
    wBinH = 1;
    wBinV = 1;
    ROIx = XRes;
    ROIy = YRes;

    strGeneral.wSize = sizeof(strGeneral);
    strGeneral.strCamType.wSize = sizeof(strGeneral.strCamType);
    strCamType.wSize = sizeof(strCamType);
    strSensor.wSize = sizeof(strSensor);
    strSensor.strDescription.wSize = sizeof(strSensor.strDescription);
    strSensor.strDescription2.wSize = sizeof(strSensor.strDescription2);
    strDescription.wSize = sizeof(strDescription);
    strTiming.wSize = sizeof(strTiming);
    strStorage.wSize = sizeof(strStorage);
    strRecording.wSize = sizeof(strRecording);

    BUFFER_MAXIMUM = 2;
    bufferCounter = 0;
}

Pixelfly_Cam::~Pixelfly_Cam()
{
    Close_Camera();
}

int Pixelfly_Cam::Open_Camera(){
    int err = PCO_OpenCamera(&hCam, wCamNum);
    PCO_GetGeneral(hCam, &strGeneral);
    PCO_GetCameraType(hCam, &strCamType);
    PCO_GetSensorStruct(hCam, &strSensor);
    PCO_GetCameraDescription(hCam, &strDescription);
    PCO_GetTimingStruct(hCam, &strTiming);
    PCO_GetRecordingStruct(hCam, &strRecording);
    PCO_ResetSettingsToDefault(hCam);
    PCO_SetBitAlignment(hCam,0x0001);
    Set_Trigger(0x0000);
    Set_Exposure(100);
    //Start_Recording();

    PCO_SensorInfo strSensorInfo;
    strSensorInfo.wSize = sizeof(PCO_SensorInfo);
    strSensorInfo.hCamera = hCam;
    strSensorInfo.iCamNum = 0;
    strSensorInfo.iConversionFactor = strSensor.strDescription.wConvFactDESC[0];
    strSensorInfo.iDarkOffset = 30;
    strSensorInfo.iDataBits = strSensor.strDescription.wDynResDESC;
    strSensorInfo.iSensorInfoBits = CONVERT_SENSOR_UPPERALIGNED;// Input data is upper aligned
    strSensorInfo.strColorCoeff.da11 = 1.0;
    strSensorInfo.strColorCoeff.da12 = 0.0;
    strSensorInfo.strColorCoeff.da13 = 0.0;
    strSensorInfo.strColorCoeff.da21 = 0.0;
    strSensorInfo.strColorCoeff.da22 = 1.0;
    strSensorInfo.strColorCoeff.da23 = 0.0;
    strSensorInfo.strColorCoeff.da31 = 0.0;
    strSensorInfo.strColorCoeff.da32 = 0.0;
    strSensorInfo.strColorCoeff.da33 = 1.0;
    PCO_ConvertCreate(&hBWConv,&strSensorInfo,PCO_BW_CONVERT);
    return err;
}

int Pixelfly_Cam::Close_Camera(){
    PCO_SetRecordingState(hCam,0x0000);
    PCO_CancelImages(hCam);
    PCO_ConvertDelete(hBWConv);
    Free_Buffer();
    return PCO_CloseCamera(hCam);
}

int Pixelfly_Cam::Free_Buffer(){
    for(int i=0 ;i<16; i++){
        SHORT free = i;
        PCO_FreeBuffer(hCam,free);
    }
    //PCO_CancelImages(hCam);
    for(int i=0; i<wBuf.size(); i++){
        delete wBuf.at(i);
    }
    images.clear();
    bufferCounter=0;
    BufEvent.clear();
    wBuf.clear();
    sBufNr.clear();
    return 0;
}

int Pixelfly_Cam::Set_Exposure(int usec){
    dwExposure = usec;
    int err = PCO_SetDelayExposureTime(hCam,dwDelay,dwExposure,wTimeBaseDelay,wTimeBaseExposure);
    return err;
}

int Pixelfly_Cam::Set_Trigger(int mode){
    wTriggerMode = mode;
    int err = PCO_SetTriggerMode(hCam,wTriggerMode);
    return err;
}

int Pixelfly_Cam::Allocate_Buffer(){
    HANDLE BE = 0;
    SHORT Number = -1;
    WORD *Buffer = new WORD[uint(XRes)*uint(YRes)];
    DWORD Size = uint(XRes)*uint(YRes)*sizeof(WORD);
    PCO_AllocateBuffer(hCam,&Number,Size,&Buffer,&BE);
    BufEvent.append(BE);
    sBufNr.append(Number);
    wBuf.append(Buffer);
    //PCO_AddBufferEx(hCam,DWORD(Number),DWORD(Number),Number,XRes,YRes,wBitPerPixel);
    //qDebug() << "Buffer add" << Number;
    return int(Number);
}

int Pixelfly_Cam::Start_Recording(int trigger){
    //Free_Buffer();
    //if(Open_Camera()==0){
        Set_Trigger(trigger);
        PCO_GetCameraHealthStatus(hCam,&dwWarn,&dwError,&dwStatus);
        for(int i=0; i<BUFFER_MAXIMUM; i++)Allocate_Buffer();
        bufferCounter = 0;
        PCO_ArmCamera(hCam);
        PCO_SetRecordingState(hCam,0x0001);
        return 0;
    //}
    //else return -1;
}

int Pixelfly_Cam::Stop_Recording(){
    PCO_GetCameraHealthStatus(hCam,&dwWarn,&dwError,&dwStatus);
    //int err = Close_Camera();
    int err = PCO_SetRecordingState(hCam,0x0000);
    Free_Buffer();
    return err;
}

int Pixelfly_Cam::Wait_for_Image(int msec){
    PCO_AddBufferEx(hCam,DWORD(0),DWORD(0),sBufNr.at(bufferCounter),XRes,YRes,wBitPerPixel);
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
        BYTE image_data_8[uint(XRes)*uint(YRes)];
        QList<int> image_data;
        image_data.append(int(XRes));
        image_data.append(int(YRes));
        for(int y=1; y<int(YRes)+1; y++){
            for(int x=0; x<int(XRes); x++){
                image_data_8[x+XRes*(y-1)]= wBuf.at(bufferCounter)[x+XRes*(y-1)]/64;
                //image_data_8[x+XRes*(y-1)]= wBuf.at(bufferCounter)[x+XRes*(y-1)] & 0xFF;
                image_data.append(int(wBuf.at(bufferCounter)[x+XRes*(y-1)]));
            }
        }
        QImage image = QImage((uchar*)image_data_8,int(XRes),int(YRes),QImage::Format_Indexed8);
        ResetEvent(BufEvent.at(bufferCounter));
        bufferCounter++;
        if(bufferCounter>=BUFFER_MAXIMUM)bufferCounter = 0;
        emit Image_receive(image);
        emit Image_data_receive(image_data);
        //images_data.append(image_data);
    }
    //images.append(image);
    if(images.size()>=BUFFER_MAXIMUM)images.removeFirst();
    if(images_data.size()>=BUFFER_MAXIMUM)images_data.removeFirst();
    return 0;
}

int Pixelfly_Cam::Force_Trigger(){
    WORD triggered;
    PCO_ForceTrigger(hCam,&triggered);
    return triggered;
}

QImage Pixelfly_Cam::Read_Image(){
    QImage image = QImage(XRes,YRes,QImage::Format_Indexed8);
    image.fill(Qt::blue);
    if(!images.isEmpty())return images.last();
    return image;
}

int Pixelfly_Cam::Set_Buffer_Number(int n){
    BUFFER_MAXIMUM = n;
    return BUFFER_MAXIMUM;
}
