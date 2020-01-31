#ifndef DISPLAYWINDOW_H
#define DISPLAYWINDOW_H

#include "DataType/sequence.h"
#include "DataType/variable.h"
#include "math.h"
#include <QMessageBox>
#include <QDialog>
#include <QComboBox>
#include <QPixmap>
#include <QPainter>
#include <QPen>
#include <QColor>
#include <QCursor>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QSlider>
#include <QMenuBar>
#include <QObject>
#include <QString>
#include <QTimer>

namespace Ui {
class DisplayWindow;
}

class DisplayWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DisplayWindow(QWidget *parent = 0, int AO_c = 24, int DiO_c = 32);
    ~DisplayWindow();
    void initial(QList<QList<Sequence *>**>*iseq, int, int*, QList<QString> *, QList<Variable *> *, QList<QString> *sec_act);

private slots:
    void on_Display_close_clicked();
    void on_Display_refresh_clicked();
    void aochannel_displayed_refresh(int a);
    void diochannel_displayed_refresh(int a);
    void refresh();

    void on_Display_width_left_editingFinished();
    void on_Display_width_right_editingFinished();
    void on_Display_height_top_editingFinished();
    void on_Display_height_bottom_editingFinished();

    void draw();
    int findpoint_in_time(int channel, double time, int last_out);
    bool find_digiout_in_time(int channel, double time, bool last_out);

    void on_Number_of_run_Slider_sliderMoved(int position);
    void on_Number_of_run_editer_editingFinished();
    void on_Number_of_run_Slider_actionTriggered(int action);

    void save_into_file();
    void save_into_image();

private:
    Ui::DisplayWindow *ui;

    QComboBox *AO[12];
    QComboBox *DiO[6];
    QList<int> AO_display;
    QList<int> DiO_display;
    QList<QList<Sequence *>**>*sequences;
    QList<QString> *channel_calibration;
    QList<Variable*> *variables;
    QList<QString> *sections_activity;
    double left,right,bottom,top; // (V)
    double total_time; // (ms)

    int *total_number_of_run;
    int number_of_run;
    int AO_height,DiO_height;
    int Analog_output_channel;
    int Digital_output_channel;

    QPixmap pixmap;
    QColor color[24];
    QLabel *cursor;
    QMenuBar *menubar;

protected:
    bool eventFilter(QObject *, QEvent *);
    void resizeEvent(QResizeEvent*);
};

#endif // DISPLAYWINDOW_H
