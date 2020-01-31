#include "calculationmonitor.h"
#include "ui_calculationmonitor.h"

CalculationMonitor::CalculationMonitor(QWidget *parent, int nao, int ndio) :
    QDialog(parent),
    ui(new Ui::CalculationMonitor)
{
    ui->setupUi(this);

    const int number_of_digital_output_card = ndio/32;
    Analog_output_channel = nao;
    Digital_output_channel = ndio;
    Digital_channel_progress = new int[number_of_digital_output_card];

    grey = new QPixmap(15,15);
    green = new QPixmap(15,15);
    grey->fill(QColor(235,235,235));
    green->fill(QColor(235,235,235));
    QPainter painter(grey);
    painter.setBrush(Qt::darkGray);
    painter.drawEllipse(0,0,14,14);
    painter.end();
    painter.begin(green);
    painter.setBrush(Qt::darkGreen);
    painter.drawEllipse(0,0,14,14);
    painter.end();

    for(int i=0; i<Analog_output_channel; i++){
        QLabel *name = new QLabel(QString::number(i+1),ui->Main);
        QLabel *indicator = new QLabel(ui->Main);
        name->setGeometry(10+i%4*45,22+i/4*26,20,20);
        indicator->setGeometry(30+(i%4)*45,24+i/4*26,15,15);
        indicator->setPixmap(*grey);
        names.append(name);
        indicators.append(indicator);
    }

    QLabel *digi = new QLabel("Digital Output",ui->Main);
    int offset = 40+(Analog_output_channel+1)/4*26;
    digi->setGeometry(3,offset,100,20);
    int digicards = Digital_output_channel/32;
    for(int i=0; i<digicards; i++){
        QLabel *name = new QLabel("Card NO."+QString::number(i+1),ui->Main);
        QLabel *indicator = new QLabel(ui->Main);
        name->setGeometry(10,25+offset+i*25,62,20);
        indicator->setGeometry(72,25+offset+i*25,20,20);
        indicator->setPixmap(*grey);
        names.append(name);
        indicators.append(indicator);
    }
}

CalculationMonitor::~CalculationMonitor()
{
    delete ui;
}

void CalculationMonitor::Update(int index){
    if(index<Analog_output_channel)indicators.at(index)->setPixmap(*green);
    else{
        Digital_channel_progress[(index-Analog_output_channel)/32]+=(index-Analog_output_channel)%32+1;
        for(int i=0;i<Digital_output_channel/32;i++){
            if(Digital_channel_progress[i]>=528)indicators.at((index-Analog_output_channel)/32)->setPixmap(*green);
        }
    }
}
