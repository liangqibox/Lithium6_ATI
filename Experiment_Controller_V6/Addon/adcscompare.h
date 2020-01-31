#ifndef ADCSCOMPARE_H
#define ADCSCOMPARE_H

#include <QWidget>
#include "qiscriptenginecluster.h"
#include "md5.h"
#include "DataType/sequence.h"
#include "DataType/variable.h"
#include "DataType/sectionactivity.h"
#include <math.h>
#include <QScriptEngine>
#include <QScriptValue>
#include <QFileDialog>

namespace Ui {
class AdcsCompare;
}

class AdcsCompare : public QWidget
{
    Q_OBJECT

public:
    explicit AdcsCompare(QWidget *parent = 0);
    ~AdcsCompare();

private slots:
    void on_Load_clicked();
    void Load_File(QString filename);

private:
    Ui::AdcsCompare *ui;

    double cycle_time;         //ms
    int time_resolution;       //us
    int Analog_output_channel;
    int Digital_output_channel;

    QList<QList<Sequence*>**> *sequences;
    QList<Variable*> *variables;
    VariableClass *variableClass;
    QList<VariableClass*> *DisplayClass;
    QList<SectionActivity*> *sections_activity;
    QList<QString> *sections_name;
    QList<QString> *section_array;
    QList<QList<Sequence*>**> *sections;

    QList<QString> *channel_name;
    QList<int> *channel_color;
    QList<QString> *channel_calibration;
    QList<QStringList> *External_File_Transfer;
};

#endif // ADCSCOMPARE_H
