#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QMainWindow>
#include "definition.h"
#include "qiscriptenginecluster.h"
#include "ChannelCalculator.h"
#include "SequenceCalculator.h"
#include "md5.h"
#include "include/controllerdatagroup.h"
#include "DataType/sequence.h"
#include "DataType/variable.h"
#include "DataType/sectionactivity.h"
#include "Addon/variablemonitor.h"
#include "Addon/variableexpleror.h"
#include "Addon/displaywindow.h"
#include "Addon/cycledisplayer.h"
#include "Addon/channelediter.h"
#include "Addon/sectionediter.h"
#include "Addon/realtime.h"
#include "Addon/setting.h"
#include "Addon/systemlog.h"
#include "Addon/external.h"
#include "Addon/indicator.h"
#include "Addon/analogrecording.h"
#include "Addon/safety.h"
#include "Addon/autosave.h"
#include "Addon/qienginedebug.h"
#include "Addon/remotecontrolserver.h"
#include "Addon/ddscontrol.h"
#include "Addon/offsetlock_frequencycounter.h"
#include "Optotune/lenscontrol_optotune.h"
#include "Pixelfly/cameracontrol.h"
#include "Pixelfly/cameracontrolqe.h"
#include "iXonControl/ixoncontrol.h"
#include <math.h>
#include <QPixmap>
#include <QObject>
#include <QWidget>
#include <QList>
#include <QStringList>
#include <QLineEdit>
#include <QTimer>
#include <QDateTime>
#include <QSignalMapper>
#include <QList>
#include <QComboBox>
#include <QToolBox>
#include <QLayout>
#include <QLCDNumber>
#include <QString>
#include <QTableWidget>
#include <QMenu>
#include <QAction>
#include <QProgressDialog>
#include <QDir>
#include <QFileDialog>
#include <QTextStream>
#include <QThread>
#include <QEventLoop>
#include <QScriptEngine>
#include <QScriptValue>
#include <QElapsedTimer>
#include <Adwin.h>

namespace Ui {
class MainController;
}

class MainController : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainController(QWidget *parent = 0);
    ~MainController();
    void program_initial();

signals:
    void reloading(int num_of_scan);
    void new_cycle(QString msg);
    void Mission_used_state_updata(int state);
    void section_change();
    void program_terminate();
    void setting_load(ControllerDataGroup*);
    void program_setting_change(QStringList);
    void program_reset();

    void Channel_Editor_Click();
    void Variable_Monitor_Click();
    void Section_Editor_Click();
    void Channel_Display_Click();

private slots:
    void Load_Default_Setting();
    void ScriptEngineSetup();
    void Refresh();
    void Clear();
    void link_to_DataGroup();
    void on_Boot_clicked();
    void on_Emergency_stop_clicked();
    void on_Channel_Display_clicked();
    void on_Real_Time_clicked();
    void on_Reset_clicked();
    void system_setting_changed(QStringList);
    void on_Process_start_clicked();
    void on_Time_resolution_returnPressed();
    void on_Process_load_clicked();
    void Reduce_Algorithm(int **);
    void Transfer_Data(long **reduced_data, long *transfer_processdelay, int transfer_size, int cpu_time);
    void show_progress_window(QMessageBox *msgbox);
    void on_Scan_clicked();
    void calculate_cycle_time();
    void emit_new_cycle();

    void check_cycle_state();
    void check_hardware_state();
    void cycle_changed(bool c = true);

    void save_setting_clicked();
    void save_setting(QString filename);
    void load_setting_clicked();
    void load_setting(QString filename);

    void on_Variable_monitor_clicked();
    void on_Channel_editer_clicked();
    void on_Section_editer_clicked();
    void Variable_monitor_close();
    void Channel_editor_close();
    void Section_editor_close();

    void global_timer_move();
    void system_log(int event, int scan);
    void System_log_open();

    double input_to_voltage(int in);
    int voltage_to_output(double v);

    void on_Extra_Addon_Close_clicked();
    void on_Extra_Addon_Open_clicked();
    void open_Addon(int index);

    QString MD5_of_current_cycle();
    void Save_Tranfer_Data_to_File(long int **transdata, long *transprodelay, int size, QString md5);
    int Load_Tranfer_Data_from_File(long int **transdata, long *transprodelay, QString md5);

    void Receive_Remote_Command(QString com);
    void RemoteServer_Log();
    void ScriptEngineDebug();
    void Delay(int millisecondsToWait);
    void program_infomation();

private:
    Ui::MainController *ui;

    int voltage_resolution;
    QStringList program_setting;
    int Analog_output_channel;
    int Digital_output_channel;

    double cycle_time;         //ms
    double currentRunning_cycle_time; //ms
    double progress_time;
    double total_time;         //ms
    double run_time;           //ms
    int time_resolution;       //us

    int total_number_of_scan[10];
    int number_of_run;
    int number_of_scan;
    bool connected;
    bool running;
    bool scanning;
    bool cycle_change;
    bool seq_wrong;
    bool remote_start,remote_load;
    QString MD5_Last_Cycle;
    QStringList addons_names;

    QTimer *refresh_timer;
    QTimer *cycle_check_timer;
    QTimer *hardwareControl_check_timer;
    QTimer *global_timer;

    ControllerDataGroup *DataGroup;

    QList<QList<Sequence*>**> *sequences;
    QList<Variable*> *variables;
    VariableClass *variableClass;
    QList<VariableClass*> *DisplayClass;
    QList<SectionActivity*> *sections_activity;
    QList<QString> *sections_name;
    QList<QString> *section_array;
    QList<QList<Sequence*>**> *sections;
    //QList<QList<QList<Sequence*>*>*> *DDS_sequences;
    //QList<int> *DDS_channels;

    QList<QString> *channel_name;
    QList<int> *channel_color;
    QList<QString> *channel_calibration;
    QList<QStringList> *External_File_Transfer;
    QList<QString> Analog_input_Setting;

    QList<QWidget*> Widget_Addon;
    AutoSave *auto_save;
    Indicator *Adwin_state;

    //QiScriptEngineCluster *QiEngine;
    QScriptEngine *QiEngine;
    RemoteControlServer *remoteControl;
    //SequenceCalculator *sequenceCalculator;

    VariableExpleror *variableExpleror;
    Channelediter *channelEditer;
    bool isOpen_ChannelEditor;
    bool isOpen_SectionEditor;
    bool isOpen_VariableMonitor;
};

#endif // MAINCONTROLLER_H
