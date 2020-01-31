#include "variableexpleror.h"
#include "ui_variableexpleror.h"
#include <QDebug>

VariableExpleror::VariableExpleror(QWidget *parent, QList<Variable *> *vari, VariableClass *vc, QList<VariableClass *> *dc, QScriptEngine *eng) :
    QDialog(parent),
    ui(new Ui::VariableExpleror)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window);

    Displays = new QList<VariableClassDisplay*>;
    DisplayClass = dc;
    variables = vari;
    variableClass = vc;
    currentDisplay = variableClass;
    Engine = eng;
    ui->Class->setColumnWidth(0,156);
    ui->Class->setColumnWidth(1,43);

    Initialize();
    for(int i=0; i<DisplayClass->size(); i++)Display_add(DisplayClass->at(i));
    ui->Variable->installEventFilter(this);
    ui->Class->installEventFilter(this);
}

VariableExpleror::~VariableExpleror()
{
    delete ui;
}

void VariableExpleror::Close(){
    ui->Variable->clear();
    Displays->clear();
    this->close();
}

bool VariableExpleror::eventFilter(QObject *watched, QEvent *event){
    if(watched==ui->Variable||watched->parent()==ui->Variable){
        if(event->type()==QEvent::ContextMenu){
            show_Variable_Class_Menu();
            return false;
        }
        return false;
    }
    if(event->type()==QEvent::KeyPress){
        QKeyEvent *keyevent = static_cast<QKeyEvent*>(event);
        if(keyevent->key()==Qt::Key_Delete){
            if(watched == ui->Class){
                if(ui->Class->selectedItems().isEmpty())return false;
                QWidget* deleting = (QWidget*)ui->Class->selectedItems().first();
                Delete_class(deleting);
                return false;
            }
            else if(watched==ui->Variable||watched->parent()==ui->Variable){
                on_Del_clicked();
                return false;
            }
        }
    }
    return false;
}

void VariableExpleror::resizeEvent(QResizeEvent *){
    ui->Class->setGeometry(ui->Class->x(),ui->Class->y(),ui->Class->width(),this->height()-40);
    ui->Variable->setGeometry(ui->Variable->x(),ui->Variable->y(),ui->Variable->width(),(this->height()-40)/(1+int(Displays->size()/5))-22);
    for(int i=0; i<Displays->size(); i++){
        Displays->at(i)->move((i+1)%5*280+219,21+(this->height()-40)/(1+int(Displays->size()/5))*int((i+1)/5));
        Displays->at(i)->set_height((this->height()-40)/(1+int(Displays->size()/5))-22);
    }
}

void VariableExpleror::Initialize(){
    currentDisplay = variableClass;
    ui->Class->addTopLevelItem(variableClass);
    ui->Class->expandAll();
    ui->Class->itemAt(0,0)->setSelected(true);
    Update();
}

void VariableExpleror::Update(){
    while(ui->Variable->rowCount()>0){
        ui->Variable->removeRow(0);
    }

    for(int i=0; i<currentDisplay->get_VariableSize(); i++){
        VariableWidget *newWidget = new VariableWidget(0,currentDisplay->get_Variable(i));
        newWidget->set_color(currentDisplay->get_VariableColor(i));
        ui->Variable->setRowCount(ui->Variable->rowCount()+1);
        ui->Variable->setCellWidget(ui->Variable->rowCount()-1,0,newWidget);
        newWidget->installEventFilter(this);
    }
}

void VariableExpleror::show_Variable_Class_Menu(){
    QMenu *menu = new QMenu();
    QAction *newclass = new QAction("New Class",menu);
    //QAction *del = new QAction("Delete",menu);
    QMenu *moveto = new QMenu("Move to",menu);
    menu->addAction(newclass);
    //menu->addAction(del);
    menu->addSeparator();
    menu->addMenu(moveto);
    QSignalMapper *mapper = new QSignalMapper(newclass);
    mapper->setMapping(newclass,(QWidget*)currentDisplay);
    connect(newclass,SIGNAL(triggered()),mapper,SLOT(map()));
    connect(mapper,SIGNAL(mapped(QWidget*)),this,SLOT(Create_new_class(QWidget*)));
    
    if(variableClass->childCount()==0){
        QAction *n = new QAction("No Subclass available",moveto);
        n->setDisabled(true);
        moveto->addAction(n);
    }
    else{
        VariableClass *major = variableClass;
        VariableClass *minor = static_cast<VariableClass*>(major->child(0));
        QMenu *ma = moveto;
        bool end = false;
        while(!end){
            if(minor->childCount()!=0){
                VariableClass *temp = minor;
                major = minor;
                minor = static_cast<VariableClass*>(temp->child(0));
                QMenu *submenu = new QMenu(major->text(0),ma);
                ma->addMenu(submenu);
                ma = submenu;
            }
            else{ //Create list, move next
                QAction *v = new QAction(minor->text(0),ma);
                ma->addAction(v);
                QSignalMapper *signalmapper = new QSignalMapper(v);
                signalmapper->setMapping(v,(QWidget*)minor);
                connect(v,SIGNAL(triggered()),signalmapper,SLOT(map()));
                connect(signalmapper,SIGNAL(mapped(QWidget*)),this,SLOT(Moveto_class(QWidget*)));
                while(major->indexOfChild(minor)==(major->childCount()-1)&&!end){
                    QAction *thisfolder = new QAction(major->text(0),ma);
                    ma->addAction(thisfolder);
                    QAction* firstAction = ma->actions().first();
                    QAction* separator = ma->insertSeparator(firstAction);
                    ma->insertAction(separator,thisfolder);
                    QSignalMapper *signalmapper1 = new QSignalMapper(thisfolder);
                    signalmapper1->setMapping(thisfolder,(QWidget*)major);
                    connect(thisfolder,SIGNAL(triggered()),signalmapper1,SLOT(map()));
                    connect(signalmapper1,SIGNAL(mapped(QWidget*)),this,SLOT(Moveto_class(QWidget*)));
                    if(major!=variableClass){
                        VariableClass *temp = major;
                        minor = major;
                        major = static_cast<VariableClass*>(temp->parent());
                        ma = static_cast<QMenu*>(ma->parent());
                    }
                    else{
                        end = true;
                        break;
                    }
                }
                if(!end)minor = static_cast<VariableClass*>(major->child(major->indexOfChild(minor)+1));
            }
        }
    }

    QMenu *color = new QMenu("set color",menu);
    QStringList colors;
    colors << "White" << "Black" << "Gray" << "Red" << "Green" << "Blue" << "Yellow";
    for(int i=0; i<7; i++){
        QAction *c = new QAction(colors.at(i),color);
        QSignalMapper *signalmapper = new QSignalMapper(c);
        signalmapper->setMapping(c,i);
        connect(c,SIGNAL(triggered()),signalmapper,SLOT(map()));
        connect(signalmapper,SIGNAL(mapped(int)),this,SLOT(change_WidgetColor(int)));
        color->addAction(c);
    }
    menu->addSeparator();
    menu->addMenu(color);

    menu->popup(QCursor::pos());
}

void VariableExpleror::on_New_clicked()
{
    Variable *newVari = new Variable(0,"NewVariable",0,Engine,variables);
    variables->append(newVari);

    currentDisplay->apend_Variable(newVari);
    VariableWidget *newWidget = new VariableWidget(0,newVari);
    ui->Variable->setRowCount(ui->Variable->rowCount()+1);
    ui->Variable->setCellWidget(ui->Variable->rowCount()-1,0,newWidget);

    connect(newVari,SIGNAL(Changed()),this,SLOT(Report_Edited()));
}

void VariableExpleror::on_Del_clicked()
{
    QList<QTableWidgetSelectionRange> deleting = ui->Variable->selectedRanges();
    for(int i=0; i<deleting.size(); i++){
        for(int j=deleting.at(i).bottomRow(); j>deleting.at(i).topRow()-1; j--){
            VariableWidget *temp = static_cast<VariableWidget*>(ui->Variable->cellWidget(j,0));
            Variable *vari = temp->get_Variable();
            ui->Variable->removeRow(j);
            currentDisplay->remove_Variable(j);
            for(int k=0; k<variables->size(); k++){
                if(vari==variables->at(k)){
                    variables->removeAt(k);
                    break;
                }
            }
            delete temp;
            delete vari;
        }
    }
}

void VariableExpleror::Create_new_class(QWidget *upper){
    bool ok = false;
    QString text = QInputDialog::getText(this, tr(""),tr("New Class name:"), QLineEdit::Normal,"New Class",&ok);

    if(ok&&(!text.isEmpty())){
        VariableClass *upperclass = (VariableClass*)upper;
        VariableClass *newclass = new VariableClass;
        newclass->setText(0,text);
        upperclass->addChild(newclass);

        if(ui->Variable->hasFocus())Moveto_class((QWidget*)newclass);
    }
}

void VariableExpleror::Delete_class(QWidget *del){
    VariableClass *deleting = (VariableClass*)del;
    if(deleting==variableClass)return;

    VariableClass *upper = static_cast<VariableClass*>(deleting->parent());
    QList<Variable*> move_varis = deleting->Variables();
    if(deleting->get_VariableSize()>0){
        QMessageBox msgBox;
        msgBox.setText("This Class contain variables.");
        msgBox.setInformativeText("Move the variables to upper class?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        switch(ret){
        case QMessageBox::Cancel:
            return;
            break;
        case QMessageBox::Save:
            for(int i=0; i<move_varis.size(); i++)upper->apend_Variable(move_varis.at(i));
            deleting->remove_AllVariable();
            break;
        case QMessageBox::Discard:
            break;
        default:
            break;
        }
    }
    QList<QTreeWidgetItem*> lower = deleting->takeChildren();
    upper->addChildren(lower);
    QList<Variable*> varis = deleting->Variables();
    for(int i=0; i<varis.size(); i++){
        delete varis.at(i);
    }
    if(currentDisplay==deleting){
        currentDisplay = upper;
        currentDisplay->setSelected(true);
        Update();
    }
    upper->removeChild((QTreeWidgetItem*)deleting);
    for(int i=0; i<Displays->size(); i++){
        if(deleting==Displays->at(i)->get_VariableClass()){
            Display_remove(Displays->at(i));
        }
    }
    delete deleting;
}

void VariableExpleror::Moveto_class(QWidget *mt){
    VariableClass *moveto = (VariableClass*)mt;
    QList<QTableWidgetSelectionRange> moving = ui->Variable->selectedRanges();
    for(int i=1; i<moving.size(); i++){
        for(int j=0; j<i; j++){
            if(moving.at(i-j).topRow()<moving.at(i-j-1).topRow()){
                moving.swap(i-j,i-j-1);
            }
            else break;
        }
    }
    for(int i=moving.size()-1; i>-1; i--){
        for(int j=0; j<moving.at(i).rowCount(); j++){
            int index = moving.at(i).bottomRow()-j;
            VariableWidget *temp = static_cast<VariableWidget*>(ui->Variable->cellWidget(index,0));
            Variable *vari = temp->get_Variable();
            int color = currentDisplay->get_VariableColor(index);
            ui->Variable->removeCellWidget(index,0);
            ui->Variable->removeRow(index);
            currentDisplay->remove_Variable(index);
            //delete temp;

            moveto->apend_Variable(vari,color);
        }
    }
}

void VariableExpleror::change_WidgetColor(int color){
    QList<QTableWidgetSelectionRange> moving = ui->Variable->selectedRanges();
    for(int i=moving.size()-1; i>-1; i--){
        for(int j=0; j<moving.at(i).rowCount(); j++){
            int index = moving.at(i).bottomRow()-j;
            VariableWidget *temp = static_cast<VariableWidget*>(ui->Variable->cellWidget(index,0));
            temp->set_color(color);
            currentDisplay->set_VariableColor(index,color);
        }
    }
}

void VariableExpleror::on_Move_up_clicked()
{
    if(ui->Variable->selectedRanges().isEmpty())return;
    int selected = ui->Variable->selectedRanges().first().topRow();
    if(selected==0)return;

    currentDisplay->move_Variable(selected,selected-1);

    QWidget* socure = ui->Variable->cellWidget(selected,0);
    VariableWidget *temp = static_cast<VariableWidget*>(socure);
    VariableWidget *newWidget = new VariableWidget(0,temp->get_Variable());
    ui->Variable->removeCellWidget(selected,0);
    ui->Variable->removeRow(selected);
    ui->Variable->insertRow(selected-1);
    ui->Variable->setCellWidget(selected-1,0,(QWidget*)newWidget);
    ui->Variable->selectRow(selected-1);
}

void VariableExpleror::on_Move_down_clicked()
{
    if(ui->Variable->selectedRanges().isEmpty())return;
    int selected = ui->Variable->selectedRanges().first().topRow();
    if(selected==(ui->Variable->rowCount()-1))return;

    currentDisplay->move_Variable(selected,selected+1);
    QWidget* socure = ui->Variable->cellWidget(selected,0);
    VariableWidget *temp = static_cast<VariableWidget*>(socure);
    VariableWidget *newWidget = new VariableWidget(0,temp->get_Variable());
    ui->Variable->removeCellWidget(selected,0);
    ui->Variable->removeRow(selected);
    ui->Variable->insertRow(selected+1);
    ui->Variable->setCellWidget(selected+1,0,(QWidget*)newWidget);
    ui->Variable->selectRow(selected+1);
}

void VariableExpleror::on_Class_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
    VariableClass *vc = static_cast<VariableClass*>(current);
    currentDisplay = vc;
    Update();
}

/*void VariableExpleror::on_Class_itemClicked(QTreeWidgetItem *item, int column)
{
    if(column==0||column==1){
        VariableClass *vc = static_cast<VariableClass*>(item);
        currentDisplay = vc;
        Update();
    }
}*/

void VariableExpleror::on_AddDisplay_clicked()
{
    DisplayClass->append(currentDisplay);
    Display_add(currentDisplay);
}

void VariableExpleror::Display_add(VariableClass *add){
    VariableClassDisplay *newDisplay = new VariableClassDisplay(this,add);
    newDisplay->set_height(this->height()-62);
    newDisplay->show();
    Displays->append(newDisplay);
    connect(newDisplay,SIGNAL(DisplayRemove(VariableClassDisplay*)),this,SLOT(Display_remove(VariableClassDisplay*)));
    if(Displays->size()<5){
        this->setMaximumWidth(this->width()+280);
        this->setMinimumWidth(this->width()+280);
    }
    resizeEvent(NULL);
}

void VariableExpleror::Display_remove(VariableClassDisplay *deleting){
    int index = 0;
    for(int i=0; i<Displays->size(); i++){
        if(Displays->at(i)==deleting){
            index = i;
            break;
        }
    }
    Displays->removeAt(index);
    DisplayClass->removeAt(index);
    deleting->hide();
    delete deleting;
    if(Displays->size()<4){
        this->setMinimumWidth(this->width()-280);
        this->setMaximumWidth(this->width()-280);
    }
    resizeEvent(NULL);
}

void VariableExpleror::Display_moveLeft(VariableClassDisplay *display){

}

void VariableExpleror::Display_moveRight(VariableClassDisplay *display){

}

void VariableExpleror::Display_moveUp(VariableClassDisplay *display){

}

void VariableExpleror::Display_moveDown(VariableClassDisplay *display){

}

void VariableExpleror::on_Update_Display_clicked()
{
    for(int i=0; i<Displays->size(); i++){
        Displays->at(i)->Update();
    }
}

void VariableExpleror::Setup_report(bool buildConnection){
    if(buildConnection)for(int i=0; i<variables->size(); i++)connect(variables->at(i),SIGNAL(Changed()),this,SLOT(Report_Edited()));
    else for(int i=0; i<variables->size(); i++)disconnect(variables->at(i),SIGNAL(Changed()),this,SLOT(Report_Edited()));
}

void VariableExpleror::Report_Edited(){
    emit report_variable_Edited();
}

void VariableExpleror::raise(){

}
