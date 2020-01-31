#include "variable.h"
#include <QDebug>

QString numbers = "0123456789.-";

Variable::Variable(QWidget *parent, QString iname, int itype, QScriptEngine *eng, QList<Variable *> *vari) :
    QWidget(parent)
{
    name = iname;
    type = itype;
    variables = vari;
    fitting_mode = 0;
    Engine = eng;
    for(int i=0;i<4;i++)value[i]=0;
}

Variable::~Variable()
{
    emit Delete();
}

Variable &Variable::operator =(Variable &v){
    fomular = v.get_fomular();
    if(!fomular.isEmpty())translated_fomular = translate_fomular(fomular);
    filename = v.get_filename();
    if(!filename.isEmpty())File_load();
    for(int i=0; i<4; i++)value[i] = v.get_value()[i];
    return *this;
}

void Variable::set_type(int t){
    if(type==1&&value[0]>0.5&&t!=1){
        emit Scan(false);
    }
    else if(t==1&&type!=1&&value[0]>0.5){
        emit Scan(true);
    }
    type = t;
    emit Changed();
}

bool Variable::set_name(QString n){
    if(n.isEmpty())return false;
    bool used = false;
    for(int i=0; i<variables->size(); i++){
        if(variables->at(i)->get_name()==n){
            used = true;
            break;
        }
    }
    if(used)return false;
    else{
        name = n;
        emit name_changed(name);
        Reload();
        return true;
    }
    return false;
}

void Variable::set_value(int i, double v){
    if(i==0&&v>0.5&&type==1&&value[0]<0.5){
        emit Scan(true);
    }
    else if(i==0&&v<0.5&&type==1&&value[0]>0.5){
        emit Scan(false);
    }
    value[i] = v;
    if(type==0 && i==0){
        Engine->globalObject().setProperty(name,Engine->evaluate(QString::number(value[0])));
    }
    else if(type==1){
        QString v1 = QString::number(value[0]);
        QString v2 = QString::number(value[1]);
        QString v3 = QString::number(value[2]);
        QString v4 = QString::number(value[3]);
        QString f = "(function(current){"
                    "if(current==-1)return v1;"
                    "if(current==-2)return v2;"
                    "if(current==-3)return v3;"
                    "if(current==-4)return v4;"
                    "if(v1==0)return v2;"
                    "var v = v2+v4*Current_Step(current,v1);"
                    "if(v4>0&&v>v3)v = v3;"
                    "else if(v4<0&&v<v3)v = v3;"
                    "return v;})";
        f.replace("v1",v1);
        f.replace("v2",v2);
        f.replace("v3",v3);
        f.replace("v4",v4);
        V = Engine->evaluate(f);
        Engine->globalObject().setProperty(name,V);
    }
    emit Changed();
}

void Variable::set_file(QString f){
    filename = f;
    File_load();
    emit Changed();
}

void Variable::set_fomular(QString f){
    fomular = f;
    translated_fomular = translate_fomular(fomular);

    QString function = "(function(current,x,seqt,sect,cyct){return " + translated_fomular + ";})";
    if(!Engine->canEvaluate(function)){
        QMessageBox msgBox;
        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Invaild Formula.");
        msgBox.exec();
    }
    else{
        V = Engine->evaluate(function);
        Engine->globalObject().setProperty(name,V);
    }
    emit Changed();
}

QList<Variable *> *Variable::get_variables(){
    return variables;
}

int Variable::get_type(){
    return type;
}

double* Variable::get_value(){
    return value;
}

int Variable::get_fitting(){
    return fitting_mode;
}

QString Variable::get_name(){
    return name;
}

QString Variable::get_filename(){
    return filename;
}

QString Variable::get_fomular(){
    return fomular;
}

QString Variable::get_translated_fomular(){
    return translated_fomular;
}

double Variable::get_scan(int current){
    QScriptValueList arg;
    arg << current;
    return V.call(QScriptValue(),arg).toNumber();
}

double Variable::get_calculated(int current, double x, double seqt, double sect, double cyct){
    QScriptValueList args;
    args << current << x << seqt << sect << cyct;
    return V.call(QScriptValue(),args).toNumber();
}

double Variable::get_calibrated(double a){
    if(fitting_mode==0){
        if(file_data[0].isEmpty())return 0;
        else if(a<file_data[0].at(0)) return (file_data[1].at(1)-file_data[1].at(0))/(file_data[0].at(1)-file_data[0].at(0)) * (a-file_data[0].at(0)) + file_data[1].at(0);
        int ii=0;
        for(int i=0; i<file_data[0].size()-1; i++){
            if(file_data[0].at(i)<=a && a<=file_data[0].at(i+1)){
                ii = i;
                break;
            }
        }
        return (file_data[1].at(ii+1)-file_data[1].at(ii))/(file_data[0].at(ii+1)-file_data[0].at(ii)) * (a-file_data[0].at(ii)) + file_data[1].at(ii);
    }
    else if(fitting_mode==1){
        QScriptValueList arg;
        arg << a;
        return V.call(QScriptValue(),arg).toNumber();
    }
    else if(fitting_mode==1){
        return V.call().toNumber();
    }
    return 0;
}

double Variable::get_scan(QScriptEngine *engine, int current){
    QScriptValueList arg;
    arg << current;
    return engine->globalObject().property(name).call(QScriptValue(),arg).toNumber();
}

double Variable::get_calculated(QScriptEngine *engine, int current,double x,double seqt,double sect,double cyct){
    QScriptValueList args;
    args << current << x << seqt << sect << cyct;
    return engine->globalObject().property(name).call(QScriptValue(),args).toNumber();
}

double Variable::get_calibrated(QScriptEngine *engine, double a){
    if(fitting_mode==0){
        if(file_data[0].isEmpty())return 0;
        else if(a<file_data[0].at(0)) return (file_data[1].at(1)-file_data[1].at(0))/(file_data[0].at(1)-file_data[0].at(0)) * (a-file_data[0].at(0)) + file_data[1].at(0);
        int ii=0;
        for(int i=0; i<file_data[0].size()-1; i++){
            if(file_data[0].at(i)<=a && a<=file_data[0].at(i+1)){
                ii = i;
                break;
            }
        }
        return (file_data[1].at(ii+1)-file_data[1].at(ii))/(file_data[0].at(ii+1)-file_data[0].at(ii)) * (a-file_data[0].at(ii)) + file_data[1].at(ii);
    }
    else if(fitting_mode==1){
        QScriptValueList arg;
        arg << a;
        return engine->globalObject().property(name).call(QScriptValue(),arg).toNumber();
    }
    else if(fitting_mode==2){
        return engine->globalObject().property(name).call().toNumber();
    }
    return 0;
}

double Variable::get_output(int current, double x, double seqt, double sect, double cyct){
    double output = 0;
    switch (type) {
    case 0:
        output = value[0];
        break;
    case 1:
        output = get_scan(current);
        break;
    case 2:
        output = get_calculated(current,x,seqt,sect,cyct);
        break;
    case 3:
        output = get_calibrated(x);
        break;
    default:
        break;
    }
    return output;
}

double Variable::get_output(QScriptEngine *engine, int current, double x, double seqt, double sect, double cyct){
    double output = 0;
    switch (type) {
    case 0:
        output = value[0];
        break;
    case 1:
        output = get_scan(engine,current);
        break;
    case 2:
        output = get_calculated(engine,current,x,seqt,sect,cyct);
        break;
    case 3:
        output = get_calibrated(engine,x);
        break;
    default:
        break;
    }
    return output;
}

void Variable::File_load()
{
    if(type!=3)return;
    file_data[0].clear();
    file_data[1].clear();
    QFile read(filename);
    if(!read.open(QIODevice::ReadOnly | QIODevice::Text)){
        read.close();
        Engine->globalObject().setProperty(name,Engine->evaluate("(function(x){return 0;}"));
        return;
    }
    QTextStream in(&read);
    if(fitting_mode!=2){
        while (!in.atEnd()){
            QString temp = in.readLine();
            for(int i=0; i<temp.size(); i++){
                bool isNumber = false;
                for(int j=0 ; j<numbers.size(); j++){
                    if(temp.at(i)==numbers.at(j)){
                        isNumber = true;
                        break;
                    }
                }
                if(!isNumber)temp.replace(i,1,'@');
            }
            QStringList temp2 = temp.split('@',QString::SkipEmptyParts,Qt::CaseInsensitive);
            if(temp2.size()>=2){
                file_data[0].append(temp2.at(0).toDouble());
                file_data[1].append(temp2.at(1).toDouble());
            }
        }
    }
    else if(fitting_mode==2){
        QString script = in.readAll();
        V = Engine->evaluate(script);
        Engine->globalObject().setProperty(name,V);
    }
    if(fitting_mode==1){
        tk::spline cal;
        std::vector<double> X (file_data[0].size());
        std::vector<double> Y (file_data[1].size());
        for (int i=0; i<file_data[0].size(); i++){
            X.at(i)=file_data[0].at(i);
            Y.at(i)=file_data[1].at(i);
        }
        cal.set_points(X,Y,true);
        QString M[5];
        QString size = QString::number(cal.m_x.size());
        for(uint i=0; i<cal.m_a.size();i++){
            M[0].append(QString::number(cal.m_x[i])+',');
            M[1].append(QString::number(cal.m_y[i])+',');
            M[2].append(QString::number(cal.m_a[i])+',');
            M[3].append(QString::number(cal.m_b[i])+',');
            M[4].append(QString::number(cal.m_c[i])+',');
        }
        for(int i=0; i<5; i++){
            M[i].chop(1);
            M[i].prepend('[');
            M[i].append(']');
        }
        QString f = "(function(x){"
                    "var m_x = "+M[0]+";"
                    "var m_y = "+M[1]+";"
                    "var m_a = "+M[2]+";"
                    "var m_b = "+M[3]+";"
                    "var m_c = "+M[4]+";"
                    "var n = "+size+";"
                    "var idx = 0;"
                    "for(var i=0;i<n;i++){if(m_x[i]>x){if(i>0)idx = i-1;break;}}"
                    "var h=x-m_x[idx];"
                    "var interpol;"
                    "if(x<m_x[0]){interpol=("+QString::number(cal.m_b0)+"*h+"+QString::number(cal.m_c0)+")*h+m_y[0];}"
                    "else if(x>m_x[n-1]){interpol=(m_b[n-1]*h+m_c[n-1])*h+m_y[n-1];}"
                    "else{interpol=((m_a[idx]*h+m_b[idx])*h+m_c[idx])*h+m_y[idx];}"
                    "return interpol;})";
        V = Engine->evaluate(f);
        Engine->globalObject().setProperty(name,V);
    }
    else if(fitting_mode==0){
        QString F[2];
        for(int i=0; i<file_data[0].size();i++){
            F[0].append(QString::number(file_data[0].at(i))+',');
            F[1].append(QString::number(file_data[1].at(i))+',');
        }
        for(int i=0; i<2; i++){
            F[i].chop(1);
            F[i].prepend('[');
            F[i].append(']');
        }
        QString f = "(function(x){"
                    "var F0 = "+F[0]+";"
                    "var F1 = "+F[1]+";"
                    "if(x<F0[0])return (F1[1]-F1[0])/(F0[1]-F0[0])*(x-F0[0])+F1[0];"
                    "var ii=0;"
                    "for(var i=0;i<"+QString::number(file_data[0].size()-1)+"; i++){"
                    "ii = i;if(F0[i]<=x && x<=F0[i+1]){break;}}"
                    "return (F1[ii+1]-F1[ii])/(F0[ii+1]-F0[ii])*(x-F0[ii])+F1[ii];})";
        V = Engine->evaluate(f);
        Engine->globalObject().setProperty(name,V);
    }
    read.close();
}

QString Variable::translate_fomular(QString raw){
    while(raw.contains("^")){
        QString x,y;
        int mid = raw.indexOf('^');
        int braket = 0;
        for(int i=mid-1; i>-1; i--){
            QChar temp = raw.at(i);
            if((temp=='+'||temp=='-'||temp=='*'||temp=='/'||temp=='(')&&braket==0)break;
            if(temp==')')braket++;
            else if(temp=='(')braket--;
            x.prepend(temp);
        }
        for(int i=mid+1; i<raw.size(); i++){
            QChar temp = raw.at(i);
            if((temp=='+'||temp=='-'||temp=='*'||temp=='/'||temp==')')&&braket==0)break;
            if(temp=='(')braket++;
            else if(temp==')')braket--;
            y.append(temp);
        }
        raw.replace(x+"^"+y,"Math.pow("+x+","+y+")");
    }

    for(int i=0; i<variables->size(); i++){
        QString name = variables->at(i)->get_name();
        if(raw.contains("["+name+"]")){
            switch (variables->at(i)->get_type()) {
            case 0:
                raw.replace("["+name+"]",name);
                break;
            case 1:
                raw.replace("["+name+"]",name+"(current)");
                break;
            case 2:
                raw.replace("["+name+"]",name+"(current,x,seqt,sect,cyct)");
                break;
            case 3:
                if(variables->at(i)->get_fitting()==2)raw.replace("["+name+"]",name+"(current,x,seqt,sect,cyct)");
                else raw.replace("["+name+"]",name);
                break;
            default:
                break;
            }
        }
    }
    return raw;
}

void Variable::change_fitting_mode(int i){
    fitting_mode = i;
    File_load();
}

void Variable::Reload(){
    switch(type){
    case 0:
        set_value(0,value[0]);
        break;
    case 1:
        set_value(1,value[1]);
        break;
    case 2:
        set_fomular(fomular);
        break;
    case 3:
        File_load();
        break;
    }
}
