#ifndef CONTROLLERDATAGROUP
#define CONTROLLERDATAGROUP

#include <QObject>
#include <include/variableclass.h>
#include <DataType/sectionactivity.h>
#include <DataType/sequence.h>
#include <DataType/variable.h>

class ControllerDataGroup : public QObject
{
    Q_OBJECT

public:
    explicit ControllerDataGroup(QObject *parent = 0):QObject(parent){
        this->setParent(parent);
    }

    ~ControllerDataGroup(){}

    QList<QList<Sequence*>**> *sequences;
    QList<Variable*> *variables;
    VariableClass *variableClass;
    QList<VariableClass*> *DisplayClass;

    QList<SectionActivity*> *sections_activity;
    QList<QString> *sections_name;
    QList<QString> *section_array;
    QList<QList<Sequence*>**> *sections;

    QList<QString> *channel_name;
    QList<int> *channel_color;
    QList<QString> *channel_calibration;

    QList<QStringList> *External_File_Transfer;
    QList<QString> Analog_Input_Setting;

    QScriptEngine *QiEngine;
};


#endif // CONTROLLERDATAGROUP

