#include "versioninformation.h"
#include "ui_versioninformation.h"

VersionInformation::VersionInformation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VersionInformation)
{
    ui->setupUi(this);
    this->setFixedSize(QSize(256, 273));
    this->setWindowTitle("Version Information");
}

VersionInformation::~VersionInformation()
{
    delete ui;
}

void VersionInformation::on_pushButton_clicked()
{
    this->close();
}

void VersionInformation::loadInfo(struct iXonCamera::systemInformation sysInfo)
{
    ui->lineEdit_fwv->setText(QString::number(sysInfo.firmwareVersion).append('.').append(QString::number(sysInfo.firmwareBuild)));
    ui->lineEdit_drv->setText(QString::number(sysInfo.driverVersion).append('.').append(QString::number(sysInfo.driverRevision)));
    ui->lineEdit_cof->setText(QString::number(sysInfo.COFVersion));
    ui->lineEdit_eprom->setText(QString::number(sysInfo.EPROMVersion));
    ui->lineEdit_pcbv->setText(QString::number(sysInfo.PCBVersion));
    ui->lineEdit_decv->setText(QString::number(sysInfo.decodeVersion));
    ui->lineEdit_dllv->setText(QString::number(sysInfo.dllVersion).append('.').append(QString::number(sysInfo.dllRevision)));
    ui->lineEdit_ccd->setText(sysInfo.CCDType);
}
