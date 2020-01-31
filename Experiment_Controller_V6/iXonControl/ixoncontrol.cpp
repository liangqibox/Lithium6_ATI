#include "ixoncontrol.h"
#include "ui_ixoncontrol.h"

#define MINSAFETEMP -20
#define STATUS_BAR_MESSAGE_TIMEOUT 2000

iXonControl::iXonControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::iXonControl)
{
    ui->setupUi(this);
    startPbText = "Start";

    camera = new iXonCamera;
    m_cameraInitError = false;
    if(camera->initError()) //Aborts Initialization if camera is not initialized
    {
        m_cameraInitError = true;
        return;
    }
    connect(camera, SIGNAL(sendMessage(QString)), this, SLOT(showMessage(QString)));
    connect(this, SIGNAL(messageTransfer(QString)), camera, SLOT(receiveExperimentControllerMessage(QString)));


    /*SETS UP THE DATA THREAD */
    dataThread = new QThread;
    camera->dataHandler()->moveToThread(dataThread);
    //connect(dataThread, SIGNAL(finished()), camera->dataHandler(), SLOT(deleteLater()));
    //connect(camera->dataHandler(), SIGNAL(taskFinished()), dataThread, SLOT(quit()));
    //connect(dataThread, SIGNAL(finished()), dataThread, SLOT(deleteLater()));

    dataThread->start(QThread::NormalPriority);

    connect(camera->dataHandler(), SIGNAL(saveModeChanged(int)), this, SLOT(showSaveMode(int)));
    connect(camera->dataHandler(), SIGNAL(filepathChanged()), this, SLOT(showFilepath()));
    connect(camera->dataHandler(), SIGNAL(frameSaved(QString)), this, SLOT(loadVideoPicture(QString)));


    /*SETS UP THE ACQMONITOR THREAD */
    acqMonitorThread = new QThread;
    camera->acqMonitor()->moveToThread(acqMonitorThread);
    //connect(acqMonitorThread, SIGNAL(finished()), camera->acqMonitor(), SLOT(deleteLater()));
    //connect(camera->dataHandler(), SIGNAL(taskFinished()), dataThread, SLOT(quit()));
    //connect(acqMonitorThread, SIGNAL(finished()), acqMonitorThread, SLOT(deleteLater()));

    acqMonitorThread->start(QThread::HighPriority);


    /*SETS UP THE TEMPERATURE THREAD*/
    tempThread = new QThread;
    camera->tempControl()->moveToThread(tempThread);
    connect(tempThread, SIGNAL(finished()), camera->tempControl(), SLOT(setOverheatQuitEvent()));
    //connect(tempThread, SIGNAL(finished()), tempThread, SLOT(deleteLater()));

    connect(camera->tempControl(), SIGNAL(curTempChanged(int, int)), this, SLOT(showCurTemp(int, int)));
    connect(camera->tempControl(), SIGNAL(targetTempChanged(int)), this, SLOT(showTargetTemp(int)));
    connect(camera->tempControl(), SIGNAL(coolerEnabledChanged(bool)), this, SLOT(showCoolerEnabled(bool)));
    connect(camera->tempControl(), SIGNAL(tempRangeChanged(int, int)), this, SLOT(showTempRange(int,int)));
    connect(this, SIGNAL(setupTemperatureController()), camera->tempControl(), SLOT(initTemperatureController()));

    connect(camera->tempControl(), SIGNAL(fanSpeedChanged(int)), this, SLOT(showFanSpeed(int)));

    tempThread->start(QThread::NormalPriority);
    camera->tempControl()->setFanSpeed(TemperatureController::FAN_HIGH);


    /*SETS UP OVERHEAT MONITOR THREAD*/
    overheatThread = new QThread;
    camera->tempControl()->TECMonitor()->moveToThread(overheatThread);
    //connect(overheatThread, SIGNAL(finished()), camera->tempControl()->TECMonitor(), SLOT(deleteLater()));
    //connect(overheatThread, SIGNAL(finished()), overheatThread, SLOT(deleteLater()));
    connect(camera->tempControl()->TECMonitor(), SIGNAL(overheatWarning()), this, SLOT(showOverheatWarning()));

    overheatThread->start(QThread::LowPriority);


    /*CONNECTS SIGNALS FOR ACQ/RD/ROI/TR PARAMETERS */
    connect(camera, SIGNAL(acqParamChanged(struct iXonCamera::acquisitionParameter)), this, SLOT(showAcqParam(struct iXonCamera::acquisitionParameter)));
    connect(camera, SIGNAL(rdParamChanged(struct iXonCamera::readoutParameter)), this, SLOT(showRdParam(struct iXonCamera::readoutParameter)));
    connect(camera, SIGNAL(ROIParamChanged(struct iXonCamera::ROIParameter)), this, SLOT(showROIParam(struct iXonCamera::ROIParameter)));
    connect(camera, SIGNAL(shParamChanged(struct iXonCamera::shutterParameter)), this, SLOT(showShParam(struct iXonCamera::shutterParameter)));
    connect(camera, SIGNAL(updateCountModes()), this, SLOT(manageCountModes()));
    connect(camera, SIGNAL(countModeChanged(int)), this, SLOT(showCountMode(int)));


    /* SETS UP WINDOWS */
    liveLabel = new QLabel(0);

    RTInt = new RandomTrackInterface(this);
    connect(RTInt, SIGNAL(sendSettings(QVector<int>)), camera, SLOT(setRTPosition(QVector<int>)));
    connect(camera, SIGNAL(RTListChanged(QVector<int>)), RTInt, SLOT(setTableText(QVector<int>)));

    ROIPre = new ROIPreview(this);
    connect(ROIPre, SIGNAL(sendImgPos(int,int,int,int)), this, SLOT(fillROIPos(int,int,int,int)));


    /* CONNECTS SIGNAL FOR ACQ STATUS */
    connect(camera, SIGNAL(acquiringStatusChanged(bool)), this, SLOT(manageAcqStartPbText(bool)));


    /* STARTS TEMPERATURE MONITOR */
    emit setupTemperatureController();
    emit camera->tempControl()->startOverheatMonitor();


    /*LOADS SETTINGS*/
    defaultPath = QDir::toNativeSeparators(QDir::currentPath());
    loadSettings(true, true, true, true, true, defaultPath + QDir::separator() + "defaultSettings.ixoncfg");

    children = ui->centralWidget->findChildren<QWidget *>();
    childWasDisabled = new bool[children.size()];
}

iXonControl::~iXonControl()
{
    delete[] childWasDisabled;
    if(m_cameraInitError)
    {
        delete camera;
        delete ui;
        return;
    }

    dataThread->quit();
    dataThread->wait();

    tempThread->quit();
    tempThread->wait();

    acqMonitorThread->quit();
    acqMonitorThread->wait();

    overheatThread->quit(); //TEMPTHREAD MUST BE EXITED BEFORE CLOSING THIS THREAD, SINCE TEMPTHREAD WILL EMIT THE REQUIRED STOP EVENT
    overheatThread->wait();


    dataThread->deleteLater();
    tempThread->deleteLater();
    overheatThread->deleteLater();
    acqMonitorThread->deleteLater();


    delete liveLabel;
    delete RTInt;
    delete ROIPre;

    delete camera;
    delete ui;
}

void iXonControl::processExperimentControllerMessage(QString message)
{
    emit messageTransfer(message);
}

/* SYSTEM CONTROL */
void iXonControl::closeEvent(QCloseEvent *event) //protects against closing when temperature is too low or saving is in progress
{
    int status;
    bool accept_temp, accept_save;
    accept_temp = true;
    accept_save = true;

    int temp = camera->tempControl()->currentTemperature(&status); //if there is an error, app just closes and hopes for the best
    if(status == DRV_NOT_INITIALIZED || status == DRV_ERROR_ACK)
    {
        event->accept();
        return;
    }
    else if(temp < MINSAFETEMP)
    {
        QMessageBox tempErrBox;
        tempErrBox.setText("Warning: Sensor Temperature too low for safe exit!");
        tempErrBox.setInformativeText("Exiting now may cause the temperature to rise too quickly.\n"
                                      "Set temperature to above " +QString::number(MINSAFETEMP)+"°C to exit safely.");
        tempErrBox.setIcon(QMessageBox::Warning);
        tempErrBox.setStandardButtons(QMessageBox::Ignore | QMessageBox::Cancel);
        tempErrBox.setDefaultButton(QMessageBox::Cancel);
        int ret = tempErrBox.exec();

        switch(ret)
        {
        case QMessageBox::Ignore:
            accept_temp = true;
            break;
        case QMessageBox::Cancel:
            accept_temp = false;
            break;
        }
    }


    if(camera->dataHandler()->savingInProgress())
    {
        QMessageBox saveErrBox;
        saveErrBox.setText("Warning: Saving is currently in progress!");
        saveErrBox.setInformativeText("Exiting now will abort the saving process and data will be lost.");
        saveErrBox.setIcon(QMessageBox::Critical);
        saveErrBox.setStandardButtons(QMessageBox::Ignore | QMessageBox::Cancel);
        saveErrBox.setDefaultButton(QMessageBox::Cancel);
        int ret = saveErrBox.exec();

        switch(ret)
        {
        case QMessageBox::Ignore:
            accept_save = true;
            camera->abortAcquiring();
            break;
        case QMessageBox::Cancel:
            accept_save = false;
            break;
        }
    }

    if(camera->isAcquiring())
    {
        QMessageBox acqErrBox;
        acqErrBox.setText("Warning: An acquisition is currently in progress!");
        acqErrBox.setInformativeText("Exiting now will abort the acquisition and data will be lost.");
        acqErrBox.setIcon(QMessageBox::Critical);
        acqErrBox.setStandardButtons(QMessageBox::Ignore | QMessageBox::Cancel);
        acqErrBox.setDefaultButton(QMessageBox::Cancel);
        int ret = acqErrBox.exec();

        switch(ret)
        {
        case QMessageBox::Ignore:
            accept_save = true;
            camera->abortAcquiring();
            break;
        case QMessageBox::Cancel:
            accept_save = false;
            break;
        }
    }

    if(accept_temp && accept_save) event->accept();
    else event->ignore();

    return;
}

void iXonControl::showMessage(QString message)
{
//    this->statusBar()->showMessage(message, STATUS_BAR_MESSAGE_TIMEOUT);
}

/* ACQUISITION CONTROL */
void iXonControl::on_pB_startAcq_clicked()
{
    if(!camera->isAcquiring())
    {
        ui->pB_applyROI->click(); //for now to
        camera->startAcquiring();

    }
    else if(camera->isAcquiring())
    {
        camera->abortAcquiring();
    }
}

void iXonControl::manageAcqStartPbText(bool isAcquiring)
{

    if(isAcquiring)
    {
        ui->pB_startAcq->setText("Abort");

        int i  = 0;
        for(QWidget *x : children) //C++ 11 fancyness
        {
            if (x->isEnabled())
            {
                childWasDisabled[i] = false;
                x->setDisabled(true);
            }
            else childWasDisabled[i] = true;
            i++;
        }
    }

    else if(!isAcquiring)
    {
        ui->pB_startAcq->setText(startPbText);

        int i  = 0;
        for(QWidget *x : children) //C++ 11 fancyness
        {
            if (!childWasDisabled[i])
            {
                x->setDisabled(false);
            }
            childWasDisabled[i] = false;
            i++;
        }
    }

    //THESE ARE ALWAYS ENABLED
    ui->gB_acq->setDisabled(false);
    ui->pB_startAcq->setDisabled(false);

}

void iXonControl::on_cB_aqcMode_activated(int index)
{
    if(index > 4 || index < 0) return;
    camera->setAcqMode(index + 1); //valid modes range from 1 to 5, index runs from 0 to 4
    return;
}

void iXonControl::on_lE_exposure_editingFinished()
{
    bool ok;
    float time = ui->lE_exposure->text().toFloat(&ok);

    if(ok) camera->setExpTime(time);
    else ui->lE_exposure->setText(QString::number(camera->acqParam().exposureTime));

    return;
}

void iXonControl::on_lE_accCycle_editingFinished()
{
    bool ok;
    float time = ui->lE_accCycle->text().toFloat(&ok);

    if(ok) camera->setAccCycleTime(time);
    else ui->lE_accCycle->setText(QString::number(camera->acqParam().accCycleTime));

    return;
}

void iXonControl::on_lE_kinCycle_editingFinished()
{
    bool ok;
    float time = ui->lE_kinCycle->text().toFloat(&ok);

    if(ok) camera->setKinCycleTime(time);
    else ui->lE_kinCycle->setText(QString::number(camera->acqParam().kinCycleTime));

    return;
}

void iXonControl::on_chB_FT_toggled(bool checked)
{
    camera->setFTMode(checked);
}

void iXonControl::on_lE_accNumber_editingFinished()
{
    bool ok;
    int num = ui->lE_accNumber->text().toInt(&ok);

    if(ok) camera->setAccNumber(num);
    else ui->lE_accNumber->setText(QString::number(camera->acqParam().accNumber));

    return;
}

void iXonControl::on_lE_kinNumber_editingFinished()
{
    bool ok;
    int num = ui->lE_kinNumber->text().toInt(&ok);

    if(ok) camera->setKinNumber(num);
    else ui->lE_kinNumber->setText(QString::number(camera->acqParam().kinNumber));

    return;
}

void iXonControl::on_chB_cosmic_clicked(bool checked)
{
    camera->setCosmicFilter(checked);
}

void iXonControl::showAcqParam(struct iXonCamera::acquisitionParameter aP)
{
    /* EXPOSURE LINE EDITS AND COMBO BOX */
    ui->cB_aqcMode->setCurrentIndex(aP.acqMode-1);

    QString expString;
    if(aP.acqMode != iXonCamera::ACQ_MODE_FAST_KINETIC) expString = QString::number(aP.exposureTime);
    else expString = QString::number(aP.FKexpTime);
    ui->lE_exposure->setText(expString);

    QString accString = QString::number(aP.accCycleTime);
    ui->lE_accCycle->setText(accString);

    QString kinString = QString::number(aP.kinCycleTime);
    ui->lE_kinCycle->setText(kinString);

    QString accNumString = QString::number(aP.accNumber);
    ui->lE_accNumber->setText(accNumString);

    QString kinNumString = QString::number(aP.kinNumber);
    ui->lE_kinNumber->setText(kinNumString);

    ui->chB_cosmic->setChecked(camera->rdParam().cosmicRayFilterEnabled);

    if(aP.trMode == iXonCamera::TR_MODE_INTERNAL) startPbText = "Start";
    else startPbText = "Arm";
    ui->pB_startAcq->setText(startPbText);

    manageAcqGrayout(aP);

    /* FT CHECKBOX */
    ui->chB_FT->setChecked(aP.FTenabled);

    /* TRIGGER */
    ui->cB_trMode->clear();
    if(camera->isTrAvailable(iXonCamera::TR_MODE_INTERNAL)) ui->cB_trMode->addItem("Internal");
    if(camera->isTrAvailable(iXonCamera::TR_MODE_EXTERNAL)) ui->cB_trMode->addItem("External");
    if(camera->isTrAvailable(iXonCamera::TR_MODE_EXTERNAL_FAST)) ui->cB_trMode->addItem("Fast External");
    if(camera->isTrAvailable(iXonCamera::TR_MODE_EXTERNAL_START)) ui->cB_trMode->addItem("External Start");
    if(camera->isTrAvailable(iXonCamera::TR_MODE_BULB)) ui->cB_trMode->addItem("Bulb");

    ui->cB_trMode->setCurrentIndex(camera->convertFromTrIndex(aP.trMode));

    /*LIVEVIEW*/
    if(aP.acqMode != iXonCamera::ACQ_MODE_RUN_TILL_ABORT)
    {
        ui->pB_liveView->setDisabled(true);
        ui->chB_saveVideo->setDisabled(true);
    }
    else
    {
        ui->pB_liveView->setDisabled(false);
        ui->chB_saveVideo->setDisabled(false);
    }



    return;
}

void iXonControl::manageAcqGrayout(struct iXonCamera::acquisitionParameter aP)
{
    bool extTrigger = camera->acqParam().trMode == iXonCamera::TR_MODE_EXTERNAL || camera->acqParam().trMode == iXonCamera::TR_MODE_EXTERNAL_FAST || camera->acqParam().trMode == iXonCamera::TR_MODE_BULB;


    switch(aP.acqMode)
    {
    case iXonCamera::ACQ_MODE_SINGLE:
    ui->lE_accCycle->setDisabled(true);
    ui->lE_accNumber->setDisabled(true);
    ui->lE_kinCycle->setDisabled(true);
    ui->lE_kinNumber->setDisabled(true);
    ui->chB_FT->setDisabled(true);
    ui->chB_cosmic->setDisabled(true);
    break;
    case iXonCamera::ACQ_MODE_ACCUMULATE:
    ui->lE_accNumber->setDisabled(false);
    ui->lE_kinCycle->setDisabled(true);
    ui->lE_kinNumber->setDisabled(true);


    if(extTrigger)
    {
        ui->lE_accCycle->setDisabled(true);
    }
    else
    {
        ui->lE_accCycle->setDisabled(false);
    }

    ui->chB_FT->setDisabled(false);

    if(aP.accNumber < 2) ui->chB_cosmic->setDisabled(true);
    else ui->chB_cosmic->setDisabled(false);

    break;
    case iXonCamera::ACQ_MODE_KINETIC:
    ui->lE_accNumber->setDisabled(false);
    ui->lE_accCycle->setDisabled(true);
    ui->lE_kinNumber->setDisabled(false);
    ui->chB_FT->setDisabled(false);
    if(extTrigger)
    {
        ui->lE_kinCycle->setDisabled(true);
    }
    else
    {
        ui->lE_kinCycle->setDisabled(false);
    }

    if(aP.accNumber < 2 && aP.kinNumber < 2) ui->chB_cosmic->setDisabled(true);
    else ui->chB_cosmic->setDisabled(false);

    break;
    case iXonCamera::ACQ_MODE_FAST_KINETIC:
    ui->lE_accCycle->setDisabled(true);
    ui->lE_accNumber->setDisabled(true);
    ui->lE_kinCycle->setDisabled(true);
    ui->lE_kinNumber->setDisabled(false);
    ui->chB_FT->setDisabled(true);
    ui->chB_cosmic->setDisabled(true);
    break;
    case iXonCamera::ACQ_MODE_RUN_TILL_ABORT:
    ui->lE_accCycle->setDisabled(true);
    ui->lE_accNumber->setDisabled(true);
    ui->lE_kinNumber->setDisabled(true);

    if(extTrigger)
    {
        ui->lE_kinCycle->setDisabled(true);
    }
    else
    {
        ui->lE_kinCycle->setDisabled(false);
    }

    ui->chB_FT->setDisabled(false);
    ui->chB_cosmic->setDisabled(true);
    default:
        break;
    }


    if(aP.FTenabled)
    {
        ui->lE_accCycle->setDisabled(true);
        ui->lE_kinCycle->setDisabled(true);
    }
    if(camera->ROIParam().rdMode == iXonCamera::RD_MODE_FVB || camera->ROIParam().rdMode == iXonCamera::RD_MODE_IMAGE) ui->chB_crop->setDisabled(!aP.FTenabled);

    if(camera->acqParam().trMode == iXonCamera::TR_MODE_BULB) ui->lE_exposure->setDisabled(true);
    else ui->lE_exposure->setDisabled(false);
}

void iXonControl::on_cB_trMode_activated(int index)
{
    camera->setTrMode(camera->convertToTrIndex(index));
}

void iXonControl::on_pB_liveView_clicked()
{
    if(!liveLabel->isVisible())
    {
        QImage img;
        img.load(QDir::toNativeSeparators(QDir::currentPath()) + QDir::separator() + "black_screen.PNG");
        liveLabel->setPixmap(QPixmap::fromImage(img));
        liveLabel->move(this->pos() + QPoint(500, 0));
        liveLabel->setWindowTitle("Live View");
        liveLabel->setFixedSize(img.size());
        liveLabel->show();
    }
    else liveLabel->hide();
}

void iXonControl::loadVideoPicture(QString filename)
{
    QImage img;
    img.load(filename);
    img = img.scaled(1024, 1024);
    liveLabel->setPixmap(QPixmap::fromImage(img));
    liveLabel->setFixedSize(img.size());
}

void iXonControl::on_chB_saveVideo_clicked(bool checked)
{
    camera->setSaveVideo(checked);
}


/* READOUT CONTROL */
void iXonControl::on_cB_rdMode_activated(int index)
{
    if(camera->acqParam().acqMode != iXonCamera::ACQ_MODE_FAST_KINETIC && !camera->ROIParam().cropEnabled)
    {
        if(index > 4 || index < 0) return;
        camera->setRdMode(index); //valid modes range from 0 to 4
    }
    else
    {
        if(index > 1 || index < 0) return;
        if(index == 0) camera->setRdMode(0); //FVB
        else if(index == 1) camera->setRdMode(4); //IMAGE
    }
    return;
}

void iXonControl::manageShiftSpeedLists()
{

    //refreshes the lists just in case
    //probably only HS need to be refreshed, however, this makes absolutely sure that only acually available settings are displayed
    int err_check;
    err_check = camera->retrieveHSSpeeds();
    if(err_check != DRV_SUCCESS) return;
    err_check = camera->retrieveVSSpeeds();
    if(err_check != DRV_SUCCESS) return;
    err_check = camera->retrievePAGains();
    if(err_check != DRV_SUCCESS) return;

    unsigned int numFastest = camera->rdParam().numFastestRecVSSpeed;

    ui->cB_VSSpeed->clear();
    for(unsigned int i =  0; i < camera->rdParam().availableVSSpeeds.size(); i++)
    {
        ui->cB_VSSpeed->addItem(QString::number(camera->rdParam().availableVSSpeeds[i]));

        if(i < numFastest) ui->cB_VSSpeed->setItemText(i, QString("[").append(ui->cB_VSSpeed->itemText(i)).append(QString("]")));
    }

    ui->cB_HSSpeed->clear();
    for(int i = camera->rdParam().availableHSSpeeds.size() - 1; i >= 0; i--) //the list is fetched with highest speed at index 0, we read it backwards here
    {
        ui->cB_HSSpeed->addItem(QString::number(camera->rdParam().availableHSSpeeds[i]));
    }

    ui->cB_PAGain->clear();
    for(unsigned int i = 0; i < camera->rdParam().availablePAGains.size(); i++)
    {
        ui->cB_PAGain->addItem(QString("Gain ").append(QString::number(camera->rdParam().availablePAGains[i])));
    }
}

void iXonControl::showRdParam(iXonCamera::readoutParameter rP)
{
    manageShiftSpeedLists();

    ui->cB_VSSpeed->setCurrentIndex(rP.numVSSpeed);
    ui->cB_HSSpeed->setCurrentIndex((rP.availableHSSpeeds.size() - 1) - rP.numHSSpeed);
    ui->cB_VVolt->setCurrentIndex(rP.numVSVolt);
    ui->rB_EMon->setChecked(rP.EMEnabled);
    ui->rB_EMoff->setChecked(!rP.EMEnabled);
    ui->cB_PAGain->setCurrentIndex(rP.numPAGain);


    ui->sB_EMGain->setMinimum(rP.minEMGain);
    ui->sB_EMGain->setMaximum(rP.maxEMGain);
    ui->sB_EMGain->setValue(rP.curEMGain);
    ui->chB_EMGainAdv->setChecked(rP.EMAdvanced);

    ui->chB_baseClamp->setChecked(rP.baseClamp);

    if(!rP.EMEnabled)
    {
        ui->chB_EMGainAdv->setDisabled(true);
        ui->sB_EMGain->setDisabled(true);
    }
    else
    {
        ui->chB_EMGainAdv->setDisabled(false);
        ui->sB_EMGain->setDisabled(false);
    }
}

void iXonControl::on_cB_VSSpeed_activated(int index)
{
    camera->setVSSpeed(index);
}

void iXonControl::on_cB_HSSpeed_activated(int index)
{
    camera->setHSSpeed((camera->rdParam().availableHSSpeeds.size() - 1) - index); //list is back to front in vector
}

void iXonControl::on_cB_PAGain_activated(int index)
{
    camera->setPAGain(index);
}

void iXonControl::on_rB_EMon_clicked(bool checked)
{
    camera->setEMenabled(checked);
}

void iXonControl::on_rB_EMoff_clicked(bool checked)
{
    camera->setEMenabled(!checked);
}

void iXonControl::on_cB_VVolt_activated(int index)
{
    camera->setVSVolt(index);
}


void iXonControl::on_sB_EMGain_editingFinished()
{
    int gain = ui->sB_EMGain->value();
    camera->setEMGain(gain);
}

void iXonControl::on_chB_EMGainAdv_clicked(bool checked)
{
    if(checked)
    {
        QMessageBox GainWarnBox;
        GainWarnBox.setText("Warning: With high EM gain, exposure to more than 10 photons/sec will harm the sensor.");
        GainWarnBox.setInformativeText("Do you want to continue?");
        GainWarnBox.setIcon(QMessageBox::Warning);
        GainWarnBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = GainWarnBox.exec();

        if(ret == QMessageBox::Yes) camera->setEMGainAdvanced(checked);
        else
        {
            camera->setEMGainAdvanced(false);
            ui->chB_EMGainAdv->setChecked(false);
        }
    }
    else if(!checked)
    {
        camera->setEMGainAdvanced(checked); //just to make sure
    }
}

void iXonControl::on_chB_baseClamp_clicked(bool checked)
{
    camera->setBaseClamp(checked);
}



/* ROI CONTROL */
void iXonControl::showROIParam(iXonCamera::ROIParameter RP) //mb group lE together to make it more readable
{
    if(camera->acqParam().acqMode == iXonCamera::ACQ_MODE_FAST_KINETIC || RP.cropEnabled)
    {
        ui->cB_rdMode->clear();
        ui->cB_rdMode->addItem("FVB");
        ui->cB_rdMode->addItem("Image");
        rdModesHidden = true;
    }
    else if(rdModesHidden)
    {
        ui->cB_rdMode->clear();
        ui->cB_rdMode->addItem("FVB");
        ui->cB_rdMode->addItem("Multitrack");
        ui->cB_rdMode->addItem("Randomtrack");
        ui->cB_rdMode->addItem("Singletrack");
        ui->cB_rdMode->addItem("Image");
        rdModesHidden = false;
    }

     switch(RP.rdMode)
     {
     case iXonCamera::RD_MODE_FVB:
         ui->cB_rdMode->setCurrentIndex(ui->cB_rdMode->findText("FVB"));
         break;
     case iXonCamera::RD_MODE_SINGLETRACK:
         ui->cB_rdMode->setCurrentIndex(ui->cB_rdMode->findText("Singletrack"));
         break;
     case iXonCamera::RD_MODE_MULTITRACK:
         ui->cB_rdMode->setCurrentIndex(ui->cB_rdMode->findText("Multitrack"));
         break;
     case iXonCamera::RD_MODE_RANDOMTRACK:
         ui->cB_rdMode->setCurrentIndex(ui->cB_rdMode->findText("Randomtrack"));
         break;
     case iXonCamera::RD_MODE_IMAGE:
         ui->cB_rdMode->setCurrentIndex(ui->cB_rdMode->findText("Image"));
         break;
     }


    ui->lE_vertBin->setDisabled(true);
    ui->lE_ROI1->hide();
    ui->lE_ROI2->hide();
    ui->lE_ROI3->hide();
    ui->lE_ROI4->hide();

    ui->lb_ROI1->hide();
    ui->lb_ROI2->hide();
    ui->lb_ROI3->hide();
    ui->lb_ROI4->hide();

    ui->pB_preview->hide();
    ui->pB_RTSetup->hide();

    ui->lb_MTFirst->hide();
    ui->lb_MTFirstName->hide();

    ui->chB_crop->setChecked(RP.cropEnabled);
    ui->chB_crop->setDisabled(true);

    if(camera->acqParam().acqMode != iXonCamera::ACQ_MODE_FAST_KINETIC)
    {
        switch(RP.rdMode)
        {
        case iXonCamera::RD_MODE_FVB:
            ui->lE_horBin->setText(QString::number(RP.horBin));

            if(camera->acqParam().FTenabled) ui->chB_crop->setDisabled(false); //KINDA UGLY

            if(RP.cropEnabled)
            {
                ui->lb_ROI1->show();
                ui->lb_ROI2->show();
                ui->lb_ROI1->setText("Height");
                ui->lb_ROI2->setText("Width");

                ui->lE_ROI1->show();
                ui->lE_ROI2->show();
                ui->lE_ROI1->setText(QString::number(RP.imgYEnd));
                ui->lE_ROI2->setText(QString::number(RP.imgXEnd));
            }
            break;
        case iXonCamera::RD_MODE_SINGLETRACK:
            ui->lE_horBin->setText(QString::number(RP.horBin));

            ui->lb_ROI1->show();
            ui->lb_ROI2->show();
            ui->lb_ROI1->setText("Height");
            ui->lb_ROI2->setText("Center");

            ui->lE_ROI1->show();
            ui->lE_ROI2->show();
            qDebug() << "showroi:" << "showing single track";
            ui->lE_ROI1->setText(QString::number(RP.STHeight));
            ui->lE_ROI2->setText(QString::number(RP.STCenter));
            break;
        case iXonCamera::RD_MODE_MULTITRACK:
            ui->lE_horBin->setText(QString::number(RP.horBin));


            ui->lb_ROI1->show();
            ui->lb_ROI2->show();
            ui->lb_ROI3->show();
            ui->lb_ROI1->setText("Height");
            ui->lb_ROI2->setText("Number");
            ui->lb_ROI3->setText("Offset");

            ui->lE_ROI1->show();
            ui->lE_ROI2->show();
            ui->lE_ROI3->show();
            ui->lE_ROI1->setText(QString::number(RP.MTHeight));
            ui->lE_ROI2->setText(QString::number(RP.MTNum));
            ui->lE_ROI3->setText(QString::number(RP.MTOffset));

            ui->lb_MTFirst->show();
            ui->lb_MTFirstName->show();
            ui->lb_MTFirst->setText(QString::number(RP.MTFirstRow));
            break;
        case iXonCamera::RD_MODE_RANDOMTRACK:
            ui->pB_RTSetup->show();
            break;
        case iXonCamera::RD_MODE_IMAGE:
            ui->lE_vertBin->setDisabled(false);
            ui->lE_vertBin->setText(QString::number(RP.imgVertBin));

            ui->lE_horBin->setText(QString::number(RP.imgHorBin));

            if(camera->acqParam().FTenabled) ui->chB_crop->setDisabled(false); //KINDA UGLY

            ui->lb_ROI1->show();
            ui->lb_ROI2->show();
            ui->lb_ROI3->show();
            ui->lb_ROI4->show();
            ui->lb_ROI1->setText("X min");
            ui->lb_ROI2->setText("X max");
            ui->lb_ROI3->setText("Y min");
            ui->lb_ROI4->setText("Y max");

            ui->lE_ROI1->show();
            ui->lE_ROI2->show();
            ui->lE_ROI3->show();
            ui->lE_ROI4->show();
            ui->lE_ROI1->setText(QString::number(RP.imgXStart));
            ui->lE_ROI2->setText(QString::number(RP.imgXEnd));
            ui->lE_ROI3->setText(QString::number(RP.imgYStart));
            ui->lE_ROI4->setText(QString::number(RP.imgYEnd));

            ui->pB_preview->show();

            break;
        }
    }
    else
    {
        if(RP.rdMode == iXonCamera::RD_MODE_IMAGE)
        {
            ui->lE_vertBin->setDisabled(false);
            ui->lE_vertBin->setText(QString::number(RP.FKVertBin));
        }
        else
        {
            ui->lE_vertBin->setDisabled(true);
        }

        ui->lE_horBin->setText(QString::number(RP.FKHorBin));


        ui->lb_ROI1->show();
        ui->lb_ROI2->show();
        ui->lb_ROI1->setText("Height");
        ui->lb_ROI2->setText("Offset");

        ui->lE_ROI1->show();
        ui->lE_ROI2->show();
        ui->lE_ROI1->setText(QString::number(RP.FKHeight));
        ui->lE_ROI2->setText(QString::number(RP.FKOffset));

    }
    QString cropSize = QString::number(camera->xpixel()) + " x " + QString::number(camera->ypixel());
    ui->lb_cropSize->setText(cropSize);

    return;
}

void makeNearestPowerOfTwo(int *num) //as long as num is smaller than camera->xpixel
{
    if((!(*num & (*num - 1)) == 0) && *num != 0) //checks whether num is a power of 2 via clever bit manipulation (courtesy of stackexchange)
    {
        int i = 0;
        while(i <= log2(1024))
        {
            if(*num < pow(2,i)) break;
            i++;
        }
        if(pow(2,i) - *num < *num - pow(2, i-1)) *num = pow(2, i);
        else *num = pow(2, i-1);
        return;
    }
    else return;
}
void makeNearestMultiple(int *input, int base) //makes input the nearest multiple of base, smaller than 1024
{
    if(base == 0 || base == 1) return;
    int diff = *input % base;
    if(diff > base / 2 && *input - diff + base <= 1024) *input = *input - diff + base;
    else *input = *input - diff;
    return;
}

void iXonControl::on_pB_applyROI_clicked()
{
    int err_check;
    bool okH, okV, ok1, ok2, ok3, ok4;
    int hBin, vBin;
    int sHeight, sCenter;
    int mHeight, mNumber, mOffset;
    int imgXStart, imgXEnd, imgYStart, imgYEnd;
    int FKHeight, FKOffset;

    hBin = ui->lE_horBin->text().toInt(&okH);
    if(!okH) hBin = 1;
    vBin = ui->lE_vertBin->text().toInt(&okV);
    if(!okV) vBin = 1;

    if(camera->acqParam().acqMode != iXonCamera::ACQ_MODE_FAST_KINETIC)
    {
        switch(camera->ROIParam().rdMode)
        {
        case iXonCamera::RD_MODE_FVB:
            if(camera->ROIParam().cropEnabled)
            {
                imgYEnd = ui->lE_ROI1->text().toInt(&ok1);
                imgXEnd = ui->lE_ROI2->text().toInt(&ok2);

                if(ok1 && ok2)
                {
                   makeNearestMultiple(&imgXEnd, hBin);
                   camera->setFVB(hBin, imgXEnd, imgYEnd);
                }
                else
                {
                    ui->lE_ROI1->setText(QString::number(camera->ROIParam().imgXEnd));
                    ui->lE_ROI2->setText(QString::number(camera->ROIParam().imgYEnd));
                }
            }
            else
            {
                makeNearestPowerOfTwo(&hBin);
                camera->setFVB(hBin);
            }
            break;
        case iXonCamera::RD_MODE_SINGLETRACK:
            sHeight = ui->lE_ROI1->text().toInt(&ok1);
            sCenter = ui->lE_ROI2->text().toInt(&ok2);

            if(ok1 && ok2)
            {
                makeNearestPowerOfTwo(&hBin);
                camera->setSTPosition(sHeight, sCenter, hBin);
            }
            else
            {
                ui->lE_ROI1->setText(QString::number(camera->ROIParam().STHeight));
                ui->lE_ROI2->setText(QString::number(camera->ROIParam().STCenter));
            }
            break;
        case iXonCamera::RD_MODE_MULTITRACK:
            mHeight = ui->lE_ROI1->text().toInt(&ok1);
            mNumber = ui->lE_ROI2->text().toInt(&ok2);
            mOffset = ui->lE_ROI3->text().toInt(&ok3);

            if(ok1 && ok2 && ok3)
            {
                makeNearestPowerOfTwo(&hBin);
                err_check = camera->setMultPosition(mNumber, mHeight, mOffset, hBin);
            }
            else
            {
                ui->lE_ROI1->setText(QString::number(camera->ROIParam().MTHeight));
                ui->lE_ROI2->setText(QString::number(camera->ROIParam().MTNum));
                ui->lE_ROI3->setText(QString::number(camera->ROIParam().MTOffset));
            }
            break;
        case iXonCamera::RD_MODE_RANDOMTRACK:
            //DO BINNING STUFF
            break;
        case iXonCamera::RD_MODE_IMAGE:
            imgXStart = ui->lE_ROI1->text().toInt(&ok1);
            imgXEnd = ui->lE_ROI2->text().toInt(&ok2);
            imgYStart = ui->lE_ROI3->text().toInt(&ok3);
            imgYEnd = ui->lE_ROI4->text().toInt(&ok4);

            if(ok1 && ok2 && ok3 && ok4)
            {
                int xSize = imgXEnd - imgXStart + 1;
                int ySize = imgYEnd - imgYStart + 1;

                makeNearestMultiple(&xSize, hBin);
                makeNearestMultiple(&ySize, vBin);

                imgXEnd = imgXStart + xSize - 1;
                imgYEnd = imgYStart + ySize - 1;

                err_check = camera->setImagePosition(hBin, vBin, imgXStart, imgXEnd, imgYStart, imgYEnd);
            }

            else
            {
                ui->lE_ROI1->setText(QString::number(camera->ROIParam().imgXStart));
                ui->lE_ROI2->setText(QString::number(camera->ROIParam().imgXEnd));
                ui->lE_ROI3->setText(QString::number(camera->ROIParam().imgYStart));
                ui->lE_ROI4->setText(QString::number(camera->ROIParam().imgYEnd));
            }
        }
    }
    else
    {
        FKHeight = ui->lE_ROI1->text().toInt(&ok1);
        FKOffset = ui->lE_ROI2->text().toInt(&ok2);

        makeNearestPowerOfTwo(&hBin);
        if(camera->ROIParam().rdMode == iXonCamera::RD_MODE_IMAGE) makeNearestMultiple(&FKHeight, vBin);
        else vBin = 1;
        if(ok1 && ok2) camera->setFKParam(camera->ROIParam().rdMode, FKHeight, FKOffset, camera->acqParam().kinNumber, camera->acqParam().FKexpTime, hBin, vBin);
    }
}

void iXonControl::fillROIPos(int xStart, int xEnd, int yStart, int yEnd)
{
    ui->lE_ROI1->setText(QString::number(xStart));
    ui->lE_ROI2->setText(QString::number(xEnd));
    ui->lE_ROI3->setText(QString::number(yStart));
    ui->lE_ROI4->setText(QString::number(yEnd));

    on_pB_applyROI_clicked();
}

void iXonControl::on_pB_preview_clicked()
{
    ROIPre->setROI(camera->ROIParam().imgXStart, camera->ROIParam().imgXEnd, camera->ROIParam().imgYStart, camera->ROIParam().imgYEnd);
    ROIPre->exec();
}

void iXonControl::on_pB_RTSetup_clicked()
{
    RTInt->setTableText(camera->ROIParam().CTSettings);
    RTInt->exec();
}

void iXonControl::on_chB_crop_clicked(bool checked)
{
    camera->setCrop(checked);
    on_pB_applyROI_clicked();
}


/* COUNT CONVERT */
void iXonControl::manageCountModes()
{
    ui->rB_countAD->setDisabled(true);
    ui->rB_countElectrons->setDisabled(true);
    ui->rB_countPhotons->setDisabled(true);

    if(camera->isCountAvailable(0))
    {
        ui->rB_countAD->setDisabled(false);
    }
    if(camera->isCountAvailable(1))
    {
        ui->rB_countElectrons->setDisabled(false);
    }
    if(camera->isCountAvailable(2))
    {
        ui->rB_countPhotons->setDisabled(false);
    }

    ui->lE_lambda->setText(QString::number(camera->countLambda()));

    return;
}

void iXonControl::showCountMode(int mode)
{
    switch(mode)
    {
    case 0:
        ui->rB_countAD->setChecked(true);
        ui->rB_countElectrons->setChecked(false);
        ui->rB_countPhotons->setChecked(false);
        ui->lE_lambda->setDisabled(true);
        break;
    case 1:
        ui->rB_countAD->setChecked(false);
        ui->rB_countElectrons->setChecked(true);
        ui->rB_countPhotons->setChecked(false);
        ui->lE_lambda->setDisabled(true);
        break;
    case 2:
        ui->rB_countAD->setChecked(false);
        ui->rB_countElectrons->setChecked(false);
        ui->rB_countPhotons->setChecked(true);
        ui->lE_lambda->setDisabled(false);
        break;
    }

    manageCountModes();
    return;

}

void iXonControl::on_rB_countAD_clicked(bool checked)
{
    if(checked) camera->setCountMode(0);
}

void iXonControl::on_rB_countElectrons_clicked(bool checked)
{
    if(checked) camera->setCountMode(1);
}

void iXonControl::on_rB_countPhotons_clicked(bool checked)
{
    if(checked) camera->setCountMode(2);
}

void iXonControl::on_lE_lambda_editingFinished()
{
    bool ok = false;
    float lambda = ui->lE_lambda->text().toFloat(&ok);
    if(ok) camera->setCountWavelength(lambda);
    else ui->lE_lambda->setText(QString::number(camera->countLambda()));
    return;
}


/* SHUTTER CONTROL */
void iXonControl::showShParam(struct iXonCamera::shutterParameter sP)
{
    ui->cB_shMode->setCurrentIndex(sP.shMode);

    ui->cB_shExtMode->setCurrentIndex(sP.shExtMode);

    QString openString = QString::number(sP.openTime);
    ui->lE_shOpenTime->setText(openString);

    QString closeString = QString::number(sP.closeTime);
    ui->lE_shCloseTime->setText(closeString);

    ui->rB_TTLHigh->setChecked(sP.TTLHigh);
    ui->rB_TTLLow->setChecked(!sP.TTLHigh);

    manageShGrayout(sP);
}

void iXonControl::manageShGrayout(iXonCamera::shutterParameter sP)
{
    if(
       (sP.shMode == iXonCamera::SH_MODE_PERMOPEN && sP.shExtMode == iXonCamera::SH_MODE_PERMOPEN)
       || (sP.shMode == iXonCamera::SH_MODE_PERMCLOSED || sP.shExtMode == iXonCamera::SH_MODE_PERMCLOSED)
       )
    {
        ui->lE_shCloseTime->setDisabled(true);
        ui->lE_shOpenTime->setDisabled(true);
    }
    else
    {
        ui->lE_shCloseTime->setDisabled(false);
        ui->lE_shOpenTime->setDisabled(false);
    }
}

void iXonControl::on_cB_shMode_activated(int index)
{
    if(index > 5 || index < 0) return;

    struct iXonCamera::shutterParameter sP = camera->shParam();
    sP.shMode = static_cast<iXonCamera::shutterMode>(index);
    camera->loadShutter(sP);
}

void iXonControl::on_cB_shExtMode_activated(int index)
{
    if(index > 5 || index < 0) return;

    struct iXonCamera::shutterParameter sP = camera->shParam();
    sP.shExtMode = static_cast<iXonCamera::shutterMode>(index);
    camera->loadShutter(sP);
}

void iXonControl::on_rB_TTLHigh_clicked(bool checked)
{
    struct iXonCamera::shutterParameter sP = camera->shParam();
    sP.TTLHigh = checked;
    camera->loadShutter(sP);
}

void iXonControl::on_rB_TTLLow_clicked(bool checked)
{
    struct iXonCamera::shutterParameter sP = camera->shParam();
    sP.TTLHigh = !checked;
    camera->loadShutter(sP);
}

void iXonControl::on_lE_shOpenTime_editingFinished()
{
    struct iXonCamera::shutterParameter sP = camera->shParam();

    bool ok;
    float temp;
    temp = ui->lE_shOpenTime->text().toFloat(&ok); //openTime requires an int, however this autoconverts to the nearest int
    if(ok)
    {
        sP.openTime = temp;
        camera->loadShutter(sP);
    }
    else camera->loadShutter(camera->shParam());
}

void iXonControl::on_lE_shCloseTime_editingFinished()
{
    struct iXonCamera::shutterParameter sP = camera->shParam();

    bool ok;
    float temp;
    temp = ui->lE_shCloseTime->text().toFloat(&ok); //closeTime requires an int, however this autoconverts to the nearest int
    if(ok)
    {
        sP.closeTime = temp;
        camera->loadShutter(sP);
    }
    else camera->loadShutter(camera->shParam());
}


/*TEMPERATURE CONTROL*/
void iXonControl::showCurTemp(int temp, int status)
{
    ui->lb_temp->setText(QString::number(temp)+" °C");
    QString statusString;

    switch(status)
    {
    case DRV_TEMP_STABILIZED:
        statusString = "Stabilized";
        break;
    case DRV_TEMP_DRIFT:
        statusString = "Drifting";
        break;
    case DRV_TEMP_NOT_REACHED:
        statusString = "Cooling...";
        break;
    case DRV_TEMP_NOT_STABILIZED:
        statusString = "Stabilizing...";
        break;
    case DRV_TEMP_OFF:
        statusString = "Cooling off";
        break;
    case DRV_ACQUIRING:
        statusString = "Acquiring...";
        break;
    default:
        statusString = "Error";
    }
    ui->lb_tempStatus->setText(statusString);
}

void iXonControl::showTargetTemp(int temp)
{
    if(camera->tempControl()->coolerEnabled())
    {
        ui->lE_targetTemp->setText(QString::number(temp));
        ui->sl_temp->setValue(temp);
        ui->lb_targetTemp->setText(QString::number(temp)+" °C");
    }
    else
    {
        ui->lE_targetTemp->setText("");
        ui->lb_targetTemp->setText("-");
        ui->sl_temp->setValue(ui->sl_temp->maximum());
    }
}

void iXonControl::showTempRange(int min, int max)
{
    ui->sl_temp->setMinimum(min);
    ui->sl_temp->setMaximum(max);
}

void iXonControl::showCoolerEnabled(bool enabled)
{
    ui->rB_coolOn->setChecked(enabled);
    ui->rB_coolOff->setChecked(!enabled);

    manageTempGrayout(enabled);
}

void iXonControl::manageTempGrayout(bool enabled)
{
    if(enabled)
    {
        ui->lE_targetTemp->setDisabled(false);
        ui->sl_temp->setDisabled(false);
    }
    else
    {
        ui->lE_targetTemp->setDisabled(true);
        ui->sl_temp->setDisabled(true);
    }
}

void iXonControl::on_lE_targetTemp_editingFinished()
{
    bool ok;
    int temp = ui->lE_targetTemp->text().toInt(&ok);

    if(ok)
    {
        ui->sl_temp->setValue(temp);
        camera->tempControl()->setTemperature(temp);
    }
    else
    {
        ui->lE_targetTemp->setText(QString::number(camera->tempControl()->targetTemp()));
    }

    return;
}

void iXonControl::on_sl_temp_sliderMoved(int position)
{
    ui->lE_targetTemp->setText(QString::number(position));
}


void iXonControl::on_sl_temp_sliderReleased()
{
    int temp = ui->sl_temp->value();
    camera->tempControl()->setTemperature(temp);
}

void iXonControl::on_rB_coolOn_clicked(bool checked)
{
    ui->rB_coolOff->setChecked(!checked);
    manageTempGrayout(checked);
    camera->tempControl()->setCooler(checked);
}

void iXonControl::on_rB_coolOff_clicked(bool checked)
{
    ui->rB_coolOn->setChecked(!checked);
    manageTempGrayout(!checked);
    camera->tempControl()->setCooler(!checked);
}

void iXonControl::showOverheatWarning()
{
    QMessageBox overWarnBox;
    overWarnBox.setText("CRITICAL WARNING: COOLER HAS OVERHEATED");
    overWarnBox.setIcon(QMessageBox::Critical);
    overWarnBox.exec();
}

void iXonControl::showFanSpeed(int speed)
{
    switch(speed)
    {
    case TemperatureController::FAN_HIGH:
        ui->rB_fanHigh->setChecked(true);
        ui->rB_fanLow->setChecked(false);
        ui->rB_fanOff->setChecked(false);
        break;
    case TemperatureController::FAN_LOW:
        ui->rB_fanHigh->setChecked(false);
        ui->rB_fanLow->setChecked(true);
        ui->rB_fanOff->setChecked(false);
        break;
    case TemperatureController::FAN_OFF:
        ui->rB_fanHigh->setChecked(false);
        ui->rB_fanLow->setChecked(false);
        ui->rB_fanOff->setChecked(true);
        break;
    }
}

void iXonControl::on_rB_fanHigh_clicked(bool checked)
{
    ui->rB_fanLow->setChecked(!checked);
    ui->rB_fanOff->setChecked(!checked);

    camera->tempControl()->setFanSpeed(TemperatureController::FAN_HIGH);
}

void iXonControl::on_rB_fanLow_clicked(bool checked)
{
    ui->rB_fanHigh->setChecked(!checked);
    ui->rB_fanOff->setChecked(!checked);

    camera->tempControl()->setFanSpeed(TemperatureController::FAN_LOW);
}

void iXonControl::on_rB_fanOff_clicked(bool checked)
{
    ui->rB_fanHigh->setChecked(!checked);
    ui->rB_fanLow->setChecked(!checked);

    camera->tempControl()->setFanSpeed(TemperatureController::FAN_OFF);
}



/*FILE MANAGEMENT AND SAVING*/
void iXonControl::on_pB_browse_clicked()
{
    QString inputPath = QFileDialog::getExistingDirectory(this, "Choose Directory", camera->dataHandler()->filepath());
    if(inputPath == "") inputPath = camera->dataHandler()->filepath();
    QString filepath = QDir::toNativeSeparators(inputPath);

    camera->dataHandler()->updateFilepath(filepath);

    ui->lE_pathName->setText(camera->dataHandler()->filepath());

}

void iXonControl::on_lE_pathName_editingFinished()
{

    QFileInfo fileInfo(ui->lE_pathName->text());
    if(fileInfo.isWritable()) //mb other condition checks need to be implemented as well
    {
        QString filepath = fileInfo.path();
        camera->dataHandler()->updateFilepath(filepath);
    }
    else camera->dataHandler()->updateFilepath(camera->dataHandler()->filepath());
    return;
}

void iXonControl::on_lE_fileName_editingFinished()
{
    QFileInfo fileInfo(ui->lE_fileName->text());
    QString filename = fileInfo.baseName();
    camera->dataHandler()->updateFilename(filename);

    return;
}

void iXonControl::showFilepath()
{
    ui->lE_pathName->setText(camera->dataHandler()->filepath());
    ui->lE_fileName->setText(camera->dataHandler()->filename());
}

void iXonControl::showSaveMode(int mode)
{
    ui->rB_PNG->setChecked(false);
    ui->rB_DAT->setChecked(false);
    ui->rB_both->setChecked(false);

    switch(mode)
    {
    case 0:
        ui->rB_PNG->setChecked(true);
        break;
    case 1:
        ui->rB_DAT->setChecked(true);
        break;
    case 2:
        ui->rB_both->setChecked(true);
        break;
    }
    return;
}

void iXonControl::on_rB_PNG_clicked(bool checked)
{
    if(checked) camera->dataHandler()->setSaveMode(0);
}

void iXonControl::on_rB_DAT_clicked(bool checked)
{
    if(checked) camera->dataHandler()->setSaveMode(1);
}

void iXonControl::on_rB_both_clicked(bool checked)
{
    if(checked) camera->dataHandler()->setSaveMode(2);
}


/* SAVING AND LOADING OF SETTINGS */

/* SOME FREE HELPER FUNCTIONS AND MACROS FOR SETTING SAVING */
#define VARNAME(variable)           varname(#variable)
#define PRINTVAR(variable, stream)  printVar(variable, VARNAME(variable), stream)
QString varname(const char *var)
{
    QString out(var);

    out = out.mid(out.lastIndexOf(".")+1);
    out = out.mid(out.lastIndexOf(">")+1);

    out = out.left(out.indexOf("(")); //mb make it "()"
    return out;
}
void printVar(int var, QString varname, QTextStream &out)
{
    out << varname << " = " << var << endl;
    return;
}
void printVar(uint var, QString varname, QTextStream &out)
{
    out << varname << " = " << var << endl;
    return;
}
void printVar(bool var, QString varname, QTextStream &out)
{
    out << varname << " = " << var << endl;
    return;
}
void printVar(float var, QString varname, QTextStream &out)
{
    out << varname << " = " << var << endl;
    return;
}
void printVar(QString var, QString varname, QTextStream &out)
{
    out << varname << " = " << var << endl;
    return;
}
void printVar(QVector<long> var, QString varname, QTextStream &out)
{
    out << varname << " = ";
    for(int i = 0; i < var.size(); i++)
    {
        out << var[i];
        if(i != var.size() - 1) out << " ";
    }
    out << endl;
}

void iXonControl::saveRTToFile(QString filename)
{
    QFile savefile(filename);
    if(savefile.open(QFile::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&savefile);

        out << "[RANDOM TRACK SETTINGS]" << endl;
        for(auto i = camera->ROIParam().CTSettings.begin(); i != camera->ROIParam().CTSettings.end(); i++)
        {
            out << *i << endl;
        }

        savefile.close();
    }
    return;
}


void iXonControl::saveSettings(bool acq, bool rd, bool ROI, bool shutter, bool output, QString filename)
{
    QFile savefile(filename);
    QFileInfo saveInfo(savefile);
    if(savefile.open(QFile::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&savefile);
        if(output)
        {
            out << "[OUTPUT]" << endl;
            PRINTVAR(camera->dataHandler()->filepath(), out);
            PRINTVAR(camera->dataHandler()->filename(), out);
            PRINTVAR(camera->dataHandler()->saveMode(), out);
            PRINTVAR(camera->countMode(), out);
            PRINTVAR(camera->countLambda(), out);
            out << endl;
        }
        if(acq)
        {
            out << "[ACQUISITION]" << endl;
            PRINTVAR(camera->acqParam().acqMode, out);
            PRINTVAR(camera->acqParam().exposureTime, out);
            PRINTVAR(camera->acqParam().accCycleTime, out);
            PRINTVAR(camera->acqParam().kinCycleTime, out);
            PRINTVAR(camera->acqParam().accNumber, out);
            PRINTVAR(camera->acqParam().kinNumber, out);
            PRINTVAR(camera->acqParam().FTenabled, out);
            PRINTVAR(camera->acqParam().FKexpTime, out);

            PRINTVAR(camera->acqParam().trMode, out);


            out << endl;
        }
        if(rd) //MIGHT BE PROBLEMATIC WITH A FEW OF THE PARAMETERS SINCE THEY ARE LOADED DYNAMICALLY
        {
            out << "[READOUT]" << endl;

            PRINTVAR(camera->rdParam().numVSSpeed, out);
            PRINTVAR(camera->rdParam().numHSSpeed, out);
            PRINTVAR(camera->rdParam().numPAGain, out);
            PRINTVAR(camera->rdParam().numVSVolt, out);
            PRINTVAR(camera->rdParam().EMEnabled, out);
            PRINTVAR(camera->rdParam().EMAdvanced, out);
            PRINTVAR(camera->rdParam().EMGainMode, out);
            PRINTVAR(camera->rdParam().curEMGain, out);
            PRINTVAR(camera->rdParam().baseClamp, out);
            PRINTVAR(camera->rdParam().cosmicRayFilterEnabled, out);

            out << endl;

        }
        if(ROI)
        {
            out << "[ROI]" << endl;
            PRINTVAR(camera->ROIParam().rdMode, out);

            switch(camera->ROIParam().rdMode)
            {
            case iXonCamera::RD_MODE_FVB:
                PRINTVAR(camera->ROIParam().horBin, out);
                break;
            case iXonCamera::RD_MODE_SINGLETRACK:
                PRINTVAR(camera->ROIParam().horBin, out);
                PRINTVAR(camera->ROIParam().STCenter, out);
                PRINTVAR(camera->ROIParam().STHeight, out);
                break;
            case iXonCamera::RD_MODE_MULTITRACK:
                PRINTVAR(camera->ROIParam().MTNum, out);
                PRINTVAR(camera->ROIParam().MTHeight, out);
                PRINTVAR(camera->ROIParam().MTOffset, out);
                break;
            case iXonCamera::RD_MODE_RANDOMTRACK:
                saveRTToFile(saveInfo.path() + QDir::separator() + saveInfo.baseName() + "_RTSettings" + ".ixonrtcfg");
                break;
            case iXonCamera::RD_MODE_IMAGE:
                PRINTVAR(camera->ROIParam().imgHorBin, out);
                PRINTVAR(camera->ROIParam().imgVertBin, out);
                PRINTVAR(camera->ROIParam().imgXStart, out);
                PRINTVAR(camera->ROIParam().imgXEnd, out);
                PRINTVAR(camera->ROIParam().imgYStart, out);
                PRINTVAR(camera->ROIParam().imgYEnd, out);
                break;
            }
            if(camera->acqParam().acqMode == iXonCamera::ACQ_MODE_FAST_KINETIC)
            {
                PRINTVAR(camera->ROIParam().FKHeight, out);
                PRINTVAR(camera->ROIParam().FKOffset, out);
                PRINTVAR(camera->ROIParam().FKHorBin, out);
                PRINTVAR(camera->ROIParam().FKVertBin, out);
            }

             out << endl;
        }
        if(shutter)
        {
            out << "[SHUTTER]" << endl;

            PRINTVAR(camera->shParam().shMode, out);
            PRINTVAR(camera->shParam().shExtMode, out);
            PRINTVAR(camera->shParam().closeTime, out);
            PRINTVAR(camera->shParam().openTime, out);
            PRINTVAR(camera->shParam().TTLHigh, out);

            out << endl;
        }
        savefile.close();
    }
    return;
}


/* SOME FREE HELPER FUNCTIONS AND MACROS FOR LOADING */
#define LOADVAR(variable, string)   if(loadVar(&variable, VARNAME(variable), string)) continue
bool loadVar(int *var, QString varname, QString line)
{
    bool ok = false;
    if(line.contains(varname))
    {
        int temp;
        line = line.mid(line.indexOf("=")+2);
        line = line.left(line.lastIndexOf("\n"));
        temp = line.toInt(&ok);
        if(ok) *var = temp;
    }
    return ok;
}
bool loadVar(uint *var, QString varname, QString line)
{
    bool ok = false;
    if(line.contains(varname))
    {
        int temp;
        line = line.mid(line.indexOf("=")+2);
        line = line.left(line.lastIndexOf("\n"));
        temp = line.toInt(&ok);
        if(ok) *var = temp;
    }
    return ok;
}
bool loadVar(bool *var, QString varname, QString line)
{
    bool ok = false;
    if(line.contains(varname))
    {
        int temp;
        line = line.mid(line.indexOf("=")+2);
        line = line.left(line.lastIndexOf("\n"));
        temp = line.toInt(&ok);
        if(ok) *var = temp;
    }
    return ok;
}
bool loadVar(float *var, QString varname, QString line)
{
    bool ok = false;
    if(line.contains(varname))
    {
        float temp;
        line = line.mid(line.indexOf("=")+2);
        line = line.left(line.lastIndexOf("\n"));
        temp = line.toFloat(&ok);
        if(ok) *var = temp;
    }
    return ok;
}
bool loadVar(QString *var, QString varname, QString line)
{
    bool ok = false;
    if(line.contains(varname))
    {
        line = line.mid(line.indexOf("=")+2);
        line = line.left(line.lastIndexOf("\n"));
        *var = line;
        ok = true;
    }
    return ok;
}
bool loadVar(iXonCamera::acquisitionMode *var, QString varname, QString line)
{
    bool ok = false;
    if(line.contains(varname))
    {
        int temp;
        line = line.mid(line.indexOf("=")+2);
        line = line.left(line.lastIndexOf("\n"));
        temp = line.toInt(&ok);
        if(ok) *var = static_cast<iXonCamera::acquisitionMode>(temp);
    }
    return ok;
}
bool loadVar(iXonCamera::readoutMode *var, QString varname, QString line)
{
    bool ok = false;
    if(line.contains(varname))
    {
        int temp;
        line = line.mid(line.indexOf("=")+2);
        line = line.left(line.lastIndexOf("\n"));
        temp = line.toInt(&ok);
        if(ok) *var = static_cast<iXonCamera::readoutMode>(temp);
    }
    return ok;
}
bool loadVar(iXonCamera::shutterMode *var, QString varname, QString line)
{
    bool ok = false;
    if(line.contains(varname))
    {
        int temp;
        line = line.mid(line.indexOf("=")+2);
        line = line.left(line.lastIndexOf("\n"));
        temp = line.toInt(&ok);
        if(ok) *var = static_cast<iXonCamera::shutterMode>(temp);
    }
    return ok;
}
bool loadVar(iXonCamera::triggerMode *var, QString varname, QString line)
{
    bool ok = false;
    if(line.contains(varname))
    {
        int temp;
        line = line.mid(line.indexOf("=")+2);
        line = line.left(line.lastIndexOf("\n"));
        temp = line.toInt(&ok);
        if(ok) *var = static_cast<iXonCamera::triggerMode>(temp);
    }
    return ok;
}
bool loadVar(QVector<long> *var, QString varname, QString line)
{
    bool ok = false;
    if(line.contains(varname))
    {
        QVector<long> temp;
        QString newLine;
        int entry;
        QString entryText;
        line = line.mid(line.indexOf("=")+2);
        line = line.left(line.lastIndexOf("\n"));
        int backupCounter = 0;
        while(backupCounter < 10)
        {
            backupCounter++;

            entryText = line.left(line.indexOf(" "));
            entry = entryText.toInt(&ok);

            if(ok) temp.push_back(entry);
            else if(!ok) return false;

            newLine = line.mid(line.indexOf(" ")+1);
            if(newLine == line) break;
            else line = newLine;
        }
        *var = temp;
    }
    return ok;
}

void iXonControl::loadRTFromFile(QString filename)
{
    QFile loadfile(filename);
    if(loadfile.open(QFile::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&loadfile);
        QString line;
        bool ok;
        int temp;

        QVector<int> settings = {};
        while(!in.atEnd())
        {
            line = in.readLine();
            temp = line.toInt(&ok);
            if(ok) settings.push_back(temp);
        }
        loadfile.close();

        if(settings.size() % 2 == 0) camera->setRTPosition(settings);
    }
    return;
}


void iXonControl::loadSettings(bool acq, bool rd, bool ROI, bool shutter, bool output, QString loadname)
{
    QFile loadfile(loadname);
    QFileInfo loadInfo(loadfile);
    if(loadfile.open(QFile::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&loadfile);
        QString line;
        int currentSection = -1;

        QString filename, filepath;
        int saveMode;
        int countMode;
        float countLambda;
        iXonCamera::acquisitionParameter aP;
        iXonCamera::readoutParameter rP;
        iXonCamera::ROIParameter ROIP;
        iXonCamera::shutterParameter sP;

        while(!in.atEnd())
        {
            line = in.readLine();
            if(line.contains("[")) //checks whether to load the section or not
            {
                if(line.contains("OUTPUT") && output)
                {
                    currentSection = 0;
                    continue;
                }
                else if(line.contains("ACQUISITION") && acq)
                {
                    currentSection = 1;
                    continue;

                }
                else if(line.contains("READOUT") && rd)
                {
                    currentSection = 2;
                    continue;

                }
                else if(line.contains("ROI") && ROI)
                {
                    currentSection = 3;
                    continue;

                }
                else if(line.contains("SHUTTER") && shutter)
                {
                    currentSection = 4;
                    continue;

                }
                else continue;
            }

            if(currentSection == 0)
            {
                LOADVAR(filename, line);
                LOADVAR(filepath, line);
                LOADVAR(saveMode, line);
                LOADVAR(countMode, line);
                LOADVAR(countLambda, line);
            }
            else if(currentSection == 1)
            {
                LOADVAR(aP.acqMode, line);
                LOADVAR(aP.exposureTime, line);
                LOADVAR(aP.accCycleTime, line);
                LOADVAR(aP.kinCycleTime, line);
                LOADVAR(aP.accNumber, line);
                LOADVAR(aP.kinNumber, line);
                LOADVAR(aP.FTenabled, line);
                LOADVAR(aP.FKexpTime, line);
                LOADVAR(aP.trMode, line);
            }
            else if(currentSection == 2)
            {
                LOADVAR(rP.numVSSpeed, line);
                LOADVAR(rP.numHSSpeed, line);
                LOADVAR(rP.numPAGain, line);
                LOADVAR(rP.numVSVolt, line);
                LOADVAR(rP.EMEnabled, line);
                LOADVAR(rP.EMAdvanced, line);
                LOADVAR(rP.EMGainMode, line);
                LOADVAR(rP.curEMGain, line);
                LOADVAR(rP.baseClamp, line);
                LOADVAR(rP.cosmicRayFilterEnabled, line);
            }
            else if(currentSection == 3)
            {
                LOADVAR(ROIP.rdMode, line);
                LOADVAR(ROIP.horBin, line);
                LOADVAR(ROIP.STCenter, line);
                LOADVAR(ROIP.STHeight, line);
                LOADVAR(ROIP.MTNum, line);
                LOADVAR(ROIP.MTHeight, line);
                LOADVAR(ROIP.MTOffset, line);
                loadRTFromFile(loadInfo.path() + QDir::separator() +  loadInfo.baseName() + "_RTSettings" + ".ixonrtcfg");
                LOADVAR(ROIP.imgHorBin, line);
                LOADVAR(ROIP.imgVertBin, line);
                LOADVAR(ROIP.imgXStart, line);
                LOADVAR(ROIP.imgXEnd, line);
                LOADVAR(ROIP.imgYStart, line);
                LOADVAR(ROIP.imgYEnd, line);
                LOADVAR(ROIP.FKHeight, line);
                LOADVAR(ROIP.FKOffset, line);
                LOADVAR(ROIP.FKHorBin, line);
                LOADVAR(ROIP.FKVertBin, line);
            }
            else if(currentSection == 4)
            {

                LOADVAR(sP.shMode, line);
                LOADVAR(sP.shExtMode, line);
                LOADVAR(sP.closeTime, line);
                LOADVAR(sP.openTime, line);
                LOADVAR(sP.TTLHigh, line);
            }
             else continue;
        }
        loadfile.close();

        if(shutter)   camera->loadShutter(sP);
        if(ROI) camera->loadROI(ROIP);
        if(rd) camera->loadReadout(rP);
        if(acq) camera->loadAcquisition(aP);

        if(output)
        {
            camera->dataHandler()->updateFilepath(filepath);
            camera->dataHandler()->updateFilename(filename);
            camera->dataHandler()->setSaveMode(saveMode);
            camera->setCountMode(countMode);
            camera->setCountWavelength(countLambda);
        }
    }
}

void iXonControl::on_pB_saveSettings_clicked()
{
    SettingLoader *saver = new SettingLoader(0, true, defaultPath);
    connect(saver, SIGNAL(settingSaveSignal(bool,bool,bool,bool,bool,QString)), this, SLOT(saveSettings(bool,bool,bool,bool,bool,QString)));
    saver->exec();
    delete saver;
}

void iXonControl::on_pB_loadSettings_clicked()
{
    SettingLoader *loader = new SettingLoader(0, 0, defaultPath);
    connect(loader, SIGNAL(settingLoadSignal(bool,bool,bool,bool,bool,QString)), this, SLOT(loadSettings(bool,bool,bool,bool,bool,QString)));
    connect(loader, SIGNAL(loadDefaultsSignal()), camera, SLOT(loadHardcodedDefaults()));
    loader->exec();
    delete loader;
}

/*END*/

