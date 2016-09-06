#ifndef UPDATECONFIG_H
#define UPDATECONFIG_H

#include <QDialog>

namespace Ui {
class updateConfig;
}

class updateConfig : public QDialog
{
  Q_OBJECT

public:
  explicit updateConfig(QWidget *parent = 0);
  ~updateConfig();
  QString configFilePath;
  QByteArray configData;

private slots:
  void on_openConfig_clicked();

  void on_updateConfigBt_clicked();

private:
  Ui::updateConfig *ui;
signals:
  void updateConfigReady();
};

#endif // UPDATECONFIG_H
