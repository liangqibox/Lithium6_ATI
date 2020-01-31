#include "photoncountsetup.h"
#include "ui_photoncountsetup.h"

PhotonCountSetup::PhotonCountSetup(int maxNumberDivisions, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PhotonCountSetup),
    maxNoDiv(maxNumberDivisions)
{
    ui->setupUi(this);

    fillNoDiv();
    ui->cB_noDiv->setCurrentIndex(0);
    on_cB_noDiv_activated(ui->cB_noDiv->currentIndex());

    curRanges = {};

    this->setWindowTitle("Photon Counting Setup");

}

PhotonCountSetup::~PhotonCountSetup()
{
    delete ui;
}

void PhotonCountSetup::fillNoDiv()
{
    if(maxNoDiv != -1)
    {
        for(int i = 1; i <= maxNoDiv; i++)
        {
            ui->cB_noDiv->addItem(QString::number(i));
        }
    }
    return;
}

void PhotonCountSetup::on_cB_noDiv_activated(int index)
{
    curNoDiv = index + 1;
    manageLineEditCount(curNoDiv);
}

void PhotonCountSetup::on_buttonBox_accepted()
{
    bool ok;
    QVector<long> ranges;
    int temp;

    //prepare for some uuugly code
    temp = ui->lE_lv1->text().toInt(&ok);
    if(ok) ranges.push_back(temp);

    if(curNoDiv == 2 || curNoDiv == 3)
    {
        temp = ui->lE_lv2->text().toInt(&ok);
        if(ok) ranges.push_back(temp);
    }

    if(curNoDiv == 3)
    {
        temp = ui->lE_lv3->text().toInt(&ok);
        if(ok) ranges.push_back(temp);
    }

    temp = ui->lE_lv4->text().toInt(&ok);
    if(ok) ranges.push_back(temp);

    if(ranges.size() == curNoDiv + 1) emit photonRangesSet(ranges);
    else emit photonRangesSet(curRanges);


    return;
}

void PhotonCountSetup::showRanges(QVector<long> ranges)
{
    //this section uses the fact that noDiv == 3 for this camera
    curRanges = ranges;
    manageLineEditCount(ranges.size() - 1);
    ui->cB_noDiv->setCurrentIndex(ranges.size() - 2);
    switch(ranges.size() - 1)
    {
    case 3:
        ui->lE_lv4->setText(QString::number(ranges[3]));
        ui->lE_lv3->setText(QString::number(ranges[2]));
        ui->lE_lv2->setText(QString::number(ranges[1]));
        ui->lE_lv1->setText(QString::number(ranges[0]));
        break;
    case 2:
        ui->lE_lv4->setText(QString::number(ranges[2]));
        ui->lE_lv2->setText(QString::number(ranges[1]));
        ui->lE_lv1->setText(QString::number(ranges[0]));
        break;
    case 1:
        ui->lE_lv4->setText(QString::number(ranges[1]));
        ui->lE_lv1->setText(QString::number(ranges[0]));
        break;
    }
}

void PhotonCountSetup::manageLineEditCount(int count)
{
    //this section uses the fact that noDiv == 3 for this camera
    switch(count)
    {
    case 1:
        ui->lE_lv1->show();
        ui->lE_lv2->hide();
        ui->lE_lv3->hide();
        ui->lE_lv4->show();

        ui->lb_lv1->show();
        ui->lb_lv2->hide();
        ui->lb_lv3->hide();
        ui->lb_lv4->show();
        break;
    case 2:
        ui->lE_lv1->show();
        ui->lE_lv2->show();
        ui->lE_lv3->hide();
        ui->lE_lv4->show();

        ui->lb_lv1->show();
        ui->lb_lv2->show();
        ui->lb_lv3->hide();
        ui->lb_lv4->show();
        break;
    case 3:
        ui->lE_lv1->show();
        ui->lE_lv2->show();
        ui->lE_lv3->show();
        ui->lE_lv4->show();

        ui->lb_lv1->show();
        ui->lb_lv2->show();
        ui->lb_lv3->show();
        ui->lb_lv4->show();
        break;
    default: //should there ever be more than 3 div, dont show them
        ui->lE_lv1->show();
        ui->lE_lv2->hide();
        ui->lE_lv3->hide();
        ui->lE_lv4->show();

        ui->lb_lv1->show();
        ui->lb_lv2->hide();
        ui->lb_lv3->hide();
        ui->lb_lv4->show();
        break;
    }
}
