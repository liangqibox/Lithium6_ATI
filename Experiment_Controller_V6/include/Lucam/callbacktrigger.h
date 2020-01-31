#ifndef CALLBACKTRIGGER_H
#define CALLBACKTRIGGER_H

#include <QWidget>
#include <QImage>
#include <windows.h>

class CallbackTrigger : public QWidget
{
    Q_OBJECT

public:
    explicit CallbackTrigger(QWidget *parent = 0){this->setParent(parent);}
    ~CallbackTrigger(){}

    void Emit_Singal(){
        emit Image_Collected(image);
    }

    void Add_Image(QImage img){
        image = img;
    }

    void setHW(bool a){
        isHW = a;
    }

    bool is_HW(){
        return isHW;
    }

    HANDLE Camera;
    QList<int> image_data;
    QImage image;
    uint Height,Width;
    bool isHW;

signals:
    void Image_Collected(QImage);

};

#endif // CALLBACKTRIGGER_H
