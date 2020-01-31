#ifndef VARIABLEWIDGET_H
#define VARIABLEWIDGET_H


#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMenu>
#include <QPixmap>
#include <QFileDialog>
#include "include/qicombobox.h"
#include "DataType/variable.h"

namespace Ui {
class VariableWidget;
}

class VariableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VariableWidget(QWidget *parent = 0, Variable *vari = NULL);
    ~VariableWidget();
    Variable* get_Variable();

public slots:
    void set_color(int color);

private slots:
    void Type_Changed(int index);
    void on_Name_cellChanged(int row, int column);
    void on_Value_cellChanged(int row, int column);
    void newSetup();

    void on_Value_cellDoubleClicked(int row, int column);
    void show_Filedialog();

private:
    Ui::VariableWidget *ui;
    Variable *variable;
    QiCombobox *Type;

    QList<QColor> colors;
};

#endif // VARIABLEWIDGET_H
