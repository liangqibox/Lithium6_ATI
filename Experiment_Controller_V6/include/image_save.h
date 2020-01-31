#ifndef IMAGE_SAVE_H
#define IMAGE_SAVE_H

#include <QObject>
#include <QDir>
#include "include/png++/png.hpp"

class Image_Save : public QObject
{
    Q_OBJECT

public:
    explicit Image_Save(QObject *parent = 0):
    QObject(parent){
        this->setParent(parent);
    }

    ~Image_Save(){}

public slots:
    bool Add_image(QList<int> image){
        images_data.append(image);
        return true;
    }

    void Save_image_16bit(QString filename){
        if(images_data.isEmpty())return;
        int XRes = images_data.first().at(0);
        int YRes = images_data.first().at(1);
        png::image<png::gray_pixel_16> image(XRes,YRes);
        for(int y=0; y<YRes; y++){
            for(int x=0; x<XRes; x++){
                image[y][x] = png::gray_pixel_16(images_data.first().at(x+XRes*y+2));
            }
        }
        QDir dir;
        QString path = filename;
        path.truncate(path.lastIndexOf('/')+1);
        QString temp_filename = path + "System_used_temp_filename_" + QString::number(qrand()%100) + ".PNG";
        image.write(temp_filename.toLatin1().data());
        dir.rename(temp_filename,filename);
        images_data.removeFirst();
    }

    void Save_image_8bit(QString filename){
        if(images_data.isEmpty())return;
        int XRes = images_data.first().at(0);
        int YRes = images_data.first().at(1);
        png::image<png::gray_pixel> image(XRes,YRes);
        for(int y=0; y<YRes; y++){
            for(int x=0; x<XRes; x++){
                image[y][x] = png::gray_pixel(images_data.first().at(x+XRes*y+2));
            }
        }
        QDir dir;
        QString path = filename;
        path.truncate(path.lastIndexOf('/')+1);
        QString temp_filename = path + "System_used_temp_filename_" + QString::number(qrand()%100) + ".PNG";
        image.write(temp_filename.toLatin1().data());
        dir.rename(temp_filename,filename);
        images_data.removeFirst();
    }

private:
    QList<QList<int> > images_data;
};

#endif // IMAGE_SAVE_H
