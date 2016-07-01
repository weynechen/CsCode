#ifndef IMAGETOBINDIALOG_H
#define IMAGETOBINDIALOG_H

#include <QDialog>

namespace Ui {
class ImageToBinDialog;
}

class plotArea:public QWidget
{
    Q_OBJECT
public:
    explicit plotArea(QWidget *parent);
protected:
    void paintEvent(QPaintEvent *);
};

class ImageToBinDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageToBinDialog(QWidget *parent);
    ~ImageToBinDialog();
    void showImage(QString path);

private:
    Ui::ImageToBinDialog *ui;
    QImage *Image;
    plotArea *plot;
protected:
    void paintEvent(QPaintEvent *);

};

#endif // IMAGETOBINDIALOG_H
