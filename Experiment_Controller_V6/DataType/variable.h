#ifndef VARIABLE_H
#define VARIABLE_H

#include "math.h"
#include "spline.h"
#include "qiscriptenginecluster.h"
#include <QMessageBox>
#include <vector>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QString>
#include <QList>
#include <QMouseEvent>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QSignalMapper>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QScriptValue>
#include <QScriptEngine>
#include <QScriptContext>

namespace Ui {
class Variable;
}

class Variable : public QWidget
{
    Q_OBJECT

public:
    explicit Variable(QWidget *parent = 0, QString iname = "only_system_used_null", int itype = 0,
                      QScriptEngine *eng = NULL, QList<Variable *> *vari = NULL);
    ~Variable();
    Variable & operator =(Variable&);

    void set_type(int t);
    bool set_name(QString);
    void set_value(int i, double v);
    void set_file(QString f);
    void set_fomular(QString f);

    int get_type();
    double* get_value();
    int get_fitting();
    QString get_name();
    QString get_fomular();
    QString get_translated_fomular();
    QString get_filename();
    double get_scan(int current);
    double get_calibrated(double a);
    double get_calculated(int current,double x,double seqt,double sect,double cyct);
    double get_output(int current,double x,double seqt,double sect,double cyct);
    QList<Variable *>* get_variables();

    double get_scan(QScriptEngine *engine, int current);
    double get_calibrated(QScriptEngine *engine, double a);
    double get_calculated(QScriptEngine *engine, int current,double x,double seqt,double sect,double cyct);
    double get_output(QScriptEngine *engine, int current,double x,double seqt,double sect,double cyct);

public slots:
    void change_fitting_mode(int);
    void Reload();
    void File_load();

private slots:
    QString translate_fomular(QString);

signals:
    void name_changed(QString);
    void Delete();
    void Changed();
    void Scan(bool);

private:
    int type;
    QString name;
    QString fomular;
    QString translated_fomular;
    QString filename;
    double value[4];
    int fitting_mode;
    QList<double> file_data[2];
    QScriptEngine *Engine;
    //QiScriptEngineCluster *Engine;
    QScriptValue V;

    QList<Variable *> *variables;
};

#endif // VARIABLE_H
