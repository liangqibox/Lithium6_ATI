#include "randomtrackinterface.h"
#include "ui_randomtrackinterface.h"

#define TRACKNUMBER 50

RandomTrackInterface::RandomTrackInterface(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RandomTrackInterface)
{
    ui->setupUi(this);
    ui->tW_RT->setColumnCount(2);
    ui->tW_RT->setRowCount(TRACKNUMBER);

    this->setWindowTitle("Random Track Setup");
    this->setFixedSize(346, 300);

}

RandomTrackInterface::~RandomTrackInterface()
{
    delete ui;
}


void RandomTrackInterface::setTableText(QVector<int> settings)
{
    if(settings.size() % 2 != 0) return;
    int RTNum = settings.size() / 2;
    for(int i = 0; i < ui->tW_RT->rowCount(); i++)
    {
        for(int j = 0; j < ui->tW_RT->columnCount(); j++)
        {
            QTableWidgetItem *temp;

            if(i < RTNum)
            {
                temp = new QTableWidgetItem(QString::number(settings.first()));
                settings.pop_front();
            }
            else
            {
                temp = new QTableWidgetItem("");
            }

            ui->tW_RT->setItem(i, j, temp);

        }
    }
    return;
}

void RandomTrackInterface::on_buttonBox_accepted()
{
    bool ok;
    int input;
    QVector<int> RTSettings = {};
    for(int i = 0; i < ui->tW_RT->rowCount(); i++)
    {
        for(int j = 0; j < ui->tW_RT->columnCount(); j++)
        {
            if(ui->tW_RT->item(i, j) == NULL) goto endloop;
            input = ui->tW_RT->item(i, j)->text().toInt(&ok);
            if(!ok || (!RTSettings.isEmpty() && input < RTSettings.last())) goto endloop;
            else if(ok) RTSettings.push_back(input);
            else goto endloop;
        }
    }
endloop:
    emit sendSettings(RTSettings);
    this->close();
}

void RandomTrackInterface::on_buttonBox_rejected()
{
    this->close();
}
