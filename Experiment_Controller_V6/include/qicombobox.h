#ifndef QICOMBOBOX_H
#define QICOMBOBOX_H

#include <QWidget>
#include <QComboBox>
#include <QWheelEvent>

class QiCombobox : public QComboBox
{
    Q_OBJECT

public:
    explicit QiCombobox(QWidget *parent = 0):QComboBox(parent){}
    ~QiCombobox(){}

private:

protected:
    void wheelEvent(QWheelEvent *e){
            e->ignore();
        }
};

#endif // QICOMBOBOX_H
