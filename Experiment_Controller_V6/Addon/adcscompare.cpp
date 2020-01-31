#include "adcscompare.h"
#include "ui_adcscompare.h"

AdcsCompare::AdcsCompare(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdcsCompare)
{
    ui->setupUi(this);
}

AdcsCompare::~AdcsCompare()
{
    delete ui;
}

void AdcsCompare::on_Load_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,"Load Setting",0,tr("Setting File (*.adcs)"));
    if(!filename.isNull())Load_File(filename);
}

void AdcsCompare::Load_File(QString filename){
    ui->Display->clear();

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))return;
    ui->Profile->setText(filename);
    QTextStream in(&file);
    QStringList newsetting;
    QString temp;
    temp = in.readLine().remove(0,6);
    if(temp == "T9") newsetting.append("0");
    else if(temp == "T10") newsetting.append("1");
    else if(temp == "T11") newsetting.append("2");
    else if(temp == "T12") newsetting.append("3");
    temp = in.readLine().remove("Analog_output_channel:");
    newsetting.append(QString::number(temp.toInt()/8));
    temp = in.readLine().remove("Digital_output_channel:");
    newsetting.append(QString::number(temp.toInt()/32));
    newsetting.append("online");

    if(Analog_output_channel != 8*newsetting.at(1).toInt())
        ui->Display->append("Analog output channel: Current "+QString::number(Analog_output_channel)+"  File "+QString::number(8*newsetting.at(1).toInt()));
    if(Digital_output_channel != 32*newsetting.at(2).toInt())
        ui->Display->append("Digital output channel: Current "+QString::number(Digital_output_channel)+"  File "+QString::number(8*newsetting.at(2).toInt()));

    while(!in.atEnd()){
        QString temp = in.readLine();
        if(temp=="cycle_time:"){
            QString n = in.readLine();
            double lcycle_time = n.toDouble();
            if(lcycle_time!=cycle_time)ui->Display->append("Cycle Time: Current "+QString::number(cycle_time)+"  File "+QString::number(lcycle_time));
        }
        else if(temp=="total_time:"){
            QString n = in.readLine();
        }
        else if(temp=="time_resolution:"){
            QString n = in.readLine();
            double ltime_resolution = n.toDouble();
            if(ltime_resolution!=time_resolution)ui->Display->append("Cycle Time: Current "+QString::number(time_resolution)+"  File "+QString::number(ltime_resolution));
        }
        else if(temp=="Auto_saving"){
            QString n = in.readLine();
            n = in.readLine();
            n = in.readLine();
            n = in.readLine();
        }
        else if(temp=="AO Channels:"){
            QString name = in.readLine();
            int counter = 0;
            while(name!="End of AO Channel"){
                if(counter < Analog_output_channel){
                    QString cal = in.readLine();
                    int col = in.readLine().toInt();
                    if(name!=channel_name->at(counter))
                        ui->Display->append("AO"+QString::number(counter+1)+" Name: Current "+channel_name->at(counter)+"  File "+name);
                    if(cal!=channel_calibration->at(counter))
                        ui->Display->append("AO"+QString::number(counter+1)+" Calibration: Current "+channel_calibration->at(counter)+"  File "+cal);
                    if(col!=channel_color->at(counter))
                        ui->Display->append("AO"+QString::number(counter+1)+" Color: Current "+channel_color->at(counter)+"  File "+col);
                    name = in.readLine();
                    counter++;
                }
            }
        }
        else if(temp=="DiO Channels:"){
            int counter = Analog_output_channel;
            QString name = in.readLine();
            while(name!="End of DiO Channel"){
                if(counter < Digital_output_channel){
                    QString col = in.readLine();
                    if(name!=channel_name->at(counter))
                        ui->Display->append("DiO"+QString::number(counter+1-Analog_output_channel)+" Name: Current "+channel_name->at(counter)+"  File "+name);
                    if(col=="Normal" || col=="Inverted"){
                        if(col!=channel_calibration->at(counter))
                            ui->Display->append("DiO"+QString::number(counter+1-Analog_output_channel)+" Calibration: Current "+channel_calibration->at(counter)+"  File "+col);
                        col = in.readLine();
                    }
                    if(col.toInt()!=channel_color->at(counter))
                        ui->Display->append("DiO"+QString::number(counter+1-Analog_output_channel)+" Color: Current "+channel_color->at(counter)+"  File "+col);
                    name = in.readLine();
                    counter++;
                }
            }
        }
        else if(temp=="Variables:"){
            int variCount = 0;
            QString name = in.readLine();
            while(name!="End of Variables"){
                int exist = -1;
                bool same = true;
                int type =in.readLine().toInt();
                QString f;
                for(int i=0; i<variables->size(); i++){
                    if(name==variables->at(i)->get_name()){
                        exist = i;
                        break;
                    }
                }
                if(exist>-1)if(type!=variables->at(exist)->get_type())same = false;
                for(int i=0; i<4; i++){
                    double v = in.readLine().toDouble();
                    if(exist>-1)if(type<2&&v!=variables->at(exist)->get_value()[i])same = false;
                }
                f = in.readLine();
                if(exist>-1)if(f!=variables->at(exist)->get_filename())same = false;
                f = in.readLine();
                if(exist>-1)if(f!=variables->at(exist)->get_fomular())same = false;
                f = in.readLine();
                if(exist>-1)if(type==3&&f.toInt()!=variables->at(exist)->get_fitting())same = false;
                if(!exist)ui->Display->append("Variable "+name+" does not exist in Current profile");
                else if(!same)ui->Display->append("Variable "+name+" different.");
                name = in.readLine();
                variCount++;
            }
            if(variCount!=variables->size())ui->Display->append("Variable Count/n Current:"+QString::number(variables->size())+"   File:"+QString::number(variCount));
        }
        else if(temp=="VariableClass:"){
            //ifvariableclass = true;
            QString vc_in = in.readLine();
            while(vc_in!="End of VariableClass"){
                /*VariableClass* vc = variableClass;
                QStringList classPosition_string = vc_in.split(':');
                QList<int> classPosition;
                for(int i=0; i<classPosition_string.size(); i++)classPosition.append(classPosition_string.at(i).toInt());
                QStringList vc_variable = in.readLine().split(':');
                for(int i=1; i<classPosition.size(); i++){
                    while(classPosition.at(i)>(vc->childCount()-1)){
                        VariableClass* newClass = new VariableClass;
                        vc->addChild((QTreeWidgetItem*)newClass);
                    }
                    vc = (VariableClass*)vc->child(classPosition.at(i));
                }
                vc->setText(0,vc_variable.at(0));
                for(int i=1; i<vc_variable.size(); i++){
                    QStringList v = vc_variable.at(i).split('!');
                    QString name = v.first();
                    int color = 0;
                    if(v.size()>1)color = v.at(1).toInt();
                    for(int j=0; j<variables->size(); j++){
                        if(name==variables->at(j)->get_name()){
                            vc->apend_Variable(variables->at(j),color);
                            break;
                        }
                    }
                }*/
                vc_in = in.readLine();
            }
        }
        else if(temp=="DisplayClass"){
            QString dc = in.readLine();
            while(dc!="End of DisplayClass"){

            }
        }
        else if(temp=="Section Array:"){
            QString name = in.readLine();
            while(name!="End of Section Array"){
                //section_array->append(name);
                name = in.readLine();
            }
        }
        else if(temp=="Section Activity:"){
            QString act = in.readLine();
            int counter = 0;
            while(act!="End of Section Activity"){
                /*SectionActivity *secact = new SectionActivity;
                bool ok = false;
                double temp = act.toDouble(&ok);
                if(ok)secact->set_value(temp);
                else{
                    for(int i=0; i<variables->size(); i++){
                        if(act==variables->at(i)->get_name()){
                            secact->set_variable(variables->at(i));
                        }
                    }
                }
                sections_activity->append(secact);*/
                bool same = true;
                bool ok = false;
                double temp = act.toDouble(&ok);
                if(sections_activity->at(counter)->get_variable()==NULL&&ok)
                    if(sections_activity->at(counter)->get_value()!=temp)same = false;
                else if(!ok&&sections_activity->at(counter)->get_variable()->get_name()!=act)same = false;
                else if(ok&&sections_activity->at(counter)->get_variable()!=NULL)same = false;
                if(!same)ui->Display->append("Section Active "+QString::number(counter+1)+" different.");
                act = in.readLine();
                counter++;
            }
        }
        else if(temp=="Sequences AOut:"){
            /*QString ao = in.readLine();
            int ii=-1;
            int channel = 0;
            while(ao!="End of AO Sequences"){
                if(ao=="AO1:"){
                    QList<Sequence*> **sec = new QList<Sequence*>*[Analog_output_channel+Digital_output_channel];
                    for(int i=0; i<Analog_output_channel+Digital_output_channel; i++)sec[i]=new QList<Sequence*>;
                    sequences->append(sec);
                    ii++;
                }
                if(ao.contains(':')){
                    ao.remove("AO");
                    ao.remove(':');
                    channel = ao.toInt()-1;
                    ao = in.readLine();
                }
                else{
                    if(channel < Analog_output_channel){
                        Sequence *temp = new Sequence(this,false,variables,variableClass);
                        sequences->at(ii)[channel] -> append(temp);
                        connect(temp,SIGNAL(Changed(bool)),this,SLOT(cycle_changed()));
                        temp->set_boundary(ao);
                        double v[5];
                        v[0]=in.readLine().toDouble();
                        for(int i=1; i<5; i++)v[i]=in.readLine().toDouble();
                        temp->set_value(v);
                        for(int i=0; i<4; i++){
                            QString name = in.readLine();
                            if(name!="only_system_used_null"){
                                for(int j=0; j<variables->size(); j++){
                                    if(name==variables->at(j)->get_name())temp->set_vari(i,variables->at(j));
                                }
                            }
                        }
                    }
                    ao = in.readLine();
                }
            }*/
        }
        else if(temp=="Sequences DiO:"){
            /*QString di = in.readLine();
            int ii = -1;
            int channel = 0;
            while(di!="End of DiO Sequences"){
                if(di=="DiO1:")ii++;
                if(di.contains(':')){
                    di.remove("DiO");
                    di.remove(':');
                    channel = di.toInt()-1;
                    di = in.readLine();
                }
                else{
                    if(channel<Digital_output_channel){
                        Sequence *temp = new Sequence(this,true,variables,variableClass);
                        sequences->at(ii)[channel+Analog_output_channel] -> append(temp);
                        connect(temp,SIGNAL(Changed(bool)),this,SLOT(cycle_changed()));
                        temp->set_value(1,di.toDouble());
                        for(int i=2; i<4; i++){
                            double v = in.readLine().toDouble();
                            temp->set_value(i,v);
                        }
                        for(int i=0; i<3; i++){
                            QString name = in.readLine();
                            if(name!="only_system_used_null"){
                                for(int j=0; j<variables->size(); j++){
                                    if(name==variables->at(j)->get_name())temp->set_vari(i,variables->at(j));
                                }
                            }
                        }
                    }
                    di = in.readLine();
                }
            }*/
        }
        else if(temp=="Sections:"){
            /*QString ao = in.readLine();
            int ii=-1;
            int channel = 0;
            while(ao!="End of Sections"){
               if(ao.at(0)==':'){
                    QList<Sequence*> **sec = new QList<Sequence*>*[Analog_output_channel+Digital_output_channel];
                    for(int i=0; i<Analog_output_channel+Digital_output_channel; i++)sec[i]=new QList<Sequence*>;
                    sections->append(sec);
                    ao.remove(':');
                    sections_name->append(ao);
                    ii++;
                    ao = in.readLine();
                }
                else if(ao.contains("AO")){
                    ao.remove("AO");
                    ao.remove(':');
                    channel = ao.toInt()-1;
                    ao = in.readLine();
                }
                else if(ao.contains("DiO")){
                    ao.remove("DiO");
                    ao.remove(':');
                    channel = ao.toInt()-1+50+Analog_output_channel;
                    ao = in.readLine();
                }
                else if(channel<Analog_output_channel){
                    Sequence *temp = new Sequence(this,false,variables,variableClass);
                    sections->at(ii)[channel] -> append(temp);
                    double v[5];
                    v[0]=ao.toDouble();
                    for(int i=1; i<5; i++)v[i]=in.readLine().toDouble();
                    temp->set_value(v);
                    for(int i=0; i<4; i++){
                        QString name = in.readLine();
                        if(name!="only_system_used_null"){
                            for(int j=0; j<variables->size(); j++){
                                if(name==variables->at(j)->get_name())temp->set_vari(i,variables->at(j));
                            }
                        }
                    }
                    ao = in.readLine();
                }
                else if(channel>49 && (channel-50-Analog_output_channel)<Digital_output_channel){
                    int chl = channel - 50;
                    Sequence *temp = new Sequence(this,true,variables,variableClass);
                    sections->at(ii)[chl] -> append(temp);
                    double v[5];
                    v[1]=ao.toDouble();
                    for(int i=2; i<4; i++)v[i]=in.readLine().toDouble();
                    v[0] = -1;
                    v[4] = 0;
                    temp->set_value(v);
                    for(int i=0; i<3; i++){
                        QString name = in.readLine();
                        if(name!="only_system_used_null"){
                            for(int j=0; j<variables->size(); j++){
                                if(name==variables->at(j)->get_name())temp->set_vari(i,variables->at(j));
                            }
                        }
                    }
                    ao = in.readLine();
                }
                else ao = in.readLine();
            }*/
        }
        else if(temp=="Transfer Files"){
            /*External_File_Transfer->clear();
            QString tra = in.readLine();
            while(tra!="End of Transfer Files"){
                External_File_Transfer->append(tra.split('|'));
                tra = in.readLine();
            }*/
        }
    }
    //file.close();
    //for(int i=0; i<variables->size(); i++)variables->at(i)->Reload();
    /*static_cast<External*>(Widget_Addon.at(FILETRANSFER))->Reset(variables);
    if(!ifvariableclass){
        for(int i=0; i<variables->size(); i++)variableClass->apend_Variable(variables->at(i));
    }
    if(sections_activity->size() < sequences->size()){
        int temp = sections_activity->size();
        for(int i=0; i<(sequences->size()-temp); i++){
            SectionActivity *secact = new SectionActivity;
            secact->set_value(1);
            sections_activity->append(secact);
        }
    }*/
}
