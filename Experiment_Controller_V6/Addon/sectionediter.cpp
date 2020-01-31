#include "sectionediter.h"
#include "ui_sectionediter.h"

Sectionediter::Sectionediter(QWidget *parent, int AO, int DiO) :
    QDialog(parent),
    ui(new Ui::Sectionediter)
{
    ui->setupUi(this);
    this->installEventFilter(this);

    connect(ui->Scroll_maina->verticalScrollBar(),SIGNAL(valueChanged(int)),ui->Scroll_timea->verticalScrollBar(),SLOT(setValue(int)));
    connect(ui->Scroll_timea->verticalScrollBar(),SIGNAL(valueChanged(int)),ui->Scroll_maina->verticalScrollBar(),SLOT(setValue(int)));
    connect(ui->Scroll_maind->verticalScrollBar(),SIGNAL(valueChanged(int)),ui->Scroll_timed->verticalScrollBar(),SLOT(setValue(int)));
    connect(ui->Scroll_timed->verticalScrollBar(),SIGNAL(valueChanged(int)),ui->Scroll_maind->verticalScrollBar(),SLOT(setValue(int)));

    Analog_output_channel = AO;
    Digital_output_channel = DiO;

    current = new QList<Sequence*> *[Analog_output_channel+Digital_output_channel];
    widgets = new QList<Editerwidget*> *[Analog_output_channel+Digital_output_channel];
    cyc = new QLCDNumber *[Analog_output_channel+Digital_output_channel];

    QPixmap temp = QPixmap(135,82);
    temp.fill(QColor(225,225,225));
    moveable = new QLabel(this);
    moveable->setPixmap(temp);
    moveable->setGeometry(0,0,135,82);
    moveable->setFrameShape(QFrame::Panel);
    moveable->setFrameShadow(QFrame::Raised);
    moveable->setLineWidth(2);
    moveable->hide();
}

Sectionediter::~Sectionediter()
{
    emit Closed();
    delete ui;
}

void Sectionediter::initial(QList<Variable *> *ivari, VariableClass *vc, QList<QList<Sequence *> **> *isec, QList<QString> *iname, QList<QString> *chl_name, int *total_run){
    QFont lab;
    lab.setPixelSize(13);
    lab.setBold(true);
    variables = ivari;
    variableClass = vc;
    sections = isec;
    sections_name = iname;
    channel_name = chl_name;
    total_number_of_run = total_run;
    adding = false;
    dragging = false;

    if(!sections->isEmpty()){
        for(int i=0; i<sections->size(); i++){
            QListWidgetItem *listitem = new QListWidgetItem(ui->Section_list);
            listitem->setText(sections_name->at(i));
            listitem->setFlags(listitem->flags()|Qt::ItemIsEditable);
            ui->Section_list->addItem(listitem);
        }
        ui->Section_list->item(0)->setSelected(true);

        for(int i=0; i<Analog_output_channel+Digital_output_channel; i++)current[i] = sections->first()[i];
//        load_section();
    }

    for(int i=0; i<Analog_output_channel; i++){
        widgets[i] = new QList<Editerwidget*>;
        current[i] = new QList<Sequence*>;

        QLabel *temp = new QLabel(ui->timea);
        temp->setText(channel_name->at(i));
        temp->setAlignment(Qt::AlignHCenter);
        temp->setGeometry(10,15+85*i,90,25);
        temp->setFont(lab);
        cyc[i] = new QLCDNumber(ui->timea);
        cyc[i]->setGeometry(10,45+85*i,91,25);
        cyc[i]->setDigitCount(6);
        temp->show();
        cyc[i]->show();
    }
    for(int i=0; i<Analog_output_channel-1; i++){
        QFrame *line = new QFrame(ui->timea);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        line->setGeometry(0,85*i+82,110,3);
        line->show();
    }
    for(int i=0; i<Digital_output_channel; i++){
        widgets[i+Analog_output_channel] = new QList<Editerwidget*>;
        current[i+Analog_output_channel] = new QList<Sequence*>;

        QLabel *temp = new QLabel(ui->timed);
        temp->setText(channel_name->at(Analog_output_channel+i));
        temp->setAlignment(Qt::AlignHCenter);
        temp->setGeometry(10,15+85*i,90,25);
        temp->setFont(lab);
        cyc[Analog_output_channel+i] = new QLCDNumber(ui->timed);
        cyc[Analog_output_channel+i]->setGeometry(10,45+85*i,91,25);
        cyc[Analog_output_channel+i]->setDigitCount(6);
        temp->show();
        cyc[Analog_output_channel+i]->show();
    }
    for(int i=0; i<Digital_output_channel-1; i++){
        QFrame *line = new QFrame(ui->timed);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        line->setGeometry(0,85*i+82,110,3);
        line->show();
    }
    change_window_size(400);
    ui->timea->setMinimumHeight(85*Analog_output_channel);
    ui->maina->setMinimumHeight(85*Analog_output_channel);
    ui->timed->setMinimumHeight(85*Digital_output_channel);
    ui->maind->setMinimumHeight(85*Digital_output_channel);

    QTimer *timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(calculate_time()));
    timer->start(250);
}

void Sectionediter::resizeEvent(QResizeEvent *a){
    ui->Scroll_maina->setGeometry(110,0,this->width()-309,this->height()-69);
    ui->Scroll_timea->setGeometry(0,0,111,this->height()-86);
    ui->Scroll_maind->setGeometry(110,0,this->width()-309,this->height()-69);
    ui->Scroll_timed->setGeometry(0,0,111,this->height()-86);
    ui->line->setGeometry(173,0,21,this->height());
    ui->Tab->setGeometry(190,45,this->width()-193,this->height()-47);
}

bool Sectionediter::eventFilter(QObject *obj, QEvent *event){
    if(event->type()==QEvent::MouseButtonPress){
        QMouseEvent *a = static_cast<QMouseEvent*>(event);
        if(a->button()==Qt::LeftButton){
            if(190<a->x()&&a->x()<300&&10<a->y()&&a->y()<40){
                moveable->move(a->x()-67,a->y()-41);
                moveable->show();
                dragging = true;
                adding = true;
                return true;
            }
            else if(obj->parent()==ui->maina){
                Editerwidget *w = static_cast<Editerwidget*>(obj);
                moveable->move(a->x()-67+300+w->x()-ui->Scroll_maina->horizontalScrollBar()->value(),a->y()-41+60+w->y()-ui->Scroll_maina->verticalScrollBar()->value());
                moveable->show();
                dragging = true;
                w->hide();
                return true;
            }
            else if(obj->parent()==ui->maind){
                Editerwidget *w = static_cast<Editerwidget*>(obj);
                moveable->move(a->x()-67+300+w->x()-ui->Scroll_maind->horizontalScrollBar()->value(),a->y()-41+60+w->y()-ui->Scroll_maind->verticalScrollBar()->value());
                moveable->show();
                dragging = true;
                w->hide();
                return true;
            }
            return false;
        }
        return false;
    }
    else if(event->type()==QEvent::MouseMove){
        QMouseEvent *a = static_cast<QMouseEvent*>(event);
        if(dragging&&adding){
            moveable->move(a->x()-67,a->y()-41);
            return true;
        }
        else if(dragging){
            QWidget *w = static_cast<QWidget*>(obj);
            if(ui->Tab->currentIndex()==0)moveable->move(a->x()-67+300+w->x()-ui->Scroll_maina->horizontalScrollBar()->value(),a->y()-41+60+w->y()-ui->Scroll_maina->verticalScrollBar()->value());
            if(ui->Tab->currentIndex()==1)moveable->move(a->x()-67+300+w->x()-ui->Scroll_maind->horizontalScrollBar()->value(),a->y()-41+60+w->y()-ui->Scroll_maind->verticalScrollBar()->value());
            return true;
        }
        return false;
    }
    else if(event->type()==QEvent::MouseButtonRelease){
        QMouseEvent *a = static_cast<QMouseEvent*>(event);
        if(dragging&&adding){
            moveable->hide();
            dragging = false;
            adding = false;
            int channel;
            int position;
            bool analog = !ui->Tab->currentIndex();
            if(a->x()>300 && a->y()>60){
                if(analog){
                    channel = (a->y()-60+ui->Scroll_maina->verticalScrollBar()->value())/85;
                    position = (a->x()-301+ui->Scroll_maina->horizontalScrollBar()->value())/136;
                }
                else{
                    channel = (a->y()-60+ui->Scroll_maind->verticalScrollBar()->value())/85;
                    position = (a->x()-301+ui->Scroll_maind->horizontalScrollBar()->value())/136;
                }
                add_widget(channel,position,analog);
            }
            return true;
        }
        else if(dragging){
            QWidget *w = static_cast<QWidget*>(obj);
            moveable->hide();
            dragging = false;
            bool analog = !ui->Tab->currentIndex();
            int x,y;
            if(analog){
                x = a->x()+w->x()+300-ui->Scroll_maina->horizontalScrollBar()->value();
                y = a->y()+w->y()+60-ui->Scroll_maina->verticalScrollBar()->value();
            }
            else{
                x = a->x()+w->x()+300-ui->Scroll_maind->horizontalScrollBar()->value();
                y = a->y()+w->y()+60-ui->Scroll_maind->verticalScrollBar()->value();
            }
            if(x<300 || (x>300+ui->maina->width()) || y<60 || (y>60+ui->maina->height())){
                delete_widget(w,analog);
            }
            else{
                int channel,position;
                if(a->x()<0)position = a->x()/136 - 1;
                else position = a->x()/136;
                if(a->y()<0)channel = a->y()/85 - 1;
                else channel = a->y()/85;
                w->show();
                move_widget(channel,position,w,analog);
            }
            return true;
        }
        return false;
    }
    return false;
}

void Sectionediter::change_window_size(int x){
    ui->maina->setMinimumWidth(x);
    ui->maind->setMinimumWidth(x);
}

void Sectionediter::load_section(){
    for(int i=0; i<Analog_output_channel+Digital_output_channel; i++){
        for(int j=0; j<widgets[i]->size(); j++)delete widgets[i]->at(j);
        if(!widgets[i]->isEmpty())widgets[i]->clear();
    }

    int wid = 0;
    for(int i=0; i<Analog_output_channel; i++){
        if(!current[i]->isEmpty()){
            for(int j=0; j<current[i]->size(); j++){
                Editerwidget *w = new Editerwidget(ui->maina);
                w->initial(current[i]->at(j),variables);
                w->move(1+136*j,85*i);
                w->installEventFilter(this);
                widgets[i]->append(w);
                w->show();
                if(j>wid)wid = j;
            }
        }
    }
    for(int i=Analog_output_channel; i<Analog_output_channel+Digital_output_channel; i++){
        if(!current[i]->isEmpty()){
            for(int j=0; j<current[i]->size(); j++){
                Editerwidget *w = new Editerwidget(ui->maind);
                w->initial(current[i]->at(j),variables);
                w->move(1+136*j,85*(i-Analog_output_channel));
                w->installEventFilter(this);
                widgets[i]->append(w);
                w->show();
                if(j>wid)wid = j;
            }
        }
    }
    change_window_size(wid*136+300);
}

void Sectionediter::on_Close_clicked()
{
    this->accept();
}

void Sectionediter::on_Section_add_clicked()
{
    QListWidgetItem *name = new QListWidgetItem;
    QList<Sequence*> **sec;
    sec = new QList<Sequence*>*[Analog_output_channel+Digital_output_channel];
    for(int i=0; i<Analog_output_channel+Digital_output_channel; i++){
        sec[i]=new QList<Sequence*>;
        current[i] = sec[i];
    }
    name->setText("New section");
    ui->Section_list->addItem(name);
    name->setFlags(name->flags()|Qt::ItemIsEditable);
    sections_name->append("New Section");
    sections->append(sec);
    name->setSelected(true);
    load_section();
    emit section_change();
}

void Sectionediter::on_Section_copy_clicked()
{
    if(ui->Section_list->count()==0)return;
    int index = 0;
    for(int i=0; i<sections->size(); i++)if(ui->Section_list->item(i)->isSelected())index = i;

    QListWidgetItem *name = new QListWidgetItem;
    QList<Sequence*> **sec;
    sec = new QList<Sequence*>*[Analog_output_channel+Digital_output_channel];
    for(int i=0; i<Analog_output_channel+Digital_output_channel; i++){
        sec[i]=new QList<Sequence*>;
        for(int j=0; j<current[i]->size(); j++){
            bool isDiO = false;
            if(i>=Analog_output_channel)isDiO = true;
            Sequence *s = new Sequence(0,isDiO,variables,variableClass);
            *s = *current[i]->at(j);
            sec[i]->append(s);
        }
        current[i] = sec[i];
    }
    name->setText(ui->Section_list->item(index)->text()+"_copy");
    ui->Section_list->addItem(name);
    name->setFlags(name->flags()|Qt::ItemIsEditable);
    sections_name->append(ui->Section_list->item(index)->text()+"_copy");
    sections->append(sec);
    name->setSelected(true);
    load_section();
    emit section_change();
}

void Sectionediter::on_Section_delete_clicked()
{
    int index = 0;
    for(int i=0; i<sections->size(); i++)if(ui->Section_list->item(i)->isSelected())index = i;

    for(int i=0; i<Analog_output_channel+Digital_output_channel; i++){
        for(int j=0; j<sections->at(index)[i]->size(); j++){
            delete sections->at(index)[i]->at(j);
        }
        sections->at(index)[i]->clear();
    }
    delete sections->at(index);
    sections->removeAt(index);
    sections_name->removeAt(index);
    delete ui->Section_list->item(index);

    if(!sections->isEmpty()){
        ui->Section_list->item(0)->setSelected(true);
        for(int i=0; i<Analog_output_channel+Digital_output_channel; i++)current[i] = sections->first()[i];
    }
    load_section();
    emit section_change();
}

void Sectionediter::on_Section_list_itemChanged(QListWidgetItem *item)
{
    sections_name->removeAt(ui->Section_list->row(item));
    sections_name->insert(ui->Section_list->row(item),item->text());
    emit section_change();
}

void Sectionediter::on_Section_list_clicked(const QModelIndex &index)
{
    if(current!=sections->at(index.row()))for(int i=0; i<Analog_output_channel+Digital_output_channel; i++)current[i] = sections->at(index.row())[i];
    load_section();
}

void Sectionediter::add_widget(int channel, int position, bool analog){
    Editerwidget *w = new Editerwidget();
    Sequence *s = new Sequence(0,!analog,variables,variableClass);
    if(analog)w->setParent(ui->maina);
    else{
        w->setParent(ui->maind);
        channel = channel + Analog_output_channel;
    }
    if(current[channel]->size()<position)position = current[channel]->size();
    w->initial(s,variables);
    w->installEventFilter(this);
    widgets[channel]->insert(position,w);
    current[channel]->insert(position,s);
    w->move(position*136+1,channel*85);
    w->show();
    sort_widget();
}

void Sectionediter::delete_widget(QWidget *w, bool analog){
    int channel = w->y()/85;
    int position = (w->x()-1)/136;

    if(!analog)channel = channel + Analog_output_channel;
    Editerwidget *e = static_cast<Editerwidget*>(w);
    current[channel]->removeAt(position);
    widgets[channel]->removeAt(position);
    delete e->seq;
    delete w;
    sort_widget();
}

void Sectionediter::move_widget(int tochannel, int toposition, QWidget *w, bool analog){
    if(tochannel==0 && toposition==0)return;
    int channel = w->y()/85;
    int position = (w->x()-1)/136;
    if(!analog)channel = channel + Analog_output_channel;

    Editerwidget *e = static_cast<Editerwidget*>(w);
    if(widgets[channel+tochannel]->size()>position+toposition)toposition = widgets[channel+tochannel]->size()-position;
    current[channel]->removeAt(position);
    widgets[channel]->removeAt(position);
    current[channel+tochannel]->insert(position+toposition,e->seq);
    widgets[channel+tochannel]->insert(position+toposition,e);
    sort_widget();
}

void Sectionediter::sort_widget(){
    int wid=0;
    for(int i=0; i<Analog_output_channel+Digital_output_channel; i++){
        for(int j=0; j<widgets[i]->size(); j++){
            if(i<Analog_output_channel)widgets[i]->at(j)->move(j*136+1,i*85);
            else widgets[i]->at(j)->move(j*136+1,(i-Analog_output_channel)*85);
            if(j>wid)wid = j;
        }
    }
    change_window_size(wid*136+300);
}

void Sectionediter::calculate_time(){
    double sec_time = 0;
    for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
        double channel_time = 0;
        for(int k=0; k<current[j]->size(); k++){
            double seq_time = current[j]->at(k)->get_time(current_run);
            if(seq_time>0)channel_time += seq_time;
        }
        cyc[j]->display(channel_time);
        if(channel_time>sec_time)sec_time = channel_time;
    }
    ui->Section_time->display(sec_time);
}
