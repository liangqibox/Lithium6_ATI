#ifndef RANDOMTRACKINTERFACE_H
#define RANDOMTRACKINTERFACE_H

#include <QDialog>
#include <QVector>
#include <QTableWidgetItem>
#include <QTableWidget>


namespace Ui {
class RandomTrackInterface;
}

class RandomTrackInterface : public QDialog
{
    Q_OBJECT

public:
    explicit RandomTrackInterface(QWidget *parent = 0);
    ~RandomTrackInterface();

public slots:
    void setTableText(QVector<int> settings);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::RandomTrackInterface *ui;

signals:
    void sendSettings(QVector<int> RTSettings);
};

#endif // RANDOMTRACKINTERFACE_H
