#ifndef SCANCHANGEACTION_H
#define SCANCHANGEACTION_H

#include <QObject>
#include <QTreeWidgetItem>
#include <DataType/variable.h>

class ScanChangeAction : public QObject
{
    Q_OBJECT

public:
    ScanChangeAction(){
    }
    ~ScanChangeAction(){
    }
    void setClass(QTreeWidgetItem *vc){
        variableClass = vc;
    }
    void connectWith(Variable *vari){
        connect(vari,SIGNAL(Scan(bool)),this,SLOT(ScanTriggered(bool)));
    }
    void disconnectWith(Variable *vari){
        disconnect(vari,0,this,0);
    }

private slots:
    void ScanTriggered(bool scan){
        bool ok = false;
        int i = variableClass->text(1).toInt(&ok);
        if(!ok)i = 0;
        if(scan){
            i++;
            variableClass->setText(1,QString::number(i));
        }
        else{
            i--;
            if(i<0)i=0;
            if(i==0)variableClass->setText(1,"");
            else variableClass->setText(1,QString::number(i));
        }
    }

private:
    QTreeWidgetItem *variableClass;
};

#endif // SCANCHANGEACTION_H
