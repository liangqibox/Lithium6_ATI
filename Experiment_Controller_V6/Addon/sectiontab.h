#ifndef SECTIONTAB_H
#define SECTIONTAB_H

#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QSignalMapper>
#include <QInputDialog>
#include <DataType/variable.h>
#include <DataType/sectionactivity.h>
#include "include/variableclass.h"

namespace Ui {
class Sectiontab;
}

class Sectiontab : public QWidget
{
    Q_OBJECT

signals:
    void activity_change();
    void group(Sectiontab*,bool);
    void save_section(Sectiontab*,QString);
    void delete_section(Sectiontab*);

public:
    explicit Sectiontab(QWidget *parent = 0, QString name = " ", QList<Variable*> *varis = NULL, VariableClass *vc = NULL, SectionActivity *act = NULL,int *scan = 0);
    ~Sectiontab();

    void set_time(double time);
    void set_name(QString name);
    bool get_active();
    QString get_value();
    QString get_name();

public slots:
    void set_variable(QString v);

private slots:
    void on_Active_editingFinished();
    void Update();
    void set_variable(int index);
    void show_variable_menu();
    void show_operation_menu();
    void operation(int i);
    void on_Group_clicked();

private:
    Ui::Sectiontab *ui;

    QString Section_name;
    int *number_of_scan;
    QList<Variable*> *variables;
    VariableClass *variableClass;
    SectionActivity* active;

protected:
    void resizeEvent(QResizeEvent*);
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // SECTIONTAB_H
