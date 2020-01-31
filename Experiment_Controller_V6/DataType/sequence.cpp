#include "sequence.h"
#include <QDebug>

Sequence::Sequence(QWidget *parent, bool isDiO, QList<Variable *> *varis, VariableClass *vc) :
    QWidget(parent)
{
    variables = varis;
    variableClass = vc;
    iswrong = false;
    ischanged = false;
    if(isDiO)value[0] = -1;
    else value[0]=0;
    upperbound = 10;
    lowerbound = -10;
    for(int i=0; i<4; i++){
        value[i+1]=0;
        vari[i] = NULL;
    }
}

Sequence::~Sequence(){}

Sequence::Sequence(Sequence *seq, QList<Variable *> *varis, VariableClass *vc){
    iswrong = false;
    variables = varis;
    variableClass = vc;
    double *v = seq->get_value();
    Variable **va = seq->get_variables();
    for(int i=0; i<5; i++)value[i] = v[i];
    for(int i=0; i<4; i++)vari[i] = va[i];
    upperbound = seq->upper_bound();
    lowerbound = seq->lower_bound();
}

Sequence &Sequence::operator =(Sequence &s){
    double *v = s.get_value();
    Variable **va = s.get_variables();
    for(int i=0; i<5; i++)value[i] = v[i];
    for(int i=0; i<4; i++)vari[i] = va[i];
    upperbound = s.upper_bound();
    lowerbound = s.lower_bound();
    return *this;
}

double Sequence::get_type(){
    return value[0];
}

double* Sequence::get_value(){
    return value;
}

double Sequence::get_value(int index, int number_of_scan, double seqt, double sect, double cyct){
    if(number_of_scan<1)number_of_scan=1;
    if(vari[index]==NULL)return value[index+1];
    else{
        switch(vari[index]->get_type()){
        case 0:
            return vari[index]->get_value()[0];
            break;
        case 1:
            return vari[index]->get_scan(number_of_scan);
            break;
        case 2:
            return vari[index]->get_calculated(number_of_scan,0,seqt,sect,cyct);
            break;
        case 3:
            return vari[index]->get_calibrated(value[0]);
            break;
        }
    }
    return 0;
}

bool Sequence::get_wrong(){
    return iswrong;
}

bool Sequence::get_change(){
    return ischanged;
}

double Sequence::upper_bound(){
    return upperbound;
}

double Sequence::lower_bound(){
    return lowerbound;
}

Variable** Sequence::get_variables(){
    return vari;
}

double Sequence::get_time(int number_of_scan){
    if(number_of_scan<1)number_of_scan=0;  //0 or? 1
    if(vari[0]==NULL)return value[1];
    else return vari[0]->get_output(number_of_scan,0,0,0,0);
    return 0;
}

double Sequence::get_output(int number_of_scan, double seqt, double sect, double cyct, QString cal){
    double v[4];
    for(int i=0; i<4; i++){
        if(vari[i]==NULL)v[i] = value[i+1];
        else v[i] = vari[i]->get_output(number_of_scan,0,seqt,sect,cyct);
    }

    double voltage = 0;
    double k1 = exp(v[3]/2);
    double k2 = exp(-v[3]/2);
    switch(int(value[0])){
    case -2:
        voltage = (v[2]-v[1])/v[0]*seqt+v[1];
        break;
        break;
    case -1:
        if(v[1]<=0)voltage = v[2];
        else if((seqt/v[1]-int(seqt/v[1]))<=v[2])voltage = 1;
        else voltage = 0;
        break;
    case 0:
        break;
    case 1:
        voltage = v[1];
        break;
    case 2:
        voltage = (v[2]-v[1])/v[0]*seqt+v[1];
        break;
    case 3:
        voltage = (v[1]+1)*exp(seqt*(log((v[2]+1)/(v[1]+1))/v[0]))-1;
        if(v[1]<v[2] && voltage>v[2])voltage = v[2];
        else if(v[1]>v[2] && voltage<v[2])voltage = v[2];
        break;
    case 4:
        voltage = v[1]*sin(2*3.141592654*v[2]*seqt)+v[3];
        break;
    case 5:
        if((seqt/v[2]-int(seqt/v[2]))<=v[3])voltage = v[1];
        else voltage = 0;
        break;
    case 6:
        voltage = ((v[1]*(1+k1)-v[2]*(1+k2))/(k1-k2))+(((v[1]*(1+k1)-v[2]*(k1*k1+k1))/(1-k1*k1))-(((v[1]*(1+k1))-v[2]*(1+k2))/(k1-k2)))/(1+exp(-1*(v[3]*(seqt-v[0]/2))/(v[0])));
        break;
    case 7:
        voltage = -1;
        break;
    default:
        break;
    }
    if(cal!="Not calibrated" && value[0]!=-1){
        for(int l=0; l<variables->size(); l++){
            if(variables->at(l)->get_name()==cal){
                if(variables->at(l)->get_type()==2)voltage = variables->at(l)->get_calculated(number_of_scan,voltage,seqt,sect,cyct);
                else if(variables->at(l)->get_type()==3)voltage = variables->at(l)->get_calibrated(voltage);
                break;
            }
        }
    }
    if(cal=="Inverted"){
        if(voltage==0)voltage = 1;
        else voltage = 0;
    }
    if(voltage > upperbound){
        voltage = upperbound;
        set_wrong(true);
    }
    else if(voltage < lowerbound){
        voltage = lowerbound;
        set_wrong(true);
    }
    return voltage;
}

double Sequence::get_time(QScriptEngine *engine, int number_of_scan){
    if(number_of_scan<1)number_of_scan=1;
    if(vari[0]==NULL)return value[1];
    else return vari[0]->get_output(engine,number_of_scan,0,0,0,0);
    return 0;
}

double Sequence::get_output(QScriptEngine *engine, int number_of_scan, double seqt, double sect, double cyct, QString cal){
    double v[4];
    for(int i=0; i<4; i++){
        if(vari[i]==NULL)v[i] = value[i+1];
        else v[i] = vari[i]->get_output(engine,number_of_scan,0,seqt,sect,cyct);
    }

    double voltage = 0;
    double k1 = exp(v[3]/2);
    double k2 = exp(-v[3]/2);
    switch(int(value[0])){
    case -2:
        voltage = (v[2]-v[1])/v[0]*seqt+v[1];
        break;
    case -1:
        if(v[1]<=0)voltage = v[2];
        else if((seqt/v[1]-int(seqt/v[1]))<=v[2])voltage = 1;
        else voltage = 0;
        break;
    case 0:
        break;
    case 1:
        voltage = v[1];
        break;
    case 2:
        voltage = (v[2]-v[1])/v[0]*seqt+v[1];
        break;
    case 3:
        voltage = (v[1]+1)*exp(seqt*(log((v[2]+1)/(v[1]+1))/v[0]))-1;
        if(v[1]<v[2] && voltage>v[2])voltage = v[2];
        else if(v[1]>v[2] && voltage<v[2])voltage = v[2];
        break;
    case 4:
        voltage = v[1]*sin(2*3.141592654*v[2]*seqt)+v[3];
        break;
    case 5:
        if((seqt/v[2]-int(seqt/v[2]))<=v[3])voltage = v[1];
        else voltage = 0;
        break;
    case 6:
        voltage = ((v[1]*(1+k1)-v[2]*(1+k2))/(k1-k2))+(((v[1]*(1+k1)-v[2]*(k1*k1+k1))/(1-k1*k1))-(((v[1]*(1+k1))-v[2]*(1+k2))/(k1-k2)))/(1+exp(-1*(v[3]*(seqt-v[0]/2))/(v[0])));
        break;
    case 7:
        voltage = -1;
        break;
    default:
        break;
    }
    if(cal!="Not calibrated" && value[0]!=-1){
        for(int l=0; l<variables->size(); l++){
            if(variables->at(l)->get_name()==cal){
                if(variables->at(l)->get_type()==2)voltage = variables->at(l)->get_calculated(engine,number_of_scan,voltage,seqt,sect,cyct);
                else if(variables->at(l)->get_type()==3)voltage = variables->at(l)->get_calibrated(engine,voltage);
                break;
            }
        }
    }
    if(cal=="Inverted")voltage = !voltage;
    if(voltage > upperbound){
        voltage = upperbound;
        set_wrong(true);
    }
    else if(voltage < lowerbound){
        voltage = lowerbound;
        set_wrong(true);
    }
    return voltage;
}

void Sequence::set_value(double ivalue[]){
    for(int i=0;i<5;i++)value[i]=ivalue[i];
}

void Sequence::set_value(int i, double v){
    value[i] = v;
    set_change(true);
}

void Sequence::set_vari(int i, Variable *ivari){
    vari[i] = ivari;
    if(ivari!=NULL){
        QSignalMapper *signalmap = new QSignalMapper(ivari);
        QString s = QString::number(i)+"!"+"only_system_used_null!del";
        signalmap->setMapping(ivari,s);
        connect(ivari,SIGNAL(Delete()),signalmap,SLOT(map()));
        connect(signalmap,SIGNAL(mapped(QString)),this,SLOT(set_variable(QString)));
    }
    set_change(true);
}

void Sequence::set_variable(QString va){
    QStringList v = va.split('!');
    QString s = v.at(0)+"!";
    int j = v.at(0).toInt();
    if(v.at(1)=="only_system_used_null"){
        s += vari[j]->get_name()+"!del";
        set_vari(j,NULL);
    }
    else{
        for(int i=0; i<variables->size(); i++){
            if(variables->at(i)->get_name()==v.at(1)){
                set_vari(j,variables->at(i));
                break;
            }
        }
        s += vari[j]->get_name()+"!add";
    }
    emit Variable_add(s);
}

void Sequence::set_wrong(bool w){
    iswrong = w;
    emit Wrong(iswrong);
}

void Sequence::set_change(bool c){
    ischanged = c;
    emit Changed(c);
}

void Sequence::set_boundary(QString boundary){
    QStringList list = boundary.split('!');
    lowerbound = list.at(0).toDouble();
    upperbound = list.at(1).toDouble();
}

void Sequence::set_boundary(double lower, double upper){
    lowerbound = lower;
    upperbound = upper;
}

void Sequence::Right_clicked(int index){
    if(index>-1){
        QMenu *menu = new QMenu(this);
        if(vari[index]!=NULL){
            QAction *action = new QAction(menu);
            QSignalMapper *signalmap = new QSignalMapper(menu);
            action->setText("Delete Variable");
            menu->addAction(action);
            connect(action,SIGNAL(triggered()),signalmap,SLOT(map()));
            signalmap->setMapping(action,QString::number(index)+"!only_system_used_null");
            connect(signalmap,SIGNAL(mapped(QString)),this,SLOT(set_variable(QString)));
            menu->addSeparator();
        }

        VariableClass *vc = variableClass;
        QMenu *m = menu;
        do{
            for(int j=0; j<vc->get_VariableSize(); j++){
                QAction *action = new QAction(vc->get_Variable(j)->get_name(),m);
                QSignalMapper *signalmap = new QSignalMapper(m);
                if(vc->get_Variable(j)==vari[index]){
                    action->setCheckable(true);
                    action->setChecked(true);
                }
                m->addAction(action);
                connect(action,SIGNAL(triggered()),signalmap,SLOT(map()));
                signalmap->setMapping(action,QString::number(index)+"!"+vc->get_Variable(j)->get_name());
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

        /*QMenu *submenu[4];
        submenu[0] = new QMenu("Fixed",menu);
        submenu[1] = new QMenu("Scaned",menu);
        submenu[2] = new QMenu("Calculated",menu);
        submenu[3] = new QMenu("Data File",menu);
        menu->addSection("Use Variable");
        for(int i=0; i<variables->size();i++){
            QAction *action = new QAction(menu);
            QSignalMapper *signalmap = new QSignalMapper(menu);
            action->setText(variables->at(i)->get_name());
            if(variables->at(i)==vari[index]){
                action->setCheckable(true);
                action->setChecked(true);
            }
            submenu[variables->at(i)->get_type()]->addAction(action);
            connect(action,SIGNAL(triggered()),signalmap,SLOT(map()));
            signalmap->setMapping(action,QString::number(index)+"!"+variables->at(i)->get_name());
            connect(signalmap,SIGNAL(mapped(QString)),this,SLOT(set_variable(QString)));
        }
        for(int i=0; i<4; i++){
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
        menu->addMenu(submenu[3]);*/
        menu->popup(QCursor::pos());
    }
}
