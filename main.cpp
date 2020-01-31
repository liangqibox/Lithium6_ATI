#include "maincontroller.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<iXonCamera::acquisitionParameter>("acquisitionParameter");
    qRegisterMetaType<iXonCamera::acquisitionMode>("acquisitionMode");
    qRegisterMetaType<iXonCamera::readoutParameter>("readoutParameter");
    qRegisterMetaType<iXonCamera::ROIParameter>("ROIParameter");
    qRegisterMetaType<iXonCamera::shutterParameter>("shutterParameter");

    MainController w;
    w.move(50,50);
    w.show();

    return a.exec();
}
