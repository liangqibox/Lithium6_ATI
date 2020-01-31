#include "settingloader.h"
#include "ui_settingloader.h"

SettingLoader::SettingLoader(QWidget *parent, bool isSaver, QString defaultPath) :
    QDialog(parent),
    ui(new Ui::SettingLoader)
{
    ui->setupUi(this);

    this->setFixedSize(QSize(250,170));
    if(isSaver == true)
    {
        saver = true;
        ui->lb_desc->setText("Please select a savefile location.");
        ui->pB_activate->setText("Save");
        this->setWindowTitle("Save Settings");
        ui->cB_acq->hide();
        ui->cB_rd->hide();
        ui->cB_ROI->hide();
        ui->cB_sh->hide();
        ui->cB_file->hide();
        ui->pB_loadDef->hide();
        this->setFixedHeight(100);
    }
    else
    {
        saver = false;
        this->setWindowTitle("Load Settings");
    }
    defPath = defaultPath + QDir::separator();
    defName = "newSettings.ixoncfg";
    ui->lE_filepath->setText(defPath + defName);
}

SettingLoader::~SettingLoader()
{
    delete ui;
}

void SettingLoader::on_pB_browse_clicked()
{
    QString filepath;
    if(saver) filepath = QFileDialog::getSaveFileName(this, tr("Save File"), defPath, tr("iXonSettings (*.ixoncfg)"));
    else filepath = QFileDialog::getOpenFileName(this, tr("Choose File"), defPath, tr("iXonSettings(*.ixoncfg)"));
    if(filepath == "") return;
    filepath = QDir::toNativeSeparators(filepath);
    ui->lE_filepath->setText(filepath);
}

void SettingLoader::on_pB_cancel_clicked()
{
    this->close();
    this->deleteLater();
}

void SettingLoader::on_pB_activate_clicked()
{
    bool acq = ui->cB_acq->isChecked();
    bool rd = ui->cB_rd->isChecked();
    bool ROI = ui->cB_ROI->isChecked();
    bool sh = ui->cB_sh->isChecked();
    bool file = ui->cB_file->isChecked();
    QString filepath = ui->lE_filepath->text();

    //if(saver) emit settingSaveSignal(acq, rd, ROI, sh, file, filepath);
    if(saver) emit settingSaveSignal(true, true, true, true, true, filepath);

    else emit settingLoadSignal(acq, rd, ROI, sh, tr, filepath);

    this->close();
    this->deleteLater();

}

void SettingLoader::on_pB_loadDef_clicked()
{
    emit loadDefaultsSignal();
    this->close();
}
