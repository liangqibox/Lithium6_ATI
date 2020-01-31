#include "variableclassdisplay.h"
#include "ui_variableclassdisplay.h"

VariableClassDisplay::VariableClassDisplay(QWidget *parent, VariableClass *vc) :
    QWidget(parent),
    ui(new Ui::VariableClassDisplay)
{
    ui->setupUi(this);

    variableClass = vc;
    ui->ClassName->setText(variableClass->text(0));
    connect(variableClass->signalTrigger,SIGNAL(triggered(bool)),this,SLOT(Update()));
    Update();
}

VariableClassDisplay::~VariableClassDisplay()
{
    delete ui;
}

void VariableClassDisplay::Update(){
    ui->Display->clear();
    ui->Display->setRowCount(variableClass->get_VariableSize());

    for(int i=0; i<variableClass->get_VariableSize(); i++){
        VariableWidget *newWidget = new VariableWidget(0,variableClass->get_Variable(i));
        newWidget->set_color(variableClass->get_VariableColor(i));
        ui->Display->setCellWidget(i,0,newWidget);
    }
}

void VariableClassDisplay::set_height(int height){
    this->setGeometry(this->x(),this->y(),this->width(),height+22);
    ui->Display->setGeometry(0,22,281,height);
}

void VariableClassDisplay::on_Delete_clicked()
{
    emit DisplayRemove(this);
}

void VariableClassDisplay::on_MoveLeft_clicked()
{
    emit DisplayMoveLeft(this);
}

void VariableClassDisplay::on_MoveRight_clicked()
{
    emit DisplayMoveRight(this);
}

void VariableClassDisplay::on_MoveUp_clicked()
{
    emit DisplayMoveUp(this);
}

void VariableClassDisplay::on_MoveDown_clicked()
{
    emit DisplayMoveDown(this);
}

VariableClass* VariableClassDisplay::get_VariableClass(){
    return variableClass;
}
