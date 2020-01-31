#include "roipreview.h"
#include "ui_roipreview.h"

ROIPreview::ROIPreview(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ROIPreview)
{
    ui->setupUi(this);
    this->setWindowTitle("ROI Preview");
    connect(ui->lb_preview, SIGNAL(mousePosChanged(QPoint,QPoint)), this, SLOT(showROI(QPoint,QPoint)));

    imagePath = QDir::currentPath() + "/" + "black_screen.PNG";
    loadImage(imagePath);

    ui->pB_apply->setFocus();
}

ROIPreview::~ROIPreview()
{
    delete ui;
}

void ROIPreview::setROI(int xS, int xE, int yS, int yE)
{
    xStart = xS;
    xEnd = xE;
    yStart = yS;
    yEnd= yE;

    fillTextBoxes(xStart, xEnd, yStart, yEnd);
    ui->lb_preview->setBand(xStart, xEnd, yStart, yEnd);

    return;
}

void ROIPreview::showROI(QPoint origin, QPoint mousePos)
{
    xStart = std::min(origin.x(), mousePos.x());
    xEnd = std::max(origin.x(), mousePos.x());
    yStart = std::min(origin.y(), mousePos.y());
    yEnd = std::max(origin.y(), mousePos.y());

    fillTextBoxes(xStart, xEnd, yStart, yEnd);
}

void ROIPreview::fillTextBoxes(int xS, int xE, int yS, int yE)
{
    ui->lE_ROI1->setText(QString::number(xS));
    ui->lE_ROI2->setText(QString::number(xE));
    ui->lE_ROI3->setText(QString::number(yS));
    ui->lE_ROI4->setText(QString::number(yE));
    return;
}

void ROIPreview::loadImage(QString filename)
{
    QImage img;
    img.load(filename);
    if(img.isNull()) return;

    img = img.scaled(IMGSCALE, IMGSCALE);
    img = img.mirrored(true, true); //this is needed since sensor coordinates are not the same as the output coordinates. It's not optimal, but it works.
    ui->lb_preview->setPixmap(QPixmap::fromImage(img));

    return;
}

void ROIPreview::on_pB_load_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Choose Image", QDir::currentPath(), "Images(*.PNG)");
    if(!filename.isNull()) loadImage(filename);
}

void ROIPreview::on_pB_apply_clicked()
{
    emit sendImgPos(xStart, xEnd, yStart, yEnd);
    this->close();
}

void ROIPreview::on_pB_cancel_clicked()
{
    this->close();
}

void ROIPreview::on_lE_ROI1_editingFinished()
{
    bool ok = false;
    int temp = ui->lE_ROI1->text().toInt(&ok);
    if(ok) xStart = temp;
    setROI(xStart, xEnd, yStart, yEnd);
    ui->rB_custom->setChecked(true);

    return;
}

void ROIPreview::on_lE_ROI2_editingFinished()
{
    bool ok = false;
    int temp = ui->lE_ROI2->text().toInt(&ok);
    if(ok) xEnd = temp;
    setROI(xStart, xEnd, yStart, yEnd);
    ui->rB_custom->setChecked(true);

    return;
}

void ROIPreview::on_lE_ROI3_editingFinished()
{
    bool ok = false;
    int temp = ui->lE_ROI3->text().toInt(&ok);
    if(ok) yStart = temp;
    setROI(xStart, xEnd, yStart, yEnd);
    ui->rB_custom->setChecked(true);

    return;
}

void ROIPreview::on_lE_ROI4_editingFinished()
{
    bool ok = false;
    int temp = ui->lE_ROI4->text().toInt(&ok);
    if(ok) yEnd = temp;
    setROI(xStart, xEnd, yStart, yEnd);
    ui->rB_custom->setChecked(true);

    return;
}

//THESE ARE THE RECOMMENDED CROP POSITIONS, WHICH ARE NOT 100% CENTRAL, IF COMPLETE CENTRALIZATION IS REQUIRED, NEED TO REWRITE

void ROIPreview::on_rB_1024_clicked(bool checked)
{
    if(checked)
    {
        setROI(1, 1024, 1, 1024);
    }
}

void ROIPreview::on_rB_512_clicked(bool checked)
{
    if(checked)
    {
        setROI(241, 752, 256, 767);
    }
}

void ROIPreview::on_rB_256_clicked(bool checked)
{
    if(checked)
    {
        setROI(369, 624, 384, 639);
    }
}

void ROIPreview::on_rB_128_clicked(bool checked)
{
    if(checked)
    {
        setROI(433, 560, 448, 575);
    }
}

void ROIPreview::on_rB_64_clicked(bool checked)
{
    if(checked)
    {
        setROI(476, 539, 480, 543);
    }
}

void ROIPreview::on_rB_32_clicked(bool checked)
{
    if(checked)
    {
        setROI(487, 518, 496, 527);
    }
}
