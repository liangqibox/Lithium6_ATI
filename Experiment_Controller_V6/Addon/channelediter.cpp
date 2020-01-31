#include "channelediter.h"
#include "ui_channelediter.h"
#include <QDebug>

Channelediter::Channelediter(QWidget *parent, int AO, int DiO) :
    QDialog(parent),
    ui(new Ui::Channelediter)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window);
    Analog_output_channel = AO;
    Digital_output_channel = DiO;

    ui->maina->setMinimumHeight(Analog_output_channel*85);
    ui->timea->setMinimumHeight(Analog_output_channel*85);
    ui->maind->setMinimumHeight(Digital_output_channel*85);
    ui->timed->setMinimumHeight(Digital_output_channel*85);

    ui->AOut->show();
    ui->DiOut->show();
    connect(ui->Scroll_maina->horizontalScrollBar(),SIGNAL(valueChanged(int)),ui->Scroll_section->horizontalScrollBar(),SLOT(setValue(int)));
    connect(ui->Scroll_maina->horizontalScrollBar(),SIGNAL(valueChanged(int)),ui->Scroll_maind->horizontalScrollBar(),SLOT(setValue(int)));
    connect(ui->Scroll_maina->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(background_move(int)));
    connect(ui->Scroll_maina->verticalScrollBar(),SIGNAL(valueChanged(int)),ui->Scroll_timea->verticalScrollBar(),SLOT(setValue(int)));
    connect(ui->Scroll_maina->verticalScrollBar(),SIGNAL(valueChanged(int)),ui->Scroll_maind->verticalScrollBar(),SLOT(setValue(int)));
    connect(ui->Scroll_maina->verticalScrollBar(),SIGNAL(valueChanged(int)),ui->Scroll_timed->verticalScrollBar(),SLOT(setValue(int)));
    connect(ui->Scroll_maind->horizontalScrollBar(),SIGNAL(valueChanged(int)),ui->Scroll_section->horizontalScrollBar(),SLOT(setValue(int)));
    connect(ui->Scroll_maind->horizontalScrollBar(),SIGNAL(valueChanged(int)),ui->Scroll_maina->horizontalScrollBar(),SLOT(setValue(int)));
    connect(ui->Scroll_maind->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(background_move(int)));
    connect(ui->Scroll_maind->verticalScrollBar(),SIGNAL(valueChanged(int)),ui->Scroll_timed->verticalScrollBar(),SLOT(setValue(int)));
    connect(ui->Scroll_maind->verticalScrollBar(),SIGNAL(valueChanged(int)),ui->Scroll_maina->verticalScrollBar(),SLOT(setValue(int)));
    connect(ui->Scroll_timea->verticalScrollBar(),SIGNAL(valueChanged(int)),ui->Scroll_maina->verticalScrollBar(),SLOT(setValue(int)));
    connect(ui->Scroll_timed->verticalScrollBar(),SIGNAL(valueChanged(int)),ui->Scroll_maind->verticalScrollBar(),SLOT(setValue(int)));

    color[0].setRgb(245,245,245);
    color[1].setRgb(235,235,235);
    color[2].setRgb(255,192,203);
    color[3].setRgb(255,160,122);
    color[4].setRgb(155,250,250);
    color[5].setRgb(238,130,238);
    color[6].setRgb(154,205,50);
    color[7].setRgb(135,206,235);
    color[8].setRgb(250,250,150);
    color[9].setRgb(240,128,128);
}

Channelediter::~Channelediter()
{
    emit Closed();
    for(int i=0; i<background->size(); i++)delete background->at(i);
    for(int i=0; i<section_tab->size(); i++)delete section_tab->at(i);
    for(int i=0; i<section_seperator->size(); i++)delete section_seperator->at(i);
    for(int i=0; i<section_dummyBlock->size(); i++)delete section_dummyBlock->at(i);
    for(int i=0; i<chlname->size(); i++)delete chlname->at(i);
    for(int i=0; i<cal_label->size(); i++)delete cal_label->at(i);
    delete moveable;
    delete moveableS;
    delete background;
    delete section_tab;
    delete section_seperator;
    delete section_dummyBlock;
    delete chlname;
    delete cal_label;
    delete ui;
}

void Channelediter::Clear(){
    for(int i=0; i<background->size(); i++)delete background->at(i);
    for(int i=0; i<section_tab->size(); i++)delete section_tab->at(i);
    for(int i=0; i<section_seperator->size(); i++)delete section_seperator->at(i);
    for(int i=0; i<section_dummyBlock->size(); i++)delete section_dummyBlock->at(i);
    for(int i=0; i<chlname->size(); i++)delete chlname->at(i);
    for(int i=0; i<cal_label->size(); i++)delete cal_label->at(i);
    delete moveable;
    delete moveableS;
    delete background;
    delete section_tab;
    delete section_seperator;
    delete section_dummyBlock;
    delete chlname;
    delete cal_label;
}

void Channelediter::initial(QList<QList<Sequence *>**>*iseq, QList<Variable *> *ivari, VariableClass *vc, QList<QList<Sequence *> **> *isec,
                            QList<QString> *isecn, QList<QString> *isec_ay, QList<SectionActivity *> *isec_act, QList<QString> *ichlname, QList<QString> *chl_cal,
                            QList<int> *chl_col, int *total_run, int *number_of_scan){
    QFont lab;
    lab.setPixelSize(13);
    lab.setBold(true);
    QFont lab2;
    lab2.setPixelSize(8);
    lab2.setBold(false);

    sequences = iseq;
    variables = ivari;
    variableClass = vc;
    sections = isec;
    sections_name = isecn;
    section_array = isec_ay;
    sections_activity = isec_act;
    channel_name = ichlname;
    channel_color = chl_col;
    channel_calibration = chl_cal;
    total_number_of_run = total_run;
    current_run = number_of_scan;
    dragging = false;
    moving_ew = false;
    adding_ew = false;
    moving_sec =false;
    adding_sec = false;
    changing_color = false;
    moveable = new QLabel(this);
    moveableS = new QLabel(this);
    background = new QList<QLabel*>;
    section_tab = new QList<Sectiontab*>;
    section_seperator = new QList<QFrame*>;
    section_dummyBlock = new QList<QFrame*>;
    chlname = new QList<QLineEdit*>;
    cal_label = new QList<QLabel*>;
    digital_invert = new QList<QPushButton*>;

    QPixmap temp = QPixmap(135,82);
    temp.fill(QColor(225,225,225));
    moveable = new QLabel(this);
    moveable->setPixmap(temp);
    moveable->setGeometry(0,0,135,82);
    moveable->setFrameShape(QFrame::Panel);
    moveable->setFrameShadow(QFrame::Raised);
    moveable->setLineWidth(2);
    moveable->hide();

    ui->Add_sequence->installEventFilter(this);
    moveableS->setFrameShape(QFrame::Panel);
    moveableS->setFrameShadow(QFrame::Raised);
    moveableS->setLineWidth(3);
    moveableS->setFont(lab);
    moveableS->setAlignment(Qt::AlignVCenter);
    moveableS->setGeometry(0,0,200,70);
    moveableS->hide();

    section_names_import();
    this->installEventFilter(this);

    if(channel_calibration->size()<Analog_output_channel+1){
        for(int i=0; i<Digital_output_channel; i++)channel_calibration->append("Normal");
    }

    for(int i=0; i<10; i++){
        QLabel *col = new QLabel(ui->Colors);
        QPixmap fil = QPixmap(20,20);
        fil.fill(color[i]);
        col->setGeometry(10+(i%5)*20,15+(i/5)*20,20,20);
        col->setFrameStyle(QFrame::Panel);
        col->setFrameShadow(QFrame::Sunken);
        col->setLineWidth(2);
        col->setPixmap(fil);
        col->show();
        col->installEventFilter(this);
    }

    for(int i=0; i<Analog_output_channel; i++){
        QLabel *temp = new QLabel(ui->timea);
        temp->setText("AO "+QString::number(i+1));
        temp->setAlignment(Qt::AlignHCenter);
        temp->setGeometry(10,68+85*i,25,20);
        temp->setFont(lab2);
        QLabel *cal = new QLabel(ui->timea);
        cal->setText(channel_calibration->at(i));
        cal->setAlignment(Qt::AlignHCenter);
        cal->setGeometry(40,68+85*i,60,20);
        cal->setFont(lab2);
        cal_label->append(cal);
        QLineEdit *name = new QLineEdit(ui->timea);
        chlname->append(name);
        name->setText(channel_name->at(i));
        name->setGeometry(10,15+85*i,85,25);
        connect(name,SIGNAL(textEdited(QString)),this,SLOT(channel_name_changed(QString)));
        name->show();
        QPushButton* button = new QPushButton(ui->timea);
        QSignalMapper *signalmap = new QSignalMapper(button);
        button->setGeometry(10,45+85*i,70,20);
        button->setText("Calibration");
        connect(button,SIGNAL(clicked()),signalmap,SLOT(map()));
        signalmap->setMapping(button,i);
        connect(signalmap,SIGNAL(mapped(int)),this,SLOT(show_calibration_menu(int)));
        button->show();
        QLabel *back = new QLabel(ui->maina);
        background->append(back);
        QPixmap fil = QPixmap(ui->maina->width(),85);

        fil.fill(color[channel_color->at(i)]);
        back->setGeometry(0,85*i,ui->maina->width(),85);
        back->setPixmap(fil);
        back->show();
    }
    for(int i=0; i<Analog_output_channel-1; i++){
        QFrame *line = new QFrame(ui->timea);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        line->setGeometry(0,85*i+82,110,3);
        line->show();
    }
    for(int i=0; i<Digital_output_channel; i++){
        QLabel *temp = new QLabel(ui->timed);
        temp->setText("DiO "+QString::number(i+1));
        temp->setAlignment(Qt::AlignHCenter);
        temp->setGeometry(10,68+85*i,25,20);
        temp->setFont(lab2);
        QLineEdit *name = new QLineEdit(ui->timed);
        chlname->append(name);
        name->setText(channel_name->at(i+Analog_output_channel));
        name->setGeometry(10,15+85*i,85,25);
        connect(name,SIGNAL(textEdited(QString)),this,SLOT(channel_name_changed(QString)));
        name->show();
        QLabel *back = new QLabel(ui->maind);
        background->append(back);
        QPixmap fil = QPixmap(ui->maind->width(),85);
        fil.fill(color[channel_color->at(i+Analog_output_channel)]);
        back->setGeometry(0,85*i,ui->maind->width(),85);
        back->setPixmap(fil);
        back->show();
        QPushButton* button = new QPushButton(ui->timed);
        QSignalMapper *signalmap = new QSignalMapper(button);
        button->setGeometry(10,45+85*i,70,20);
        button->setText(channel_calibration->at(i+Analog_output_channel));
        connect(button,SIGNAL(clicked()),signalmap,SLOT(map()));
        signalmap->setMapping(button,i+Analog_output_channel);
        connect(signalmap,SIGNAL(mapped(int)),this,SLOT(show_calibration_menu(int)));
        digital_invert->append(button);
        button->show();
    }
    for(int i=0; i<Digital_output_channel-1; i++){
        QFrame *line = new QFrame(ui->timed);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        line->setGeometry(0,85*i+82,110,3);
        line->show();
    }

    widgets = new QList<QList<Editerwidget*>**>;
    int wid = 0;
    for(int i=0; i<sequences->size(); i++){
        int sec_wid=0;
        //bool isSection = true;
        QList<Editerwidget*> **sec = new QList<Editerwidget*>*[Analog_output_channel+Digital_output_channel];
        for(int j=0; j<Analog_output_channel+Digital_output_channel; j++)sec[j] = new QList<Editerwidget*>;
        widgets->append(sec);
        //if(section_array->at(i)=="null")isSection = false;

        for(int j=0; j<Analog_output_channel; j++){
            for(int k=0; k<sequences->at(i)[j]->size(); k++){
                Editerwidget *temp = new Editerwidget(ui->maina);
                temp->initial(sequences->at(i)[j]->at(k),variables);
                temp->move(1+136*(k+wid),85*j);
                temp->installEventFilter(this);
                widgets->at(i)[j]->append(temp);
                temp->show();
                if((k+1)>sec_wid)sec_wid=k+1;
                connect(sequences->at(i)[j]->at(k),SIGNAL(Changed(bool)),this,SLOT(Report_Edited()));
            }
        }
        for(int j=Analog_output_channel; j<Analog_output_channel+Digital_output_channel; j++){
            for(int k=0; k<sequences->at(i)[j]->size(); k++){
                Editerwidget *temp = new Editerwidget(ui->maind);
                temp->initial(sequences->at(i)[j]->at(k),variables);
                temp->move(1+136*(k+wid),85*j);
                temp->installEventFilter(this);
                widgets->at(i)[j]->append(temp);
                temp->show();
                if((k+1)>sec_wid)sec_wid=k+1;
                connect(sequences->at(i)[j]->at(k),SIGNAL(Changed(bool)),this,SLOT(Report_Edited()));
            }
        }
        Sectiontab *tab = new Sectiontab(ui->section,"   ",variables,variableClass,sections_activity->at(i),current_run);
        connect(tab,SIGNAL(save_section(Sectiontab*,QString)),this,SLOT(Section_save(Sectiontab*,QString)));
        connect(tab,SIGNAL(delete_section(Sectiontab*)),this,SLOT(delete_section(Sectiontab*)));
        connect(sections_activity->at(i),SIGNAL(Changed()),this,SLOT(Report_Edited()));
        section_tab->append(tab);
        sections_group.append(false);
        tab->setGeometry(1+136*wid,5,136*sec_wid,70);
        if(section_array->at(i)!="null")tab->set_name(section_array->at(i));
        connect(tab,SIGNAL(group(Sectiontab*,bool)),this,SLOT(section_groupped(Sectiontab*,bool)));
        //tab->installEventFilter(this);
        tab->show();
        wid = wid + sec_wid;
    }

    sort_widget();
    calcualte_times();
}

bool Channelediter::eventFilter(QObject *obj, QEvent *event){
    if(event->type()==QEvent::MouseButtonPress){
        QMouseEvent *a = static_cast<QMouseEvent*>(event);
        if(a->button()==Qt::LeftButton){
            if(obj->parent()==ui->maina){
                Editerwidget *w = static_cast<Editerwidget*>(obj);
                int x = a->x()+w->x()+110-ui->Scroll_maina->horizontalScrollBar()->value()-67;
                int y = a->y()+w->y()+159-ui->Scroll_maina->verticalScrollBar()->value()-41;
                moveable->move(x,y);
                moveable->show();
                w->hide();
                dragging = true;
                moving_ew = true;
                return true;
            }
            else if(obj->parent()==ui->maind){
                Editerwidget *w = static_cast<Editerwidget*>(obj);
                int x = a->x()+w->x()+110-ui->Scroll_maind->horizontalScrollBar()->value()-67;
                int y = a->y()+w->y()+159-ui->Scroll_maind->verticalScrollBar()->value()-41;
                moveable->move(x,y);
                moveable->show();
                w->hide();
                dragging = true;
                moving_ew = true;
                return true;
            }
            else if(obj->parent()==ui->Colors){
                changing_color = true;
                return true;
            }
            else if(obj->parent()==ui->section){
                Sectiontab *tab = static_cast<Sectiontab*>(obj);
                int x = a->x()+tab->x()+ui->Scroll_section->x()-ui->Scroll_section->horizontalScrollBar()->value()-tab->width()/2;
                int y = a->y()+tab->y()+ui->Scroll_section->y()-25;
                moveableS->move(x,y);
                moveableS->setText(tab->get_name());
                moveableS->show();
                dragging = true;
                moving_sec = true;
                return true;
            }
            else if(250<a->x()&&a->x()<340&&10<a->y()&&a->y()<60){
                moveable->move(a->x()-67,a->y()-41);
                moveable->show();
                dragging = true;
                adding_ew = true;
                return true;
            }
            else if(350<a->x()&&a->x()<550&&35<a->y()&&a->y()<60){
                QString name = ui->section_name->currentText();
                dragging = true;
                adding_sec = true;
                moveableS->setGeometry(a->x()-100,a->y()-25,200,40);
                moveableS->setText(name);
                moveableS->show();
                return true;
            }
            return false;
        }
    }
    else if(event->type()==QEvent::MouseMove){
        if(dragging&&adding_sec){
            QMouseEvent *a = static_cast<QMouseEvent*>(event);
            moveableS->move(a->x()-100,a->y()-25);
            return true;
        }
        else if(dragging&&adding_ew){
            QMouseEvent *a = static_cast<QMouseEvent*>(event);
            /*if(obj->parent()==ui->maina||obj->parent()==ui->maind){
                QWidget *w = static_cast<QWidget*>(obj);
                int x = a->x()+w->x()+110-ui->Scroll_maina->horizontalScrollBar()->value()-67;
                int y = a->y()+w->y()+159-ui->Scroll_maina->verticalScrollBar()->value()-41;
                moveable->move(x,y);
            }*/
            moveable->move(a->x()-67+250,a->y()-41+10);
            return true;
        }
        else if(dragging&&moving_ew){
            QMouseEvent *a = static_cast<QMouseEvent*>(event);
            QWidget *w = static_cast<QWidget*>(obj);
            int x = a->x()+w->x()+110-ui->Scroll_maina->horizontalScrollBar()->value()-67;
            int y = a->y()+w->y()+159-ui->Scroll_maina->verticalScrollBar()->value()-41;
            moveable->move(x,y);
            return true;
        }
        else if(dragging&&moving_sec){
            QMouseEvent *a = static_cast<QMouseEvent*>(event);
            QLabel *tab = static_cast<QLabel*>(obj);
            int x = a->x()+tab->x()+ui->Scroll_section->x()-ui->Scroll_section->horizontalScrollBar()->value()-tab->width()/2;
            int y = a->y()+tab->y()+ui->Scroll_section->y()-tab->height()/2;
            moveableS->move(x,y);
            return true;
        }
        return false;
    }
    else if(event->type()==QEvent::MouseButtonRelease){
        if(dragging&&adding_sec){
            dragging = false;
            adding_sec = false;
            QMouseEvent *a = static_cast<QMouseEvent*>(event);
            int position = (a->x()-111+ui->Scroll_section->horizontalScrollBar()->value())/136;
            if(a->y()>80&&position>-1)add_section(position);
            moveableS->hide();
            return true;
        }
        else if(dragging&&adding_ew){
            QMouseEvent *a = static_cast<QMouseEvent*>(event);
            moveable->hide();
            int channel,position;
            dragging = false;
            adding_ew = false;
            bool analog = !bool(ui->tabWidget->currentIndex());
            if(analog){
                channel = (a->y()-159+ui->Scroll_maina->verticalScrollBar()->value())/85;
                //position = (a->x()-111+ui->Scroll_maina->horizontalScrollBar()->value());
                position = (a->x()+140+ui->Scroll_maina->horizontalScrollBar()->value());
            }
            else{
                channel = (a->y()-159+ui->Scroll_maind->verticalScrollBar()->value())/85;
                //position = (a->x()-111+ui->Scroll_maind->horizontalScrollBar()->value());
                position = (a->x()+140+ui->Scroll_maina->horizontalScrollBar()->value());
            }
            if(channel>-1&&position>-1)add_widget(channel,position,analog,NULL);
            return true;
        }
        else if(dragging&&moving_ew){
            QMouseEvent *a = static_cast<QMouseEvent*>(event);
            QWidget *w = static_cast<QWidget*>(obj);
            w->show();
            dragging = false;
            moving_ew = false;
            int channel,position;
            bool analog = !ui->tabWidget->currentIndex();
            int x = a->x()+w->x()+110-ui->Scroll_maina->horizontalScrollBar()->value();
            int y = a->y()+w->y()+159-ui->Scroll_maina->verticalScrollBar()->value();
            if(QApplication::keyboardModifiers() == Qt::AltModifier){
                moveable->hide();
                delete_widget(w,analog);
                return true;
            }
            if(x<110||(x>(110+ui->Scroll_maina->width()))){
                moveable->hide();
                delete_widget(w,analog);
                return true;
            }
            else if(y<159||(y>159+ui->Scroll_maina->height())){
                moveable->hide();
                delete_widget(w,analog);
                return true;
            }
            if(a->x()<0)position = a->x()/136 - 1;
            else position = a->x()/136;
            if(a->y()<0)channel = a->y()/85 - 1;
            else channel = a->y()/85;
            moveable->hide();
            if(QApplication::keyboardModifiers() == Qt::ShiftModifier)copy_widget(channel,position,w,analog);
            else move_widget(channel,position,w,analog);
            return true;
        }
        else if(dragging&&moving_sec){
            dragging = false;
            QMouseEvent *a = static_cast<QMouseEvent*>(event);
            Sectiontab *tab = static_cast<Sectiontab*>(obj);
            if((a->y()<-45)||(a->y()>45))delete_section(tab);
            moveableS->hide();
            return true;
        }
        else if(changing_color){
            QMouseEvent *a = static_cast<QMouseEvent*>(event);
            QLabel *l = static_cast<QLabel*>(obj);
            changing_color = false;
            int x,y;
            bool analog = !ui->tabWidget->currentIndex();
            x = a->x()+ui->Colors->x()+l->x();
            if(analog)y = a->y()+ui->Colors->y()+l->y()-114+ui->Scroll_maina->verticalScrollBar()->value();
            else y = a->y()+ui->Colors->y()+l->y()-114+ui->Scroll_maind->verticalScrollBar()->value();
            int i = (l->x()-10)/20+(l->y()-15)/20*5;
            if(x>110 && y>0){
                int channel;
                if(analog) channel= y/85;
                else channel = y/85 + Analog_output_channel;
                QPixmap fil = QPixmap(ui->maina->width(),85);
                fil.fill(color[i]);
                background->at(channel)->setPixmap(fil);
                channel_color->removeAt(channel);
                channel_color->insert(channel,i);
            }
            return true;
        }
        return false;
    }
    return false;
}

/*void Channelediter::add_widget(int channel, int position, bool analog){
    int ii=0;
    int kk=0;
    bool newsection=false;

    if(section_tab->isEmpty())newsection = true;
    for(int i=0; i<section_tab->size(); i++){
        if(position<(section_tab->at(i)->x()+section_tab->at(i)->width()-20)){
            ii=i;
            break;
        }
        else{
            if(section_tab->size()==i+1){
                if(section_tab->last()->isHidden()){
                    ii=i;
                    break;
                }
                else{
                    if(position<(section_tab->last()->x()+section_tab->last()->width()+20)){
                        ii=i;
                        break;
                    }
                    else{
                        ii=i+1;
                        newsection = true;
                        break;
                    }
                }
            }
            else if(position<(section_tab->at(i)->x()+section_tab->at(i)->width()+40)){
                if(section_tab->at(i)->isHidden()){
                    ii=i;
                    break;
                }
                else if(section_tab->at(i+1)->isHidden()){
                    ii=i+1;
                    break;
                }
                else {
                    ii=i+1;
                    newsection = true;
                    break;
                }
            }
        }
    }
    if(newsection){
        QList<Editerwidget*> **sec_ew = new QList<Editerwidget*>*[Analog_output_channel+Digital_output_channel];
        QList<Sequence*> **sec_seq = new QList<Sequence*>*[Analog_output_channel+Digital_output_channel];
        for(int i=0; i<Analog_output_channel+Digital_output_channel; i++){
            sec_ew[i] = new QList<Editerwidget*>;
            sec_seq[i] = new QList<Sequence*>;
        }
        SectionActivity *act = new SectionActivity;
        Sectiontab *tab = new Sectiontab(ui->section,"  ",variables,act,current_run);
        section_tab->insert(ii,tab);
        sections_activity->insert(ii,act);
        sections_group.insert(ii,false);
        tab->setGeometry(5,5,136,70);
        connect(tab,SIGNAL(group(Sectiontab*,bool)),this,SLOT(section_groupped(Sectiontab*,bool)));
        tab->installEventFilter(this);
        tab->show();
        widgets->insert(ii,sec_ew);
        sequences->insert(ii,sec_seq);
        section_array->insert(ii,"null");
    }
    Editerwidget *temp = new Editerwidget;
    Sequence *seqd;
    if(analog){
        seqd = new Sequence(0,false,variables);
        temp->setParent(ui->maina);
    }
    else{
        seqd = new Sequence(0,true,variables);
        temp->setParent(ui->maind);
        channel = channel + Analog_output_channel;
    }
    for(int k=0; k<widgets->at(ii)[channel]->size(); k++){
        if(widgets->at(ii)[channel]->at(k)->x()<position){
            kk=k;
        }
    }
    temp->initial(seqd,variables);
    temp->installEventFilter(this);
    if(widgets->at(ii)[channel]->isEmpty()){
        widgets->at(ii)[channel]->append(temp);
        sequences->at(ii)[channel]->append(seqd);
    }
    else{
        widgets->at(ii)[channel]->insert(kk+1,temp);
        sequences->at(ii)[channel]->insert(kk+1,seqd);
    }
    temp->move(widgets->at(ii)[channel]->at(kk)->x()+136,85*channel);
    temp->show();
    seqd->set_change(true);
    sort_widget();
}*/

void Channelediter::add_widget(int channel, int position, bool analog, Sequence *seq){
    int ii=-1;
    int kk=0;
    bool newsection=false;

    for(int i=0; i<section_tab->size(); i++){
        if(position<=(section_tab->at(i)->x()+section_tab->at(i)->width())){
            ii=i;
            break;
        }
    }
    if(section_tab->isEmpty()){
        newsection = true;
        ii = 0;
    }
    else if(ii<0){
        ii = section_tab->size()-1;
    }
    if(newsection){
        QList<Editerwidget*> **sec_ew = new QList<Editerwidget*>*[Analog_output_channel+Digital_output_channel];
        QList<Sequence*> **sec_seq = new QList<Sequence*>*[Analog_output_channel+Digital_output_channel];
        for(int i=0; i<Analog_output_channel+Digital_output_channel; i++){
            sec_ew[i] = new QList<Editerwidget*>;
            sec_seq[i] = new QList<Sequence*>;
        }
        SectionActivity *act = new SectionActivity;
        Sectiontab *tab = new Sectiontab(ui->section,"  ",variables,variableClass,act,current_run);
        connect(tab,SIGNAL(save_section(Sectiontab*,QString)),this,SLOT(Section_save(Sectiontab*,QString)));
        connect(tab,SIGNAL(delete_section(Sectiontab*)),this,SLOT(delete_section(Sectiontab*)));
        connect(act,SIGNAL(Changed()),this,SLOT(Report_Edited()));
        section_tab->insert(ii,tab);
        sections_activity->insert(ii,act);
        sections_group.insert(ii,false);
        tab->setGeometry(5,5,136,70);
        connect(tab,SIGNAL(group(Sectiontab*,bool)),this,SLOT(section_groupped(Sectiontab*,bool)));
        //tab->installEventFilter(this);
        tab->show();
        widgets->insert(ii,sec_ew);
        sequences->insert(ii,sec_seq);
        section_array->insert(ii,"null");
    }
    Editerwidget *temp = new Editerwidget;
    Sequence *seqd;
    if(seq == NULL) seqd = new Sequence(0,!analog,variables,variableClass);
    else seqd = new Sequence(seq,variables,variableClass);
    if(analog){
        temp->setParent(ui->maina);
    }
    else{
        temp->setParent(ui->maind);
        channel = channel + Analog_output_channel;
    }

    for(int k=0; k<widgets->at(ii)[channel]->size(); k++){
        if(widgets->at(ii)[channel]->at(k)->x()<(position-67)){
            kk=k;
        }
    }
    temp->initial(seqd,variables);
    temp->installEventFilter(this);
    if(widgets->at(ii)[channel]->isEmpty()){
        widgets->at(ii)[channel]->append(temp);
        sequences->at(ii)[channel]->append(seqd);
    }
    else{
        widgets->at(ii)[channel]->insert(kk+1,temp);
        sequences->at(ii)[channel]->insert(kk+1,seqd);
    }
    temp->move(widgets->at(ii)[channel]->at(kk)->x()+136,85*channel);
    temp->show();
    seqd->set_change(true);
    connect(seqd,SIGNAL(Changed(bool)),this,SLOT(Report_Edited()));
    sort_widget();
}

void Channelediter::move_widget(int tochannel, int toposition, QWidget *w, bool analog){
    int ii=0;
    int section_remove;
    int jj=w->y()/85;
    int kk=0;
    bool isempty=true;
    toposition = toposition + w->x()/136;

    if(!analog)jj = jj + Analog_output_channel;
    Editerwidget *e = static_cast<Editerwidget*>(w);
    for(int i=0; i<widgets->size(); i++){
       for(int k=0; k<widgets->at(i)[jj]->size(); k++){
           if(widgets->at(i)[jj]->at(k)==e){
               ii=i;
               kk=k;
               break;
           }
        }
    }
    widgets->at(ii)[jj]->removeAt(kk);
    sequences->at(ii)[jj]->removeAt(kk);
    section_remove = ii;

    for(int i=0; i<section_tab->size(); i++){
        if(section_tab->at(i)->x()<=(toposition*136+68)&&(toposition*136+68)<(section_tab->at(i)->x()+section_tab->at(i)->width())){
            ii=i;
            break;
        }
    }
    kk=0;
    for(int k=0; k<widgets->at(ii)[jj+tochannel]->size(); k++){
        if(widgets->at(ii)[jj+tochannel]->at(k)->x()<(toposition*136-87)){
            kk=k;
        }
    }
    widgets->at(ii)[tochannel+jj]->insert(kk+1,e);
    sequences->at(ii)[tochannel+jj]->insert(kk+1,e->seq);
    e->seq->set_change(true);

    for(int i=0; i<Analog_output_channel+Digital_output_channel; i++)if(!widgets->at(section_remove)[i]->isEmpty())isempty=false;
    if(isempty){
        widgets->removeAt(section_remove);
        sequences->removeAt(section_remove);
        section_array->removeAt(section_remove);
        delete section_tab->at(section_remove);
        section_tab->removeAt(section_remove);
        delete sections_activity->at(section_remove);
        sections_activity->removeAt(section_remove);
    }
    sort_widget();
}

void Channelediter::copy_widget(int tochannel, int toposition, QWidget *w, bool analog){
    //int ii=0;
    int jj=w->y()/85;
    //int kk=0;
    toposition = toposition + w->x()/136;
    Editerwidget *e = static_cast<Editerwidget*>(w);
    e->show();
    add_widget(jj+tochannel,toposition*136-87,analog,e->seq);
}

void Channelediter::delete_widget(QWidget *w, bool analog){
    int jj = w->y()/85;
    int ii=0;
    int kk=0;
    bool isempty=true;
    if(!analog)jj = jj + Analog_output_channel;
    Editerwidget *e = static_cast<Editerwidget*>(w);
    for(int i=0; i<widgets->size(); i++){
       for(int k=0; k<widgets->at(i)[jj]->size(); k++){
           if(widgets->at(i)[jj]->at(k)==e){
               ii=i;
               kk=k;
               break;
           }
        }
    }
    widgets->at(ii)[jj]->removeAt(kk);
    sequences->at(ii)[jj]->removeAt(kk);
    delete e->seq;
    delete w;

    for(int i=0; i<Analog_output_channel+Digital_output_channel; i++)if(!widgets->at(ii)[i]->isEmpty())isempty=false;
    if(isempty){
        widgets->removeAt(ii);
        sequences->removeAt(ii);
        section_array->removeAt(ii);
        delete section_tab->at(ii);
        section_tab->removeAt(ii);
        delete sections_activity->at(ii);
        sections_activity->removeAt(ii);
    }
    sort_widget();
}

void Channelediter::add_section(int position){
    if(ui->section_name->currentText().isEmpty())return;
    bool valid = false;
    int ii=0;
    int index = ui->section_name->currentIndex();
    for(int i=0; i<Analog_output_channel+Digital_output_channel; i++)if(!sections->at(index)[i]->isEmpty())valid=true;
    if(!valid)return;

    for(int i=0; i<section_tab->size(); i++){
        if(position*136 < (section_tab->at(i)->x()+0.5*section_tab->at(i)->width())){
            ii=i;
            break;
        }
        else if(i==(section_tab->size()-1)){
            ii=i+1;
            break;
        }
    }

    QList<Sequence*> **sec_seq = new QList<Sequence*>*[Analog_output_channel+Digital_output_channel];
    QList<Editerwidget*> **sec_ew = new QList<Editerwidget*>*[Analog_output_channel+Digital_output_channel];
    for(int i=0; i<Analog_output_channel; i++){
        sec_seq[i] = new QList<Sequence*>;
        sec_ew[i] = new QList<Editerwidget*>;
        for(int j=0; j<sections->at(index)[i]->size(); j++){
            Editerwidget *w = new Editerwidget(ui->maina);
            Sequence *seq = new Sequence(0,false,variables,variableClass);
            *seq = *sections->at(index)[i]->at(j);
            w->initial(seq,variables);
            w->installEventFilter(this);
            sec_ew[i]->append(w);
            sec_seq[i]->append(seq);
            connect(seq,SIGNAL(Changed(bool)),this,SLOT(Report_Edited()));
            w->show();
        }
    }
    for(int i=Analog_output_channel; i<Analog_output_channel+Digital_output_channel; i++){
        sec_seq[i] = new QList<Sequence*>;
        sec_ew[i] = new QList<Editerwidget*>;
        for(int j=0; j<sections->at(index)[i]->size(); j++){
            Editerwidget *w = new Editerwidget(ui->maind);
            Sequence *seqd = new Sequence(0,true,variables,variableClass);
            *seqd = *sections->at(index)[i]->at(j);
            w->initial(seqd,variables);
            w->installEventFilter(this);
            sec_ew[i]->append(w);
            sec_seq[i]->append(seqd);
            connect(seqd,SIGNAL(Changed(bool)),this,SLOT(Report_Edited()));
            w->show();
        }
    }
    widgets->insert(ii,sec_ew);
    sequences->insert(ii,sec_seq);
    section_array->insert(ii,sections_name->at(index));

    SectionActivity *act = new SectionActivity;
    Sectiontab *tab = new Sectiontab(ui->section,sections_name->at(index),variables,variableClass,act,current_run);
    connect(tab,SIGNAL(save_section(Sectiontab*,QString)),this,SLOT(Section_save(Sectiontab*,QString)));
    connect(tab,SIGNAL(delete_section(Sectiontab*)),this,SLOT(delete_section(Sectiontab*)));
    connect(act,SIGNAL(Changed()),this,SLOT(Report_Edited()));
    section_tab->insert(ii,tab);
    sections_group.insert(ii,false);
    sections_activity->insert(ii,act);
    //connect(tab,SIGNAL(activity_change()),this,SLOT(section_active_change()));
    connect(tab,SIGNAL(group(Sectiontab*,bool)),this,SLOT(section_groupped(Sectiontab*,bool)));
    tab->setGeometry(0,5,200,70);
    //tab->installEventFilter(this);
    tab->show();
    sort_widget();
}

void Channelediter::delete_section(Sectiontab *tab){
    int ii=0;
    for(int i=0; i<section_tab->size(); i++)if(section_tab->at(i)==tab)ii=i;

    for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
        for(int k=0; k<widgets->at(ii)[j]->size(); k++){
            delete sequences->at(ii)[j]->at(k);
            delete widgets->at(ii)[j]->at(k);
        }
        sequences->at(ii)[j]->clear();
        widgets->at(ii)[j]->clear();
    }
    QList<Sequence*> **del_Seq = sequences->at(ii);
    QList<Editerwidget*> **del_Widget = widgets->at(ii);
    Sectiontab *del_SecTab = section_tab->at(ii);
    SectionActivity *del_SecAct = sections_activity->at(ii);
    widgets->removeAt(ii);
    sequences->removeAt(ii);
    section_tab->removeAt(ii);
    sections_activity->removeAt(ii);
    section_array->removeAt(ii);
    sections_group.removeAt(ii);
    delete del_Seq;
    delete del_Widget;
    del_SecTab->deleteLater();
    del_SecAct->deleteLater();

    sort_widget();
}

void Channelediter::Section_save(Sectiontab *tab, QString name){
    bool newSection = true;
    int index = sections_name->size();
    int ii = 0;
    for(int i=0; i<sections_name->size(); i++){
        if(name==sections_name->at(i)){
            newSection = false;
            index = i;
            break;
        }
    }
    for(int i=0; i<section_tab->size(); i++){
        if(tab==section_tab->at(i)){
            ii = i;
            break;
        }
    }
    if(newSection){
        sections_name->append(name);
        ui->section_name->addItem(name);
        QList<Sequence*>** section = new QList<Sequence*>*[Analog_output_channel+Digital_output_channel];
        for(int i=0; i<Analog_output_channel+Digital_output_channel; i++)section[i] = new QList<Sequence*>;
        sections->append(section);
    }
    sections_name->replace(index,name);
    for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
        for(int k=0; k<sections->at(index)[j]->size(); k++){
            delete sections->at(index)[j]->at(k);
        }
        sections->at(index)[j]->clear();

        for(int k=0; k<sequences->at(ii)[j]->size(); k++){
            Sequence *seq = new Sequence(sequences->at(ii)[j]->at(k),variables,variableClass);
            sections->at(index)[j]->append(seq);
        }
    }
}

void Channelediter::sort_widget(){
    for(int i=0; i<section_seperator->size(); i++)delete section_seperator->at(i);
    for(int i=0; i<section_dummyBlock->size(); i++)delete section_dummyBlock->at(i);
    section_seperator->clear();
    section_dummyBlock->clear();

    int total_width=0;
    for(int i=0; i<widgets->size(); i++){
        int sec_width=0;
        for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
            for(int k=0; k<widgets->at(i)[j]->size(); k++){
                if(j<Analog_output_channel)widgets->at(i)[j]->at(k)->move(1+(total_width+k)*136,85*j);
                else widgets->at(i)[j]->at(k)->move(1+(total_width+k)*136,85*(j-Analog_output_channel));
                if((k+1)>sec_width)sec_width = k+1;
                if(sections_group.at(i))widgets->at(i)[j]->at(k)->hide();
                else widgets->at(i)[j]->at(k)->show();
            }
        }
        if(sections_group.at(i)){
            sec_width = 1;
            QFrame *dB = new QFrame(ui->maina);
            section_dummyBlock->append(dB);
            dB->setFrameStyle(QFrame::VLine|QFrame::Raised);
            dB->setLineWidth(5);
            dB->setMidLineWidth(110);
            dB->setGeometry(total_width*136+1,1,133,ui->maina->height()-3);
            dB->show();
            QFrame *dB2 = new QFrame(ui->maind);
            section_dummyBlock->append(dB2);
            dB2->setFrameStyle(QFrame::Box|QFrame::Raised);
            dB2->setLineWidth(5);
            dB2->setMidLineWidth(10);
            dB2->setGeometry(total_width*136+1,1,133,ui->maind->height()-3);
            dB2->show();
        }
        section_tab->at(i)->setGeometry(1+136*total_width,5,136*sec_width,70);
        total_width = total_width + sec_width;

        QFrame *sep = new QFrame(ui->maina);
        section_seperator->append(sep);
        sep->setFrameShadow(QFrame::Plain);
        sep->setLineWidth(2);
        sep->setFrameStyle(QFrame::VLine);
        sep->setGeometry(total_width*136-1,0,2,ui->maina->height());
        sep->show();
        QFrame *sep2 = new QFrame(ui->maind);
        section_seperator->append(sep2);
        sep2->setFrameShadow(QFrame::Plain);
        sep2->setLineWidth(2);
        sep2->setFrameStyle(QFrame::VLine);
        sep2->setGeometry(total_width*136-1,0,2,ui->maind->height());
        sep2->show();
    }
    change_window_size(total_width*136+300);
    Report_Edited();
}

void Channelediter::change_window_size(int x){
    ui->maina->setMinimumWidth(x);
    ui->section->setMinimumWidth(x);
    ui->maind->setMinimumWidth(x);
}

void Channelediter::resizeEvent(QResizeEvent*){
    ui->Scroll_section->setGeometry(110,80,this->width()-126,79);
    ui->tabWidget->setGeometry(-1,143,this->width()+4,this->height()-139);
    ui->Scroll_timea->setGeometry(-1,-1,111,this->height()-175);
    ui->Scroll_maina->setGeometry(109,-1,this->width()-109,this->height()-158);
    ui->Scroll_timed->setGeometry(-1,-1,111,this->height()-175);
    ui->Scroll_maind->setGeometry(109,-1,this->width()-109,this->height()-158);
    ui->line->setGeometry(0,60,this->width(),16);

    for(int i=0; i<background->size(); i++){
        QPixmap fil = QPixmap(ui->maina->width(),85);
        fil.fill(color[channel_color->at(i)]);
        background->at(i)->setGeometry(background->at(i)->x(),background->at(i)->y(),ui->maina->width(),85);
        background->at(i)->setPixmap(fil);
    }
}

void Channelediter::background_move(int x){
    for(int i=0; i<background->size(); i++){
        background->at(i)->move(x,background->at(i)->y());
    }
}

void Channelediter::show_calibration_menu(int chl){
    if(chl>Analog_output_channel-1){
        chl = chl - Analog_output_channel;
        if(digital_invert->at(chl)->text()=="Normal"){
            channel_calibration->replace(chl+Analog_output_channel,"Inverted");
            digital_invert->at(chl)->setText("Inverted");
        }
        else{
            channel_calibration->replace(chl+Analog_output_channel,"Normal");
            digital_invert->at(chl)->setText("Normal");
        }
    }
    else{
        QMenu *menu = new QMenu(this);
        QMenu *submenu[2];
        submenu[0] = new QMenu("Calcualted",menu);
        submenu[1] = new QMenu("Data File",menu);
        menu->addSection("Use Variable");
        QSignalMapper *signalmap = new QSignalMapper(menu);

        for(int i=0; i<variables->size();i++){
            if(variables->at(i)->get_type()==2 || variables->at(i)->get_type()==3){
                QAction *action = new QAction(menu);
                QString sending = QString(QString::number(chl)+":"+variables->at(i)->get_name());
                action->setText(variables->at(i)->get_name());
                if(cal_label->at(chl)->text()==variables->at(i)->get_name()){
                    action->setCheckable(true);
                    action->setChecked(true);
                }
                submenu[variables->at(i)->get_type()-2]->addAction(action);
                connect(action,SIGNAL(triggered()),signalmap,SLOT(map()));
                signalmap->setMapping(action,sending);
                connect(signalmap,SIGNAL(mapped(QString)),this,SLOT(channel_calibration_changed(QString)));
            }
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
        menu->addSeparator();
        QAction *cancel = new QAction(menu);
        QString sending = QString(QString::number(chl)+":"+"Not calibrated");
        cancel->setText("Cancel calibration");
        connect(cancel,SIGNAL(triggered()),signalmap,SLOT(map()));
        signalmap->setMapping(cancel,sending);
        connect(signalmap,SIGNAL(mapped(QString)),this,SLOT(channel_calibration_changed(QString)));
        menu->addMenu(submenu[0]);
        menu->addMenu(submenu[1]);
        menu->addAction(cancel);
        menu->popup(QCursor::pos());
    }
}

void Channelediter::on_Change_apply_clicked()
{
    emit apply_clicked();
}

void Channelediter::on_edit_lock_clicked()
{
    if(ui->edit_lock->isChecked()){
        ui->maina->setDisabled(true);
        ui->maind->setDisabled(true);
        ui->timea->setDisabled(true);
        ui->timed->setDisabled(true);
        ui->section->setDisabled(true);
        this->removeEventFilter(this);
        for(int i=0; i<widgets->size(); i++){
            for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
                for(int k=0; k<widgets->at(i)[j]->size(); k++)widgets->at(i)[j]->at(k)->removeEventFilter(this);
            }
            if(!section_tab->at(i)->isHidden())section_tab->at(i)->removeEventFilter(this);
        }
    }
    else{
        ui->maina->setEnabled(true);
        ui->maind->setEnabled(true);
        ui->timea->setEnabled(true);
        ui->timed->setEnabled(true);
        ui->section->setEnabled(true);
        this->installEventFilter(this);
        for(int i=0; i<widgets->size(); i++){
            for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
                for(int k=0; k<widgets->at(i)[j]->size(); k++)widgets->at(i)[j]->at(k)->installEventFilter(this);
            }
            if(!section_tab->at(i)->isHidden())section_tab->at(i)->installEventFilter(this);
        }
    }
}

void Channelediter::section_names_import(){
    ui->section_name->clear();
    for(int i=0; i<sections_name->size(); i++){
        if(sections_name->at(i)!="New Section")ui->section_name->addItem(sections_name->at(i));
    }
}

void Channelediter::calcualte_times(){
    double total_time = 0;
    for(int i=0; i<sequences->size(); i++){
        double sec_time = 0;
        for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
            double channel_time = 0;
            for(int k=0; k<sequences->at(i)[j]->size(); k++){
                double seq_time = sequences->at(i)[j]->at(k)->get_time(*current_run);
                if(seq_time>0)channel_time += seq_time;
            }
            if(channel_time>sec_time)sec_time = channel_time;
        }
        section_tab->at(i)->set_time(sec_time);
        if(section_tab->at(i)->get_active())total_time +=sec_time;
    }
    ui->time_total->display(total_time);
}

void Channelediter::channel_name_changed(QString){
    channel_name->clear();
    for(int i=0; i<chlname->size(); i++){
        channel_name->append(chlname->at(i)->text());
    }
}

void Channelediter::channel_calibration_changed(QString receive){
    int chl = receive.mid(0,receive.indexOf(':')).toInt();
    QString name = receive.mid(receive.indexOf(':')+1,-1);
    cal_label->at(chl)->setText(name);
    channel_calibration->replace(chl,name);
}

void Channelediter::section_groupped(Sectiontab *tab, bool groupped){
    for(int i=0; i<section_tab->size(); i++){
        if(tab==section_tab->at(i)){
            sections_group.replace(i,groupped);
            break;
        }
    }
    sort_widget();
}

void Channelediter::Report_Edited(){
    emit report_cycle_Edited();
}

void Channelediter::debug_function(int i){
    ui->Scroll_maind->verticalScrollBar()->setValue(i);
    qDebug() << "Bar value" << i;
    qDebug() << "Analog " << ui->Scroll_maina->verticalScrollBar()->value();
    qDebug() << "Digital " << ui->Scroll_maind->verticalScrollBar()->value();
}

void Channelediter::Delay(int millisecondsToWait){
    QTime dieTime = QTime::currentTime().addMSecs(millisecondsToWait);
    while(QTime::currentTime() < dieTime){
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
    }
}
