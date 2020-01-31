#ifndef CYCLEDISPLAYER_H
#define CYCLEDISPLAYER_H

#include "Addon/indicator.h"
#include "DataType/sequence.h"
#include "DataType/variable.h"
#include "DataType/sectionactivity.h"
#include "qiscriptenginecluster.h"
#include "SequenceCalculator.h"
#include <QWidget>
#include <QTableWidgetItem>
#include <QColorDialog>
#include <QPainter>
#include <QColor>
#include <QPen>

namespace Ui {
class CycleDisplayer;
}

class CycleDisplayer : public QWidget
{
    Q_OBJECT

public:
    explicit CycleDisplayer(QWidget *parent = 0, QScriptEngine *eng = NULL, QList<QList<Sequence*>**> *seq = NULL, QList<Variable*> *vari = NULL,
                            QList<SectionActivity*> *sec_act = NULL,
                            QList<QString> *chl_cal = NULL, QList<QString> *chl_name = NULL, int aout = 0, int diout = 0, bool controlled = false);
    ~CycleDisplayer();

private slots:
    void on_Add_clicked();
    void on_Delete_clicked();
    void on_List_cellClicked(int row, int);
    void on_List_cellDoubleClicked(int row, int column);
    void on_Switch_mode_valueChanged(int value);
    void on_Number_of_Scan_editingFinished();
    void on_Time_resolution_editingFinished();
    void on_Time_start_editingFinished();
    void on_Time_end_editingFinished();
    void on_Load_clicked();

    void clear_Draw_data();
    void calculate_cycle_data();
    void calculate_Draw_data();
    void Draw();

    QScriptEngine *copyEngine(QScriptEngine *eng);

private:
    Ui::CycleDisplayer *ui;
    Indicator *indicator;

    QColor color[24];
    QList<int> Channel;
    QList<QColor> Channel_color;

    int Base[32];
    int Analog_output_channel;
    int Digital_output_channel;
    int *total_number_of_scan;
    int number_of_scan;
    int time_resolution;
    double time_start,time_end;
    double cycle_time;
    QList<double> Draw_time;
    QList<int> *Draw_data;

    QList<QList<Sequence*>**> *sequences;
    QList<Variable*> *variables;
    QList<SectionActivity*> *sections_activity;
    QList<QString> *channel_calibration;
    QList<QString> *channel_name;

    //QiScriptEngineCluster *QiEngine;
    QScriptEngine *QiEngine;

protected:
    bool eventFilter(QObject *, QEvent *);
    void resizeEvent(QResizeEvent*resize);
};

#endif // CYCLEDISPLAYER_H
