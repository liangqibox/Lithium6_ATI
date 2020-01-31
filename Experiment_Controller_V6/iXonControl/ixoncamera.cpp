#include "ixoncamera.h"

#define TIMEUNIT (1/1000.) // * s = 1ms
#define SHUTTER_TIME_ERROR_RANGE 100


iXonCamera::iXonCamera(QObject *parent) : QObject(parent), m_driverPath(QDir::toNativeSeparators(QDir::currentPath()))
{
    m_dataHandler = new DataHandler;
    m_acqMonitor = new AcquisitionMonitor;

    m_tempControl = new TemperatureController;
    m_temperatureWarningEnabled  = true;

    if(checkForCorrectInit() != DRV_SUCCESS) return;

    m_isAcquiring = false;
    m_saveVideo = false;

    m_photonCountMaxThreshold  = sizeof(DWORD);

    GetDetector(&m_xpixel, &m_ypixel);

    getGainRange();
    getGainValue();
    getGainAdvanced();

    /* ACQ MONITOR */
    connect(this, SIGNAL(acqStart(ushort, acquisitionMode)), m_acqMonitor, SLOT(monitorAcqStatus(ushort, acquisitionMode)));
    connect(m_acqMonitor, SIGNAL(acqSuccess()), this, SLOT(stopAcquiring()));
    connect(m_acqMonitor, SIGNAL(acqError()), this, SLOT(abortAcquiring()));
    connect(m_acqMonitor, SIGNAL(frameCaptured()), this, SLOT(handleFrame()));


    /* DATA HANDLER */
    connect(this, SIGNAL(sendAcqParameters(int,int)), m_dataHandler, SLOT(setAcqParameters(int,int)));
    connect(this, SIGNAL(saveSignal(int, int, int)), m_dataHandler, SLOT(saveLastAcqData(int, int, int)));
    connect(this, SIGNAL(saveFrameSignal(int,int)), m_dataHandler, SLOT(getNewImage(int,int)));
    connect(this, SIGNAL(saveVideoFrameSignal(int, int, int, bool)), m_dataHandler, SLOT(saveVideoFramePNG(int, int, int, bool)));
    connect(this, SIGNAL(saveAllSignal()), m_dataHandler, SLOT(saveAll()));
    connect(this, SIGNAL(deleteTempFileSignal()), m_dataHandler, SLOT(deleteTempFiles()));


    connect(this, SIGNAL(updateCountModes()), this, SLOT(resetCountMode()));

    /* LOADING DEFAULTS */
    m_acqParam = {};
    m_rdParam = {};
    m_ROIParam = {};
    m_shParam = {};

    setDefaults();
    loadHardcodedDefaults(); //loads defaults once, so that even if something goes wrong with loading from file, valid settings will be loaded

    m_systemInfo = {};
    loadSystemInformation();

    /*
    AndorCapabilities *cap = new AndorCapabilities;
    cap->ulSize = sizeof(AndorCapabilities);
    GetCapabilities(cap);
    */
}

iXonCamera::~iXonCamera()
{
    emit deleteTempFileSignal();
    turnOff();

    m_tempControl->deleteLater();
    m_dataHandler->deleteLater();
    m_acqMonitor->deleteLater();
}

/* SYSTEM CONTROL */
int iXonCamera::initCamera()
{
    int err_check;
    char c_path[256] = "";
    strcpy(c_path, m_driverPath.toStdString().c_str());

    err_check = Initialize(c_path);
    if(err_check != DRV_SUCCESS) return err_check;
    m_turnedOn = true;
    emit sendMessage("Camera initialized successfully");

    return err_check;
}

void iXonCamera::turnOff()
{
    if(m_turnedOn)
    {
        struct shutterParameter sP = {};
        sP.shMode = SH_MODE_PERMCLOSED;
        sP.shExtMode = SH_MODE_PERMCLOSED;
        loadShutter(sP);
        qDebug() << "Camera shutdown";

        m_turnedOn = false;
    }

    ShutDown();
    return;
}

int iXonCamera::checkForCorrectInit()
{
    m_initError = false;
    if(initCamera() != DRV_SUCCESS)
    {
        QMessageBox initWarningBox;
        initWarningBox.setText("Critical Error: Camera could not be initialized.");
        initWarningBox.setInformativeText("If the camera is not switched on, do so now and press ok. If this message appears again, check whether the driver files are in the executeable's directory.");
        initWarningBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Abort);
        int ret = initWarningBox.exec();
        if(ret == QMessageBox::Ok)
        {
            if(checkForCorrectInit() == DRV_SUCCESS) return DRV_SUCCESS;
            m_initError = false;
        }
        else
        {
            m_turnedOn = false;
            m_initError = true;
            return DRV_NOT_INITIALIZED;
        }
    }

    return DRV_SUCCESS;
}

void iXonCamera::loadSystemInformation()
{
    unsigned int a = 0;
    unsigned int b = 0;
    GetHardwareVersion(&m_systemInfo.PCBVersion,&m_systemInfo.decodeVersion ,&a, &b, &m_systemInfo.firmwareVersion, &m_systemInfo.firmwareBuild);
    GetSoftwareVersion(&m_systemInfo.EPROMVersion, &m_systemInfo.COFVersion, &m_systemInfo.driverRevision, &m_systemInfo.driverVersion, &m_systemInfo.dllRevision, &m_systemInfo.dllVersion);

    char buffer[256] = "";
    GetHeadModel(buffer);
    m_systemInfo.CCDType = QString::fromLatin1(buffer);
    return;
}

void iXonCamera::receiveExperimentControllerMessage(QString message)
{
    if(!message.isEmpty())
    {
        bool ok;

        int timeout = message.left(message.indexOf(":")).toInt(&ok);
        if(!ok) return; //should throw error
        message = message.mid(message.indexOf(":") + 1);

        int runNum = message.left(message.indexOf(":")).toInt(&ok);
        if(!ok) return;
        message = message.mid(message.indexOf(":") + 1);


        int scanNum = message.left(message.indexOf(":")).toInt(&ok);
        if(!ok) return;
        message = message.mid(message.indexOf(":") + 1);

        if(m_isAcquiring) abortAcquiring();
        m_curRunNum = runNum;
        m_curScanNum = scanNum;
        emit sendAcqParameters(m_curRunNum, m_curScanNum);
        startAcquiring(timeout);
    }

    return;
}

/* ACQUISITION CONTROL*/
int iXonCamera::startAcquiring(int timeout)
{
    int tempStatus;
    m_tempControl->currentTemperature(&tempStatus);
    bool confirmed = true;

    if(tempStatus != DRV_TEMP_STABILIZED && m_temperatureWarningEnabled == true) //warns if temperature is not stable
    {
        QMessageBox StabWarnBox;
        QCheckBox *dontShowAgainCB = new QCheckBox("Don't show again");
        StabWarnBox.setText("Temperature is not stable. Do you want to continue anyway?");
        StabWarnBox.setIcon(QMessageBox::Warning);
        StabWarnBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        StabWarnBox.setCheckBox(dontShowAgainCB);
        (StabWarnBox.button(QMessageBox::Ok))->setText("Continue");

        int ret = StabWarnBox.exec();

        if(ret == QMessageBox::Ok) confirmed = true;
        else confirmed = false;

        if(dontShowAgainCB->isChecked()) m_temperatureWarningEnabled = false;
        dontShowAgainCB->deleteLater();
    }
    if(!confirmed) return DRV_CANCEL; //mb not appropriate error code

    int acqStatus;
    int err_check = GetStatus(&acqStatus);
    if(err_check != DRV_SUCCESS) return err_check;
    if(acqStatus != DRV_IDLE) return acqStatus;


    err_check = StartAcquisition();
    startAcqStatusMonitor(timeout);

    QString message;

    switch(err_check)
    {
    case DRV_SPOOLSETUPERROR:
        message = "Spool setup error";
        break;
    case DRV_BINNING_ERROR:
        message = "Binning error";
        break;
    case DRV_INVALID_FILTER:
        message = "Invalid filter";
        break;
    case DRV_ERROR_PAGELOCK:
        message = "Unable to allocate memory";
        break;
    case DRV_ACQUISITION_ERRORS:
        message = "Invalid acquisition settings";
        break;
    case DRV_ERROR_ACK:
        message = "Unable to communicate with card";
        break;
    case DRV_VXDNOTINSTALLED:
        message = "VXD not installed";
        break;
    case DRV_INIERROR:
        message = "Unable to load DETECTOR.INI";
        break;
    case DRV_NOT_INITIALIZED:
        message = "Camera not initialized";
        break;
    default:
        return err_check; //SUCCESSFUL RETURN
    }

    //IF DEFAULT IN SWITCH IS NOT REACHED OPENS A MESSAGEBOX AND TELLS THE USER WHAT WENT WRONG
    QMessageBox errorBox;
    errorBox.setText(message);
    errorBox.setIcon(QMessageBox::Critical);
    errorBox.setStandardButtons(QMessageBox::Ok);
    errorBox.exec();

    return err_check;
}

int iXonCamera::startAcqStatusMonitor(ushort timeout)
{
    emit acqStart(timeout, m_acqParam.acqMode);
    m_isAcquiring = true;
    m_elapsedAcc = 0;
    m_elapsedKin = 0;
    emit acquiringStatusChanged(true);
    qDebug() << "Acquisition in progress...";
    emit sendMessage("Acquisition in progress...");
    if(m_acqParam.trMode != TR_MODE_INTERNAL) emit sendMessage("Waiting for trigger signal...");


    return DRV_SUCCESS;
}

int iXonCamera::stopAcqStatusMonitor(bool success)
{
    m_isAcquiring = false;
    emit acquiringStatusChanged(false);
    if(success)
    {
        if(m_acqParam.acqMode == ACQ_MODE_KINETIC || m_acqParam.acqMode == ACQ_MODE_FAST_KINETIC)
            emit saveSignal(xDataPoints(), yDataPoints(), m_acqParam.kinNumber);
        else if(m_acqParam.acqMode == ACQ_MODE_RUN_TILL_ABORT);
            //do nothing
        else
            emit saveSignal(xDataPoints(), yDataPoints(), 1);

        return DRV_SUCCESS;
    }
    else
    {
        return DRV_ACQUISITION_ERRORS; //mb not appropriate error code
    }
}

int iXonCamera::abortAcquiring()
{
    int err_check = AbortAcquisition();
    stopAcqStatusMonitor(false);
    qDebug() << "WARNING: ACQUISITION ABORTED WITH ERROR CODE" << err_check;
    emit sendMessage("Acquisition aborted");
    return err_check;
}

int iXonCamera::stopAcquiring()
{
    int err_check = stopAcqStatusMonitor(true);
    qDebug() << "Acquisition completed with error code " << err_check;
    emit sendMessage("Acquisition complete");
    return err_check;
}

void iXonCamera::handleFrame()
{
    long acc, kin;
    GetAcquisitionProgress(&acc, &kin);

    m_elapsedAcc = acc;
    m_elapsedKin = kin;
    emit sendMessage("Acquisition progress: Accumulation " + QString::number(acc) + " of Kinetic Cycle " + QString::number(kin));
    qDebug() << "Acquisition progress:" << "Acc Cycles: " << acc << "Kin cycles:" << kin;


    if(m_acqParam.acqMode == ACQ_MODE_RUN_TILL_ABORT)
    {
        m_frameCount++;
        emit saveVideoFrameSignal(xDataPoints(), yDataPoints(), m_elapsedKin, m_saveVideo);
    }

    return;
}

int iXonCamera::setSaveVideo(bool save)
{
    int status;
    int err_check = GetStatus(&status);
    if(err_check == DRV_SUCCESS && status == DRV_IDLE) m_saveVideo = save;
    return status;
}

int iXonCamera::xDataPoints()
{
    int dp;
    switch(m_ROIParam.rdMode)
    {
    case RD_MODE_FVB:
    case RD_MODE_SINGLETRACK:
    case RD_MODE_MULTITRACK:
    case RD_MODE_RANDOMTRACK:
        dp = m_xpixel / m_ROIParam.horBin;
        if(m_acqParam.acqMode == ACQ_MODE_FAST_KINETIC) dp = m_xpixel / m_ROIParam.FKHorBin;
        break;
    case RD_MODE_IMAGE:
        dp = (m_ROIParam.imgXEnd - m_ROIParam.imgXStart + 1) / m_ROIParam.imgHorBin ; //+1 since borders are inclusive
        if(m_acqParam.acqMode == ACQ_MODE_FAST_KINETIC) dp = m_xpixel / m_ROIParam.FKHorBin;
        break;
    }
    return dp;
}

int iXonCamera::yDataPoints()
{
    int dp;
    switch(m_ROIParam.rdMode)
    {
    case RD_MODE_FVB:
    case RD_MODE_SINGLETRACK:
        dp = 1;
        break;
    case RD_MODE_MULTITRACK:
        dp = m_ROIParam.MTNum;
        break;
    case RD_MODE_RANDOMTRACK:
        dp = m_ROIParam.CTNumber;
        break;
    case RD_MODE_IMAGE:
        dp = (m_ROIParam.imgYEnd - m_ROIParam.imgYStart + 1) / m_ROIParam.imgVertBin; //+1 since borders are inclusive
        if(m_acqParam.acqMode == ACQ_MODE_FAST_KINETIC) dp = m_ROIParam.FKHeight / m_ROIParam.FKVertBin;
        break;
    }

    return dp;
}

/* ROI PARAMETERS */
int iXonCamera::setupHorBinning(int hor) //not applicable to img mode
{
    int err_check;
    int maxHor;
    err_check = GetMaximumBinning(m_ROIParam.rdMode, 0, &maxHor);
    if(err_check != DRV_SUCCESS) return err_check;

    if(hor < 1) hor = 1;
    else if(hor > maxHor) hor = maxHor;
    else if (m_xpixel % hor != 0) return DRV_BINNING_ERROR;

    switch(m_ROIParam.rdMode)
    {
    case RD_MODE_FVB:
        err_check = SetFVBHBin(hor);
        break;
    case RD_MODE_SINGLETRACK:
        err_check = SetSingleTrackHBin(hor);
        break;
    case RD_MODE_MULTITRACK:
        err_check = SetMultiTrackHBin(hor);
        break;
    case RD_MODE_RANDOMTRACK:
        err_check = SetCustomTrackHBin(hor);
        break;
    case RD_MODE_IMAGE: //this readmode has to be setup in setImagePosition
        return DRV_NOT_AVAILABLE;
    }

    if(err_check == DRV_SUCCESS)
    {
        m_ROIParam.horBin = hor;
    }
    return err_check;
}

int iXonCamera::setFVB(int hBin, int cropW, int cropH)
{
    int err_check = DRV_SUCCESS; //initialized to success in case of no crop

    if(m_ROIParam.cropEnabled)
    {
        err_check = 0;
        if(m_ROIParam.rdMode != RD_MODE_IMAGE && m_ROIParam.rdMode != RD_MODE_FVB) return DRV_NOT_AVAILABLE;
        err_check = SetIsolatedCropMode(true, cropH, cropW, 1, hBin);

        if(err_check == DRV_SUCCESS)
        {
            err_check = GetDetector(&m_xpixel, &m_ypixel);
            if(err_check == DRV_SUCCESS)
            {
                if(err_check == DRV_SUCCESS)
                {
                    m_ROIParam.imgXStart = 1;
                    m_ROIParam.imgXEnd = cropW;
                    m_ROIParam.imgYStart = 1;
                    m_ROIParam.imgYEnd = cropH;
                    m_ROIParam.imgHorBin = hBin;
                }
            }
        }
        qDebug() << "Crop set to" << m_xpixel << m_ypixel << "err code" << err_check;
        GetDetector(&m_xpixel, &m_ypixel);

    }

    if(err_check != DRV_SUCCESS) resetCrop();

    err_check = setupHorBinning(hBin);

    emit ROIParamChanged(m_ROIParam);
    updateExpParameters();
    return err_check;
}

int iXonCamera::setSTPosition(int height, int center, int hBin)
{
    int err_check = setupHorBinning(hBin);
    if(err_check != DRV_SUCCESS) return err_check;


    err_check = SetSingleTrack(center, height);
    if(err_check == DRV_SUCCESS)
    {
        m_ROIParam.STCenter = center;
        m_ROIParam.STHeight = height;

    }
    if(err_check == DRV_P1INVALID || err_check == DRV_P2INVALID)
    {
        int err_check2 = SetSingleTrack(512, 1);
        if(err_check2 == DRV_SUCCESS)
        {
            m_ROIParam.STCenter = 512;
            m_ROIParam.STHeight = 1;
        }
    }
    emit ROIParamChanged(m_ROIParam);
    updateExpParameters();
    return err_check;
}

int iXonCamera::setMultPosition(int num, int height, int offset, int hBin)
{
    int err_check = setupHorBinning(hBin);
    if(err_check != DRV_SUCCESS) return err_check;

    int dummy; //unused return variable
    int firstRow;
    err_check = SetMultiTrack(num, height, offset, &firstRow, &dummy);
    if(err_check == DRV_SUCCESS)
    {
        m_ROIParam.MTNum = num;
        m_ROIParam.MTHeight = height;
        m_ROIParam.MTOffset = offset;
        m_ROIParam.MTFirstRow = firstRow;
    }
    else if(err_check == DRV_P1INVALID)
    {
        if(num > m_ypixel) num = m_ypixel;
        else if(num < 1) num = 1;
        setMultPosition(num, height, offset, hBin); //watch out for infinite loop
    }

    emit ROIParamChanged(m_ROIParam);
    updateExpParameters();
    return err_check;
}

int iXonCamera::setImagePosition(int hBin, int vBin, int xStart, int xEnd, int yStart , int yEnd)
{
    int err_check;

    if(m_ROIParam.cropEnabled)
    {
        if(m_ROIParam.rdMode != RD_MODE_IMAGE) return DRV_NOT_AVAILABLE;
        err_check = SetIsolatedCropModeEx(true, yEnd - yStart + 1, xEnd - xStart + 1, vBin, hBin, xStart, yStart);

        if(err_check == DRV_SUCCESS)
        {
            err_check = GetDetector(&m_xpixel, &m_ypixel);
            if(err_check == DRV_SUCCESS)
            {
                err_check = SetImage(hBin, vBin, 1, m_xpixel, 1, m_ypixel);
                if(err_check == DRV_SUCCESS)
                {
                    m_ROIParam.imgXStart = xStart;
                    m_ROIParam.imgXEnd = xEnd;
                    m_ROIParam.imgYStart = yStart;
                    m_ROIParam.imgYEnd = yEnd;
                    m_ROIParam.imgHorBin = hBin;
                    m_ROIParam.imgVertBin = vBin;
                }
            }
        }
        qDebug() << "Crop set to" << m_xpixel << m_ypixel << "err code" << err_check;
        GetDetector(&m_xpixel, &m_ypixel);

    }

    if(!m_ROIParam.cropEnabled || err_check != DRV_SUCCESS)
    {
        resetCrop();
        err_check = SetImage(hBin, vBin, xStart, xEnd, yStart, yEnd);
        if(err_check == DRV_SUCCESS)
        {
            m_ROIParam.imgXStart = xStart;
            m_ROIParam.imgYStart = yStart;
            m_ROIParam.imgXEnd = xEnd;
            m_ROIParam.imgYEnd = yEnd;
            m_ROIParam.imgHorBin = hBin;
            m_ROIParam.imgVertBin = vBin;
        }
        GetDetector(&m_xpixel, &m_ypixel);
    }
    emit ROIParamChanged(m_ROIParam);
    updateExpParameters();

    return err_check;

}

int iXonCamera::loadImageBorder(int xStart, int xEnd, int yStart, int yEnd)
{
    return setImagePosition(m_ROIParam.imgHorBin, m_ROIParam.imgVertBin, xStart, xEnd, yStart, yEnd);
}

int iXonCamera::setRTPosition(QVector<int> RTData)
{
    if(RTData.size() % 2 != 0) return DRV_P2INVALID;
    int RTNum = RTData.size() / 2;

    int err_check = SetRandomTracks(RTNum, RTData.data());
    if(err_check == DRV_SUCCESS)
    {
        m_ROIParam.CTSettings = RTData;
        m_ROIParam.CTNumber = RTNum;
    }
    updateExpParameters();
    emit ROIParamChanged(m_ROIParam);
    emit RTListChanged(m_ROIParam.CTSettings);
    return err_check;
}

int iXonCamera::setCrop(bool enabled)
{
    int err_check;
    if(enabled)
    {
        if(m_ROIParam.rdMode != RD_MODE_FVB && m_ROIParam.rdMode != RD_MODE_IMAGE) err_check = DRV_NOT_AVAILABLE;
        else
        {
            m_ROIParam.cropEnabled = true;
            err_check = DRV_SUCCESS;
        }
    }
    else
    {
        err_check = resetCrop();
    }

    emit ROIParamChanged(m_ROIParam);
    return err_check;
}

int iXonCamera::resetCrop()
{
    if(m_ROIParam.cropEnabled)
    {
//        int xS, xE, yS, yE;
//        xS = m_ROIParam.imgXStart;
//        xE = m_ROIParam.imgXEnd;
//        yS = m_ROIParam.imgYStart;
//        yE = m_ROIParam.imgYEnd;

        int err_check = SetIsolatedCropMode(0, 1024, 1024, 1, 1);
        if(err_check != DRV_SUCCESS) qDebug() << "CRITICAL ERROR, CANNOT UNSET CROP MODE";
        m_ROIParam.cropEnabled = false;
        qDebug() << "crop reset" << err_check;
        emit ROIParamChanged(m_ROIParam);
        return err_check;
    }

    else return DRV_SUCCESS;
}

void iXonCamera::loadROI(ROIParameter RP)
{
    this->blockSignals(true);
    if(setRdMode(RP.rdMode) != DRV_SUCCESS) setRdMode(m_defaultROI.rdMode);

    if(setFVB(RP.horBin) != DRV_SUCCESS)
        setFVB(m_defaultROI.horBin);
    if(setSTPosition(RP.STHeight, RP.STCenter, RP.horBin) != DRV_SUCCESS)
        setSTPosition(m_defaultROI.STHeight, m_defaultROI.STCenter, m_defaultROI.horBin);
    if(setMultPosition(RP.MTNum, RP.MTHeight, RP.MTOffset, RP.horBin) != DRV_SUCCESS)
        setMultPosition(m_defaultROI.MTNum, m_defaultROI.MTHeight, m_defaultROI.MTOffset, m_defaultROI.horBin);
    if(setImagePosition(RP.imgHorBin, RP.imgVertBin, RP.imgXStart, RP.imgXEnd, RP.imgYStart, RP.imgYEnd) != DRV_SUCCESS)
        setImagePosition(m_defaultROI.imgHorBin, m_defaultROI.imgVertBin, m_defaultROI.imgXStart, m_defaultROI.imgXEnd, m_defaultROI.imgYStart, m_defaultROI.imgYEnd);
    this->blockSignals(false);
    emit ROIParamChanged(m_ROIParam);
    updateExpParameters();
}

void iXonCamera::resetROIPosition()
{
    setFVB(m_defaultROI.horBin);
    setSTPosition(m_defaultROI.STHeight, m_defaultROI.STCenter, m_defaultROI.horBin);
    setMultPosition(m_defaultROI.MTNum, m_defaultROI.MTHeight, m_defaultROI.MTOffset, m_defaultROI.horBin);
    setImagePosition(m_defaultROI.imgHorBin, m_defaultROI.imgVertBin, m_defaultROI.imgXStart, m_defaultROI.imgXEnd, m_defaultROI.imgYStart, m_defaultROI.imgYEnd);

    return;
}

/* READOUT PARAMETERS */
int iXonCamera::setRdMode(int mode) //resets hor binning on change
{
    this->blockSignals(true);
    int err_check;
    if(m_acqParam.acqMode != ACQ_MODE_FAST_KINETIC)
    {
        if(mode != RD_MODE_FVB && mode != RD_MODE_IMAGE) resetCrop();
        err_check = SetReadMode(mode);
        if(err_check == DRV_SUCCESS)
        {
            m_ROIParam.rdMode = static_cast<readoutMode>(mode);
            if(mode != RD_MODE_IMAGE) setupHorBinning(1); //resets binning (for now)
            if(!isTrAvailable(m_acqParam.trMode)) setTrMode(TR_MODE_INTERNAL);
            resetROIPosition();
        }
    }
    else
    {
        if(mode == RD_MODE_FVB || mode == RD_MODE_IMAGE)
        {
            err_check = setFKRdMode(mode);
            if(err_check == DRV_SUCCESS) m_ROIParam.rdMode = static_cast<readoutMode>(mode);
        }
        else err_check = DRV_NOT_AVAILABLE;
    }

    this->blockSignals(false);
    emit ROIParamChanged(m_ROIParam);
    updateExpParameters();
    return err_check;
}

int iXonCamera::retrieveVSSpeeds()
{
    int numSpeeds, numFastest;
    int err_check;

    err_check = GetNumberVSSpeeds(&numSpeeds);
    if(err_check != DRV_SUCCESS) return err_check;

    m_rdParam.availableVSSpeeds.clear(); // after confirmation that new speeds have been fetched, clear the old vector of speeds

    float temp;
    for(int i = 0; i < numSpeeds ; i++)
    {
        err_check = GetVSSpeed(i, &temp);
        if(err_check == DRV_SUCCESS) m_rdParam.availableVSSpeeds.push_back(temp);
        else break;
    }

    err_check = GetFastestRecommendedVSSpeed(&numFastest, &temp);
    if(err_check == DRV_SUCCESS) m_rdParam.numFastestRecVSSpeed = numFastest;

    return err_check;
}

int iXonCamera::retrieveHSSpeeds()
{
    int numSpeeds;
    int err_check;

    err_check = GetNumberHSSpeeds(0, !m_rdParam.EMEnabled, &numSpeeds); // The camera only has ADChannel 0.
    if(err_check != DRV_SUCCESS) return err_check;

    m_rdParam.availableHSSpeeds.clear(); // after confirmation that new speeds have been fetched, clear the old vector of speeds

    float temp;
    for(int i = 0; i < numSpeeds ; i++)
    {
        err_check = GetHSSpeed(0, !m_rdParam.EMEnabled, i, &temp);
        if(err_check == DRV_SUCCESS) m_rdParam.availableHSSpeeds.push_back(temp);
        else break;
    }

    return err_check;
}

int iXonCamera::retrievePAGains()
{
    int numGains = 0;
    int err_check = GetNumberPreAmpGains(&numGains); // this function appears to be bugged, it returns one number too high
    if(err_check != DRV_SUCCESS) return err_check;

    m_rdParam.availablePAGains.clear();

    float gain;
    for(int i = 0; i < numGains - 1; i++) //subtracts 1 from numgains to compensate for the bug mentioned above
    {
        err_check = GetPreAmpGain(i, &gain); //this function appears to simply set gain = i + 1, which is not very useful but hey
        if(err_check == DRV_SUCCESS) m_rdParam.availablePAGains.push_back(gain);
        else break;
    }

    return err_check;
}

int iXonCamera::setVSSpeed(int index)
{
    retrieveVSSpeeds();
    if(index >= m_rdParam.availableVSSpeeds.size()) index = 0;
    int err_check = SetVSSpeed(index);
    if(err_check == DRV_SUCCESS)
    {
        m_rdParam.VSSpeed = m_rdParam.availableVSSpeeds[index];
        m_rdParam.numVSSpeed = index;

        emit rdParamChanged(m_rdParam);
        updateExpParameters();
    }

    return err_check;
}

int iXonCamera::setHSSpeed(int index)
{
    retrieveHSSpeeds();
    int err_check = SetHSSpeed(!m_rdParam.EMEnabled, index);
    if(err_check == DRV_SUCCESS)
    {
        m_rdParam.HSSpeed = m_rdParam.availableHSSpeeds[index];
        m_rdParam.numHSSpeed = index;

        emit rdParamChanged(m_rdParam);
        updateExpParameters();
    }

    return err_check;
}

int iXonCamera::setEMenabled(bool on) //updates the HSSpeed list, sets HSSpeed to fastest value (0)
{
    int err_check = SetHSSpeed(!on,0); //Always changes to fastest speed, this avoids a lot of issues, and is just a very minor inconvinience for the user
    if(err_check == DRV_SUCCESS)
    {
        m_rdParam.EMEnabled = on;
        setHSSpeed(0); // updates the actual speed
        //since setHSSpeed emits the rdParamChanged signal, there is no need to emit again
        //updateExpParameters() is also called by setHSSpeed
    }

    emit updateCountModes();
    return err_check;
}

int iXonCamera::setVSVolt(int numVolt)
{
    if(numVolt == m_rdParam.numVSVolt) return DRV_SUCCESS;

    int err_check = SetVSAmplitude(numVolt);
    if(err_check == DRV_SUCCESS)
    {
        m_rdParam.numVSVolt = numVolt;
        emit rdParamChanged(m_rdParam);
        updateExpParameters();
    }
    return err_check;

}

int iXonCamera::setPAGain(int numGain)
{
    retrievePAGains();
    if(numGain >= m_rdParam.availablePAGains.size() && numGain != 0) numGain = m_rdParam.availablePAGains.size() - 1;
    if(numGain < 0) numGain = 0;
    int err_check = SetPreAmpGain(numGain);
    if(err_check == DRV_SUCCESS)
    {
        m_rdParam.numPAGain = numGain;
        m_rdParam.PAGain = m_rdParam.availablePAGains[numGain];
    }
    emit rdParamChanged(m_rdParam);
    updateExpParameters();

    return err_check;
}

int iXonCamera::getGainRange()
{
    int min, max;
    int err_check = GetEMGainRange(&min, &max);
    if(err_check == DRV_SUCCESS)
    {
        m_rdParam.minEMGain = min;
        m_rdParam.maxEMGain = max;
        emit rdParamChanged(m_rdParam);
    }
    return err_check;
}

int iXonCamera::getGainValue()
{
    int gain;
    int err_check = GetEMCCDGain(&gain);
    if(err_check == DRV_SUCCESS)
    {
        m_rdParam.curEMGain = gain;
    }

    return err_check;
}

int iXonCamera::getGainAdvanced()
{
    bool enabled;
    int state;
    int err_check = GetEMAdvanced(&state);
    enabled = state;
    if(err_check == DRV_SUCCESS)
    {
        m_rdParam.EMAdvanced = enabled;
    }
    return err_check;
}

int iXonCamera::setEMGain(int gain)
{
    int err_check = getGainRange();
    if(err_check != DRV_SUCCESS) return err_check;

    if(gain > m_rdParam.maxEMGain) gain = m_rdParam.maxEMGain;
    else if(gain < m_rdParam.minEMGain) gain = m_rdParam.minEMGain;

    err_check = SetEMCCDGain(gain);
    if(err_check == DRV_SUCCESS)
    {
        m_rdParam.curEMGain = gain;
        emit rdParamChanged(m_rdParam);
    }

    emit updateCountModes();
    return err_check;
}

int iXonCamera::setEMGainMode(int mode) // will always set REALGAIN (mode == 3) which is the only useful option
{
    if(mode < 0 || mode > 3) return DRV_P1INVALID;

    //int err_check = SetEMGainMode(mode);
    int realGain = 3;
    int err_check = SetEMGainMode(realGain); //will always set READGAIN

    if(err_check == DRV_SUCCESS)
    {
        m_rdParam.EMGainMode = realGain; //will set REALGAIN
        getGainRange(); //emits the signal
    }

    emit updateCountModes();
    return err_check;
}

int iXonCamera::setEMGainAdvanced(bool enabled)
{
    int err_check = SetEMAdvanced(enabled);
    if(err_check == DRV_SUCCESS)
    {
        m_rdParam.EMAdvanced = enabled;
        getGainRange(); //emits the signal
    }

    emit updateCountModes();
    return err_check;
}

int iXonCamera::setBaseClamp(bool enabled)
{
    int err_check = SetBaselineClamp(enabled);
    if(err_check == DRV_SUCCESS) m_rdParam.baseClamp = enabled;

    emit updateCountModes();
    emit rdParamChanged(m_rdParam);
    return err_check;
}

int iXonCamera::setCosmicFilter(bool enabled)
{
    int on;
    if(enabled) on = 2;
    else on = 0;

    int err_check = SetFilterMode(on);
    if(err_check == DRV_SUCCESS) m_rdParam.cosmicRayFilterEnabled = enabled;
    updateExpParameters();
    return err_check;
}

void iXonCamera::loadReadout(struct readoutParameter rP)
{
    this->blockSignals(true);
    retrieveVSSpeeds();
    retrieveHSSpeeds();

    getGainRange();
    if(setVSSpeed(rP.numVSSpeed) != DRV_SUCCESS) setVSSpeed(m_defaultRd.numVSSpeed);
    if(setHSSpeed(rP.numHSSpeed) != DRV_SUCCESS) setHSSpeed(m_defaultRd.numHSSpeed);
    if(setVSVolt(rP.numVSVolt) != DRV_SUCCESS) setVSVolt(m_defaultRd.numVSVolt);
    if(setPAGain(rP.numPAGain) != DRV_SUCCESS) setPAGain(m_defaultRd.numPAGain);
    if(setEMGain(rP.curEMGain) != DRV_SUCCESS) setEMGain(m_defaultRd.curEMGain);
    if(setEMenabled(rP.EMEnabled) != DRV_SUCCESS) setEMenabled(m_defaultRd.EMEnabled);
    if(setEMGainMode(3) != DRV_SUCCESS) setEMGainMode(3); // will always load REALGAIN since this is the only useful choice
    if(setEMGainAdvanced(rP.EMAdvanced) != DRV_SUCCESS) setEMGainAdvanced(m_defaultRd.EMAdvanced);
    if(setBaseClamp(rP.baseClamp) != DRV_SUCCESS) setBaseClamp(m_defaultRd.baseClamp);
    if(setCosmicFilter(rP.cosmicRayFilterEnabled) != DRV_SUCCESS) setCosmicFilter(m_defaultRd.cosmicRayFilterEnabled);
    this->blockSignals(false);

    emit rdParamChanged(m_rdParam);
    emit ROIParamChanged(m_ROIParam);
    updateExpParameters();
}

/* COUNT MODES */
int iXonCamera::setCountMode(int mode) //0...ADC units, 1...electrons, 2...photons
{
    setPhotonCountingEnabled(false);

    int err_check;
    if(mode >= 0 && mode <= 2)
    {
        err_check = SetCountConvertMode(mode);
        if(err_check == DRV_SUCCESS) m_countMode = mode;
    }
    emit countModeChanged(m_countMode);
    return err_check;
}

bool iXonCamera::isCountAvailable(int mode)
{
    int err_check = IsCountConvertModeAvailable(mode);
    if(err_check == DRV_SUCCESS) return true;
    else return false;
}

int iXonCamera::setCountWavelength(float lambda)
{
    int err_check = SetCountConvertWavelength(lambda);
    if(err_check == DRV_SUCCESS) m_countLambda = lambda;
    emit updateCountModes();
    return err_check;
}

void iXonCamera::resetCountMode()
{
    if(!isCountAvailable(m_countMode)) setCountMode(0);
    return;
}

int iXonCamera::setPhotonCountingEnabled(bool enabled) // try if count mode has to be set to ADC manually
{
    int err_check = SetPhotonCounting(enabled);
    if(err_check == DRV_SUCCESS)
    {
        m_photonCounting = enabled;
        emit photonCountingEnabled(enabled);
    }
    return err_check;
}

int iXonCamera::setPhotonCountingRanges(QVector<long> ranges)
{
    ulong noDiv;
    QVector<long> tempRanges = ranges;
    int err_check_final, err_check1, err_check2;
    err_check1 = GetNumberPhotonCountingDivisions(&noDiv);
    if(ranges.size() > noDiv + 1  || err_check1 != DRV_SUCCESS) return DRV_NOT_AVAILABLE;

    switch(ranges.size())
    {
    case 0:
    case 1:
        err_check1 = DRV_NOT_AVAILABLE;
        break;
    case 2:
        err_check1 = SetPhotonCountingThreshold(tempRanges[0], tempRanges[1]);
        err_check2 = SetPhotonCountingDivisions(1, &(tempRanges[0]));
        break;
    default:
        err_check1 = SetPhotonCountingThreshold(tempRanges.first(), tempRanges.last());
        tempRanges.pop_back();
        tempRanges.pop_front();
        err_check2 = SetPhotonCountingDivisions(tempRanges.size(), tempRanges.data());
        break;
    }
    if(err_check1 == DRV_SUCCESS && err_check2 == DRV_SUCCESS) err_check_final = DRV_SUCCESS;
    else err_check_final = DRV_NOT_AVAILABLE;

    if(err_check_final == DRV_SUCCESS)   m_photonCountDivisions = ranges;
    emit photonCountDivisionsChanged(m_photonCountDivisions);
    return err_check_final;
}

int iXonCamera::photonCountingMaxDivisions()
{
    ulong noDiv;
    int err_check = GetNumberPhotonCountingDivisions(&noDiv);
    if(err_check == DRV_SUCCESS) return noDiv;
    else return -1;
}

/* ACQUISITION PARAMETERS*/
void iXonCamera::loadAcquisition(struct acquisitionParameter aP)
{
    this->blockSignals(true);
    if(setAcqMode(aP.acqMode) != DRV_SUCCESS) setAcqMode(m_defaultAcq.acqMode);
    if(setFTMode(aP.FTenabled) != DRV_SUCCESS) setFTMode(m_defaultAcq.FTenabled);
    if(setExpTime(aP.exposureTime) != DRV_SUCCESS) setExpTime(m_defaultAcq.exposureTime);
    if(setAccNumber(aP.accNumber) != DRV_SUCCESS) setAccNumber(m_defaultAcq.accNumber);
    if(setKinNumber(aP.kinNumber) != DRV_SUCCESS) setKinNumber(m_defaultAcq.kinNumber);
    if(setAccCycleTime(aP.accCycleTime) != DRV_SUCCESS) setAccCycleTime(m_defaultAcq.accCycleTime);
    if(setKinCycleTime(aP.kinCycleTime) != DRV_SUCCESS) setKinCycleTime(m_defaultAcq.kinCycleTime);
    if(setTrMode(aP.trMode) != DRV_SUCCESS) setTrMode(m_defaultAcq.trMode);
    this->blockSignals(false);

    updateExpParameters();
}

int iXonCamera::setAcqMode(int mode)
{
    this->blockSignals(true);
    int err_check;
    if(mode == ACQ_MODE_SINGLE || mode == ACQ_MODE_FAST_KINETIC) setFTMode(false);

    if(mode == ACQ_MODE_FAST_KINETIC && m_ROIParam.rdMode != RD_MODE_FVB) //sets rd mode to fvb or image in FK mode
    {
        setRdMode(RD_MODE_IMAGE);
    }

    if(mode != ACQ_MODE_ACCUMULATE && mode != ACQ_MODE_KINETIC) setCosmicFilter(false);

    err_check = SetAcquisitionMode(mode);
    if (err_check == DRV_SUCCESS)
    {
        if(m_acqParam.acqMode == ACQ_MODE_FAST_KINETIC)  //if previous acqmode was fast kin, reset the rdMode (and set the acqMode correctly before)
        {
            m_acqParam.acqMode = static_cast<acquisitionMode>(mode);
            setRdMode(m_ROIParam.rdMode);
        }
        else m_acqParam.acqMode = static_cast<acquisitionMode>(mode);

        if(!isTrAvailable(m_acqParam.trMode)) setTrMode(TR_MODE_INTERNAL); //trmode software not compatible with anything but run till abort
    }

    this->blockSignals(false);

    updateExpParameters();
    emit rdParamChanged(m_rdParam);
    emit ROIParamChanged(m_ROIParam);

    return err_check;
}

int iXonCamera::setExpTime(float time) //Set exposure to neares possible value, sets m_exposureTime to actual exposure, which depends also on accCycleTime & kinCycletime
{                                      //Emits the exposureTimeChanged Signal
    int err_check, err_check2;
    float maxExp;
    if(m_acqParam.acqMode != ACQ_MODE_FAST_KINETIC)
    {
        err_check2 =  GetMaximumExposure(&maxExp); //maxExp is in secs

        err_check = SetExposureTime(time*TIMEUNIT); //converts time from TIMEUNIT (ms) to secs
        if(err_check != DRV_SUCCESS && err_check2 == DRV_SUCCESS && time*TIMEUNIT > maxExp)
            err_check = SetExposureTime(maxExp);
    }
    else
    {
        err_check = setFKExp(time);
    }
    updateExpParameters();
    return err_check;
}

int iXonCamera::setAccCycleTime(float time)
{
    int err_check;

    err_check = SetAccumulationCycleTime(time*TIMEUNIT); //converts time in seconds to TIMEUNIT
    updateExpParameters();
    return err_check;
}

int iXonCamera::setKinCycleTime(float time)
{
    int err_check;
    if(m_acqParam.acqMode == ACQ_MODE_KINETIC || m_acqParam.acqMode == ACQ_MODE_FAST_KINETIC) //THIS SOLVES A REALLY WEIRD ISSUE, WHERE THE KINCYCLETIME IS ALWAYS SET TO Accnum*acctime
    {
        time /= m_acqParam.accNumber;
    }
    err_check = SetKineticCycleTime(time*TIMEUNIT); //converts time in seconds to TIMEUNIT
    updateExpParameters();
    return err_check;
}

int iXonCamera::setFTMode(bool on)
{
    int err_check;
    err_check = SetFrameTransferMode(on);
    if(err_check == DRV_SUCCESS)
    {
        m_acqParam.FTenabled = on;
    }
    if(!m_acqParam.FTenabled) setCrop(false);
    updateExpParameters();

    return err_check;
}

int iXonCamera::setAccNumber(int num)
{
    int err_check;
    err_check = SetNumberAccumulations(num);
    if(err_check == DRV_SUCCESS)
    {
        m_acqParam.accNumber = num;
        if(m_acqParam.acqMode == ACQ_MODE_KINETIC || m_acqParam.acqMode == ACQ_MODE_FAST_KINETIC) setKinCycleTime(m_acqParam.kinCycleTime); //reset kincycle to avoid mulitlpying it with accnum
    }
    updateExpParameters();
    return err_check;
}

int iXonCamera::setKinNumber(int num)
{
    int err_check;
    err_check = SetNumberKinetics(num);
    if(err_check == DRV_SUCCESS) m_acqParam.kinNumber = num;
    updateExpParameters();
    return err_check;
}

int iXonCamera::updateExpParameters()
{
    int err_check;
    float exp, acc, kin, FKExp;
    err_check = GetAcquisitionTimings(&exp, &acc, &kin);
    if(err_check == DRV_SUCCESS)
    {
        m_acqParam.exposureTime = exp / TIMEUNIT;
        m_acqParam.accCycleTime = acc / TIMEUNIT;
        m_acqParam.kinCycleTime = kin / TIMEUNIT;
    }
    err_check = GetFKExposureTime(&FKExp);
    if(err_check == DRV_SUCCESS) m_acqParam.FKexpTime = FKExp / TIMEUNIT;

    emit acqParamChanged(m_acqParam);
    return err_check;
}

int iXonCamera::setFKParam(int rdMode, int rows, int offset, int numKin, float expTime,  int hBin, int vBin)
{
    int err_check;
    if(rows < 1) rows = m_ROIParam.FKHeight;
    if(offset < 1) offset = m_ROIParam.FKOffset;
    if(numKin < 1) numKin = m_acqParam.kinNumber;
    if(expTime < 0) expTime = m_acqParam.FKexpTime;
    if(hBin < 1) hBin = m_ROIParam.FKHorBin;
    if(vBin < 1) vBin = m_ROIParam.FKVertBin;
    if(rdMode == RD_MODE_FVB || rdMode == RD_MODE_IMAGE)
    {
        err_check = SetFastKineticsEx(rows, numKin, expTime * TIMEUNIT, rdMode, hBin, vBin, offset);
        if(err_check == DRV_SUCCESS)
        {
            m_ROIParam.rdMode = static_cast<readoutMode>(rdMode);
            m_ROIParam.FKHeight = rows;
            m_ROIParam.FKOffset = offset;
            m_ROIParam.FKHorBin = hBin;
            m_ROIParam.FKVertBin = vBin;

            m_acqParam.kinNumber = numKin;
            m_acqParam.FKexpTime = expTime;
        }
    }
    else err_check = DRV_NOT_AVAILABLE;

    emit rdParamChanged(m_rdParam);
    emit ROIParamChanged(m_ROIParam);
    updateExpParameters();

    qDebug() << "FK SET" << err_check;
    return err_check;
}
int iXonCamera::setFKRdMode(int rdMode)
{
    return setFKParam(rdMode, m_ROIParam.FKHeight, m_ROIParam.FKOffset, m_acqParam.kinNumber, m_acqParam.FKexpTime, m_ROIParam.FKHorBin, m_ROIParam.FKVertBin);
}
int iXonCamera::setFKExp(float exp)
{
    return setFKParam(m_ROIParam.rdMode, m_ROIParam.FKHeight, m_ROIParam.FKOffset, m_acqParam.kinNumber, exp, m_ROIParam.FKHorBin, m_ROIParam.FKVertBin);
}
int iXonCamera::setFKrows(int rows)
{
    return setFKParam(m_ROIParam.rdMode, rows, m_ROIParam.FKOffset, m_acqParam.kinNumber, m_acqParam.FKexpTime, m_ROIParam.FKHorBin, m_ROIParam.FKVertBin);

}
int iXonCamera::setFKoffset(int offset)
{
    return setFKParam(m_ROIParam.rdMode, m_ROIParam.FKHeight, offset, m_acqParam.kinNumber, m_acqParam.FKexpTime, m_ROIParam.FKHorBin, m_ROIParam.FKVertBin);
}
int iXonCamera::setFKHBin(int hBin)
{
    return setFKParam(m_ROIParam.rdMode, m_ROIParam.FKHeight, m_ROIParam.FKOffset, m_acqParam.kinNumber, m_acqParam.FKexpTime, hBin, m_ROIParam.FKVertBin);
}
int iXonCamera::setFKVBin(int vBin)
{
    return setFKParam(m_ROIParam.rdMode, m_ROIParam.FKHeight, m_ROIParam.FKOffset, m_acqParam.kinNumber, m_acqParam.FKexpTime, m_ROIParam.FKHorBin, vBin);
}

int iXonCamera::setTrMode(triggerMode mode) //ugly
{
    int err_check;
    if(!isTrAvailable(mode))
    {
        emit acqParamChanged(m_acqParam);
        return err_check;
    }

    if(mode == TR_MODE_EXTERNAL_FAST)
    {
        err_check = SetFastExtTrigger(true);
        if(err_check != DRV_SUCCESS)
        {
            emit acqParamChanged(m_acqParam);
            return err_check;
        }
        err_check = SetTriggerMode(TR_MODE_EXTERNAL);
        if(err_check == DRV_SUCCESS) m_acqParam.trMode = static_cast<triggerMode>(mode);
    }
    else
    {
        err_check = SetFastExtTrigger(false);
        if(err_check != DRV_SUCCESS)
        {
            emit acqParamChanged(m_acqParam);
            return err_check;
        }

        err_check = SetTriggerMode(mode);
        if(err_check == DRV_SUCCESS) m_acqParam.trMode = static_cast<triggerMode>(mode);
    }


    emit acqParamChanged(m_acqParam);
    updateExpParameters();
    return err_check;
}

bool iXonCamera::isTrAvailable(triggerMode trMode)
{
    if(trMode == TR_MODE_EXTERNAL_FAST) trMode = TR_MODE_EXTERNAL;
    int isAvailable = IsTriggerModeAvailable(trMode); // isAvailable == DRV_SUCCESS means yes, == DRV_INVALID_MODE means no
    if(isAvailable == DRV_SUCCESS) return true;
    else return false;
}

iXonCamera::triggerMode iXonCamera::convertToTrIndex(int index)
{
    switch(index)
    {
    case 0:
         return TR_MODE_INTERNAL;
    case 1:
         return TR_MODE_EXTERNAL;
    case 2:
        return TR_MODE_EXTERNAL_FAST;
    case 3:
         return TR_MODE_EXTERNAL_START;
    case 4:
         return TR_MODE_BULB;
    case 5:
         return TR_MODE_SOFTWARE;
    default:
        return TR_MODE_INVALID;
    }
}

int iXonCamera::convertFromTrIndex(triggerMode trMode)
{
    switch(trMode)
    {
    case TR_MODE_INTERNAL:
        return 0;
    case TR_MODE_EXTERNAL:
        return 1;
    case TR_MODE_EXTERNAL_FAST:
        return 2;
    case TR_MODE_EXTERNAL_START:
        return 3;
    case TR_MODE_BULB:
        return 4;
    case TR_MODE_SOFTWARE:
        return 5;
    default:
        return -1;
    }
}


/* SHUTTER PARAMETERS */
int iXonCamera::loadShutter(struct shutterParameter sP)
{
    int err_check = SetShutterEx(sP.TTLHigh, sP.shMode, sP.closeTime, sP.openTime, sP.shExtMode);
    if(err_check == DRV_SUCCESS)
    {
        m_shParam = sP;
        emit shParamChanged(m_shParam);
        updateExpParameters();
    }
    else if(err_check == DRV_P3INVALID && sP.closeTime < SHUTTER_TIME_ERROR_RANGE)
    {
        int closeMin, temp, err_check2;
        err_check2 = GetShutterMinTimes(&closeMin, &temp); // could go into infinite loop if this function returns invalid time
        if(err_check2 == DRV_SUCCESS)
        {
            struct shutterParameter newParm = sP;
            newParm.closeTime = closeMin;
            err_check2 = loadShutter(newParm);
        }
    }
    else if(err_check == DRV_P4INVALID && sP.openTime < SHUTTER_TIME_ERROR_RANGE)
    {
        int openMin, temp, err_check2;
        err_check2 = GetShutterMinTimes(&temp, &openMin); // could go into infinite loop if this function returns invalid time
        if(err_check2 == DRV_SUCCESS)
        {
            struct shutterParameter newParm = sP;
            newParm.openTime = openMin;
            err_check2 = loadShutter(newParm);
        }
    }
    return err_check;
}


/* DEFAULTS */
void iXonCamera::setDefaults()
{
    m_defaultAcq = {ACQ_MODE_SINGLE,    //acqMode
                    500,                //expTime
                    500,                //accCycle
                    500,                //kinCycle
                    1,                  //accNum
                    1,                  //kinNum
                    false,              //FTEnabled
                    10,                 //FKexpTime
                    TR_MODE_INTERNAL    //TRMODE
                   };

    m_defaultRd = {{0},                 //availVSSpeeds
                   {0},                 //availHSSpeeds
                   0,                   //numVsSpeeds
                   0,                   //numHSSpeeds
                   0,                   //numFastestRecVSSpeed
                   0,                   //VSSpeed
                   0,                   //HSSpeed
                   0,                   //numVsVolt
                   {0},                 //availPAGains
                   0,                   //numPAGain
                   0,                   //PAGain
                   false,               //EMEnabled
                   0,                   //minEMGain
                   0,                   //maxEMGain
                   0,                   //curEMGain
                   false,               //EMAdvanced
                   0,                   //EMGainMode
                   0,                   //baseClamp
                   false,               //CosmicRayFilter
                  };

    m_defaultROI = {RD_MODE_FVB,        //rdMode
                    1,                  //horBin
                    1,                  //imgHorB
                    1,                  //imgVerBin
                    1,                  //STHeigt
                    512,                //STCenter
                    1,                  //MTNum
                    1,                  //MTOffset
                    1,                  //MTHeight
                    1,                  //MTFirstRow
                    1,                  //CTNumber
                    {},                 //CTSettings
                    1,                  //imgXS
                    1024,               //imgXE
                    1,                  //imgYS
                    1024,               //imgYE
                    1,                  //FKHeight
                    1,                  //FKOffset
                    1,                  //FKHorBin
                    1,                  //FKVertBin
                    false               //CropEnabled
                   };

    m_defaultSh = {true,                //TTLHigh
                   SH_MODE_PERMCLOSED,  //SHMode
                   27,                  //shOpen
                   27,                  //shClose
                   SH_MODE_PERMCLOSED}; //SHModeExt
    return;
}

void iXonCamera::loadHardcodedDefaults()
{
    m_ROIParam = m_defaultROI; //this is to initialize all ROIParam to uncritical settings, since (especially in FK mode) the program can crash if invalid Regions are loaded
                               //this does of course not change any settings in the camera itself, for this loadROI is called.
                               //It simply initializes those ROI settings which are not used for the current Read Mode, and thus not touched by loadROI()
                               //At no other place in the program should a camera Parameter struct be modified directly, since then the program will no longer have acurate records of the actual camera settings

    loadShutter(m_defaultSh);
    loadReadout(m_defaultRd);
    loadROI(m_defaultROI);
    loadAcquisition(m_defaultAcq);

    setCountMode(0);
    setCountWavelength(670);
    setPhotonCountingEnabled(false);

    return;
}
