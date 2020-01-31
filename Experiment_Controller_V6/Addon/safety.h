#ifndef SAFETY_H
#define SAFETY_H

#include <Adwin.h>
#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include "math.h"
#include "definition.h"

const int InterLockChannel = 8;

namespace Ui {
class Safety;
}

class Safety : public QWidget
{
    Q_OBJECT

public:
    explicit Safety(QWidget *parent = 0);
    ~Safety();

public slots:
    void Reload(QString msg);

private slots:
    void on_Analog_Lower_editingFinished();
    void on_Analog_Upper_editingFinished();
    void on_Digital_Channels_cellChanged(int row, int column);
    int voltage_to_output(double v);

private:
    Ui::Safety *ui;
};

#endif // SAFETY_H
