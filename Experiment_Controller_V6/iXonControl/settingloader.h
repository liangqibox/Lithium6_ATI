#ifndef SETTINGLOADER_H
#define SETTINGLOADER_H

#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <iXonControl/ixoncontrol.h>

namespace Ui {
class SettingLoader;
}

class SettingLoader : public QDialog
{
    Q_OBJECT

public:
    explicit SettingLoader(QWidget *parent = 0, bool isSaver = false, QString defaultPath = QDir::currentPath());
    ~SettingLoader();

private:
    Ui::SettingLoader *ui;
    bool saver;
    QString defPath;
    QString defName;

signals:
    void settingSaveSignal(bool acq, bool rd, bool ROI, bool shutter, bool path, QString filename);
    void settingLoadSignal(bool acq, bool rd, bool ROI, bool shutter, bool path, QString filename);
    void loadDefaultsSignal();
private slots:
    void on_pB_browse_clicked();
    void on_pB_cancel_clicked();
    void on_pB_activate_clicked();
    void on_pB_loadDef_clicked();
};

#endif // SETTINGLOADER_H
