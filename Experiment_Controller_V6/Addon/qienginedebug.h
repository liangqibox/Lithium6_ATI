#ifndef QIENGINEDEBUG_H
#define QIENGINEDEBUG_H

#include <QWidget>
#include "qiscriptenginecluster.h"

namespace Ui {
class QiEngineDebug;
}

class QiEngineDebug : public QWidget
{
    Q_OBJECT

public:
    explicit QiEngineDebug(QWidget *parent = 0, QScriptEngine *engine = NULL);
    ~QiEngineDebug();

private slots:
    void on_input_returnPressed();
    void on_pushButton_clicked();

private:
    Ui::QiEngineDebug *ui;
    //QiScriptEngineCluster *QiEngine;
    QScriptEngine *QiEngine;
};

#endif // QIENGINEDEBUG_H
