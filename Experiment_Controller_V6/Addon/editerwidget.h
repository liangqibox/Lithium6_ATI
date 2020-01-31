#ifndef EDITERWIDGET_H
#define EDITERWIDGET_H

#include "DataType/sequence.h"
#include "DataType/variable.h"
#include "include/qicombobox.h"
#include "definition.h"
#include <QWidget>
#include <QFont>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPixmap>
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QSignalMapper>

namespace Ui {
class Editerwidget;
}

class Editerwidget : public QWidget
{
    Q_OBJECT

public:
    explicit Editerwidget(QWidget *parent = 0);
    ~Editerwidget();
    void initial(Sequence *iseq, QList<Variable*>* ivari);
    void initial(Sequence *iseq, Sequence *iseq_trg, QList<Variable*>* ivari);
    void setWindowOpacity(qreal level);
    Sequence *seq;
    Sequence *seq_trigger;

private:
    Ui::Editerwidget *ui;
    QList<Variable*> *variables;
    QLineEdit *Value[4];
    QiCombobox *Type;
    int type;

signals:
    void Right_clicked(int i);
    void Mouse_event(QMouseEvent*, Editerwidget*);

public slots:
    void receive_variable(QString);
    void sequence_changed(bool changed);
    void set_color(QColor);

private slots:
    void Value_changing(int i);
    void Type_Changed(int index);
    void warning(bool wrong);

protected:
    bool eventFilter(QObject *, QEvent *);
    void mousePressEvent(QMouseEvent *);
};

#endif // EDITERWIDGET_H
