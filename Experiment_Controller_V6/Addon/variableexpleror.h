#ifndef VARIABLEEXPLEROR_H
#define VARIABLEEXPLEROR_H

#include <QDialog>
#include <QTableWidgetSelectionRange>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QMouseEvent>
#include <QSignalMapper>
#include "variablewidget.h"
#include "include/variableclass.h"
#include "include/variableclassdisplay.h"
#include "qiscriptenginecluster.h"

namespace Ui {
class VariableExpleror;
}

class VariableExpleror : public QDialog
{
    Q_OBJECT

public:
    explicit VariableExpleror(QWidget *parent = 0, QList<Variable*> *vari = NULL, VariableClass *vc = NULL, QList<VariableClass*> *dc = NULL, QScriptEngine *eng = NULL);
    ~VariableExpleror();

public slots:
    void Initialize();
    void Setup_report(bool buildConnection);
    void raise();
    void Close();

signals:
    void report_variable_Edited();

private slots:
    void Update();

    void on_Move_up_clicked();
    void on_Move_down_clicked();
    //void on_Class_itemClicked(QTreeWidgetItem *item, int column);
    void on_Class_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void show_Variable_Class_Menu();
    void on_AddDisplay_clicked();
    void on_Update_Display_clicked();

    void on_New_clicked(); //Create variable
    void on_Del_clicked(); //Delete variable
    void Create_new_class(QWidget *upper);
    void Delete_class(QWidget *del);
    void Moveto_class(QWidget *mt);
    void change_WidgetColor(int color);

    void Display_add(VariableClass *add);
    void Display_remove(VariableClassDisplay *deleting);
    void Display_moveLeft(VariableClassDisplay *display);
    void Display_moveRight(VariableClassDisplay *display);
    void Display_moveUp(VariableClassDisplay *display);
    void Display_moveDown(VariableClassDisplay *display);

    void Report_Edited();

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    Ui::VariableExpleror *ui;
    //QiScriptEngineCluster *Engine;
    QScriptEngine *Engine;

    VariableClass *variableClass;
    VariableClass *currentDisplay;
    QList<VariableClass*> *DisplayClass;
    QList<VariableClassDisplay*> *Displays;
    QList<Variable*> *variables;
};

#endif // VARIABLEEXPLEROR_H
