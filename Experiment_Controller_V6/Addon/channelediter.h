#ifndef CHANNELEDITER_H
#define CHANNELEDITER_H

#include "DataType/sequence.h"
#include "DataType/variable.h"
#include "DataType/sectionactivity.h"
#include "include/variableclass.h"
#include "editerwidget.h"
#include "sectiontab.h"
#include <QList>
#include <QTime>
#include <QTimer>
#include <QDialog>
#include <QFrame>
#include <QCursor>
#include <QMenu>
#include <QAction>
#include <QSignalMapper>
#include <QScrollArea>
#include <QScrollBar>
#include <QLCDNumber>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QFont>
#include <QPixmap>
#include <QMouseEvent>

namespace Ui {
class Channelediter;
}

class Channelediter : public QDialog
{
    Q_OBJECT

public:
    explicit Channelediter(QWidget *parent = 0, int AO = 24, int DiO = 32);
    ~Channelediter();
    void initial(QList<QList<Sequence *>**>*iseq, QList<Variable *> *ivari, VariableClass* vc, QList<QList<Sequence *> **> *isec, QList<QString> *isecn,
                 QList<QString> *isec_ay, QList<SectionActivity*> *isec_act, QList<QString> *ichlname, QList<QString>* chl_cal,
                 QList<int> *chl_col, int *total_run, int *number_of_scan);

signals:
    void report_cycle_Edited();
    void apply_clicked();
    void Closed();

public slots:
    void Clear();
    void calcualte_times();

private slots:
    void change_window_size(int x);
    void show_calibration_menu(int chl);
    void channel_name_changed(QString a);
    void channel_calibration_changed(QString receive);
    void section_groupped(Sectiontab *tab,bool groupped);

    void add_widget(int channel, int position, bool analog, Sequence *seq);
    void move_widget(int tochannel, int toposition, QWidget *w, bool analog);
    void copy_widget(int tochannel, int toposition, QWidget *w, bool analog);
    void delete_widget(QWidget *, bool analog);
    void add_section(int position);
    void delete_section(Sectiontab *);
    void Section_save(Sectiontab *tab,QString name);
    void sort_widget();

    void on_Change_apply_clicked();
    void on_edit_lock_clicked();
    void section_names_import();

    void background_move(int x);

    void Report_Edited();
    void debug_function(int i);
    void Delay(int millisecondsToWait);

private:
    Ui::Channelediter *ui;
    QLabel *moveable;
    QLabel *moveableS;
    bool dragging;
    bool moving_ew;
    bool adding_ew;
    bool moving_sec;
    bool adding_sec;
    bool changing_color;
    int Analog_output_channel;
    int Digital_output_channel;
    int *total_number_of_run;
    int *current_run;
    QColor color[10];
    QList<Sectiontab*> *section_tab;
    QList<QLineEdit*> *chlname;
    QList<QLabel*> *cal_label;
    QList<QPushButton*> *digital_invert;
    QList<QLabel*> *background;
    QList<QList<Editerwidget*>**> *widgets;
    QList<QFrame*> *section_seperator;
    QList<QFrame*> *section_dummyBlock;

    QList<Variable*> *variables;
    VariableClass *variableClass;
    QList<QList<Sequence*>**> *sequences;
    QList<QString> *sections_name;
    QList<QString> *section_array;
    QList<SectionActivity*> *sections_activity;
    QList<bool> sections_group;
    QList<QString> *channel_name;
    QList<int> *channel_color;
    QList<QString> *channel_calibration;
    QList<QList<Sequence*>**> *sections;
    QList<QList<Sequence*>*> *DDS_sequences;
    QList<int> *DDS_channels;

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void resizeEvent(QResizeEvent*);
};

#endif // SECTIONEDITER_H
