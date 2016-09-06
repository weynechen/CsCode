#ifndef IMAGETOBINDIALOG_H
#define IMAGETOBINDIALOG_H

#include <QDialog>
#include "QVBoxLayout"
namespace Ui {
class ImageToBinDialog;
}

class plotArea : public QWidget
{
  Q_OBJECT
public:
  explicit plotArea(QWidget *parent);
  QImage *Image;

protected:
  void paintEvent(QPaintEvent *);
  void wheelEvent(QWheelEvent *event);

private:
  double mRangZoom;
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
  plotArea *plot;
  QVBoxLayout *plotLayout;
  QString imagePath;

protected:
  void resizeEvent(QResizeEvent *event);

private slots:
  void on_pushButton_clicked();
};

#endif // IMAGETOBINDIALOG_H
