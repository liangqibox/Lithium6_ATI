#ifndef VARIABLECLASS_H
#define VARIABLECLASS_H

#include <QObject>
#include <QWidget>
#include <QTreeWidgetItem>
#include <QAction>
#include "DataType/variable.h"
#include "include/scanchangeaction.h"

class VariableClass : public QTreeWidgetItem
{

public:
    explicit VariableClass(){
        signalTrigger = new QAction(0);
        scanchangeAction = new ScanChangeAction;
        scanchangeAction->setClass((QTreeWidgetItem*)this);
        this->setText(1,"");
        scanning = 0;
    }
    ~VariableClass(){
        delete signalTrigger;
    }
    QList<Variable*> Variables(){
        return variables;
    }
    int get_VariableSize(){
        return variables.size();
    }
    int get_ColorSize(){
        return color.size();
    }
    Variable* get_Variable(int index){
        return variables.at(index);
    }
    int get_VariableColor(int index){
        return color.at(index);
    }
    void set_VariableColor(int index, int col = 0){
        color.replace(index,col);
    }
    void add_Variable(int index, Variable* vari, int col = 0){
        variables.insert(index,vari);
        //this->setText(1,QString::number(variables.size()));
        scanchangeAction->connectWith(vari);
        if(vari->get_type()==1&&vari->get_value()[0]>0)Scan_change(true);
        color.insert(index,col);
        signalTrigger->triggered();
    }
    void apend_Variable(Variable* vari, int col = 0){
        variables.append(vari);
        //this->setText(1,QString::number(variables.size()));
        if(vari->get_type()==1&&vari->get_value()[0]>0)Scan_change(true);
        scanchangeAction->connectWith(vari);
        color.append(col);
        signalTrigger->triggered();
    }
    void remove_Variable(int index){
        if(variables.isEmpty())return;
        if(variables.at(index)->get_type()==1&&variables.at(index)->get_value()[0]>0)Scan_change(false);
        scanchangeAction->disconnectWith(variables.at(index));
        if(index>=variables.size()){
            variables.removeLast();
            color.removeLast();
        }
        else{
            variables.removeAt(index);
            color.removeAt(index);
        }
        //if(!variables.isEmpty())this->setText(1,QString::number(variables.size()));
        //else this->setText(1,"");
        signalTrigger->triggered();
    }
    void remove_AllVariable(){
        variables.clear();
        color.clear();
        signalTrigger->triggered();
    }
    void move_Variable(int from, int to){
        if(from>=variables.size()||to>=variables.size())return;
        Variable *temp = variables.at(from);
        int tempcolor = color.at(from);
        variables.insert(to,temp);
        color.insert(to,tempcolor);
        if(from<to){
            variables.removeAt(from);
            color.removeAt(from);
        }
        else{
            variables.removeAt(from+1);
            color.removeAt(from+1);
        }
    }
    QAction *signalTrigger;
    ScanChangeAction *scanchangeAction;

public slots:
    void Delete_AllClass(){
        if(this->childCount()>0){
            while(this->childCount()!=0){
                VariableClass *temp = static_cast<VariableClass*>(this->child(0));
                temp->Delete_AllClass();
                this->removeChild(this->child(0));
            }
        }
        delete this;
    }

    void Scan_change(bool scan){
        if(scan){
            scanning++;
        }
        else{
            scanning--;
        }
        if(scanning<0)scanning=0;
        if(scanning>0){
            this->setText(1,QString::number(scanning));
        }
        else this->setText(1,"");
    }

private:
    QList<Variable*> variables;
    QList<int> color;
    int scanning;
};

#endif // VARIABLECLASS_H
