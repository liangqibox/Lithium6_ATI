#ifndef CALCULATIONMONITOR_H
#define CALCULATIONMONITOR_H

#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QPainter>

namespace Ui {
class CalculationMonitor;
}

class CalculationMonitor : public QDialog
{
    Q_OBJECT

public:
    explicit CalculationMonitor(QWidget *parent = 0, int nao = 0, int ndio = 0);
    ~CalculationMonitor();

public slots:
    void Update(int index);

private:
    Ui::CalculationMonitor *ui;

    int Analog_output_channel;
    int Digital_output_channel;
    int *Digital_channel_progress;
    QPixmap *grey,*green;
    QList<QLabel*> names;
    QList<QLabel*> indicators;
};

#endif // CALCULATIONMONITOR_H
