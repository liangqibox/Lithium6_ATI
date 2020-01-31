#ifndef VERSIONINFORMATION_H
#define VERSIONINFORMATION_H

#include <QDialog>
#include "ixoncamera.h"

namespace Ui {
class VersionInformation;
}

class VersionInformation : public QDialog
{
    Q_OBJECT

public slots:
    void loadInfo(struct iXonCamera::systemInformation sysInfo);

public:
    explicit VersionInformation(QWidget *parent = 0);
    ~VersionInformation();

private slots:
    void on_pushButton_clicked();

private:
    Ui::VersionInformation *ui;
};

#endif // VERSIONINFORMATION_H
