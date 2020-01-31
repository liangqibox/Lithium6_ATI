#ifndef SETTING_H
#define SETTING_H

#include "Addon/remotecontrolserver.h"
#include <QMessageBox>
#include <QWidget>
#include <QDir>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QString>
#include <QList>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QPushButton>
#include <QTime>
#include <QTcpServer>
#include <QTcpSocket>

namespace Ui {
class Setting;
}

class Setting : public QWidget
{
    Q_OBJECT

public:
    explicit Setting(QWidget *parent = 0, QStringList setting = QStringList()<<"T11"<<"24"<<"32"<<"online", RemoteControlServer *rcs = NULL);
    ~Setting();

signals:
    void Setting_file_changed(QString);
    void System_setting_apply(QStringList);
    void Run_Clicked();
    void Scan_Clicked();
    void Load_Clicked();
    void Remote_Control_Stop();
    void Closed();

private slots:
    void on_File_import_clicked();
    void add_files(QStringList filename);
    void on_File_remove_clicked();
    void on_File_load_clicked();
    void on_Move_up_clicked();
    void on_Move_down_clicked();
    void on_Offline_mode_clicked();
    void on_Apply_clicked();
    void on_TCP_switch_clicked();
    void on_Precalculation_clicked();
    void on_StopafterScan_clicked();

    void load_setting_list();
    void write_setting_list();
    void Setting_refresh(QStringList set);

    void on_Mission_start_clicked();
    void cycle_mission_check(int state);

    void Delay(int millisecondsToWait);

protected:
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private:
    Ui::Setting *ui;

    QStringList program_setting;
    bool Mission;
    bool Mission_used_new_profile;
    int run_count;

    RemoteControlServer *RCS;
};

#endif // SETTING_H
