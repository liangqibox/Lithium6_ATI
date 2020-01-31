#include "variablewidget.h"
#include "ui_variablewidget.h"
#include <QDebug>

VariableWidget::VariableWidget(QWidget *parent, Variable *vari) :
    QWidget(parent),
    ui(new Ui::VariableWidget)
{
    ui->setupUi(this);

    colors << Qt::white << Qt::black << Qt::gray << Qt::red << Qt::darkGreen << Qt::blue << Qt::yellow;
    set_color(0);

    variable = vari;
    Type = new QiCombobox(this);
    Type->setGeometry(10,35,60,20);
    QStringList type = QStringList() << "Fix" << "Scan" << "Calculate" << "File";
    Type->addItems(type);
    Type->show();
    Type->setCurrentIndex(variable->get_type());
    Type_Changed(variable->get_type());
    connect(Type,SIGNAL(currentIndexChanged(int)),this,SLOT(Type_Changed(int)));

    newSetup();
}

VariableWidget::~VariableWidget()
{
    delete ui;
}

Variable* VariableWidget::get_Variable(){
    return variable;
}

void VariableWidget::newSetup(){
    ui->Name->item(0,0)->setText(variable->get_name());
    disconnect(ui->Value,SIGNAL(cellChanged(int,int)),this,SLOT(on_Value_cellChanged(int,int)));
    switch(variable->get_type()){
    case 0:
        ui->Value->item(0,0)->setText(QString::number(variable->get_value()[0]));
        break;
    case 1:
        for(int i=0; i<4; i++)ui->Value->item(0,i)->setText(QString::number(variable->get_value()[i]));
        break;
    case 2:
        ui->Value->item(0,0)->setText(variable->get_fomular());
        break;
    case 3:
        ui->Value->item(0,0)->setText(variable->get_filename());
        ui->Value->item(0,1)->setText(QString::number(variable->get_fitting()));
        break;
    default:
        break;
    }
    connect(ui->Value,SIGNAL(cellChanged(int,int)),this,SLOT(on_Value_cellChanged(int,int)));
}

void VariableWidget::Type_Changed(int index){
    QStringList scan = QStringList() << "Prior" << "Start" << "End" << "Step";
    QStringList file = QStringList() << "File" << "Fitting";
    disconnect(ui->Value,SIGNAL(cellChanged(int,int)),this,SLOT(on_Value_cellChanged(int,int)));
    if(index==0){
        ui->Value->setColumnCount(1);
        ui->Value->horizontalHeaderItem(0)->setText("Value");
        ui->Value->setColumnWidth(0,(ui->Value->width()-1));
        QTableWidgetItem *v = new QTableWidgetItem(QString::number(variable->get_value()[0]));
        ui->Value->setItem(0,0,v);
    }
    else if(index==1){
        ui->Value->setColumnCount(4);
        ui->Value->setHorizontalHeaderLabels(scan);
        for(int i=0; i<ui->Value->columnCount(); i++)ui->Value->setColumnWidth(i,(ui->Value->width())/ui->Value->columnCount());
        for(int i=0; i<4; i++){
            QTableWidgetItem *item = new QTableWidgetItem(QString::number(variable->get_value()[i]));
            ui->Value->setItem(0,i,item);
        }
    }
    else if(index==2){
        ui->Value->setColumnCount(1);
        ui->Value->horizontalHeaderItem(0)->setText("Formular");
        ui->Value->setColumnWidth(0,(ui->Value->width()-1));
        QTableWidgetItem *fomular = new QTableWidgetItem(variable->get_fomular());
        ui->Value->setItem(0,0,fomular);
    }
    else if(index==3){
        ui->Value->setColumnCount(2);
        ui->Value->setHorizontalHeaderLabels(file);
        ui->Value->setColumnWidth(0,(ui->Value->width()-1)*0.8);
        ui->Value->setColumnWidth(1,(ui->Value->width()-1)*0.2);
        QTableWidgetItem *filename = new QTableWidgetItem(variable->get_filename());
        QTableWidgetItem *fitting = new QTableWidgetItem(QString::number(variable->get_fitting()));
        ui->Value->setItem(0,0,filename);
        ui->Value->setItem(0,1,fitting);
    }
    variable->set_type(index);
    connect(ui->Value,SIGNAL(cellChanged(int,int)),this,SLOT(on_Value_cellChanged(int,int)));
}

void VariableWidget::on_Name_cellChanged(int row, int column)
{
    if(row==0&&column==0){
        QString new_name = ui->Name->item(row,column)->text();
        if(new_name.contains('-')||new_name.contains('+')||new_name.contains('*')||new_name.contains('/')||new_name.contains(' ')||new_name.isEmpty()){
            ui->Name->item(row,column)->setText(variable->get_name());
        }
        else if(!variable->set_name(new_name))ui->Name->item(row,column)->setText(variable->get_name());
    }
}

void VariableWidget::on_Value_cellChanged(int row, int column)
{
    bool isNum = false;
    double temp = ui->Value->item(row,column)->text().toDouble(&isNum);
    switch(variable->get_type()){
    case 0:
        if(isNum)variable->set_value(column,temp);
        break;
    case 1:
        if(isNum)variable->set_value(column,temp);
        break;
    case 2:
        variable->set_fomular(ui->Value->item(row,column)->text());
        break;
    case 3:
        if(column==0)variable->set_file(ui->Value->item(row,column)->text());
        else if(column==1)variable->change_fitting_mode(temp);
        break;
    default:
        break;
    }
}

void VariableWidget::on_Value_cellDoubleClicked(int, int column)
{
    if(variable->get_type()==3&&column==0){
        QMenu *menu = new QMenu;
        QAction *action = new QAction("Select File",menu);
        menu->addAction(action);
        connect(action,SIGNAL(triggered()),this,SLOT(show_Filedialog()));
        menu->popup(QCursor::pos());
    }
}

void VariableWidget::show_Filedialog(){
    QString file = QFileDialog::getOpenFileName(this,tr("Select calibration file"));
    if(!file.isEmpty()){
        ui->Value->item(0,0)->setText(file);
    }
}

void VariableWidget::set_color(int color){
    QPixmap col = QPixmap(ui->Color->size());
    col.fill(colors.at(color));
    ui->Color->setPixmap(col);
}
