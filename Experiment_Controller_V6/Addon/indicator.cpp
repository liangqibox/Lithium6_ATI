#include "indicator.h"
#include "ui_indicator.h"

Indicator::Indicator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Indicator)
{
    ui->setupUi(this);
    ui->Front->setAutoFillBackground(true);
}

Indicator::~Indicator()
{
    delete ui;
}

void Indicator::S_Standby(){
    ui->Front->setText("Standby");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::lightGray);
    ui->Back->setPixmap(state);
}

void Indicator::S_Offline(){
    ui->Front->setText("Offline");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::gray);
    ui->Back->setPixmap(state);
}

void Indicator::S_connected(){
    ui->Front->setText("Connected");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::gray);
    ui->Back->setPixmap(state);
}

void Indicator::S_BootErr(){
    ui->Front->setText("Boot Error");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::red);
    ui->Back->setPixmap(state);
}

void Indicator::S_Ready(){
    ui->Front->setText("Ready");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::darkGreen);
    ui->Back->setPixmap(state);
}

void Indicator::S_Running(){
    ui->Front->setText("Running");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::green);
    ui->Back->setPixmap(state);
}

void Indicator::S_Scanning(){
    ui->Front->setText("Scanning");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::blue);
    ui->Back->setPixmap(state);
}

void Indicator::S_Stopping(){
    ui->Front->setText("Stopping");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::green);
    ui->Back->setPixmap(state);
}

void Indicator::S_Waiting(){
    ui->Front->setText("Waiting");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::darkGreen);
    ui->Back->setPixmap(state);
}

void Indicator::S_Stopped(){
    ui->Front->setText("Stopped");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::gray);
    ui->Back->setPixmap(state);
}

void Indicator::S_ScanFin(){
    ui->Front->setText("Scan Fin");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::gray);
    ui->Back->setPixmap(state);
}

void Indicator::S_SeqErr(){
    ui->Front->setText("Seq Error");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::yellow);
    ui->Back->setPixmap(state);
}

void Indicator::S_TransErr(){
    ui->Front->setText("Transfer Error");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::red);
    ui->Back->setPixmap(state);
}

void Indicator::S_Frozen(){
    ui->Front->setText("Frozen");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::red);
    ui->Back->setPixmap(state);
}

void Indicator::S_Calculating(double progress){
    ui->Front->setText("Calculating");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::gray);
    QPainter painter;
    painter.begin(&state);
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(Qt::blue);
    painter.setBrush(brush);
    painter.drawPie(-15,-25,200,100,90*16,90-int(progress*3.6*16));
    painter.end();
    ui->Back->setPixmap(state);
}

void Indicator::S_Uploading(double progress){
    ui->Front->setText("Uploading");
    QPixmap state = QPixmap(171,51);
    state.fill(Qt::blue);
    QPainter painter;
    painter.begin(&state);
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(Qt::darkGreen);
    painter.setBrush(brush);
    painter.drawPie(-15,-25,200,100,90*16,90-int(progress*3.6*16));
    painter.end();
    ui->Back->setPixmap(state);
}

void Indicator::Testing(){

}
