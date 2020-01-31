#include "systemlog.h"
#include "ui_systemlog.h"

Systemlog::Systemlog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Systemlog)
{
    ui->setupUi(this);

    QFile file("System_log.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))return;
    QTextStream in(&file);

    ui->Display->setText(in.readAll());
    file.close();
}

Systemlog::~Systemlog()
{
    delete ui;
}

void Systemlog::on_OK_clicked()
{
    accept();
}

void Systemlog::on_Clear_clicked()
{
    ui->Display->clear();
    QFile file("System_log.txt");
    if(file.remove()){
        file.open(QIODevice::WriteOnly);
        file.close();
    }
}
