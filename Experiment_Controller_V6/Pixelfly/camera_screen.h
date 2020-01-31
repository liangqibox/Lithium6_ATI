#ifndef CAMERA_SCREEN_H
#define CAMERA_SCREEN_H

#include <QDialog>
#include <QPixmap>

namespace Ui {
class Camera_Screen;
}

class Camera_Screen : public QDialog
{
    Q_OBJECT

public:
    explicit Camera_Screen(QWidget *parent = 0);
    ~Camera_Screen();

signals:
    void Preview_Close();

public slots:
    void Update(QImage image);

protected:
    void resizeEvent(QResizeEvent *);

private:
    Ui::Camera_Screen *ui;
};

#endif // CAMERA_SCREEN_H
