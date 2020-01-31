#ifndef VARIABLEMONITOR_H
#define VARIABLEMONITOR_H

#include "DataType/variable.h"
#include "qiscriptenginecluster.h"
#include <QDialog>
#include <QList>
#include <QTableWidgetItem>
#include <QKeyEvent>
#include <QSignalMapper>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QTableView>

namespace Ui {
class Variablemonitor;
}

class Variablemonitor : public QDialog
{
    Q_OBJECT

public:
    explicit Variablemonitor(QWidget *parent = 0, QList<Variable*>* varis = NULL, QiScriptEngineCluster *eng = NULL);
    ~Variablemonitor();

signals:
    void delete_variables(QString);
    void Closed();

public slots:
    void new_CycleSetting();

private slots:
    void initial();
    void Load_Default_setting();
    void Save_Default_setting();
    void Widget_resize(int index, int move);

    void on_List_fix_itemDoubleClicked(QTableWidgetItem *item);
    void on_List_scan_itemDoubleClicked(QTableWidgetItem *item);
    void on_List_calculate_itemDoubleClicked(QTableWidgetItem *item);
    void on_List_calibrate_itemDoubleClicked(QTableWidgetItem *item);
    void on_List_scan_cellChanged(int row, int column);
    void on_List_fix_cellChanged(int row, int column);
    void on_List_calculate_cellChanged(int row, int column);
    void on_List_calibrate_cellChanged(int row, int column);

    void show_filemenu(QTableWidgetItem*);
    void open_filedialog(int);
    void show_variable_menu(int);
    void receive_variname(QString name);
    void add_variable_to_calculate(int row);

    void on_List_calculate_doubleClicked(const QModelIndex &index);
    void on_List_calibrate_doubleClicked(const QModelIndex &index);

    void on_Vari_name_returnPressed();
    void on_Vari_add_clicked();
    void variable_change(QString name,bool adding);

    void on_Import_clicked();
    void on_Search_textEdited(const QString &arg1);
    void on_Search_editingFinished();

private:
    Ui::Variablemonitor *ui;

    QList<Variable*> *variables;
    int row[4];
    QString temp_filename;
    QString temp_variname;
    QString changing_variname;

    //QScriptEngine *Engine;
    QWidget *Adjust[3];
    int Widget_Size[10];
    int draging;
    QPoint mouse_pos;
    QiScriptEngineCluster *Engine;

protected:
   bool eventFilter(QObject *, QEvent *);
   void resizeEvent(QResizeEvent*resize);
};

#endif // VARIABLEMONITOR_H
