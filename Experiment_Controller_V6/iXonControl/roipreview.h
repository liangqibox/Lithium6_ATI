#ifndef ROIPREVIEW_H
#define ROIPREVIEW_H

#include <QDialog>
#include <QPixmap>
#include <QImage>
#include <QDir>
#include <QFileDialog>
#include <QPoint>
#include <algorithm>

#include "previewlabel.h"

namespace Ui {
class ROIPreview;
}

class ROIPreview : public QDialog
{
    Q_OBJECT

public:
    explicit ROIPreview(QWidget *parent = 0);

    void setROI(int xS, int xE, int yS, int yE);
    ~ROIPreview();

private:
    Ui::ROIPreview *ui;
    QString imagePath;
    int xStart, xEnd, yStart, yEnd;

public slots:
    void showROI(QPoint origin, QPoint mousePos);
    void fillTextBoxes(int xS, int xE, int yS, int yE);


private slots:
    void on_pB_load_clicked();
    void on_pB_apply_clicked();
    void on_pB_cancel_clicked();

    void loadImage(QString filename);


    void on_lE_ROI1_editingFinished();

    void on_lE_ROI2_editingFinished();

    void on_lE_ROI3_editingFinished();

    void on_lE_ROI4_editingFinished();

    void on_rB_1024_clicked(bool checked);

    void on_rB_512_clicked(bool checked);

    void on_rB_256_clicked(bool checked);

    void on_rB_128_clicked(bool checked);

    void on_rB_64_clicked(bool checked);

    void on_rB_32_clicked(bool checked);

signals:
    void sendImgPos(int xStart, int xEnd, int yStart, int yEnd);

};

#endif // ROIPREVIEW_H
