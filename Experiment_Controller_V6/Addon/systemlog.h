#ifndef SYSTEMLOG_H
#define SYSTEMLOG_H

#include <QDialog>
#include <QTextStream>
#include <QFile>

namespace Ui {
class Systemlog;
}

class Systemlog : public QDialog
{
    Q_OBJECT

public:
    explicit Systemlog(QWidget *parent = 0);
    ~Systemlog();

private slots:
    void on_OK_clicked();

    void on_Clear_clicked();

private:
    Ui::Systemlog *ui;
};

#endif // SYSTEMLOG_H
