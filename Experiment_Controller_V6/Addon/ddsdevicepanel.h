#ifndef DDSDEVICEPANEL_H
#define DDSDEVICEPANEL_H

#include <QWidget>
#include <QThread>
#include <QMenu>
#include <QPixmap>
#include <QPainter>
#include <include/qicombobox.h>
#include <include/ddscontroller.h>
#include <DataType/sequence.h>

namespace Ui {
class DDSDevicePanel;
}

class DDSDevicePanel : public QWidget
{
    Q_OBJECT

public:
    explicit DDSDevicePanel(QWidget *parent = 0, QString p = "NULL", QList<QString> *chlname = NULL, int num_of_analog = 0, int num_of_digi = 0);
    ~DDSDevicePanel();
    QString Port();

signals:
    void Merge(int);
    void ErrorReturn(QString err);

public slots:
    void update_Device(DDSCycle &cyc);
    void update_Device();
    void Reset();

private slots:
    void on_Reset_clicked();
    void on_Softtrigger_clicked();
    void on_Amplitude_returnPressed();
    void on_Phase_returnPressed();
    void Merge_Channel_Change(int index);
    void ChangeIndicator(int i);

private:
    Ui::DDSDevicePanel *ui;

    int Number_of_Run,Number_of_Scan;
    int Analog_output_channel,Digital_output_channel;
    int merge;
    float amplitude,phase;
    QString port;
    QList<QString> *channel_names;
    QThread *Thread;
    DDSController *device;
};

#endif // DDSDEVICEPANEL_H
