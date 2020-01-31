#include "variablemonitor.h"
#include "ui_variablemonitor.h"
#include <QDebug>

Variablemonitor::Variablemonitor(QWidget *parent, QList<Variable *> *varis, QiScriptEngineCluster *eng) :
    QDialog(parent),
    ui(new Ui::Variablemonitor)
{
    ui->setupUi(this);
    ui->List_fix->installEventFilter(this);
    ui->List_scan->installEventFilter(this);
    ui->List_calculate->installEventFilter(this);
    ui->List_calibrate->installEventFilter(this);

    ui->List_fix->setColumnWidth(0,89);
    ui->List_fix->setColumnWidth(1,40);
    ui->List_scan->setColumnWidth(0,90);
    ui->List_scan->setColumnWidth(1,42);
    ui->List_scan->setColumnWidth(2,42);
    ui->List_scan->setColumnWidth(3,42);
    ui->List_scan->setColumnWidth(4,42);
    ui->List_calculate->setColumnWidth(0,90);
    ui->List_calculate->setColumnWidth(1,179);
    ui->List_calibrate->setColumnWidth(0,90);
    ui->List_calibrate->setColumnWidth(1,139);
    ui->List_calibrate->setColumnWidth(2,30);

    for(int i=0; i<3; i++){
        Adjust[i] = new QWidget(this);
        Adjust[i]->installEventFilter(this);
        Adjust[i]->setCursor(Qt::SizeHorCursor);
        Adjust[i]->setMouseTracking(true);
    }
    Adjust[0]->setGeometry(140,40,20,540);
    Adjust[1]->setGeometry(420,40,20,540);
    Adjust[2]->setGeometry(710,40,20,540);

    variables = varis;
    Engine = eng;
    draging = -1;
    initial();
    Load_Default_setting();
}

Variablemonitor::~Variablemonitor()
{
    Save_Default_setting();
    emit Closed();
    delete ui;
}

bool Variablemonitor::eventFilter(QObject *obj, QEvent *event){
    if(event->type()==QKeyEvent::KeyPress){
        QKeyEvent *a = static_cast<QKeyEvent*>(event);
        if(a->key()==Qt::Key_Delete){
            QTableWidget *b = static_cast<QTableWidget*>(obj);
            int j=0;
            if(b==ui->List_fix)j=0;
            else if(b==ui->List_scan)j=1;
            else if(b==ui->List_calculate)j=2;
            else if(b==ui->List_calibrate)j=3;
            QList<QTableWidgetItem*> items = b->selectedItems();

            for(int i=0; i<items.size(); i++){
                if(items.at(i)->row()!=(-1)){
                    for(int k=0; k<variables->size(); k++){
                        if(variables->at(k)->get_name()==b->item(items.at(i)->row(),0)->text()){
                            delete variables->at(k);
                            variables->removeAt(k);
                            break;
                        }
                    }
                    b->removeRow(items.at(i)->row());
                    row[j]--;
                }
            }
            return true;
        }
        return true;
    }
    else if(event->type()==QEvent::MouseButtonPress && draging<0){
        QMouseEvent *a = static_cast<QMouseEvent*>(event);
        for(int i=0; i<3; i++){
            if(obj==Adjust[i] && a->button()==Qt::LeftButton){
                draging=i;
                return true;
            }
        }
        return false;
    }
    else if(event->type()==QEvent::MouseMove && draging>-1){
        QMouseEvent *a = static_cast<QMouseEvent*>(event);
        int x = a->x() - Adjust[draging]->width()/2;
        Widget_resize(draging,x);
    }
    else if(event->type()==QEvent::MouseButtonRelease && draging>-1){
        draging = -1;
        return true;
    }
    return false;
}

void Variablemonitor::initial(){
    temp_filename = "null";
    changing_variname = "null";
    for(int i=0; i<4; i++)row[i]=0;
    for(int i=0; i<variables->size();i++){
        if(variables->at(i)->get_type()==0){
            QTableWidgetItem *temp = new QTableWidgetItem;
            QTableWidgetItem *temp1 = new QTableWidgetItem;
            temp->setText(variables->at(i)->get_name());
            temp1->setText(QString::number(variables->at(i)->get_value()[0]));
            ui->List_fix->setRowCount(row[0]+1);
            ui->List_fix->setItem(row[0],0,temp);
            ui->List_fix->setItem(row[0],1,temp1);
            row[0]++;
        }
        else if(variables->at(i)->get_type()==1){
            QTableWidgetItem *temp = new QTableWidgetItem;
            QTableWidgetItem *temp1 = new QTableWidgetItem;
            QTableWidgetItem *temp2 = new QTableWidgetItem;
            QTableWidgetItem *temp3 = new QTableWidgetItem;
            QTableWidgetItem *temp4 = new QTableWidgetItem;
            temp->setText(variables->at(i)->get_name());
            temp1->setText(QString::number(variables->at(i)->get_value()[0]));
            temp2->setText(QString::number(variables->at(i)->get_value()[1]));
            temp3->setText(QString::number(variables->at(i)->get_value()[2]));
            temp4->setText(QString::number(variables->at(i)->get_value()[3]));
            ui->List_scan->setRowCount(row[1]+1);
            ui->List_scan->setItem(row[1],0,temp);
            ui->List_scan->setItem(row[1],1,temp1);
            ui->List_scan->setItem(row[1],2,temp2);
            ui->List_scan->setItem(row[1],3,temp3);
            ui->List_scan->setItem(row[1],4,temp4);
            row[1]++;
        }
        else if(variables->at(i)->get_type()==2){
            QTableWidgetItem *temp = new QTableWidgetItem;
            QTableWidgetItem *temp1 = new QTableWidgetItem;
            temp->setText(variables->at(i)->get_name());
            temp1->setText(variables->at(i)->get_fomular());
            ui->List_calculate->setRowCount(row[2]+1);
            ui->List_calculate->setItem(row[2],0,temp);
            ui->List_calculate->setItem(row[2],1,temp1);
            row[2]++;
        }
        else{
            QTableWidgetItem *temp = new QTableWidgetItem;
            QTableWidgetItem *temp1 = new QTableWidgetItem;
            QTableWidgetItem *temp2 = new QTableWidgetItem;
            temp->setText(variables->at(i)->get_name());
            temp1->setText(variables->at(i)->get_filename());
            temp2->setText(QString::number(variables->at(i)->get_fitting()));
            ui->List_calibrate->setRowCount(row[3]+1);
            ui->List_calibrate->setItem(row[3],0,temp);
            ui->List_calibrate->setItem(row[3],1,temp1);
            ui->List_calibrate->setItem(row[3],2,temp2);
            row[3]++;
        }
    }
}

void Variablemonitor::new_CycleSetting(){

}

void Variablemonitor::Load_Default_setting(){
    //VariableMonitor
    //10,131,160,261,440,271,730,261,1000,600
    Widget_Size[0] = 10;
    Widget_Size[1] = 131;
    Widget_Size[2] = 160;
    Widget_Size[3] = 261;
    Widget_Size[4] = 440;
    Widget_Size[5] = 271;
    Widget_Size[6] = 730;
    Widget_Size[7] = 261;
    Widget_Size[8] = 1000;
    Widget_Size[9] = 600;

    QFile file("Config.txt");
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))return;
    QTextStream in(&file);
    QStringList configlist;
    while(!in.atEnd()){
        QString temp = in.readLine();
        if(temp=="VariableMonitor"){
            QString config = in.readLine();
            configlist = config.split(',');
            break;
        }
    }
    file.close();
    for(int i=0; i<10; i++){
        Widget_Size[i] = configlist.at(i).toInt();
    }
    this->setGeometry(100,200,Widget_Size[8],Widget_Size[9]);
}

void Variablemonitor::Save_Default_setting(){
    QFile file("Config.txt");
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))return;
    QTextStream in(&file);
    QStringList all;
    while(!in.atEnd()){
        all.append(in.readLine());
    }
    file.close();

    QString replace;
    for(int i=0; i<10; i++)replace.append(","+QString::number(Widget_Size[i]));
    replace.remove(0,1);
    for(int i=0; i<all.size(); i++){
        if(all.at(i)=="VariableMonitor"){
            all.removeAt(i+1);
            all.insert(i+1,replace);
            break;
        }
    }

    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))return;
    QTextStream out(&file);
    for(int i=0; i<all.size(); i++) out << all.at(i) << endl;
    file.close();
}

void Variablemonitor::Widget_resize(int index, int move){
    double x = double(this->width())/1000.;
    int y = this->height();
    switch(index){
    case 0:
        ui->List_fix->setGeometry(ui->List_fix->x(),40,ui->List_fix->width()+move,y-60);
        ui->List_scan->setGeometry(ui->List_scan->x()+move,40,ui->List_scan->width()-move,y-60);
        Adjust[0]->setGeometry(ui->List_fix->x()+ui->List_fix->width(),40,ui->List_scan->x()-ui->List_fix->x()-ui->List_fix->width(),y-60);
        break;
    case 1:
        ui->List_scan->setGeometry(ui->List_scan->x(),40,ui->List_scan->width()+move,y-60);
        ui->List_calculate->setGeometry(ui->List_calculate->x()+move,40,ui->List_calculate->width()-move,y-60);
        Adjust[1]->setGeometry(ui->List_scan->x()+ui->List_scan->width(),40,ui->List_calculate->x()-ui->List_scan->x()-ui->List_scan->width(),y-60);
        break;
    case 2:
        ui->List_calculate->setGeometry(ui->List_calculate->x(),40,ui->List_calculate->width()+move,y-60);
        ui->List_calibrate->setGeometry(ui->List_calibrate->x()+move,40,ui->List_calibrate->width()-move,y-60);
        Adjust[2]->setGeometry(ui->List_calculate->x()+ui->List_calculate->width(),40,ui->List_calibrate->x()-ui->List_calculate->x()-ui->List_calculate->width(),y-60);
        break;
    }
    Widget_Size[1] = ui->List_fix->width()/x;
    Widget_Size[2] = ui->List_scan->x()/x;
    Widget_Size[3] = ui->List_scan->width()/x;
    Widget_Size[4] = ui->List_calculate->x()/x;
    Widget_Size[5] = ui->List_calculate->width()/x;
    Widget_Size[6] = ui->List_calibrate->x()/x;
    Widget_Size[7] = ui->List_calibrate->width()/x;
    Widget_Size[8] = this->width();
    Widget_Size[9] = this->height();
}

void Variablemonitor::resizeEvent(QResizeEvent *resize){
    double x = double(resize->size().width())/1000.;
    if(x<0) x =1;
    int y = this->height();

    ui->List_fix->setGeometry(Widget_Size[0],40,Widget_Size[1]*x,y-60);
    ui->List_scan->setGeometry(Widget_Size[2]*x,40,Widget_Size[3]*x,y-60);
    Adjust[0]->setGeometry(ui->List_fix->x()+ui->List_fix->width(),40,ui->List_scan->x()-ui->List_fix->x()-ui->List_fix->width(),y-60);
    ui->List_calculate->setGeometry(Widget_Size[4]*x,40,Widget_Size[5]*x,y-60);
    Adjust[1]->setGeometry(ui->List_scan->x()+ui->List_scan->width(),40,ui->List_calculate->x()-ui->List_scan->x()-ui->List_scan->width(),y-60);
    ui->List_calibrate->setGeometry(Widget_Size[6]*x,40,Widget_Size[7]*x,y-60);
    Adjust[2]->setGeometry(ui->List_calculate->x()+ui->List_calculate->width(),40,ui->List_calibrate->x()-ui->List_calculate->x()-ui->List_calculate->width(),y-60);

    ui->List_fix->setColumnWidth(1,ui->List_fix->width()-ui->List_fix->columnWidth(0)-2);
    ui->List_scan->setColumnWidth(1,(ui->List_scan->width()-ui->List_fix->columnWidth(0))/4-2);
    ui->List_scan->setColumnWidth(2,(ui->List_scan->width()-ui->List_fix->columnWidth(0))/4-1);
    ui->List_scan->setColumnWidth(3,(ui->List_scan->width()-ui->List_fix->columnWidth(0))/4);
    ui->List_scan->setColumnWidth(4,(ui->List_scan->width()-ui->List_fix->columnWidth(0))/4);
    ui->List_calculate->setColumnWidth(1,ui->List_calculate->width()-ui->List_calculate->columnWidth(0)-2);
    ui->List_calibrate->setColumnWidth(1,ui->List_calibrate->width()-ui->List_calibrate->columnWidth(0)-ui->List_calibrate->columnWidth(2)-2);

    Widget_Size[8] = this->width();
    Widget_Size[9] = this->height();
}

void Variablemonitor::on_List_fix_itemDoubleClicked(QTableWidgetItem *item)
{
    if(item->column()==0){
        changing_variname = item->text();
    }
}

void Variablemonitor::on_List_scan_itemDoubleClicked(QTableWidgetItem *item)
{
    if(item->column()==0){
        changing_variname = item->text();
    }
}

void Variablemonitor::on_List_calculate_itemDoubleClicked(QTableWidgetItem *item)
{
    if(item->column()==0){
        changing_variname = item->text();
    }
}

void Variablemonitor::on_List_calibrate_itemDoubleClicked(QTableWidgetItem *item)
{
    if(item->column()==0){
        changing_variname = item->text();
    }
}

void Variablemonitor::on_List_scan_cellChanged(int row, int column)
{
    if(column!=0){
    bool ok = true;
    for(int i=0;i<variables->size();i++){
        if(ui->List_scan->item(row,0)->text()==variables->at(i)->get_name()){
            double v = ui->List_scan->item(row,column)->text().toDouble(&ok);
            if(ok)variables->at(i)->set_value(column-1,v);
            else {
                QMessageBox msgBox;

                QFont font;
                font.setBold(true);
                font.setPointSize(12);
                msgBox.setFont(font);
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText("Invaild Input (Not a number).");
                msgBox.exec();
                ui->List_scan->item(row,column)->setText(QString::number(variables->at(i)->get_value()[column-1]));
            }
            return;
        }
    }
    }
    else if(changing_variname!="null"){
        QString newname = ui->List_scan->item(row,column)->text();
        bool exsit = false;
        for(int i=0; i<variables->size(); i++){
            if(variables->at(i)->get_name()==newname){
                exsit = true;
                break;
            }
        }
        if(exsit){
            ui->List_scan->item(row,column)->setText(changing_variname);
            changing_variname = "null";
            QMessageBox msgBox;

            QFont font;
            font.setBold(true);
            font.setPointSize(12);
            msgBox.setFont(font);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText("Name alreadly exsited.");
            msgBox.exec();
        }
        else{
            for(int i=0; i<variables->size(); i++){
                if(variables->at(i)->get_name()==changing_variname){
                    variables->at(i)->set_name(newname);
                    changing_variname = "null";
                    break;
                }
            }
        }
    }
}

void Variablemonitor::on_List_fix_cellChanged(int row, int column)
{
    if(column!=0){
    bool ok = true;
    for(int i=0;i<variables->size();i++){
        if(ui->List_fix->item(row,0)->text()==variables->at(i)->get_name()){
            double v = ui->List_fix->item(row,column)->text().toDouble(&ok);
            if(ok)variables->at(i)->set_value(column-1,v);
            else {
                QMessageBox msgBox;

                QFont font;
                font.setBold(true);
                font.setPointSize(12);
                msgBox.setFont(font);
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText("Invaild Input (Not a number).");
                msgBox.exec();
                ui->List_fix->item(row,column)->setText(QString::number(variables->at(i)->get_value()[column-1]));
            }
            return;
        }
    }
    }
    else if(changing_variname!="null"){
        QString newname = ui->List_fix->item(row,column)->text();
        bool exsit = false;
        for(int i=0; i<variables->size(); i++){
            if(variables->at(i)->get_name()==newname){
                exsit = true;
                break;
            }
        }
        if(exsit){
            ui->List_fix->item(row,column)->setText(changing_variname);
            changing_variname = "null";
            QMessageBox msgBox;

            QFont font;
            font.setBold(true);
            font.setPointSize(12);
            msgBox.setFont(font);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText("Name already exsited.");
            msgBox.exec();
        }
        else{
            for(int i=0; i<variables->size(); i++){
                if(variables->at(i)->get_name()==changing_variname){
                    variables->at(i)->set_name(newname);
                    changing_variname = "null";
                    break;
                }
            }
        }
    }
}

void Variablemonitor::on_List_calculate_cellChanged(int row, int column)
{
    if(column!=0){
    for(int i=0;i<variables->size();i++){
        if(ui->List_calculate->item(row,0)->text()==variables->at(i)->get_name()){
            variables->at(i)->set_fomular(ui->List_calculate->item(row,column)->text());
            ui->List_calculate->item(row,column)->setText(variables->at(i)->get_fomular());
            return;
        }
    }
    }
    else if(changing_variname!="null"){
        QString newname = ui->List_calculate->item(row,column)->text();
        bool exsit = false;
        for(int i=0; i<variables->size(); i++){
            if(variables->at(i)->get_name()==newname){
                exsit = true;
                break;
            }
        }
        if(exsit){
            ui->List_calculate->item(row,column)->setText(changing_variname);
            changing_variname = "null";
            QMessageBox msgBox;

            QFont font;
            font.setBold(true);
            font.setPointSize(12);
            msgBox.setFont(font);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText("Name already exsited.");
            msgBox.exec();
        }
        else{
            for(int i=0; i<variables->size(); i++){
                if(variables->at(i)->get_name()==changing_variname){
                    variables->at(i)->set_name(newname);
                    changing_variname = "null";
                    break;
                }
            }
        }
    }
}

void Variablemonitor::on_List_calibrate_cellChanged(int row, int column)
{
    if(column==1){
        for(int i=0;i<variables->size();i++){
            if(ui->List_calibrate->item(row,0)->text()==variables->at(i)->get_name()){
                variables->at(i)->set_file(ui->List_calibrate->item(row,1)->text());
                ui->List_calibrate->item(row,1)->setText(variables->at(i)->get_filename());
                return;
            }
        }
    }
    else if(column==2){
        for(int i=0;i<variables->size();i++){
            if(ui->List_calibrate->item(row,0)->text()==variables->at(i)->get_name()){
                bool ok;
                int fit = ui->List_calibrate->item(row,2)->text().toInt(&ok);
                if(ok && (fit==0 || fit ==1)){
                    variables->at(i)->change_fitting_mode(fit);
                    ui->List_calibrate->item(row,2)->setText(QString::number(variables->at(i)->get_fitting()));
                }
                else{
                    variables->at(i)->change_fitting_mode(0);
                    ui->List_calibrate->item(row,2)->setText("0");
                }
                return;
            }
        }
    }
    else if(changing_variname!="null"){
        QString newname = ui->List_calibrate->item(row,column)->text();
        bool exsit = false;
        for(int i=0; i<variables->size(); i++){
            if(variables->at(i)->get_name()==newname){
                exsit = true;
                break;
            }
        }
        if(exsit){
            ui->List_calibrate->item(row,column)->setText(changing_variname);
            changing_variname = "null";
            QMessageBox msgBox;

            QFont font;
            font.setBold(true);
            font.setPointSize(12);
            msgBox.setFont(font);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText("Name already exsited.");
            msgBox.exec();
        }
        else{
            for(int i=0; i<variables->size(); i++){
                if(variables->at(i)->get_name()==changing_variname){
                    variables->at(i)->set_name(newname);
                    changing_variname = "null";
                    break;
                }
            }
        }
    }
}

void Variablemonitor::show_filemenu(QTableWidgetItem *item){

    QMenu *a = new QMenu;
    QAction *action = new QAction(a);
    QSignalMapper *signalmapper = new QSignalMapper(a);
    action->setText("Select File");
    a->addAction(action);
    connect(action,SIGNAL(triggered()),signalmapper,SLOT(map()));
    signalmapper->setMapping(action,item->row());
    connect(signalmapper,SIGNAL(mapped(int)),this,SLOT(open_filedialog(int)));
    a->popup(QCursor::pos());
}

void Variablemonitor::open_filedialog(int irow){
    temp_filename = QFileDialog::getOpenFileName(this,tr("Select calibration file"));
    ui->List_calibrate->item(irow,1)->setText(temp_filename);
}

void Variablemonitor::show_variable_menu(int irow){
    QMenu *menu = new QMenu(this);
    QMenu *submenu[2];
    submenu[0] = new QMenu("Scaned",menu);
    submenu[1] = new QMenu("Calculated",menu);
    menu->addSection("Use Variable");

    for(int i=0; i<variables->size();i++){
        QAction *action = new QAction(menu);
        QSignalMapper *signal1 = new QSignalMapper(menu);
        QSignalMapper *signal2 = new QSignalMapper(menu);
        action->setText(variables->at(i)->get_name());
        if(variables->at(i)->get_type()==1)submenu[0]->addAction(action);
        else if(variables->at(i)->get_type()==2)submenu[1]->addAction(action);

        signal1->setMapping(action,variables->at(i)->get_name());
        signal2->setMapping(action,irow);
        connect(action,SIGNAL(triggered()),signal1,SLOT(map()));
        connect(action,SIGNAL(triggered()),signal2,SLOT(map()));
        connect(signal1,SIGNAL(mapped(QString)),this,SLOT(receive_variname(QString)));
        connect(signal2,SIGNAL(mapped(int)),this,SLOT(add_variable_to_calculate(int)));
    }

    for(int i=0; i<2; i++){
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
    menu->popup(QCursor::pos());
}

void Variablemonitor::receive_variname(QString name){
    temp_variname = name;
}

void Variablemonitor::add_variable_to_calculate(int row){
    ui->List_calculate->item(row,1)->setText(ui->List_calculate->item(row,1)->text()+"["+temp_variname+"]");
}

void Variablemonitor::on_List_calculate_doubleClicked(const QModelIndex &index)
{
    show_variable_menu(index.row());
}

void Variablemonitor::on_List_calibrate_doubleClicked(const QModelIndex &index)
{
    if(index.column()==1)show_filemenu(ui->List_calibrate->item(index.row(),1));
}

void Variablemonitor::on_Vari_name_returnPressed()
{
    //on_Vari_add_clicked();
}

void Variablemonitor::on_Vari_add_clicked()
{
    int type = ui->Vari_type->currentIndex();
    QString name = ui->Vari_name->text();
    bool ok = true;

    for(int i=0; i<variables->size(); i++)if(name == variables->at(i)->get_name())ok = false;

    if(!ok){
        QMessageBox msgBox;

        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Variable already exist.");
        msgBox.exec();
    }
    else if(name.isEmpty()){
        QMessageBox msgBox;

        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Name could not be empty.");
        msgBox.exec();
    }
    else{
        ui->Vari_name->clear();
        Variable *vari = new Variable(0,name,type,Engine,variables);
        variables->append(vari);
        if(type==0){
            QTableWidgetItem *temp = new QTableWidgetItem;
            QTableWidgetItem *temp1 = new QTableWidgetItem;
            temp->setText(vari->get_name());
            temp1->setText(QString::number(vari->get_value()[0]));
            ui->List_fix->setRowCount(row[0]+1);
            ui->List_fix->setItem(row[0],0,temp);
            ui->List_fix->setItem(row[0],1,temp1);
            row[0]++;
        }
        else if(type==1){
            ui->List_scan->setRowCount(row[1]+1);
            QTableWidgetItem *temp = new QTableWidgetItem;
            temp->setText(vari->get_name());
            ui->List_scan->setItem(row[1],0,temp);
            for(int i=0; i<4; i++){
                QTableWidgetItem *temp = new QTableWidgetItem;
                temp->setText(QString::number(vari->get_value()[i]));
                ui->List_scan->setItem(row[1],i+1,temp);
            }
            row[1]++;
        }
        else if(type==2){
            QTableWidgetItem *temp = new QTableWidgetItem;
            QTableWidgetItem *temp1 = new QTableWidgetItem;
            temp->setText(vari->get_name());
            temp1->setText(vari->get_fomular());
            ui->List_calculate->setRowCount(row[2]+1);
            ui->List_calculate->setItem(row[2],0,temp);
            ui->List_calculate->setItem(row[2],1,temp1);
            row[2]++;
        }
        else{
            QTableWidgetItem *temp = new QTableWidgetItem;
            QTableWidgetItem *temp1 = new QTableWidgetItem;
            temp->setText(vari->get_name());
            temp1->setText(vari->get_filename());
            ui->List_calibrate->setRowCount(row[3]+1);
            ui->List_calibrate->setItem(row[3],0,temp);
            ui->List_calibrate->setItem(row[3],1,temp1);
            row[3]++;
        }
    }
}

void Variablemonitor::variable_change(QString name, bool adding){
    if(adding){
        QTableWidgetItem *temp = new QTableWidgetItem;
        QTableWidgetItem *temp1 = new QTableWidgetItem;
        temp->setText(name);
        temp1->setText("0");
        ui->List_calculate->setRowCount(row[2]+1);
        ui->List_calculate->setItem(row[2],0,temp);
        ui->List_calculate->setItem(row[2],1,temp1);
        row[2]++;
    }
    else{
        for(int i=0; i<row[2]; i++){
            if(ui->List_calculate->item(i,0)->text()==name){
                ui->List_calculate->removeRow(i);
                row[2]--;
                break;
            }
        }
    }
}

void Variablemonitor::on_Import_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,"Load Setting",0,tr("Setting File (*.adcs)"));
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))return;
    QTextStream in(&file);

    QString temp = in.readLine();
    while(!in.atEnd()){
        if(temp=="Variables:"){
            QString name = in.readLine();
            while(name!="End of Variables"){
                int type =in.readLine().toInt();
                QString f;
                for(int i=0; i<variables->size(); i++){
                    if(name==variables->at(i)->get_name())name = name+"_2";
                }
                Variable *temp = new Variable(0,name,type,Engine,variables);
                for(int i=0; i<4; i++)temp->set_value(i,in.readLine().toDouble());
                f = in.readLine();
                if(!f.isEmpty())temp->set_file(f);
                f = in.readLine();
                if(!f.isEmpty())temp->set_fomular(f);
                f = in.readLine();
                temp->change_fitting_mode(f.toInt());
                variables->append(temp);
                name = in.readLine();
            }
        }
        temp = in.readLine();
    }
    file.close();

    ui->List_fix->clear();
    ui->List_scan->clear();
    ui->List_calculate->clear();
    ui->List_calibrate->clear();
    initial();
}

void Variablemonitor::on_Search_textEdited(const QString &arg1)
{
    if(arg1.isEmpty())return;
    for(int i=0; i<ui->List_fix->rowCount(); i++){
        for(int j=0; j<2; j++){
            if(ui->List_fix->item(i,j)->text().contains(arg1,Qt::CaseInsensitive)){
                ui->List_fix->item(i,j)->setSelected(true);
            }
            else ui->List_fix->item(i,j)->setSelected(false);
        }
    }
    for(int i=0; i<ui->List_scan->rowCount(); i++){
        for(int j=0; j<4; j++){
            if(ui->List_scan->item(i,j)->text().contains(arg1,Qt::CaseInsensitive)){
                ui->List_scan->item(i,j)->setSelected(true);
            }
            else ui->List_scan->item(i,j)->setSelected(false);
        }
    }
    for(int i=0; i<ui->List_calculate->rowCount(); i++){
        for(int j=0; j<2; j++){
            if(ui->List_calculate->item(i,j)->text().contains(arg1,Qt::CaseInsensitive)){
                ui->List_calculate->item(i,j)->setSelected(true);
            }
            else ui->List_calculate->item(i,j)->setSelected(false);
        }
    }
    for(int i=0; i<ui->List_calibrate->rowCount(); i++){
        for(int j=0; j<2; j++){
            if(ui->List_calibrate->item(i,j)->text().contains(arg1,Qt::CaseInsensitive)){
                ui->List_calibrate->item(i,j)->setSelected(true);
            }
            else ui->List_calibrate->item(i,j)->setSelected(false);
        }
    }
}

void Variablemonitor::on_Search_editingFinished()
{
    if(ui->Search->text().isEmpty()){
        ui->Search->setText("Search");
    }
}
