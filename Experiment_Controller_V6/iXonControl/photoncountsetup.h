#ifndef PHOTONCOUNTSETUP_H
#define PHOTONCOUNTSETUP_H

#include <QDialog>

namespace Ui {
class PhotonCountSetup;
}

class PhotonCountSetup : public QDialog
{
    Q_OBJECT

public:
    explicit PhotonCountSetup(int maxNumberDivisions, QWidget *parent = 0);
    ~PhotonCountSetup();

private:
    Ui::PhotonCountSetup *ui;

    void fillNoDiv();
    int maxNoDiv;
    int curNoDiv;

    QVector<long> curRanges;

signals:
    photonRangesSet(QVector<long> ranges);

private slots:
    void on_cB_noDiv_activated(int index);
    void on_buttonBox_accepted();
    void manageLineEditCount(int count);

public slots:
    void showRanges(QVector<long> ranges);
};

#endif // PHOTONCOUNTSETUP_H
