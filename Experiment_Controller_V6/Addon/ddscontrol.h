#ifndef DDSCONTROL_H
#define DDSCONTROL_H

#include <QWidget>
#include <QFileDialog>
#include <QFile>
#include <QStringList>
#include "qiscriptenginecluster.h"
#include "Addon/editerwidget.h"
#include "Addon/ddsdevicepanel.h"
#include "DataType/sequence.h"
#include "DataType/variable.h"
#include "include/ddscontroller.h"

namespace Ui {
class DDSControl;
}

class DDSControl : public QWidget
{
    Q_OBJECT

public:
    explicit DDSControl(QWidget *parent = 0, QList<Variable*> *vari = NULL, QList<QString> *chlname = NULL, QiScriptEngineCluster *eng = NULL,
                        int num_of_analog = 0, int num_of_digi = 0);
    ~DDSControl();

public slots:
    void Receive_new_cycle(QString msg);
    void new_CycleSetting(QString setting);
    QString current_Setting();

private slots:
    void Update_all_device();
    void Initialize();
    void on_Refresh_clicked();

    void on_Manual_File_clicked();
    void on_Manual_Start_clicked();
    void Load_Command_from_File(QString filename);

    void ErrorReceive(QString err);

private:
    Ui::DDSControl *ui;

    int Number_of_Run,Number_of_Scan;
    int Analog_output_channel,Digital_output_channel;
    QString Manual_Filename;
    QList<QString> Manual_Command;
    QList<QString> *channel_names;
    QList<Variable*> *variables;
    //QList<QList<QList<Sequence*>*>*> *DDS_sequences;
    QList<DDSDevicePanel*> *DDS_Devices;
    QiScriptEngineCluster *Engine;
};

#endif // DDSCONTROL_H
