#include "realtime.h"
#include "ui_realtime.h"

#include "math.h"

RealTime::RealTime(QWidget *parent, int aout, int ain, int dio, int din) :
    QDialog(parent),
    ui(new Ui::RealTime)
{
    ui->setupUi(this);
    color[0].setRgb(255,0,0,255);
    color[1].setRgb(255,215,0,255);
    color[2].setRgb(50,205,50,255);
    color[3].setRgb(135,206,235,255);
    color[4].setRgb(32,178,170,255);
    color[5].setRgb(192,192,192,255);

    Analog_output_channel = aout;
    Analog_input_channel = ain;
    Digital_output_channel = dio;
    Digital_input_channel = din;
    v_dial_position = 50;

    QStringList channels;
    for(int i=0; i<Analog_input_channel; i++){
        QString temp = "Analog input ";
        temp.append(QString::number(i+1));
        channels.append(temp);
    }

    Lattic_length_H[0] = 1000;
    Lattic_length_H[1] = 500;
    Lattic_length_H[2] = 300;
    Lattic_length_H[3] = 200;
    Lattic_length_H[4] = 100;
    Lattic_length_H[5] = 50;
    Lattic_length_H[6] = 40;
    Lattic_length_H[7] = 30;
    Lattic_length_H[8] = 20;
    Lattic_length_H[9] = 10;
    Lattic_length_V[0] = 5000;
    Lattic_length_V[1] = 2000;
    Lattic_length_V[2] = 1000;
    Lattic_length_V[3] = 500;
    Lattic_length_V[4] = 200;
    Lattic_length_V[5] = 100;
    Lattic_length_V[6] = 50;
    Lattic_length_V[7] = 20;
    Lattic_length_V[8] = 10;
    lattic_h = 1;
    ui->H_dial->setValue(lattic_h);
    ui->H_display->display(Lattic_length_H[lattic_h]);
    for(int i=0; i<number_of_channel; i++){
        lattic_v[i] = 0;
        v_position[i] = 0;
        active[i] = -1;
    }

    QPixmap backgroud = QPixmap(ui->Back->width(),ui->Back->height());
    backgroud.fill(Qt::black);
    QPainter painter;
    QPen whiteline;
    whiteline.setColor(Qt::white);
    whiteline.setWidth(1);
    whiteline.setStyle(Qt::DashLine);
    painter.begin(&backgroud);
    painter.setPen(whiteline);
    painter.drawRect(0,0,ui->Back->width()-1,ui->Back->height()-1);
    for(int i=0; i<(ui->Back->height()/squrewidth); i++)painter.drawLine(0,i*squrewidth,ui->Back->width()-1,i*squrewidth);
    for(int i=0; i<(ui->Back->width()/squrewidth); i++)painter.drawLine(i*squrewidth,0,i*squrewidth,ui->Back->height()-1);
    painter.end();
    ui->Back->setPixmap(backgroud);
    ui->Cursor->clear();
    ui->Cursor->installEventFilter(this);
    ui->Displaywindow->installEventFilter(this);

    for(int i=0; i<number_of_channel; i++){
        QFont on,feq;
        on.setPointSize(11);
        feq.setPointSize(10);
        QWidget *page = new QWidget;
        ui->Channel_control->addTab(page,"Chl "+QString::number(i+1));
        QCheckBox *checkbox = new QCheckBox("On",page);
        QLabel *label = new QLabel("Frequency: NA",page);
        QComboBox *combobox = new QComboBox(page);
        checkbox->setFont(on);
        label->setFont(feq);
        combobox->addItems(channels);
        checkbox->setGeometry(30,20,50,21);
        label->setGeometry(30,60,240,30);
        combobox->setGeometry(140,20,130,25);

        QSignalMapper *signalmapper = new QSignalMapper(this);
        connect(checkbox,SIGNAL(clicked()),signalmapper,SLOT(map()));
        connect(combobox,SIGNAL(currentIndexChanged(int)),signalmapper,SLOT(map()));
        signalmapper->setMapping(checkbox,i);
        signalmapper->setMapping(combobox,i);
        connect(signalmapper,SIGNAL(mapped(int)),this,SLOT(channel_state_change(int)));

        Display_active.append(checkbox);
        Display_channel.append(combobox);
        Channel_freq.append(label);
    }

    input_timer = new QTimer(this);
    connect(input_timer,SIGNAL(timeout()),this,SLOT(Draw()));
    connect(input_timer,SIGNAL(timeout()),this,SLOT(take_data()));

    output_timer = new QTimer(this);
}

RealTime::~RealTime()
{
    delete ui;
}

bool RealTime::eventFilter(QObject *obj, QEvent *event){
    if(obj==ui->Cursor){
        if(event->type()==QEvent::MouseMove){
            QMouseEvent *a = static_cast<QMouseEvent*>(event);
            Draw_cursor(a->x(),a->y());
            return true;
        }
        else if(event->type()==QEvent::MouseButtonPress){
            QMouseEvent *a = static_cast<QMouseEvent*>(event);
            if(a->button() == Qt::LeftButton){
                Read_cursor(a->x(),a->y());
                return true;
            }
        }
        return false;
    }
    else if(obj == ui->Displaywindow && event->type()==QEvent::MouseMove){
        ui->Cursor->clear();
        return true;
    }
    return false;
}

void RealTime::on_main_toolBox_currentChanged(int index)
{
    if(index == 0){
        output_timer->start(sample_rate);
        input_timer->stop();
    }
    else{
        output_timer->stop();
        input_timer->start(sample_rate);
    }
}

void RealTime::Draw_cursor(int x, int y){
    ui->Cursor->clear();
    QPixmap cur = QPixmap(ui->Cursor->width(),ui->Cursor->height());
    QPainter painter;
    QPen whiteline;
    cur.fill(Qt::transparent);
    whiteline.setColor(Qt::white);
    whiteline.setWidth(1);
    whiteline.setStyle(Qt::SolidLine);
    painter.begin(&cur);
    painter.setPen(whiteline);
    painter.drawLine(0,y,ui->Cursor->width()-1,y);
    painter.drawLine(x,0,x,ui->Cursor->height()-1);
    painter.end();
    ui->Cursor->setPixmap(cur);
}

void RealTime::Read_cursor(int x, int y){
    int active_channel = ui->Channel_control->currentIndex();
    double ry,rx;

    ry = double(Lattic_length_V[lattic_v[active_channel]])/1000.*(ui->Back->height()/squrewidth/2);
    ry -= double(Lattic_length_V[lattic_v[active_channel]])/1000./squrewidth*y;

    rx = double(Lattic_length_H[lattic_h])/squrewidth*x;
    //rx = double(Lattic_length_H[lattic_h])*(ui->Back->width()/squrewidth);
    //rx -= double(Lattic_length_H[lattic_h])/squrewidth*x;

    QString displayx = "X: "+ QString::number(rx)+"ms";
    QString displayy = "Y: "+ QString::number(ry)+"V";
    ui->State->append(displayx);
    ui->State->append(displayy);
    ui->State->append("----------------------");
    ui->State->verticalScrollBar()->setValue(ui->State->verticalScrollBar()->maximum());
}

void RealTime::on_H_dial_valueChanged(int value)
{
    lattic_h = value;
    ui->H_display->display(Lattic_length_H[value]);
}

void RealTime::on_Channel_control_currentChanged(int index)
{
    QString chl = "Channel "+QString::number(index+1);
    ui->Channel_Dial->setTitle(chl);

    ui->V_dial->setValue(lattic_v[index]);
    ui->V_dis->display(Lattic_length_V[lattic_v[index]]);
    ui->Position_dis->setText(QString::number(v_position[index]));
}

void RealTime::on_Position_dial_sliderMoved(int position)
{
    int rotate = position - v_dial_position;
    int active_channel = ui->Channel_control->currentIndex();

    if(rotate>50)rotate = 100-rotate;
    else if(rotate<-50)rotate = -100-rotate;
    v_position[active_channel] += double(Lattic_length_V[lattic_v[active_channel]])/1000/101.*rotate;
    v_dial_position = position;
    ui->Position_dis->setText(QString::number(v_position[active_channel]));
}

void RealTime::on_V_dial_valueChanged(int value)
{
    int active_channel = ui->Channel_control->currentIndex();
    ui->V_dis->display(Lattic_length_V[value]);
    lattic_v[active_channel] = value;
}

void RealTime::on_Position_dis_editingFinished()
{
    int active_channel = ui->Channel_control->currentIndex();
    double temp;
    bool ok;
    temp = ui->Position_dis->text().toDouble(&ok);
    if(!ok){
        ui->Position_dis->setText(QString::number(v_position[active_channel]));
        QMessageBox msgBox;
        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Invalid input.");
        msgBox.exec();
        return;
    }
    else{
        v_position[active_channel] = temp;
    }
}

void RealTime::Draw(){
    ui->Screen->clear();
    int height = ui->Screen->height()-1;
    int width = ui->Screen->width()-1;

    QPixmap screen = QPixmap(width,height);
    screen.fill(Qt::transparent);
    QPainter painter;
    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(2);
    painter.begin(&screen);
    double dx =  double(squrewidth)/double(Lattic_length_H[lattic_h]);
    //double totaly  = Lattic_length_V[lattic_v[i]]/1000*(ui->Screen->height()/squrewidth);
    for(int i=0; i<number_of_channel; i++){
        if(active[i]> -1){
            double dy = -double(squrewidth)/double(Lattic_length_V[lattic_v[active[i]]])*1000.;
            pen.setColor(color[i]);
            painter.setPen(pen);
            double y0 = 0;
            double x0 = width;
            if(!data[i].isEmpty())y0 = (height/2) + ((v_position[i]+data_to_voltage(data[i].last()))*dy);
            for(int j=data[i].size()-2; j>-1; j--){
                double dv = data_to_voltage(data[i].at(j))-data_to_voltage(data[i].at(j+1));
                double y1 = y0 + dv*dy;
                double x1 = x0 - dx*sample_rate;
                painter.drawLine(int(x0),int(y0),int(x1),int(y1));
                y0 = y1;
                x0 = x1;
                if(x0<0)break;
            }
        }
    }

    painter.end();
    ui->Screen->setPixmap(screen);
}

void RealTime::frequence_measure(){
    for(int i=0; i<number_of_channel; i++){
        if(active[i]>-1){
            double frequency = 0;
            //FFT
            Channel_freq.at(i)->setText("Frequency: "+QString::number(frequency));
        }
    }
}

void RealTime::channel_state_change(int chl){
    if(Display_active.at(chl)->isChecked()){
        active[chl] = Display_channel.at(chl)->currentIndex();
        data[chl].clear();
    }
    else{
        active[chl] = -1;
        data[chl].clear();
    }
}

void RealTime::take_data(){
    long ain[8];
    GetData_Long(101,ain,1,8);
/*
    ain[0] = 6554*sin(temp_counter/20.)+32767;
    ain[1] = 6554*sin(temp_counter/30.)+32767;
    ain[2] = 6554*sin(temp_counter/40.)+32767;
    ain[3] = 6554*sin(temp_counter/50.)+32767;

    temp_counter++;
    if(temp_counter>314159)temp_counter = 0;
*/
    for(int i=0; i<number_of_channel; i++){
        if(active[i]>-1){
            int temp = ain[active[i]];
            data[i].append(temp);
        }
        if(data[i].size()>2800){
            data[i].removeFirst();
        }
    }
}

double RealTime::data_to_voltage(int d){
    return double(d)/65535.*20 - 10;
}
