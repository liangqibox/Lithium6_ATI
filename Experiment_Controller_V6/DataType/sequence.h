#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "DataType/variable.h"
#include "include/variableclass.h"
#include <math.h>
#include <QString>
#include <QSignalMapper>
#include <QMenu>

class Sequence : public QWidget
{
    Q_OBJECT

public:
    explicit Sequence(QWidget *parent = 0, bool isDiO = false, QList<Variable *>* varis = NULL, VariableClass* vc = NULL);
    ~Sequence();
    Sequence(Sequence *seq, QList<Variable *> *varis, VariableClass *vc);
    Sequence& operator =(Sequence &);
    double get_type();
    double* get_value();
    double get_value(int index, int number_of_scan, double seqt, double sect, double cyct);
    Variable** get_variables();
    bool get_wrong();
    bool get_change();
    double upper_bound();
    double lower_bound();
    double get_time(int number_of_scan);
    double get_output(int number_of_scan, double seqt, double sect, double cyct, QString cal);

    void set_value(double ivalue[]);
    void set_value(int i, double v);
    void set_vari(int i, Variable* ivari);
    void set_wrong(bool w);
    void set_change(bool c);
    void set_boundary(QString boundary);
    void set_boundary(double lower, double upper);

    double get_time(QScriptEngine *engine, int number_of_scan);
    double get_output(QScriptEngine *engine, int number_of_scan, double seqt, double sect, double cyct, QString cal);

signals:
    void Changed(bool);
    void Variable_add(QString vari_add);
    void Wrong(bool);

public slots:
    void set_variable(QString va);
    void Right_clicked(int index);

private slots:

private:
    double value[5];
    QList<Variable*> *variables;
    VariableClass *variableClass;
    double upperbound;
    double lowerbound;
    Variable *vari[4];
    bool iswrong,ischanged;
};

#endif // SEQUENCE_H
