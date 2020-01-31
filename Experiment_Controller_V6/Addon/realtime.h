#ifndef REALTIME_H
#define REALTIME_H

#include "Adwin.h"
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QPen>
#include <QColor>
#include <QMouseEvent>
#include <QDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QSignalMapper>
#include <QMessageBox>
#include <QScrollBar>

const int number_of_channel = 6;
const int squrewidth = 70;
const int sample_rate =  50;   //ms

namespace Ui {
class RealTime;
}

class RealTime : public QDialog
{
    Q_OBJECT

public:
    explicit RealTime(QWidget *parent = 0,int aout = 0, int ain = 0, int dio = 0, int din = 0);
    ~RealTime();
    void Draw_cursor(int x, int y);
    void Read_cursor(int x, int y);

protected:
    bool eventFilter(QObject *, QEvent *);

private slots:
    void on_H_dial_valueChanged(int value);
    void on_V_dial_valueChanged(int value);
    void on_Position_dial_sliderMoved(int position);
    void on_Channel_control_currentChanged(int index);
    void on_Position_dis_editingFinished();
    void channel_state_change(int chl);

    void Draw();
    void frequence_measure();
    double data_to_voltage(int d);
    void take_data();

    void on_main_toolBox_currentChanged(int index);

private:
    Ui::RealTime *ui;

    int Lattic_length_H[10];    //ms
    int Lattic_length_V[9];    //mV
    int lattic_h,lattic_v[number_of_channel];
    double v_position[number_of_channel];
    int v_dial_position;
    int active[number_of_channel];
    QList<int> data[number_of_channel];

    int Analog_output_channel;
    int Analog_input_channel;
    int Digital_output_channel;
    int Digital_input_channel;

    QList<QCheckBox*> Display_active;
    QList<QComboBox*> Display_channel;
    QList<QLabel*> Channel_freq;

    QColor color[number_of_channel];
    QTimer *input_timer,*output_timer;

    int temp_counter;
};

#endif // REALTIME_H
