#include "imagetobindialog.h"
#include "ui_imagetobindialog.h"
#include "QPainter"
ImageToBinDialog::ImageToBinDialog(QWidget *parent = 0) :
    QDialog(parent),
    ui(new Ui::ImageToBinDialog)
{
    ui->setupUi(this);
    plot = new plotArea(ui->frame);

}

ImageToBinDialog::~ImageToBinDialog()
{
    //delete Image;
    delete ui;
}

void ImageToBinDialog::showImage(QString path)
{
    Image = new QImage(path);
    update();
}


void ImageToBinDialog::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if(!Image->isNull())
        p.drawImage(QPoint(0,0),*Image);
}

plotArea::plotArea(QWidget *parent)
{

}

void plotArea::paintEvent(QPaintEvent *)
{

}
