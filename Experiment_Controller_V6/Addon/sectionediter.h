#ifndef SECTIONEDITER_H
#define SECTIONEDITER_H

#include "DataType/variable.h"
#include "DataType/sequence.h"
#include "Addon/editerwidget.h"
#include "include/variableclass.h"
#include <QDialog>
#include <QFileDialog>
#include <QList>
#include <QMenu>
#include <QAction>
#include <QSignalMapper>
#include <QScrollArea>
#include <QScrollBar>
#include <QLCDNumber>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFont>
#include <QPixmap>
#include <QMouseEvent>
#include <QTimer>

namespace Ui {
class Sectionediter;
}

class Sectionediter : public QDialog
{
    Q_OBJECT

public:
    explicit Sectionediter(QWidget *parent = 0, int AO = 0, int DiO = 0);
    ~Sectionediter();
    void initial(QList<Variable*>*, VariableClass *, QList<QList<Sequence *> **> *, QList<QString>*, QList<QString>*, int *total_run);

signals:
    void section_change();
    void Closed();

private slots:
    void change_window_size(int x);
    void load_section();
    void on_Close_clicked();
    void on_Section_add_clicked();
    void on_Section_copy_clicked();
    void on_Section_delete_clicked();
    void on_Section_list_itemChanged(QListWidgetItem *item);
    void on_Section_list_clicked(const QModelIndex &index);

    void add_widget(int channel, int position, bool analog);
    void move_widget(int tochannel, int toposition, QWidget*, bool analog);
    void delete_widget(QWidget*, bool analog);
    void sort_widget();

    void calculate_time();

private:
    Ui::Sectionediter *ui;

    QList<QString> *channel_name;
    QList<Variable*> *variables;
    VariableClass *variableClass;
    QList<QList<Sequence*>**> *sections;
    QList<QString> *sections_name;

    QList<Sequence*> **current;
    QList<Editerwidget*> **widgets;

    QLCDNumber **cyc;
    QLabel *moveable;
    bool dragging;
    bool adding;
    int *total_number_of_run;
    int current_run;
    int Analog_output_channel;
    int Digital_output_channel;

protected:
    void resizeEvent(QResizeEvent* a);
    bool eventFilter(QObject *, QEvent *);
};

#endif // SECTIONEDITER_H
