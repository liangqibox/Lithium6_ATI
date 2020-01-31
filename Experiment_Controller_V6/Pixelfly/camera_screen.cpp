#include "camera_screen.h"
#include "ui_camera_screen.h"

Camera_Screen::Camera_Screen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Camera_Screen)
{
    ui->setupUi(this);
}

Camera_Screen::~Camera_Screen()
{
    //emit Preview_Close();
    delete ui;
}

void Camera_Screen::Update(QImage image){
    ui->Screen->setPixmap((QPixmap::fromImage(image)).scaled(ui->Screen->size(),Qt::KeepAspectRatio));
}

void Camera_Screen::resizeEvent(QResizeEvent *event){
    ui->Screen->setGeometry(10,10,this->width()-20,this->height()-20);
}
