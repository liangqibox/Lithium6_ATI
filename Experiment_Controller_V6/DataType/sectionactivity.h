#ifndef SECTIONACTIVITY_H
#define SECTIONACTIVITY_H

#include <QWidget>
#include <DataType/variable.h>
#include <definition.h>

class SectionActivity : public QWidget
{
    Q_OBJECT

public:
    explicit SectionActivity(QWidget *parent = 0);
    ~SectionActivity();

signals:
    void Changed();

public slots:
    bool get_activity(int number_of_scan);
    Variable* get_variable();
    double get_value();
    void set_variable(Variable *v);
    void set_value(double v);

private:
    Variable *vari;
    double value;
};

#endif // SECTIONACTIVITY_H
