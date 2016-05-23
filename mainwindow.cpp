#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>      //打开或保存文件对话框
#include <QFile>            //文件类
#include <QTextStream>      //文本流输入输出
#include<QMessageBox>
#include"QDebug"
#include "QSettings"
#include "QDesktopWidget"
#include "QVariant"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //添加状态栏信息
    //设置状态栏背景色
    QPalette pS;
    pS.setColor(QPalette::Window,Qt::white);
    ui->statusBar->setAutoFillBackground(true);
    ui->statusBar->setPalette(pS);

    //添加状态栏信息
    status = new QLabel(ui->statusBar);
    status->setFrameStyle(QFrame::NoFrame);
    ui->statusBar->addWidget(status);

    pS.setColor(QPalette::WindowText,Qt::darkRed);
    status->setPalette(pS);

    status->setText(QStringLiteral("串口未设置"));
    author = new QLabel(ui->statusBar);
    author->setFrameStyle(QFrame::Sunken);
    author->setText(QStringLiteral("酷赛威科技"));
    ui->statusBar->addPermanentWidget(author);


    //添加串口设置界面
    serialSettingDialog = new SettingsDialog(this);
    //添加串口
    serial = new QSerialPort(this);
    connect(ui->actionOpenSerial,SIGNAL(triggered(bool)),serialSettingDialog,SLOT(show()));
    connect(serialSettingDialog,SIGNAL(comReady()),this,SLOT(openSerialPort()));
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
        SLOT(handleError(QSerialPort::SerialPortError)));


    //添加消息显示区
    msg= new msgEdit(this);
    msgLayout= new QVBoxLayout(ui->msg_frame);
    msgLayout->setMargin(0);
    msgLayout->addWidget(msg);

    //添加代码编辑区
    codeEdit = new CodeEditor(ui->Edit_Frame);
    codeLayout = new QVBoxLayout(ui->Edit_Frame);
    codeLayout->setMargin(0);
    codeLayout->addWidget(codeEdit);
    codeEdit->setFocus();

    //添加解码
    parse = new codeParse(this);
    connect(parse,SIGNAL(Info(QString)),msg,SLOT(appendPlainText(QString)));

    //添加命令区
    command = new commandEdit(ui->command_Frame);
    commandLayout = new QVBoxLayout(ui->command_Frame);
    commandLayout->setMargin(0);
    commandLayout->addWidget(command);
    commandList<<command_type("help",&MainWindow::help)
          <<command_type("clear",&MainWindow::clearCmd);

    connect(command,SIGNAL(command(QString)),this,SLOT(parseCommand(QString)));


    //添加工具栏按钮
    ui->toolBar->addAction(ui->actionOpenFile);
    ui->toolBar->addAction(ui->actionSaveFile);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionOpenSerial);
    ui->toolBar->addAction(ui->actionCloseSerial);
    ui->toolBar->addAction(ui->actionSerialSettings);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionCompile);
    ui->toolBar->addAction(ui->actionDownload);
    ui->toolBar->addAction(ui->actionFlash);

    //菜单信号槽
    connect(ui->actionOpenFile,SIGNAL(triggered(bool)),this,SLOT(fileOpen()));
    connect(ui->actionSaveFile,SIGNAL(triggered(bool)),this,SLOT(fileSave()));
    connect(ui->actionSaveAs,SIGNAL(triggered(bool)),this,SLOT(fileSaveAs()));
    connect(ui->actionExit,SIGNAL(triggered(bool)),this,SLOT(close()));
    connect(ui->actionCompile,SIGNAL(triggered(bool)),this,SLOT(parseCode()));
    connect(ui->actionCloseSerial,SIGNAL(triggered(bool)),this,SLOT(closeSerialPort()));
    connect(ui->actionDownload,SIGNAL(triggered(bool)),this,SLOT(download()));
    connect(ui->actionFlash,SIGNAL(triggered(bool)),this,SLOT(flash()));

    //文本改变，使能保存按钮
    connect(codeEdit,SIGNAL(textChanged()),this,SLOT(enableFileSave()));
    //刚开始禁用保存
    ui->actionSaveFile->setEnabled(false);
    //禁用手动设置等功能
    ui->actionSerialSettings->setEnabled(false);
    ui->actionFont->setEnabled(false);
    ui->actionHighlightKeyWords->setEnabled(false);

    //设置窗口默认大小为电脑屏幕大小的3/4
    const QSize availableSize=QApplication::desktop()->availableGeometry(this).size();
    QVariant var(availableSize/4*3);

    //恢复用户布局
    QSettings settings("appLayout.ini",QSettings::IniFormat);
    bool flag= ui->splitter->restoreState(settings.value("horizantalLayout").toByteArray());
    //qDebug()<<flag;
    flag= ui->splitter_2->restoreState(settings.value("verticalLayout").toByteArray());
    // qDebug()<<flag;

    this->resize(settings.value("windowSize",var).toSize());
    //qDebug()<<flag;

}

MainWindow::~MainWindow()
{

    QSettings settings("appLayout.ini",QSettings::IniFormat);
    settings.setValue("horizantalLayout",ui->splitter->saveState());
    settings.setValue("verticalLayout",ui->splitter_2->saveState());
    settings.setValue("windowSize",this->size());
    delete ui;
}



void MainWindow::closeSerialPort()
{
    if (serial->isOpen())
        serial->close();
    QPalette pS;
    pS.setColor(QPalette::WindowText,Qt::darkRed);
    status->setPalette(pS);

    status->setText(tr("Disconnected"));
}

void MainWindow::download()
{
    if(parse->dataToSerial.isEmpty())
    {
        msg->appendPlainText("Error:Please compile code first");
        return;
    }
    if (serial->isOpen())
    {
      serial->write(parse->dataToSerial);
      isDownloadDone=true;
    }
    else
    {
        msg->appendPlainText("Error:Please open serial port first");
        return;
    }

}

void MainWindow::flash()
{
    if (!serial->isOpen())
    {
        msg->appendPlainText("Error:Please open serial port first");
        return;
    }

    if(!isDownloadDone)
    {
        msg->appendPlainText("Error:Please download code first");
        return;
    }

    const quint8 a[]={0xAA,0xAA,TO_FLASH,0x55,0x55};
    QByteArray ba((char*)a);
    serial->write(ba);
}


void MainWindow::handleError(QSerialPort::SerialPortError error)
{


    if (error == QSerialPort::ResourceError) {
                closeSerialPort();
        QPalette pS;
        pS.setColor(QPalette::WindowText,Qt::darkRed);
        status->setPalette(pS);
        status->setText(QStringLiteral("串口错误"));
    }

}


void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = serialSettingDialog->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    QPalette pS;
    pS.setColor(QPalette::WindowText,Qt::darkBlue);
    status->setPalette(pS);

    if (serial->open(QIODevice::ReadWrite)) {
        status->setText(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        //status->setText(tr("Open error"));
    }
}

void MainWindow::readData()
{
    QTextCursor textCursor(msg->document());
    textCursor.movePosition(QTextCursor::End);
    msg->setTextCursor(textCursor);
    msg->insertPlainText(serial->readAll());
}


void MainWindow::clearCmd(QString)
{
    command->clear();
    msg->clear();
}

void MainWindow::help(QString)
{
    command->appendPlainText(
                QStringLiteral("clear      :清除文本")
                );
}


bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly|QFile::Text))
    {
        QMessageBox::critical(this,
                              "critical",
                              "cannot write file"
                              );
        return false;
    }
    else
    {
        QTextStream out(&file);
        out<<codeEdit->toPlainText();
       // setCurrentFile(fileName);
        return true;
    }
}





void MainWindow::fileOpen()
{
    QString fileName=QFileDialog::getOpenFileName(
                this,
                "Open file",
                ".",
                "cool saven file(*.csc);;All file(*.*)"
                );
    if(fileName.isEmpty())
    {
        return;
    }

    savedFilePath=fileName;

    QFile TextFile(fileName);

    if(!TextFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,"Open error","Text file open failed!");
        return;
    }

    QTextStream ts(&TextFile);
    codeEdit->setPlainText(ts.readAll());
}

void MainWindow::fileSave()
{
    if(savedFilePath.isEmpty())
        savedFilePath=QFileDialog::getSaveFileName(this,
                                                      QStringLiteral("保存"),
                                                       ".",
                                                      "cool saven file(*.csc)"
                                                      );
    if(savedFilePath.isEmpty())
    {
        return;
    }
    else
    {
        saveFile(savedFilePath);
    }

    ui->actionSaveFile->setEnabled(false);
    this->setWindowTitle("CS Code  --  "+savedFilePath);
}

void MainWindow::fileSaveAs()
{
    QString fileName=QFileDialog::getSaveFileName(
                this,
                QStringLiteral("另存为"),
                ".",
                "cool saven file(*.csc);;All file(*.*)"
                );
    if(fileName.isEmpty())
    {
        return;
    }
    else
    {
        saveFile(fileName);
    }
}

void MainWindow::enableFileSave()
{
    if(savedFilePath.isEmpty())
    {
        this->setWindowTitle("CS Code  --  empity workspace");
        return;
    }
    else
    {

        if(codeEdit->toPlainText().isEmpty())
        {
            ui->actionSaveFile->setEnabled(false);
            this->setWindowTitle("CS Code  --  "+savedFilePath);
        }
        else
        {
            ui->actionSaveFile->setEnabled(true);
            this->setWindowTitle("*CS Code --  "+savedFilePath);

        }
    }
}

void MainWindow::parseCode()
{
    parse->updateStr(codeEdit->toPlainText());
    parse->compile();
}

void MainWindow::parseCommand(QString str)
{
    if(!str.isEmpty())
    {
        foreach(command_type c,commandList)
        {
            if(str.startsWith(c.commandStr))
            {
                (this->*(c.commandFun))(str);
                return;
            }

        }

    command->appendPlainText("Error:invaild command");
    }
}




