#include "displaywindow.h"
#include "ui_displaywindow.h"
#include <QDebug>

DisplayWindow::DisplayWindow(QWidget *parent, int AO_c, int DiO_c) :
    QDialog(parent),
    ui(new Ui::DisplayWindow)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window);

    Analog_output_channel = AO_c;
    Digital_output_channel = DiO_c;

    QCursor temp;
    temp.setShape(Qt::BlankCursor);
    cursor = new QLabel(this);
    cursor->setGeometry(20,180,600,300);
    cursor->setCursor(temp);
    cursor->setMouseTracking(true);
    cursor->show();

    menubar = new QMenuBar(this);
    QAction *save_file = new QAction(menubar);
    QAction *save_image = new QAction(menubar);

    save_file->setText("Save to File");
    save_image->setText("Save to Image");

    menubar->setGeometry(0,0,640,20);
    menubar->addAction(save_file);
    menubar->addAction(save_image);
    menubar->setVisible(true);

    connect(save_file,SIGNAL(triggered()),this,SLOT(save_into_file()));
    connect(save_image,SIGNAL(triggered()),this,SLOT(save_into_image()));

    QStringList AOList;
    QStringList DiOList;
    for(int i=0; i<Analog_output_channel+1; i++)AOList.append(QString::number(i));
    for(int i=0; i<Digital_output_channel+1; i++)DiOList.append(QString::number(i));

    for(int i=0; i<12; i++){
        int x,y;
        AO[i] = new QComboBox(ui->AODisplay_channels);
        x = 15+(i%6)*53;
        y = 18+(i/6)*25;
        AO[i]->setGeometry(x,y,43,20);
        AO[i]->addItems(AOList);
        AO[i]->setDisabled(true);
        connect(AO[i],SIGNAL(currentIndexChanged(int)),this,SLOT(aochannel_displayed_refresh(int)));
    }
    for(int i=0; i<6; i++){
        int x,y;
        DiO[i] = new QComboBox(ui->DiODisplay_channels);
        x = 15+(i%6)*53;
        y = 18+(i/6)*25;
        DiO[i]->setGeometry(x,y,43,20);
        DiO[i]->addItems(DiOList);
        DiO[i]->setDisabled(true);
        connect(DiO[i],SIGNAL(currentIndexChanged(int)),this,SLOT(diochannel_displayed_refresh(int)));
    }
    AO[0]->setEnabled(true);
    DiO[0]->setEnabled(true);

    AO_height = ui->Display_screen->height();
    DiO_height = 0;

    color[0].setRgb(255,0,0,255);
    color[1].setRgb(255,160,122,255);
    color[2].setRgb(255,105,180,255);
    color[3].setRgb(255,127,80,255);
    color[4].setRgb(255,140,0,255);
    color[5].setRgb(255,215,0,255);
    color[6].setRgb(189,183,107,255);
    color[7].setRgb(148,0,211,255);
    color[8].setRgb(106,90,205,255);
    color[9].setRgb(50,205,50,255);
    color[10].setRgb(0,250,154,255);
    color[11].setRgb(0,128,0,255);
    color[12].setRgb(154,205,50,255);
    color[13].setRgb(0,139,139,255);
    color[14].setRgb(127,255,212,255);
    color[15].setRgb(135,206,235,255);
    color[16].setRgb(30,144,255,255);
    color[17].setRgb(0,0,205,255);
    color[18].setRgb(222,184,135,255);
    color[19].setRgb(165,42,42,255);
    color[20].setRgb(255,228,225,255);
    color[21].setRgb(192,192,192,255);
    color[22].setRgb(105,105,105,255);
    color[23].setRgb(0,0,0,255);


    cursor->installEventFilter(this);
    //ui->groupBox_2->installEventFilter(this);
}

DisplayWindow::~DisplayWindow()
{
    for(int i=0; i<12; i++)delete AO[i];
    for(int i=0; i<6; i++)delete DiO[i];
    delete cursor;
    delete ui;
}

void DisplayWindow::aochannel_displayed_refresh(int a){
    AO_display.clear();
    for(int i=0; i<12; i++)if(AO[i]->currentIndex()!=0)AO_display.append(AO[i]->currentIndex()-1);
    for(int i=11; i>-1; i--){
        if(i<AO_display.size())AO[i]->setCurrentIndex(AO_display.at(i)+1);
        else if(i==AO_display.size()){
            AO[i]->setEnabled(true);
            AO[i]->setCurrentIndex(0);
        }
        else{
            AO[i]->setDisabled(true);
            AO[i]->setCurrentIndex(0);
        }
    }
    draw();
}

void DisplayWindow::diochannel_displayed_refresh(int a){
    DiO_display.clear();
    for(int i=0; i<6; i++)if(DiO[i]->currentIndex()!=0)DiO_display.append(DiO[i]->currentIndex()-1);
    for(int i=5; i>-1; i--){
        if(i<DiO_display.size())DiO[i]->setCurrentIndex(DiO_display.at(i)+1);
        else if(i==DiO_display.size()){
            DiO[i]->setEnabled(true);
            DiO[i]->setCurrentIndex(0);
        }
        else{
            DiO[i]->setDisabled(true);
            DiO[i]->setCurrentIndex(0);
        }
    }
    DiO_height = DiO_display.size()*40;
    AO_height = ui->Display_screen->height()-DiO_height;
    if(AO_height<100){
        AO_height=100;
        DiO_height=ui->Display_screen->height()-AO_height;
    }
    draw();
}

void DisplayWindow::on_Display_close_clicked()
{
    this->accept();
}

void DisplayWindow::on_Display_refresh_clicked()
{
    refresh();
}

void DisplayWindow::initial(QList<QList<Sequence *> **> *iseq, int itime , int *run, QList<QString> *chl_cal, QList<Variable*> *ivari, QList<QString> *sec_act){
    sequences = iseq;
    channel_calibration = chl_cal;
    sections_activity = sec_act;
    variables = ivari;
    top = 10;
    bottom = -10;
    left = 0;
    right = double(itime/1000);
    total_time = double(itime);
    int sum =1;

    total_number_of_run = run;
    number_of_run = 1;
    for(int i=0; i<10; i++)if(total_number_of_run[i]!=0)sum *= total_number_of_run[i];

    ui->Number_of_run_Slider->setMaximum(sum);
    QString r;
    r.setNum(right);

    ui->Display_screen->setText("");
    ui->Display_height_top->setText("10");
    ui->Display_height_bottom->setText("-10");
    ui->Display_width_left->setText("0");
    ui->Display_width_right->setText(r);
}

bool DisplayWindow::eventFilter(QObject *obj, QEvent *event){
    if(obj == cursor){
        if(event->type() == QEvent::MouseMove){
            QMouseEvent *a = static_cast<QMouseEvent*>(event);
            double x,y;
            x = a->x()*(right-left)/ui->Display_screen->width()+left;
            if(a->y()>AO_height)y = bottom;
            else y = (1 - double(a->y())/double(AO_height))*(top-bottom) + bottom;
            ui->Display_cursor_X->setText("X: "+QString::number(x));
            ui->Display_cursor_Y->setText("Y: "+QString::number(y));
            QPixmap temp = QPixmap(cursor->width(),cursor->height());
            QPainter painter;
            QPen cursorline;
            temp.fill(Qt::transparent);
            cursorline.setWidth(1);
            cursorline.setStyle(Qt::SolidLine);
            cursorline.setColor(QColor(20,20,20));
            painter.begin(&temp);
            painter.setPen(cursorline);
            painter.drawLine(a->x(),0,a->x(),cursor->height());
            painter.drawLine(0,a->y(),cursor->width(),a->y());
            painter.end();
            cursor->setPixmap(temp);
            return true;
        }
        else if(event->type() == QEvent::Wheel){
            QWheelEvent *a = static_cast<QWheelEvent*>(event);
            int rotate = a->angleDelta().y()/120;
            if(rotate>0){
                right -= (right-((right-left)*a->x()/ui->Display_screen->width()+left))*0.1;
                left += (((right-left)*a->x()/ui->Display_screen->width()+left)-left)*0.1;
            }
            else {
                right += (right-((right-left)*a->x()/ui->Display_screen->width()+left))/9;
                left -= (((right-left)*a->x()/ui->Display_screen->width()+left)-left)/9;
            }
            if(left<0)left=0;
            ui->Display_width_left->setText(QString::number(left));
            ui->Display_width_right->setText(QString::number(right));
            draw();
            return true;
        }
        else return false;
    }
    else if(obj==ui->groupBox_2 && event->type()==QEvent::MouseMove){
        cursor->clear();
        return true;
    }
    return DisplayWindow::eventFilter(obj,event);
}

void DisplayWindow::resizeEvent(QResizeEvent *){
    ui->groupBox_2->setGeometry(10,170,this->width()-20,this->height()-220);
    ui->Display_screen->setGeometry(10,10,ui->groupBox_2->width()-20,ui->groupBox_2->height()-20);
    cursor->setGeometry(20,180,ui->Display_screen->width(),ui->Display_screen->height());
    ui->Display_cursor_X->move(40,170+ui->groupBox_2->height());
    ui->Display_cursor_Y->move(190,170+ui->groupBox_2->height());
    ui->Display_refresh->move(440,180+ui->groupBox_2->height());
    ui->Display_close->move(550,180+ui->groupBox_2->height());
    menubar->setGeometry(0,0,this->width(),20);
    DiO_height = DiO_display.size()*40;
    AO_height = ui->Display_screen->height()-DiO_height;
    if(AO_height<100){
        AO_height=100;
        DiO_height=ui->Display_screen->height()-AO_height;
    }
    draw();
}

void DisplayWindow::refresh(){
    draw();
}

void DisplayWindow::draw(){
    pixmap = QPixmap(ui->Display_screen->width(),ui->Display_screen->height());
    QPainter painter;
    QPen blackpen,solidline,zeroline,dashline,pen[24];
    blackpen.setWidth(2);
    blackpen.setStyle(Qt::SolidLine);
    blackpen.setColor(QColor(0,0,0));
    solidline.setWidth(1);
    solidline.setStyle(Qt::SolidLine);
    solidline.setColor(QColor(0,0,0));
    dashline.setWidth(1);
    dashline.setStyle(Qt::DotLine);
    dashline.setColor(QColor(180,180,180));
    zeroline.setWidth(1);
    zeroline.setStyle(Qt::DashDotLine);
    zeroline.setColor(QColor(0,0,0));
    pixmap.fill();

    for(int i=0; i<24;i++){
        pen[i].setWidth(2);
        pen[i].setStyle(Qt::SolidLine);
        pen[i].setColor(color[i]);
    }

    painter.begin(&pixmap);
    painter.setPen(blackpen);
    painter.drawRect(1,1,ui->Display_screen->width()-2,ui->Display_screen->height()-2);
    painter.drawLine(0,AO_height,ui->Display_screen->width(),AO_height);
    painter.setPen(zeroline);
    painter.drawLine(0,AO_height*top/(top-bottom),ui->Display_screen->width(),AO_height*top/(top-bottom));
    for(int i=0; i<(DiO_height/40); i++)painter.drawLine(2,AO_height+i*40+35,ui->Display_screen->width()-2,AO_height+i*40+35);
    painter.setPen(dashline);

    if(top>bottom)for(int i=top;i>bottom;i--)painter.drawLine(2,AO_height*(i-bottom)/(top-bottom),ui->Display_screen->width()-2,AO_height*(i-bottom)/(top-bottom));
    else for(int i=top;i<bottom;i++)painter.drawLine(2,AO_height*(i-bottom)/(top-bottom),ui->Display_screen->width()-2,AO_height*(i-bottom)/(top-bottom));

    for(int i=0;i<AO_display.size();i++){
        int channel = AO_display.at(i);
        painter.setPen(pen[i]);
        double step = (right-left)*1000/ui->Display_screen->width();
        int y = findpoint_in_time(channel,step+left*1000,AO_height/(top-bottom)*top);
        for(int x=1;x<(ui->Display_screen->width()-2);x++){
            int nexty = findpoint_in_time(channel,(x+1)*step+left*1000,y);
            painter.drawLine(x,y,x+1,nexty);
            y = nexty;
        }
    }
    for(int i=0; i<(DiO_height/40); i++){
        int channel = DiO_display.at(i);
        painter.setPen(pen[i+AO_display.size()]);
        double step = (right-left)*1000/ui->Display_screen->width();
        int y = AO_height+i*40+35;
        bool last = find_digiout_in_time(channel,step+left*100,false);
        if(last)y = AO_height+i*40+10;
        for(int x=1;x<(ui->Display_screen->width()-2);x++){
            int nexty = AO_height+i*40+35;
            if(find_digiout_in_time(channel,(x+1)*step+left*100,last)){
                nexty = AO_height+i*40+10;
                last = true;
            }
            else last = false;
            painter.drawLine(x,y,x+1,nexty);
            y = nexty;
        }
    }

    painter.end();
    ui->Display_screen->setPixmap(pixmap);
}

int DisplayWindow::findpoint_in_time(int channel, double time, int last_out){
    double time1 = 0;
    double time2 = 0;
    double time3 = 0;
    int ii = 0;

    for(int i=0; i<sequences->size(); i++){
        double tsec = 0;
        for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
            double act = 0;
            bool ok = false;
            act = sections_activity->at(i).toDouble(&ok);
            if(!ok){
                for(int j=0; j<variables->size(); j++){
                    if(sections_activity->at(i)==variables->at(i)->get_name()){
                        switch(variables->at(i)->get_type()){
                        case 0:
                            act = variables->at(i)->get_value()[0];
                            break;
                        case 1:
                            act = variables->at(i)->get_scan(number_of_run);
                            break;
                        case 2:
                            act = variables->at(i)->get_calculated(number_of_run,0,0,0,0);
                            break;
                        default:
                            break;
                        }
                        break;
                    }
                }
            }
            if(act>0){
                for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
                    double temp=0;
                    for(int k=0; k<sequences->at(i)[j]->size(); k++){
                        temp = temp + sequences->at(i)[j]->at(k)->get_time(1);
                    }
                    if(temp>tsec)tsec = temp;
                }
            }
        }
        if(time3<=time && time<=(time3+tsec)){
            ii = i;
            time2 = time3;
            break;
        }
        else time3 += tsec;
    }

    for(int i=0; i<sequences->at(ii)[channel]->size();i++){
        if(sequences->at(ii)[channel]->at(i)->get_type()!=0){
            time1 = time2;
            time2 += sequences->at(ii)[channel]->at(i)->get_time(number_of_run);
        }
        if((time1<=time) && (time< time2)){
            double voltage = sequences->at(ii)[channel]->at(i)->get_output(number_of_run,time-time1,time-time3,time,channel_calibration->at(channel));
            return AO_height - ((voltage -bottom) * AO_height/(top-bottom));
        }
        if(i==sequences->at(ii)[channel]->size()-1 && time>time2){
            return last_out;
        }
    }
    return last_out;
}

bool DisplayWindow::find_digiout_in_time(int channel, double time, bool last_out){
    double time1 = 0;
    double time2 = 0;
    double time3 = 0;
    int ii = 0;

    for(int i=0; i<sequences->size(); i++){
        double tsec = 0;
        for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
            double act = 0;
            bool ok = false;
            act = sections_activity->at(i).toDouble(&ok);
            if(!ok){
                for(int j=0; j<variables->size(); j++){
                    if(sections_activity->at(i)==variables->at(i)->get_name()){
                        switch(variables->at(i)->get_type()){
                        case 0:
                            act = variables->at(i)->get_value()[0];
                            break;
                        case 1:
                            act = variables->at(i)->get_scan(number_of_run);
                            break;
                        case 2:
                            act = variables->at(i)->get_calculated(number_of_run,0,0,0,0);
                            break;
                        default:
                            break;
                        }
                        break;
                    }
                }
            }
            if(act>0){
                for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
                    double temp=0;
                    for(int k=0; k<sequences->at(i)[j]->size(); k++){
                        temp = temp + sequences->at(i)[j]->at(k)->get_time(1);
                    }
                    if(temp>tsec)tsec = temp;
                }
            }
        }
        if(time3<=time && time<=(time3+tsec)){
            ii = i;
            time2 = time3;
            break;
        }
        else time3 += tsec;
    }

    channel = channel + Analog_output_channel;
    for(int i=0; i<sequences->at(ii)[channel]->size();i++){
        time1 = time2;
        time2 += sequences->at(ii)[channel]->at(i)->get_time(number_of_run);
        if((time1<=time) && (time<time2)){
            bool out = sequences->at(ii)[channel]->at(i)->get_output(number_of_run,time-time1,time-time3,time,"Not calibrated");
            return out;
        }
        if(i==sequences->at(ii)[channel]->size()-1 && time>time2){
            return last_out;
        }
    }
    return last_out;
}

void DisplayWindow::on_Display_width_left_editingFinished()
{
    bool ok;
    QString temp = ui->Display_width_left->text();
    double temp_number = temp.toDouble(&ok);
    if (!ok){
        ui->Display_width_left->setText(QString::number(left));
        QMessageBox msgBox;

        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Invaild Input (Not a number).");
        msgBox.exec();
    }
    else left = temp_number;
    draw();
}

void DisplayWindow::on_Display_width_right_editingFinished()
{
    bool ok;
    QString temp = ui->Display_width_right->text();
    double tempn = temp.toDouble(&ok);
    if (!ok){
        ui->Display_width_right->setText(QString::number(right));
        QMessageBox msgBox;

        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Invaild Input (Not a number).");
        msgBox.exec();
    }
    else right = tempn;
    draw();
}

void DisplayWindow::on_Display_height_top_editingFinished()
{
    bool ok;
    QString temp = ui->Display_height_top->text();
    double tempn = temp.toDouble(&ok);
    if (!ok){
        QMessageBox msgBox;

        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Invaild Input (Not a number).");
        msgBox.exec();
    }
    else top = tempn;
    draw();
}

void DisplayWindow::on_Display_height_bottom_editingFinished()
{
    bool ok;
    QString temp = ui->Display_height_bottom->text();
    double tempn = temp.toDouble(&ok);
    if (!ok){
        QMessageBox msgBox;

        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Invaild Input (Not a number).");
        msgBox.exec();
    }
    else bottom = tempn;
    draw();
}

void DisplayWindow::on_Number_of_run_Slider_sliderMoved(int position)
{
    number_of_run = position;
    ui->Number_of_run_editer->setText(QString::number(position));
    draw();
}

void DisplayWindow::on_Number_of_run_editer_editingFinished()
{
    bool ok;
    QString temp = ui->Number_of_run_editer->text();
    number_of_run = temp.toInt(&ok);
    if (!ok){
        ui->Display_height_bottom->setText(QString::number(bottom));
        QMessageBox msgBox;
        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Invaild Input (Not a number).");
        msgBox.exec();
        number_of_run = ui->Number_of_run_Slider->sliderPosition();
    }
    ui->Number_of_run_Slider->setSliderPosition(number_of_run);
    draw();
}

void DisplayWindow::on_Number_of_run_Slider_actionTriggered(int action)
{
    if(action==0)return;
    else{
        number_of_run = ui->Number_of_run_Slider->sliderPosition();
        ui->Number_of_run_editer->setText(QString::number(number_of_run));
        draw();
    }
}

void DisplayWindow::save_into_file(){
    QString filename = QFileDialog::getSaveFileName(this,"Save File",0,tr("Text File (*.txt)"));
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))return;
    QTextStream out(&file);

    double last_a[AO_display.size()];
    bool last_d[DiO_display.size()];

    out << "Time(ms)    ";
    for(int i=0; i<AO_display.size(); i++){
        out << "AO"+QString::number(AO_display.at(i)+1)+"   ";
        last_a[i] = 0;
    }
    for(int i=0; i<DiO_display.size(); i++){
        out << "DiO"+QString::number(DiO_display.at(i)+1)+"   ";
        last_d[i]=false;
    }
    out << endl;

    for(int time=0; time<total_time; time++){
        out << time << "   ";
        for(int i=0; i<AO_display.size(); i++){
            last_a[i]=findpoint_in_time(AO_display.at(i),time,last_a[i]);
            double voltage = (AO_height-last_a[i])*(top-bottom)/AO_height+bottom;
            out << voltage << "    ";
        }
        for(int i=0; i<DiO_display.size(); i++){
            last_d[i]=find_digiout_in_time(DiO_display.at(i),time,last_d[i]);
            if(last_d[i])out << "1    ";
            else out << "0    ";
        }
        out << endl;
    }
    file.close();
}

void DisplayWindow::save_into_image(){
    QString filename = QFileDialog::getSaveFileName(this,"Save Image",0,tr("Image Files (*.png *.jpg *.bmp)"));
    pixmap.save(filename);
}
