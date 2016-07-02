#include "imagetobindialog.h"
#include "ui_imagetobindialog.h"
#include "QPainter"
#include "QDesktopWidget"
#include "QWheelEvent"
#include "QDebug"
#include "QFileDialog"
#include "QFile"
#include "QMessageBox"

ImageToBinDialog::ImageToBinDialog(QWidget *parent = 0) :
    QDialog(parent),
    ui(new Ui::ImageToBinDialog)
{
    ui->setupUi(this);
    plotLayout = new QVBoxLayout(ui->frame);
    plot = new plotArea(ui->frame);
    plotLayout->addWidget(plot);
    plotLayout->setMargin(0);

}

ImageToBinDialog::~ImageToBinDialog()
{
    //delete Image;
    delete ui;
}

void ImageToBinDialog::showImage(QString path)
{
    plot->Image = new QImage(path);
    imagePath= path;
    //计算出最佳显示区域
    const QSize availableSize = QApplication::desktop()->availableGeometry(this).size();
    int picHeight =plot->Image->size().height();
    int picWidth  = plot->Image->size().width() ;

    while(picHeight>availableSize.height()*9/10)
    {
        picHeight = picHeight*9/10;
        picWidth = picWidth*9/10;
    }
    while(picWidth>availableSize.width()*9/10)
    {
        picWidth = picWidth*9/10;
        picWidth = picWidth*9/10;
    }

    this->resize(picWidth+115,picHeight+33);
    ui->plainTextEdit->appendPlainText(QStringLiteral("图像显示有缩放\n不影响分辨率"));
    plot->update();
    QString height = QStringLiteral("图像高：")  + QString("%1").arg(plot->Image->size().height());
    QString width = QStringLiteral("图像宽：")  + QString("%1").arg(plot->Image->size().width());
    ui->plainTextEdit->appendPlainText(height);
    ui->plainTextEdit->appendPlainText(width);
}

void ImageToBinDialog::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
   // qDebug()<<this->size()<<ui->frame->size();
    update();
}


plotArea::plotArea(QWidget *parent = 0) :
    QWidget(parent)
{
    mRangZoom=1;
}

void plotArea::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if(!Image->isNull())
    {
        if(mRangZoom == 1)
        {
            p.scale((double)(this->width())/(double)(Image->width()),(double)(this->height())/(double)(Image->height()));
        }
        else
            p.scale(mRangZoom,mRangZoom);

        p.drawImage(QPoint(0,0),*Image);
    }
}

void plotArea::wheelEvent(QWheelEvent *event)
{
    double wheelSteps = event->delta()/120.0; // a single step delta is +/-120 usually

    if(wheelSteps==1)
        mRangZoom=mRangZoom*1.25;
    if((wheelSteps==-1)&&(mRangZoom>0.25))
        mRangZoom=mRangZoom*0.8;

    update();
}

void ImageToBinDialog::on_pushButton_clicked()
{
    QString imageDir = imagePath.remove(QRegExp(".[a~z]*"));
    QString fileName=QFileDialog::getSaveFileName(this,
                                                  QStringLiteral("保存BIN"),
                                                  imageDir,
                                                  "BIN(*.BIN)"
                                                  );
    if(fileName.isEmpty())
    {
        ui->plainTextEdit->appendPlainText(QStringLiteral("未指定文件名"));
        return;
    }

    QFile binFile(fileName);

    ui->plainTextEdit->appendPlainText(QStringLiteral("生成格式为RGB888\n生成文件大小为:"));
    ui->plainTextEdit->appendPlainText(QString("%1 Byte").arg(3*plot->Image->size().height() * plot->Image->size().width()));
   // plot->Image->colorTable();


    if(!binFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this,
                              QStringLiteral("写入错误"),
                              QStringLiteral("无法写入文件\r\n无权限")
                              );
    }
    else
    {
        int imageH=plot->Image->size().height();
        int imageW=plot->Image->size().width();
        int cnt =0;
        for(int y=0;y<imageH;y++)
            for(int x=0;x<imageW;x++)
            {
                QRgb rgb= plot->Image->pixel(x,y);
                char data[3];
                data[0]=(char)(rgb>>16);
                data[1]=(char)(rgb>>8);
                data[2]=(char)rgb;
                binFile.write(data,3);
                                cnt++;
                                ui->progressBar->setValue(cnt*100/(imageW*imageH));

            }
            binFile.close();

    }
}
