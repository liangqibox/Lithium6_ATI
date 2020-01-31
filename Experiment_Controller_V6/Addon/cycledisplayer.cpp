#include "cycledisplayer.h"
#include "ui_cycledisplayer.h"
#include <QDebug>

CycleDisplayer::CycleDisplayer(QWidget *parent, QScriptEngine *eng, QList<QList<Sequence*>**> *seq, QList<Variable*> *vari, QList<SectionActivity*> *sec_act,
                               QList<QString> *chl_cal, QList<QString> *chl_name, int aout, int diout, bool controlled) :
    QWidget(parent),
    ui(new Ui::CycleDisplayer)
{
    ui->setupUi(this);

    if(controlled){
        sequences = seq;
        variables = vari;
        sections_activity = sec_act;
        channel_calibration = chl_cal;
        channel_name = chl_name;
        QiEngine = eng;
        ui->File_name->setText("From Controller");
        ui->Load_file->setDisabled(true);
    }
    else{
        ui->File_name->setText("No File");
        ui->Group_Load->setDisabled(true);
        ui->Switch_mode->setDisabled(true);
    }
    Analog_output_channel = aout;
    Digital_output_channel = diout;
    cycle_time = 0;
    time_resolution = 25;
    Draw_data = new QList<int>[Analog_output_channel+Digital_output_channel];

    number_of_scan = 0;
    total_number_of_scan = new int[10];
    for(int i=0; i<10; i++)total_number_of_scan[i]=1;
    for(int i=0; i<32; i++)Base[i]=pow(2,i);

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

    indicator = new Indicator(ui->Group_Load);
    indicator->move(40,40);
    indicator->show();

    ui->List->setColumnWidth(0,179);
    ui->List->setColumnWidth(1,50);
    for(int i=0; i<Analog_output_channel+Digital_output_channel; i++){
        QString chl;
        if(i<Analog_output_channel)chl = channel_name->at(i)+" (AO"+QString::number(i+1)+")";
        else chl = channel_name->at(i)+" (DiO"+QString::number(i+1-Analog_output_channel)+")";
        ui->List_add->addItem(chl);
    }
    ui->Cursor_Screen->installEventFilter(this);

    calculate_cycle_data();
    time_start = 0;
    time_end = cycle_time;
    ui->Time_start->setText("0");
    ui->Time_end->setText(QString::number(cycle_time));
    Draw();
}

CycleDisplayer::~CycleDisplayer()
{
    clear_Draw_data();
    delete total_number_of_scan;
    delete Draw_data;
    delete ui;
}

bool CycleDisplayer::eventFilter(QObject *obj, QEvent *event){
    if(obj == ui->Cursor_Screen){
        if(event->type() == QEvent::MouseMove){
            QMouseEvent *a = static_cast<QMouseEvent*>(event);
            double x,y;
            x = a->x()*(time_end-time_start)/ui->Display_Screen->width()+time_start;
            y = (1 - double(a->y())/double(ui->Display_Screen->height()))*20-10;
            ui->Cursor_X->setText("X: "+QString::number(x));
            ui->Cursor_Y->setText("Y: "+QString::number(y));
            QPixmap temp = QPixmap(ui->Cursor_Screen->width(),ui->Cursor_Screen->height());
            QPainter painter;
            QPen cursorline;
            temp.fill(Qt::transparent);
            cursorline.setWidth(1);
            cursorline.setStyle(Qt::SolidLine);
            cursorline.setColor(QColor(20,20,20));
            painter.begin(&temp);
            painter.setPen(cursorline);
            painter.drawLine(a->x(),0,a->x(),ui->Cursor_Screen->height());
            painter.drawLine(0,a->y(),ui->Cursor_Screen->width(),a->y());
            painter.end();
            ui->Cursor_Screen->setPixmap(temp);
            return true;
        }
        else if(event->type() == QEvent::Wheel){
            QWheelEvent *a = static_cast<QWheelEvent*>(event);
            int rotate = a->angleDelta().y()/60;
            if(rotate>0){
                time_end = time_end - (time_end-time_start)*0.05;
                time_start = time_start + (time_end-time_start)*0.05;
            }
            else {
                time_end = time_end + (time_end-time_start)*0.05;
                time_start = time_start - (time_end-time_start)*0.05;
            }
            if(time_start<0)time_start=0;
            if(time_end>cycle_time)time_end=cycle_time;
            ui->Time_start->setText(QString::number(time_start));
            ui->Time_end->setText(QString::number(time_end));
            Draw();
            return true;
        }
        else return false;
    }
    else if(obj==this){
        ui->Cursor_Screen->clear();
        ui->Cursor_X->setText("X: NA");
        ui->Cursor_Y->setText("Y: NA");
        return true;
    }
    return CycleDisplayer::eventFilter(obj,event);
}

void CycleDisplayer::resizeEvent(QResizeEvent *resize){
    ui->Display_Screen->setGeometry(10,190,resize->size().width()-19,resize->size().height()-229);
    ui->Cursor_Screen->setGeometry(10,190,resize->size().width()-19,resize->size().height()-229);
    ui->Plot_range->move(resize->size().width()-311,resize->size().height()-35);
    ui->Cursor_X->move(10,resize->size().height()-38);
    ui->Cursor_Y->move(150,resize->size().height()-38);
    Draw();
}

void CycleDisplayer::on_Add_clicked()
{
    int chl = ui->List_add->currentIndex();
    QString chl_name = ui->List_add->currentText();

    ui->List->setRowCount(ui->List->rowCount()+1);
    QTableWidgetItem *name = new QTableWidgetItem(chl_name);
    QTableWidgetItem *col = new QTableWidgetItem;
    col->setBackgroundColor(color[(ui->List->rowCount()-1)%24]);
    ui->List->setItem(ui->List->rowCount()-1,0,name);
    ui->List->setItem(ui->List->rowCount()-1,1,col);

    Channel.append(chl);
    Channel_color.append(color[(ui->List->rowCount()-1)%24]);
    Draw();
}

void CycleDisplayer::on_Delete_clicked()
{
    if(!ui->List->selectedItems().isEmpty()){
        int row = ui->List->row(ui->List->selectedItems().first());
        delete ui->List->item(row,0);
        delete ui->List->item(row,1);
        ui->List->removeRow(row);
        Channel.removeAt(row);
        Channel_color.removeAt(row);
    }
    Draw();
}

void CycleDisplayer::on_List_cellClicked(int row, int)
{
    ui->List->item(row,0)->setSelected(true);
    ui->List->item(row,1)->setSelected(false);
}

void CycleDisplayer::on_List_cellDoubleClicked(int row, int column)
{
    if(column==1){
        ui->List->item(row,0)->setSelected(true);
        ui->List->item(row,1)->setSelected(false);
        QColor col = QColorDialog::getColor(Channel_color.at(row),0,"Select color",0);
        if(col.isValid()){
            Channel_color.replace(row,col);
            ui->List->item(row,1)->setBackgroundColor(col);
            Draw();
        }
    }
}

void CycleDisplayer::on_Switch_mode_valueChanged(int value)
{
    if(value==0){
        ui->Group_Load->setEnabled(true);
    }
    else{
        ui->Group_Load->setDisabled(true);
        indicator->S_Standby();
    }
}

void CycleDisplayer::on_Number_of_Scan_editingFinished()
{
    bool ok = false;
    int temp = ui->Number_of_Scan->text().toInt(&ok);
    if(ok)number_of_scan = temp;
    else ui->Number_of_Scan->setText(QString::number(number_of_scan));
}

void CycleDisplayer::on_Time_resolution_editingFinished()
{
    bool ok = false;
    int temp = ui->Time_resolution->text().toInt(&ok);
    if(ok)time_resolution = temp;
    else ui->Time_resolution->setText(QString::number(time_resolution));
}

void CycleDisplayer::on_Time_start_editingFinished()
{
    bool ok = false;
    double temp = ui->Time_start->text().toDouble(&ok);
    if(ok){
        if(temp>0)time_start = temp;
        else {
            time_start = 0;
            ui->Time_start->setText("0");
        }
    }
    else ui->Time_start->setText(QString::number(time_start));
    Draw();
}

void CycleDisplayer::on_Time_end_editingFinished()
{
    bool ok = false;
    double temp = ui->Time_end->text().toDouble(&ok);
    if(ok){
        if(temp<cycle_time)time_end = temp;
        else {
            time_end = cycle_time;
            ui->Time_start->setText(QString::number(time_end));
        }
    }
    else ui->Time_end->setText(QString::number(time_end));
    Draw();
}

void CycleDisplayer::clear_Draw_data(){
    for(int i=0; i<Analog_output_channel+Digital_output_channel; i++){
        Draw_data[i].clear();
    }
    Draw_time.clear();
}

void CycleDisplayer::calculate_cycle_data(){
    double max_time = 0;   //ms
    double time[Analog_output_channel+Digital_output_channel];       //ms
    int sum = 1;
    int step[10];
    for(int i=0; i<10; i++)step[i]=0;

    for(int i=0; i<variables->size();i++){
        if(variables->at(i)->get_type()==1&&variables->at(i)->get_value()[0]!=0){
            int temp_step = ((variables->at(i)->get_value()[2]-variables->at(i)->get_value()[1])/variables->at(i)->get_value()[3])+1;
            if(step[int(variables->at(i)->get_value()[0])-1]<temp_step)step[int(variables->at(i)->get_value()[0])-1] = temp_step;
        }
    }

    for(int i=0; i<10;i++){
        if(step[i]!=0){
            sum = sum * step[i];
        }
        total_number_of_scan[i] = step[i];
    }
    for(int i=0; i<Analog_output_channel+Digital_output_channel; i++)time[i]=0;
    for(int i=0; i<sequences->size();i++){
        double t_sec=0;
        if(sections_activity->at(i)->get_activity(number_of_scan)){
            for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
                double temp=0;
                for(int k=0; k<sequences->at(i)[j]->size(); k++){
                    double seq_time = sequences->at(i)[j]->at(k)->get_time(1);
                    if(seq_time>0)temp = temp + seq_time;
                }
                time[j] += temp;
                if(temp>t_sec)t_sec = temp;
            }
        }
        max_time += t_sec;
    }
    cycle_time = max_time;
}

void CycleDisplayer::on_Load_clicked()
{
    ui->Load->setDisabled(true);
    calculate_Draw_data();
    ui->Load->setEnabled(true);
    Draw();
}

void CycleDisplayer::calculate_Draw_data(){
    clear_Draw_data();
    calculate_cycle_data();
    if(ui->Switch_mode->value()==0){
        for(int i=0; i<sequences->size(); i++){
            for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
                for(int k=0; k<sequences->at(i)[j]->size(); k++){
                    sequences->at(i)[j]->at(k)->set_wrong(false);
                }
            }
        }
        const int total_size = cycle_time*1000/time_resolution;
        const int total_channel = Analog_output_channel+Digital_output_channel;
        for(int i=0; i<variables->size(); i++)variables->at(i)->Reload();
        int **transfer_data;
        qDebug() << "Create Raw Data";
        transfer_data = new int*[total_channel];
        for(int i=0; i<total_channel; i++){
            transfer_data[i] = new int[total_size+1];
            for(int j=0; j<total_size+1; j++)transfer_data[i][j]=-1;
        }
        qDebug() << "Setup Calculator";
        QScriptEngine *Sub_Engine = copyEngine(QiEngine);
        SequenceCalculator *calculator = new SequenceCalculator(Sub_Engine,sequences,variables,channel_calibration,sections_activity,time_resolution,transfer_data,
                                                                Analog_output_channel,Digital_output_channel,total_number_of_scan,number_of_scan,total_size,false);
        connect(calculator,SIGNAL(finishing(double)),indicator,SLOT(S_Calculating(double)));
        calculator->run();
        delete calculator;
        Sub_Engine->deleteLater();
        qDebug() << "Calculation complete, Total size: " << total_size;

        for(int i=0; i<total_channel; i++)Draw_data[i].append(transfer_data[i][0]);
        Draw_time.append(0);
        for(int i=1; i<total_size; i++){
            bool reduce = true;
            for(int j=0; j<total_channel; j++){
                if(transfer_data[j][i-1]!=transfer_data[j][i]){
                    reduce = false;
                    break;
                }
            }
            if(!reduce){
                for(int j=0; j<total_channel; j++)Draw_data[j].append(transfer_data[j][i]);
                Draw_time.append(double(i*time_resolution)/1000.);
            }
        }
        for(int i=0; i<total_channel; i++)Draw_data[i].append(transfer_data[i][total_size-1]);
        Draw_time.append(cycle_time);
        qDebug() << "Reduce complete, Data size: " << Draw_time.size();
        //qDebug() << Draw_time << "  " << Draw_data[Analog_output_channel];

        for(int i=0; i<total_channel; i++)delete transfer_data[i];
        delete transfer_data;
        qDebug() << "Raw data deleted";
        indicator->S_Ready();
    }
    else{
        QList<double> section_time;
        for(int i=0; i<sequences->size();i++){
            double t_sec=0;
            if(sections_activity->at(i)->get_activity(number_of_scan)){
                for(int j=0; j<Analog_output_channel+Digital_output_channel; j++){
                    double temp=0;
                    for(int k=0; k<sequences->at(i)[j]->size(); k++){
                        double seq_time = sequences->at(i)[j]->at(k)->get_time(number_of_scan);
                        if(seq_time>0)temp = temp + seq_time;
                    }
                    if(temp>t_sec)t_sec = temp;
                }
            }
            section_time.append(t_sec);
        }

        int step = ui->Display_Screen->width();
        //double resolution = (time_end-time_start)/step;
        for(int i=0; i<step; i++){
            //double time = time_start + i*resolution;
        }
    }
}

void CycleDisplayer::Draw(){
    if(ui->Switch_mode->value()==1)calculate_Draw_data();
    QPen zeroline;
    QPen dashline;
    zeroline.setWidth(1);
    zeroline.setStyle(Qt::SolidLine);
    dashline.setWidth(1);
    dashline.setStyle(Qt::DotLine);

    QPixmap pixmap = QPixmap(ui->Display_Screen->size());
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setPen(zeroline);
    painter.drawLine(0,ui->Display_Screen->height()/2,ui->Display_Screen->width(),ui->Display_Screen->height()/2);
    painter.setPen(dashline);
    for(int i=0; i<20; i++)painter.drawLine(0,i*ui->Display_Screen->height()/20,ui->Display_Screen->width(),i*ui->Display_Screen->height()/20);

    for(int i=0; i<Channel.size(); i++){
        QPen cpen;
        cpen.setWidth(2);
        cpen.setColor(Channel_color.at(i));
        painter.setPen(cpen);
        QPoint lastpoint = QPoint(-1,ui->Display_Screen->height()/2);
        for(int t=0; t<Draw_time.size();t++){
            double time = Draw_time.at(t);
            if(time>=time_start){
                painter.drawLine(lastpoint,QPoint((time-time_start)/(time_end-time_start)*ui->Display_Screen->width(),lastpoint.y()));
                lastpoint.setX((time-time_start)/(time_end-time_start)*ui->Display_Screen->width());
                if(Channel.at(i)<Analog_output_channel){
                    QPoint currentpoint = QPoint((time-time_start)/(time_end-time_start)*ui->Display_Screen->width(),
                                             int((65535.-Draw_data[Channel.at(i)].at(t))/65535.*double(ui->Display_Screen->height())));
                    painter.drawLine(lastpoint,currentpoint);
                    lastpoint = currentpoint;
                }
                else{
                    QPoint currentpoint;
                    if(Draw_data[Channel.at(i)].at(t)>0)currentpoint = QPoint((time-time_start)/(time_end-time_start)*ui->Display_Screen->width(),0.25*ui->Display_Screen->height());
                    else currentpoint = QPoint((time-time_start)/(time_end-time_start)*ui->Display_Screen->width(),0.5*ui->Display_Screen->height());
                    painter.drawLine(lastpoint,currentpoint);
                    lastpoint = currentpoint;
                }
            }
            else if(time>time_end)break;
            else{
                if(Channel.at(i)<Analog_output_channel)lastpoint.setY(int(double(65535.-Draw_data[Channel.at(i)].at(t))/65535.*double(ui->Display_Screen->height())));
                else if(Draw_data[Channel.at(i)].at(t)>0)lastpoint.setY(0.25*ui->Display_Screen->height());
                else lastpoint.setY(0.5*ui->Display_Screen->height());
            }
        }
    }
    painter.end();
    ui->Display_Screen->setPixmap(pixmap);
}

QScriptEngine *CycleDisplayer::copyEngine(QScriptEngine *eng){
    QiScriptEngineCluster *SubEngine = new QiScriptEngineCluster;
    QScriptValueIterator it(eng->globalObject());
    while(it.hasNext()){
        it.next();
        QString name = it.name();
        QScriptValue value = it.value();
        if(value.isNumber())SubEngine->globalObject().setProperty(name,SubEngine->evaluate(value.toString()));
        else if(value.isFunction()){
            QString code = value.toString();
            if(!code.contains("native"))SubEngine->globalObject().setProperty(name,SubEngine->evaluate("("+code+")"));
        }
        else if(value.isArray()){
            QString array = value.toString();
            array.remove('"');
            array.prepend('[');
            array.append(']');
            SubEngine->globalObject().setProperty(name,SubEngine->evaluate(array));
        }
    }
    return SubEngine;
}
