#ifndef LENSCONTROL_OPTOTUNE_H
#define LENSCONTROL_OPTOTUNE_H

#include "windows.h"
#include "DataType/variable.h"
#include <QtSerialPort/QtSerialPort>
#include <QWidget>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QPixmap>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

namespace Ui {
class LensControl_Optotune;
}

class LensControl_Optotune : public QWidget
{
    Q_OBJECT

public:
    explicit LensControl_Optotune(QWidget *parent = 0, bool isAdwin = false, QList<Variable*> *vari = NULL);
    ~LensControl_Optotune();

signals:
    void Cycle_start(bool);
    void Cycle_end(bool);
    void Focus_change(int);

private slots:
    bool Hand_shake();
    void Reset();
    bool Load_file(QString filename);
    void Run_cycle();
    void Stop_cycle();
    void ADwin_trigger(QString msg);
    void Next();
    void Send_command(double f, bool focus);
    char* focus_to_command(double f, char* send);
    char* current_to_command(int c, char send[]);
    ushort CRC_Checksum(char send[], int length);

    void on_Trigger_adwin_stateChanged(int arg1);
    void on_File_load_clicked();
    void on_Singal_clicked();
    void on_Continous_clicked();
    void on_Stop_clicked();

    void Read_temperature();
    void Cycle_progress();
    void Show_current_Focus(int focus);
    void on_Focus_Slider_actionTriggered(int action);

    void on_Connect_clicked();

private:
    Ui::LensControl_Optotune *ui;

    bool external;
    bool continous;
    int counter,progress_time,current_focus,number_of_run,number_of_scan;
    QString file_load;

    QList<int> Data_time;
    QList<double> Data_focus;
    QList<Variable*> *variables;
    QTimer *run_timer;
    QTimer *T_timer, *Progress_timer;

    ushort polynomial;
    ushort *table;

    QSerialPort serial;

protected:
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);
};

#endif // LENSCONTROL_OPTOTUNE_H
