#ifndef PREVIEWLABEL_H
#define PREVIEWLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QPoint>
#include <QToolTip>
#include <QRect>
#include <QRubberBand>
#include <algorithm>

#include <QDebug>

#define IMGSCALE 256


class PreviewLabel : public QLabel
{
    Q_OBJECT

    void mouseMoveEvent(QMouseEvent *event)
    {
        int x, y;
        x = event->pos().x() * 4;
        y = (IMGSCALE-1 - event->pos().y()) * 4;
        if(x == 0) x = 1;
        if(y == 0) y = 1;
        QPoint scaledPos(std::max(1, std::min(x, 1024)), std::max(1, std::min(y, 1024)));
        QPoint scaledOrigin(origin.x() * 4, (IMGSCALE-1 - origin.y()) * 4);
        if(mousePressed)
        {
            emit mousePosChanged(scaledOrigin, scaledPos);
            band->setGeometry(QRect(origin, event->pos()).normalized());
        }
        return;
    }

    void mousePressEvent(QMouseEvent *event)
    {
        origin = event->pos();
        band->setGeometry(QRect(origin, QSize()));
        band->show();
        mousePressed = true;
    }
    void mouseReleaseEvent(QMouseEvent *event)
    {
        mousePressed = false;
    }

public:
    explicit PreviewLabel(QWidget *parent = 0): QLabel(parent)
    {
        band = new QRubberBand(QRubberBand::Rectangle, this);
    }
    ~PreviewLabel()
    {
        delete band;
    }

public slots:
    void setBand(int xS, int xE, int yS, int yE)
    {
        QPoint topleft(xS, 1024+1 - yS); // y is flipped
        QPoint bottomright(xE, 1024+1 - yE);

        band->setGeometry(QRect(topleft / 4, bottomright / 4).normalized());
        band->show();
        return;
    }

private:
    QRubberBand *band;
    QPoint origin;

    bool mousePressed;

signals:
    void mousePosChanged(QPoint origin, QPoint pos);
};


#endif // PREVIEWLABEL_H
