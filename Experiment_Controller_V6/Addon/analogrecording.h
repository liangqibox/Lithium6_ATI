#ifndef ANALOGRECORDING_H
#define ANALOGRECORDING_H

#include <QWidget>
#include <QCheckBox>
#include <QTimer>
#include <QMessageBox>
#include <QDateTime>
#include <QFont>
#include <QMenu>
#include <QSignalMapper>
#include "definition.h"
#include "Adwin.h"
#include "DataType/variable.h"
#include "include/variableclass.h"
#include "include/controllerdatagroup.h"

namespace Ui {
class AnalogRecording;
}

class AnalogRecording : public QWidget
{
    Q_OBJECT

public:
    explicit AnalogRecording(QWidget *parent = 0, ControllerDataGroup *dg = NULL);
    ~AnalogRecording();

public slots:
    void next_recording(QString msg);
    void Controller_Setting_Load(ControllerDataGroup *);

private slots:
    void on_Ain_acquire_enable_clicked();
    void on_Ain_file_path_button_clicked();
    void on_Ain_samlpling_rate_editingFinished();
    void on_Time_Start_editingFinished();
    void on_Time_End_editingFinished();
    void show_variable_menu(QLineEdit *lineEdit);
    void set_variable(QString msg);

    void accquire_Ain_data(int length);
    void save_Ain_data(int length);
    double data_to_voltage(int in);

    void Reload(QString msg);
    void Status_Check();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::AnalogRecording *ui;

    //QList<double> AIn_Data[8];
    long *Data;
    QList<Variable*> *variables;
    VariableClass *variableClass;
    double Start,End; //ms
    int AIn_Sampling_Rate; //us
    Variable *vStart,*vEnd,*vSamplingRate;
    int number_of_scan;
    int number_of_run;

    ControllerDataGroup *DataGroup;
    QList<QCheckBox*> Channels_active;
    QTimer *status_check;
};

#endif // ANALOGRECORDING_H
