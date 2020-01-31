#ifndef INDICATOR_H
#define INDICATOR_H

#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QBrush>

namespace Ui {
class Indicator;
}

class Indicator : public QWidget
{
    Q_OBJECT

public:
    explicit Indicator(QWidget *parent = 0);
    ~Indicator();

public slots:
    void S_Standby();
    void S_Offline();
    void S_connected();
    void S_BootErr();
    void S_Ready();
    void S_Running();
    void S_Scanning();
    void S_Stopping();
    void S_Waiting();
    void S_Stopped();
    void S_ScanFin();
    void S_SeqErr();
    void S_TransErr();
    void S_Frozen();
    void S_Calculating(double progress);
    void S_Uploading(double progress);

    void Testing();

private:
    Ui::Indicator *ui;
};

#endif // INDICATOR_H
