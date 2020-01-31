#include "sectionactivity.h"

SectionActivity::SectionActivity(QWidget *parent) :
    QWidget(parent)
{
    vari = NULL;
    value = 1;
}

SectionActivity::~SectionActivity()
{
}

bool SectionActivity::get_activity(int number_of_scan){
    double act = 0;
    if(vari!=NULL){
        switch(vari->get_type()){
        case 0:
            act = vari->get_value()[0];
            break;
        case 1:
            act = vari->get_scan(number_of_scan);
            break;
        case 2:
            act = vari->get_calculated(number_of_scan,0,0,0,0);
            break;
        default:
            break;
        }
    }
    else act = value;

    if(act>0)return true;
    else return false;
}

Variable* SectionActivity::get_variable(){
    return vari;
}

double SectionActivity::get_value(){
    return value;
}

void SectionActivity::set_variable(Variable *v){
    vari = v;
    emit Changed();
}

void SectionActivity::set_value(double v){
    value = v;
    emit Changed();
}
