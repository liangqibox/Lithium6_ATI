#ifndef AUTOSAVE_H
#define AUTOSAVE_H

#include "DataType/variable.h"
#include <QWidget>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QDateTime>

namespace Ui {
class AutoSave;
}

class AutoSave : public QWidget
{
    Q_OBJECT

signals:
    void save_setting(QString);

public:
    explicit AutoSave(QWidget *parent = 0);
    ~AutoSave();
    void init_variable(QList<Variable*> *ivar);

    void set_enable(bool en);
    void set_path(QString path);
    void set_description(QString string);
    bool get_enable();
    QString get_path();
    QString get_description();

private slots:
    void Load_default_setting();
    void on_Path_clicked();
    void Update(QString msg);

    QString save_Filename();
    void save_scan_summary();
    void save_file(QString filename);

private:
    Ui::AutoSave *ui;

    int number_of_scan;
    int number_of_run;
    double cycle_time;
    QList<Variable*> *variables;

    QDateTime startTime;
    QString save_path;
    int Scan_count;
};

#endif // AUTOSAVE_H
