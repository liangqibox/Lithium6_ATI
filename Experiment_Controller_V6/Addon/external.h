#ifndef EXTERNAL_H
#define EXTERNAL_H

#include "DataType/variable.h"
#include <QWidget>
#include <QComboBox>
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QMimeData>
#include <QDateTime>
#include <QTcpSocket>
#include <QUdpSocket>

namespace Ui {
class External;
}

class External : public QWidget
{
    Q_OBJECT

public:
    explicit External(QWidget *parent = 0, QList<Variable*> *vari = NULL, QList<QStringList> *files = NULL, int *tal = NULL, int cycle = 0, int scan = 0);
    ~External();

public slots:
    void reload(QString msg);
    void Reset(QList<Variable*> *vari);

signals:
    void External_Loaded(int error);

private slots:
    void on_Test_clicked();
    void on_File_load_clicked();
    void add_files(QStringList filename);
    void on_Move_up_clicked();
    void on_Move_down_clicked();
    void on_File_remove_clicked();
    void File_save();

    int Send_file();
    QString Get_IP(int file_no);
    int Get_Port(int file_no);
    QString translate_command(QString str);
    void on_File_send_clicked();
    void on_File_list_cellDoubleClicked(int row, int column);

protected:
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private:
    Ui::External *ui;

    int cycle_time;
    int number_of_scan;
    int number_of_run;
    int *total;
    QTcpSocket *tcpSocket;
    QUdpSocket *udpSocket;
    QList<Variable *> *variables;
    QList<QStringList> *External_files;
};

#endif // EXTERNAL_H
