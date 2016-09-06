#include "updateconfig.h"
#include "ui_updateconfig.h"
#include "QFileDialog"
#include "QMessageBox"

updateConfig::updateConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::updateConfig)
{
    ui->setupUi(this);
}


updateConfig::~updateConfig()
{
    delete ui;
}


void updateConfig::on_openConfig_clicked()
{
    QString fileName =
            QFileDialog::getOpenFileName(this, "Open file", configFilePath,
                                         "config file(*.cfg)");

    if (fileName.isEmpty())
    {
        return;
    }

    configFilePath = fileName;
    ui->lineEdit->setText(fileName);
}


void updateConfig::on_updateConfigBt_clicked()
{
    QFile file(ui->lineEdit->text());

    if (!file.open(QFile::ReadOnly))
    {
        QMessageBox::critical(this, "critical", QStringLiteral("文件无读取权限"));
        return;
    }
    else
    {
        configData.clear();
        configData = file.readAll();
        emit updateConfigReady();
    }
}
