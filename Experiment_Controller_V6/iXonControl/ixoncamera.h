#ifndef IXONCAMERA_H
#define IXONCAMERA_H

#include <QObject>
#include <QList>
#include <QThread>
#include <QTimer>
#include <QVector>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QCheckBox>
#include <QPushButton>
#include <QFileDialog>

#include "datahandler.h"
#include "temperaturecontroller.h"
#include "acquisitionmonitor.h"
#include "ATMCD32D.H"

#define DEFAULT_WATCHDOG_TIME 10000


class DataHandler;
class TemperatureController;
class AcquisitionMonitor;
class iXonCamera : public QObject
{
    Q_OBJECT

public:
    typedef enum {ACQ_MODE_0, ACQ_MODE_SINGLE, ACQ_MODE_ACCUMULATE, ACQ_MODE_KINETIC, ACQ_MODE_FAST_KINETIC, ACQ_MODE_RUN_TILL_ABORT} acquisitionMode;
    typedef enum {RD_MODE_FVB, RD_MODE_MULTITRACK, RD_MODE_RANDOMTRACK, RD_MODE_SINGLETRACK, RD_MODE_IMAGE} readoutMode;
    typedef enum {SH_MODE_FULLAUTO, SH_MODE_PERMOPEN, SH_MODE_PERMCLOSED, SH_MODE_OPENFVB, SH_MODE_OPENANY} shutterMode;
    typedef enum {TR_MODE_INTERNAL = 0, TR_MODE_EXTERNAL = 1, TR_MODE_EXTERNAL_FAST = 2, TR_MODE_EXTERNAL_START = 6, TR_MODE_BULB = 7,
                  TR_MODE_SOFTWARE = 10, TR_MODE_INVALID = -1} triggerMode;
    const short trModeNumber = 5; //UUUUgly
    // MACROS DECLARED IN ATMCD32D.H ARE BITPATTERNS, AND ARE MEANT FOR GETCAPABILITIES(), DO NOT USE THEM

    struct systemInformation{
        /* HARDWARE INFORMATION */

        unsigned int PCBVersion;
        unsigned int decodeVersion;
        unsigned int firmwareVersion;
        unsigned int firmwareBuild;


        QString CCDType;

        /* SOFTWARE INFORMATION */

        unsigned int EPROMVersion;
        unsigned int COFVersion;
        unsigned int driverRevision;
        unsigned int driverVersion;
        unsigned int dllRevision;
        unsigned int dllVersion;
    };

    struct acquisitionParameter {
        acquisitionMode acqMode;
        float exposureTime;
        float accCycleTime;
        float kinCycleTime;

        int accNumber;
        int kinNumber;

        bool FTenabled;

        float FKexpTime;

        triggerMode trMode;
    };

    struct readoutParameter{
        std::vector<float> availableVSSpeeds;
        std::vector<float> availableHSSpeeds;
        unsigned int numVSSpeed;
        unsigned int numHSSpeed;
        unsigned int numFastestRecVSSpeed;
        float VSSpeed;
        float HSSpeed;

        unsigned int numVSVolt;

        std::vector<float> availablePAGains;
        unsigned int numPAGain;
        float PAGain;

        bool EMEnabled;
        int minEMGain;
        int maxEMGain;
        int curEMGain;
        bool EMAdvanced;
        int EMGainMode;

        bool baseClamp;   
        bool cosmicRayFilterEnabled;
    };

    struct ROIParameter{
        readoutMode rdMode;

        int horBin;

        int imgHorBin;
        int imgVertBin;

        int STHeight;
        int STCenter;

        int MTNum;
        int MTOffset;
        int MTHeight;
        int MTFirstRow;

        int CTNumber;
        QVector<int> CTSettings;

        int imgXStart;
        int imgXEnd;
        int imgYStart;
        int imgYEnd;

        int FKHeight;
        int FKOffset;
        int FKHorBin;
        int FKVertBin;

        bool cropEnabled;
     };

    struct shutterParameter{
        bool TTLHigh;
        shutterMode shMode;
        int closeTime; //in ms
        int openTime; //in ms
        shutterMode shExtMode;
    };

    inline int xpixel() {return m_xpixel;}
    inline int ypixel() {return m_ypixel;}

    inline int countMode() {return m_countMode;}
    inline float countLambda() {return m_countLambda;}
    inline bool photonCounting() {return m_photonCounting;}
    inline QVector<long> photonCountDivisions() {return m_photonCountDivisions;}

    inline bool turnedOn() {return m_turnedOn;}
    inline bool initError() {return m_initError;}
    inline bool isAcquiring() {return m_isAcquiring;}

    inline DataHandler *dataHandler() {return m_dataHandler;}
    inline AcquisitionMonitor *acqMonitor() {return m_acqMonitor;}
    inline TemperatureController *tempControl() {return m_tempControl;}

    struct systemInformation systemInfo() {return m_systemInfo;}
    struct acquisitionParameter acqParam() {return m_acqParam;}
    struct readoutParameter rdParam() {return m_rdParam;}
    struct ROIParameter ROIParam() {return m_ROIParam;}
    struct shutterParameter shParam() {return m_shParam;}


    explicit iXonCamera(QObject *parent = 0);
    ~iXonCamera();

private:
    bool m_turnedOn;
    bool m_initError;
    bool m_temperatureWarningEnabled;


    bool m_isAcquiring;
    int m_frameCount;
    bool m_saveVideo;



    DataHandler *m_dataHandler;
    TemperatureController *m_tempControl;
    AcquisitionMonitor *m_acqMonitor;

    int setupHorBinning(int hor);

    int m_xpixel, m_ypixel;
    int xDataPoints();
    int yDataPoints();

    int m_curRunNum;
    int m_curScanNum;
    int m_elapsedAcc;
    int m_elapsedKin;

    struct systemInformation m_systemInfo;

    struct acquisitionParameter m_acqParam;
    struct readoutParameter m_rdParam;
    struct ROIParameter m_ROIParam;
    struct shutterParameter m_shParam;

    struct acquisitionParameter m_defaultAcq;
    struct readoutParameter m_defaultRd;
    struct ROIParameter m_defaultROI;
    struct shutterParameter m_defaultSh;

    int m_countMode;
    float m_countLambda;
    bool m_photonCounting;
    QVector<long> m_photonCountDivisions;
    int m_photonCountMaxThreshold;

    void setDefaults();

    QString m_driverPath;

public slots:
    int initCamera();
    int checkForCorrectInit();
    void loadSystemInformation();
    void loadHardcodedDefaults();

    void receiveExperimentControllerMessage(QString message);

    void turnOff();


    /* TRIGGER */
    int setTrMode(triggerMode mode);
    triggerMode convertToTrIndex(int index);
    int convertFromTrIndex(triggerMode trMode);
    bool isTrAvailable(triggerMode trMode);

    int startAcquiring(int timeout = DEFAULT_WATCHDOG_TIME);
    int abortAcquiring();
    int stopAcquiring();
    void handleFrame();
    int setSaveVideo(bool save);
    int startAcqStatusMonitor(ushort timeout);
    int stopAcqStatusMonitor(bool success);

    /* READOUT CONTROL */
    int retrieveVSSpeeds();
    int retrieveHSSpeeds();
    int retrievePAGains();

    int setRdMode(int mode);
    int setVSSpeed(int index);
    int setHSSpeed(int index);
    int setEMenabled(bool on);
    int setVSVolt(int numVolt);
    int setPAGain(int numGain);
    void loadReadout(struct readoutParameter rP);

    int getGainRange();
    int getGainValue();
    int getGainAdvanced();
    int setEMGain(int gain);
    int setEMGainMode(int mode);
    int setEMGainAdvanced(bool enabled);

    int setBaseClamp(bool enabled);

    /*COUNT CONVERT */
    int setCountMode(int mode);
    bool isCountAvailable(int mode);
    int setCountWavelength(float lambda);
    void resetCountMode();

    int setPhotonCountingEnabled(bool enabled);
    int setPhotonCountingRanges(QVector<long> ranges);
    int photonCountingMaxDivisions();


    /*ROI */
    int setFVB(int hBin, int cropW = 1024, int cropH = 1024);
    int setSTPosition(int height, int center, int hBin);
    int setMultPosition(int num, int height, int offset, int hBin);
    int setRTPosition(QVector<int> RTData);
    int setImagePosition(int hBin, int vBin, int xStart, int xEnd, int yStart, int yEnd);
    int loadImageBorder(int xStart, int xEnd, int yStart, int yEnd);

    void resetROIPosition();

    int setCrop(bool enabled);
    int resetCrop();
    void loadROI(ROIParameter RP);


    /* SHUTTER CONTROL */
    int loadShutter(struct shutterParameter sP); //since the library function requires all parameters at once, it isnt feasable to split the implementation


    /* EXPOSURE CONTROL */
    int setAcqMode(int mode);
    int setFTMode(bool on);
    int setExpTime(float time);
    int setAccCycleTime(float time);
    int setKinCycleTime(float time);
    int setAccNumber(int num);
    int setKinNumber(int num);
    int updateExpParameters();
    int setCosmicFilter(bool enabled);
    void loadAcquisition(struct acquisitionParameter aP);

    int setFKParam(int rdMode, int rows, int offset, int numKin, float expTime,  int hBin, int vBin);
    int setFKRdMode(int rdMode);
    int setFKExp(float exp);
    int setFKHBin(int hBin);
    int setFKVBin(int vBin);
    int setFKoffset(int offset);
    int setFKrows(int rows);


signals:
    void initFailCloseApp();

    void sendMessage(QString message);

    void acqParamChanged(struct acquisitionParameter newParam);
    void rdParamChanged(struct readoutParameter newParam);
    void ROIParamChanged(struct ROIParameter newParam);
    void shParamChanged(struct shutterParameter newParam);

    void RTListChanged(QVector<int> RTData);

    void acqStart(ushort timeout, acquisitionMode aqMode);
    void acquiringStatusChanged(bool isAcquiring);
    void saveSignal(int xDP, int yDP, int numKin);
    void saveFrameSignal(int xDP, int yDP);
    void saveAllSignal();
    void saveVideoFrameSignal(int xDP, int yDP, int numKin, bool save);
    void deleteTempFileSignal();


    void countModeChanged(int mode);
    void updateCountModes(); //need to be updated when changing baseClamp, crop mode, EMgain
    void photonCountingEnabled(bool enabled);
    void photonCountDivisionsChanged(QVector<long> photonDivs);

    void sendAcqParameters(int runNum, int scanNum);



};

 Q_DECLARE_METATYPE(iXonCamera::acquisitionParameter)
 Q_DECLARE_METATYPE(iXonCamera::acquisitionMode)

 Q_DECLARE_METATYPE(iXonCamera::shutterParameter)
 Q_DECLARE_METATYPE(iXonCamera::readoutParameter)
 Q_DECLARE_METATYPE(iXonCamera::ROIParameter)


#endif // IXONCAMERA_H
