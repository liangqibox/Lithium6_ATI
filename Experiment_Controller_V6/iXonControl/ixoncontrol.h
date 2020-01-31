#ifndef CAMERATESTING_H
#define CAMERATESTING_H

#include <QWidget>
#include <QListView>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QDir>
#include <QDebug>
#include <QChar>
#include <QMessageBox>
#include <QCloseEvent>
#include <QTextStream>
#include <QFile>
#include <QFileDialog>
#include <QDialog>
#include <QFileInfo>
#include <QListView>
#include <QPixmap>
#include <QTableWidget>
#include <QDialogButtonBox>
#include <string>

#include "ixoncamera.h"
#include "versioninformation.h"
#include "settingloader.h"
#include "randomtrackinterface.h"
#include "roipreview.h"
#include "datahandler.h"
#include "ixoncamera.h"
#include "acquisitionmonitor.h"
#include "temperaturecontroller.h"
#include "overheatmonitor.h"
#include "photoncountsetup.h"

namespace Ui {
class iXonControl;
}
class SettingLoader;
class ROIPreview;
class RandomTrackInterface;

class iXonControl : public QWidget
{
    Q_OBJECT

public:
    explicit iXonControl(QWidget *parent = 0);
    ~iXonControl();

    inline bool cameraInitError() {return m_cameraInitError;}

private slots:

    /*MISC*/
    void showMessage(QString message);
    void processExperimentControllerMessage(QString message);


    /* READOUT */
    void manageShiftSpeedLists();
    void showRdParam(struct iXonCamera::readoutParameter rP);
    void on_cB_rdMode_activated(int index);
    void on_cB_VSSpeed_activated(int index);
    void on_cB_HSSpeed_activated(int index);
    void on_rB_EMon_clicked(bool checked);
    void on_rB_EMoff_clicked(bool checked);
    void on_cB_VVolt_activated(int index);
    void on_cB_PAGain_activated(int index);
    void on_sB_EMGain_editingFinished();
    void on_chB_EMGainAdv_clicked(bool checked);
    void on_chB_baseClamp_clicked(bool checked);


    /* ROI */
    void showROIParam(struct iXonCamera::ROIParameter RP);
    void fillROIPos(int xStart, int xEnd, int yStart, int yEnd);
    void on_pB_RTSetup_clicked();
    void on_pB_applyROI_clicked();
    void on_pB_preview_clicked();
    void on_chB_crop_clicked(bool checked);


    /* ACQUISITION */
    void on_pB_startAcq_clicked();
    void manageAcqStartPbText(bool isAcquiring);

    void on_cB_aqcMode_activated(int index);
    void on_lE_exposure_editingFinished();
    void on_lE_accCycle_editingFinished();
    void on_lE_kinCycle_editingFinished();
    void on_lE_accNumber_editingFinished();
    void on_lE_kinNumber_editingFinished();
    void on_chB_FT_toggled(bool checked);
    void on_chB_cosmic_clicked(bool checked);
    void on_cB_trMode_activated(int index);

    void showAcqParam(struct iXonCamera::acquisitionParameter eP);
    void manageAcqGrayout(struct iXonCamera::acquisitionParameter eP);


    /* SHUTTER */
    void showShParam(struct iXonCamera::shutterParameter sP);
    void manageShGrayout(struct iXonCamera::shutterParameter sP);
    void on_cB_shMode_activated(int index);
    void on_cB_shExtMode_activated(int index);
    void on_rB_TTLHigh_clicked(bool checked);
    void on_rB_TTLLow_clicked(bool checked);
    void on_lE_shOpenTime_editingFinished();
    void on_lE_shCloseTime_editingFinished();


    /*TEMPERATURE*/
    void showCurTemp(int temp, int status);
    void showTargetTemp(int temp);
    void showCoolerEnabled(bool enabled);
    //void showCoolerAfterShutDownEnabled(bool enabled);
    void showTempRange(int min, int max);
    void manageTempGrayout(bool enabled);
    void on_lE_targetTemp_editingFinished();
    void on_sl_temp_sliderMoved(int position);
    void on_sl_temp_sliderReleased();
    void on_rB_coolOn_clicked(bool checked);
    void on_rB_coolOff_clicked(bool checked);
    void showFanSpeed(int speed);

    void showOverheatWarning();


    /*FILE MANAGEMENT*/
    void on_pB_browse_clicked();
    void on_lE_pathName_editingFinished();
    void on_lE_fileName_editingFinished();
    void showFilepath();
    void showSaveMode(int mode);
    void on_rB_PNG_clicked(bool checked);
    void on_rB_DAT_clicked(bool checked);
    void on_rB_both_clicked(bool checked);

    /*SETTINGS*/
    void saveSettings(bool acq, bool rd, bool ROI, bool shutter, bool path, QString filename);
    void loadSettings(bool acq, bool rd, bool ROI, bool shutter, bool path, QString filename);
    void saveRTToFile(QString filename);
    void loadRTFromFile(QString filename);
    void on_pB_saveSettings_clicked();
    void on_pB_loadSettings_clicked();


    /*COUNT MODES*/
    void manageCountModes();
    void showCountMode(int mode);
    void on_rB_countAD_clicked(bool checked);
    void on_rB_countElectrons_clicked(bool checked);
    void on_rB_countPhotons_clicked(bool checked);
    void on_lE_lambda_editingFinished();



    /*LIVE VIEW*/
    void on_pB_liveView_clicked();
    void loadVideoPicture(QString filename);
    void on_chB_saveVideo_clicked(bool checked);



    void on_rB_fanHigh_clicked(bool checked);

    void on_rB_fanLow_clicked(bool checked);

    void on_rB_fanOff_clicked(bool checked);

private:
    void closeEvent(QCloseEvent *event);

    QThread *dataThread;
    QThread *acqMonitorThread;
    QThread *tempThread;
    QThread *overheatThread;

    QString defaultPath;
    QString currentPath;

    QString startPbText;

    Ui::iXonControl *ui;
    iXonCamera *camera;
    bool m_cameraInitError;

    bool infoLoaded;
    ROIPreview *ROIPre;
    RandomTrackInterface *RTInt;


    bool rdModesHidden = false;

    bool *childWasDisabled;
    QList<QWidget *> children;

    QLabel *liveLabel;


signals:
    void setupTemperatureController();
    void messageTransfer(QString message);
};

#endif // CAMERATESTING_H
