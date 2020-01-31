#include "safety.h"
#include "ui_safety.h"

Safety::Safety(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Safety)
{
    ui->setupUi(this);
}

Safety::~Safety()
{
    delete ui;
}

void Safety::Reload(QString msg){
    if(ui->Analog_Enable->isChecked()){
        long lower = voltage_to_output(ui->Analog_Lower->text().toDouble());
        long upper = voltage_to_output(ui->Analog_Upper->text().toDouble());
        Set_Par(ANALOG_LOWER,lower);
        Set_Par(ANALOG_UPPER,upper);
    }
    if(ui->Digital_Enable->isChecked()){
        unsigned long input = 0;
        for(int i=0; i<8; i++){
            if(ui->Digital_Channels->item(i,1)->text().toInt()>0){
                input += pow(2,i);
            }
        }
        Set_Par(DIGITAL_SAFTY_INPUT1,input);
        Set_Par(DIGITAL_SAFTY_SWITCH,1);
    }
    else{
        Set_Par(DIGITAL_SAFTY_SWITCH,0);
    }
}


void Safety::on_Analog_Lower_editingFinished()
{
    bool ok = false;
    double temp = ui->Analog_Lower->text().toDouble(&ok);
    if(!ok || temp<-10){
        ui->Analog_Lower->setText("-10");
    }
}

void Safety::on_Analog_Upper_editingFinished()
{
    bool ok = false;
    double temp = ui->Analog_Upper->text().toDouble(&ok);
    if(!ok || temp>10){
        ui->Analog_Upper->setText("10");
    }
}

void Safety::on_Digital_Channels_cellChanged(int row, int column)
{
    if(column==1){
        bool ok = false;
        ui->Digital_Channels->item(row,column)->text().toInt(&ok);
        if(!ok){
            ui->Digital_Channels->item(row,column)->setText("1");
        }
    }
}

int Safety::voltage_to_output(double v){
    return 65535*(v+10)/20;
}
