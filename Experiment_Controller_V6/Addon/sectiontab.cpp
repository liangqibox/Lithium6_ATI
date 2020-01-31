#include "sectiontab.h"
#include "ui_sectiontab.h"

#include <QDebug>

Sectiontab::Sectiontab(QWidget *parent, QString name, QList<Variable*> *varis, VariableClass *vc, SectionActivity *act , int *scan) :
    QWidget(parent),
    ui(new Ui::Sectiontab)
{
    ui->setupUi(this);

    active = act;
    number_of_scan = scan;
    variableClass = vc;
    Section_name = name;
    ui->Name->setText(name);
    variables = varis;

    ui->Active->setContextMenuPolicy(Qt::NoContextMenu);
    ui->Active->installEventFilter(this);
    ui->Name->installEventFilter(this);
    if(active->get_variable()!=NULL){
        connect(active->get_variable(),SIGNAL(Changed()),this,SLOT(Update()));
        ui->Active->setText(active->get_variable()->get_name());
        ui->Active->setDisabled(true);
    }
    else{
        ui->Active->setText(QString::number(active->get_value()));
    }

    Update();
}

Sectiontab::~Sectiontab()
{
    delete ui;
}

bool Sectiontab::eventFilter(QObject *obj, QEvent *event){
    QMouseEvent *a = static_cast<QMouseEvent*>(event);
    if(event->type()==QEvent::MouseButtonPress && a->button()==Qt::RightButton){
        if(obj==ui->Active){
            show_variable_menu();
            return true;
        }
        else{
            show_operation_menu();
            return true;
        }
    }
    return false;
}

void Sectiontab::resizeEvent(QResizeEvent *){
    ui->Back->setGeometry(0,0,this->width(),this->height());
    ui->Time->move(this->width()-80,ui->Time->y());
    ui->Active->move(this->width()-80,ui->Active->y());
    ui->Group->move(this->width()-31,ui->Group->y());
    ui->Name->setGeometry(10,10,this->width()-96,51);
    ui->Active_indicator->setGeometry(3,65,this->width()-5,5);
    Update();
}

void Sectiontab::Update(){
    if(active->get_activity(*number_of_scan)){
        QPixmap background = QPixmap(this->width(),5);
        background.fill(Qt::darkGreen);
        ui->Active_indicator->setPixmap(background);
    }
    else{
        QPixmap background = QPixmap(this->width(),5);
        background.fill(Qt::red);
        ui->Active_indicator->setPixmap(background);
    }
}

void Sectiontab::set_time(double time){
    ui->Time->display(time);
}

void Sectiontab::set_name(QString name){
    ui->Name->setText(name);
}

bool Sectiontab::get_active(){
    return active->get_activity(*number_of_scan);
}

QString Sectiontab::get_value(){
    return ui->Active->text();
}

QString Sectiontab::get_name(){
    return ui->Name->text();
}

void Sectiontab::set_variable(QString v){
    if(v == "!only_system_used_null"){
        set_variable(-1);
        return;
    }
    for(int i=0; i<variables->size(); i++){
        if(variables->at(i)->get_name()==v){
            set_variable(i);
            return;
        }
    }
    ui->Active->setText(v);
    on_Active_editingFinished();
}

void Sectiontab::set_variable(int index){
    if(index == -1){
        disconnect(active->get_variable(),SIGNAL(name_changed(QString)),ui->Active,SLOT(setText(QString)));
        disconnect(active->get_variable(),SIGNAL(Changed()),this,SLOT(Update()));
        active->set_variable(NULL);
        ui->Active->setText("1");
        ui->Active->setEnabled(true);
    }
    else{
        active->set_variable(variables->at(index));
        ui->Active->setText(variables->at(index)->get_name());
        ui->Active->setDisabled(true);
        connect(variables->at(index),SIGNAL(name_changed(QString)),ui->Active,SLOT(setText(QString)));
        connect(active->get_variable(),SIGNAL(Changed()),this,SLOT(Update()));
    }
    Update();
}

void Sectiontab::on_Active_editingFinished()
{
    if(active->get_variable()!=NULL)return;
    bool ok = false;
    double temp = ui->Active->text().toDouble(&ok);
    if(ok){
        active->set_value(temp);
    }
    else{
        ui->Active->setText("1");
        active->set_value(1);
    }
    Update();
}

void Sectiontab::show_variable_menu(){
    QMenu *menu = new QMenu(this);
    if(active->get_variable()!=NULL){
        QAction *action = new QAction(menu);
        QSignalMapper *signalmap = new QSignalMapper(menu);
        action->setText("Delete Variable");
        menu->addAction(action);
        connect(action,SIGNAL(triggered()),signalmap,SLOT(map()));
        signalmap->setMapping(action,"!only_system_used_null");
        connect(signalmap,SIGNAL(mapped(QString)),this,SLOT(set_variable(QString)));
        menu->addSeparator();
    }
    VariableClass *vc = variableClass;
    QMenu *m = menu;
    do{
        for(int j=0; j<vc->get_VariableSize(); j++){
            QAction *action = new QAction(vc->get_Variable(j)->get_name(),m);
            QSignalMapper *signalmap = new QSignalMapper(m);
            if(vc->get_Variable(j)==active->get_variable()){
                action->setCheckable(true);
                action->setChecked(true);
            }
            m->addAction(action);
            connect(action,SIGNAL(triggered()),signalmap,SLOT(map()));
            signalmap->setMapping(action,vc->get_Variable(j)->get_name());
            connect(signalmap,SIGNAL(mapped(QString)),this,SLOT(set_variable(QString)));
        }
        if(vc->get_VariableSize()==0){
            QAction *empty = new QAction(m);
            empty->setText("Empty Class");
            empty->setDisabled(true);
            m->addAction(empty);
        }
        m->addSeparator();
        if(vc->childCount()>0){
            vc = (VariableClass*)vc->child(0);
            QMenu *temp = new QMenu(vc->text(0),m);
            m->addMenu(temp);
            m = temp;
        }
        else if(vc!=variableClass){
            if((vc->parent()->indexOfChild(vc)+1)<vc->parent()->childCount()){
                vc = (VariableClass*)vc->parent()->child(vc->parent()->indexOfChild(vc)+1);
                QMenu *upper = (QMenu*)m->parent();
                QMenu *temp = new QMenu(vc->text(0),upper);
                upper->addMenu(temp);
                m = temp;
                }
            else{
                while((vc->parent()->indexOfChild(vc)+1)==vc->parent()->childCount()){
                    vc = static_cast<VariableClass*>(vc->parent());
                    m = (QMenu*)m->parent();
                    if(vc==variableClass)break;
                }
                if(vc!=variableClass){
                    vc = (VariableClass*)vc->parent()->child(vc->parent()->indexOfChild(vc)+1);
                    QMenu *upper = (QMenu*)m->parent();
                    QMenu *temp = new QMenu(vc->text(0),upper);
                    upper->addMenu(temp);
                    m = temp;
                }
            }
        }
    }while(vc!=variableClass);

    /*QMenu *menu = new QMenu(this);
    QMenu *submenu[3];
    submenu[0] = new QMenu("Fixed",menu);
    submenu[1] = new QMenu("Scaned",menu);
    submenu[2] = new QMenu("Calculated",menu);
    menu->addSection("Use Variable");

    QString vari_name = "SYSTEM_NULL";
    if(active->get_variable()!=NULL)vari_name = active->get_variable()->get_name();
    for(int i=0; i<variables->size();i++){
        QAction *action = new QAction(menu);
        QSignalMapper *signalmap = new QSignalMapper(menu);
        action->setText(variables->at(i)->get_name());
        action->setCheckable(true);
        if(variables->at(i)->get_name()==vari_name)action->setChecked(true);
        if(variables->at(i)==active->get_variable())action->setChecked(true);
        if(variables->at(i)->get_type()<3)submenu[variables->at(i)->get_type()]->addAction(action);
        connect(action,SIGNAL(triggered()),signalmap,SLOT(map()));
        signalmap->setMapping(action,i);
        connect(signalmap,SIGNAL(mapped(int)),this,SLOT(set_variable(int)));
    }
    for(int i=0; i<3; i++){
        if(submenu[i]->isEmpty()){
           QAction *null = new QAction(menu);
            submenu[i]->addAction(null);
            null->setText("None");
            null->setDisabled(true);
            null->setCheckable(false);
        }
    }
    menu->addMenu(submenu[0]);
    menu->addMenu(submenu[1]);
    menu->addMenu(submenu[2]);
    if(active->get_variable()!=NULL){
        QAction *action = new QAction(menu);
        QSignalMapper *signalmap = new QSignalMapper(menu);
        action->setText("Delete Variable");
        menu->addAction(action);
        connect(action,SIGNAL(triggered()),signalmap,SLOT(map()));
        signalmap->setMapping(action,-1);
        connect(signalmap,SIGNAL(mapped(int)),this,SLOT(set_variable(int)));
    }*/
    menu->popup(QCursor::pos());
}

void Sectiontab::show_operation_menu(){
    QStringList operationList;
    operationList << "Save Section" << "Delete Section";
    QMenu *menu = new QMenu(this);

    for(int i=0; i<operationList.size(); i++){
        QAction *action = new QAction(operationList.at(i),menu);
        QSignalMapper *signalmapper = new QSignalMapper(action);
        signalmapper->setMapping(action,i);
        connect(action,SIGNAL(triggered()),signalmapper,SLOT(map()));
        connect(signalmapper,SIGNAL(mapped(int)),this,SLOT(operation(int)));
        menu->addAction(action);
    }
    menu->popup(QCursor::pos());
}

void Sectiontab::operation(int i){
    QString name;
    bool ok = false;
    switch (i) {
    case 0:
        name = QInputDialog::getText(this,tr("QInputDialog::getText()"),tr("Section Name:"),QLineEdit::Normal,ui->Name->text(),&ok);
        if (ok&&!name.isEmpty()){
            Section_name = name;
            ui->Name->setText(Section_name);
            emit save_section(this,Section_name);
        }
        break;
    case 1:
        emit delete_section(this);
        break;
    default:
        break;
    }
}

void Sectiontab::on_Group_clicked()
{
    if(ui->Group->text()=="-"){
        ui->Group->setText("+");
        emit group(this,true);
    }
    else{
        ui->Group->setText("-");
        emit group(this,false);
    }
}
