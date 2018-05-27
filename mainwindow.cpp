#include "mainwindow.h"
#include "QDebug"
#include "QDesktopWidget"
#include "QSettings"
#include "QVariant"
#include "ui_mainwindow.h"
#include <QFile>       //文件类
#include <QFileDialog> //打开或保存文件对话框
#include <QMessageBox>
#include <QTextStream> //文本流输入输出
#include "crc.h"
#include "protocol/rec.h"
#include "protocol/transmit.h"
#include "QTime"
#include <QCoreApplication>
#include "QSerialPortInfo"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), mIsDownloadDone(false), mIsFileSaved(true),mLidarRawDataCounter(0),upgradeMsg(FW_NULL),
      mHexFile(new EncryptHex(this))
{
    ui->setupUi(this);

    initStatusBar();
    initEdit();
    initSerialPort();

    //添加图片转BIn
    mImageToBin = new ImageToBinDialog(this);
    //添加解码
    mParse = new CodeParse(this);
    connect(mParse, SIGNAL(Info(QString)), mMsg, SLOT(appendPlainText(QString)));
    //初始化更新配置文件窗口
    mUpdateConfig = new updateConfig(this);
    connect(mUpdateConfig, SIGNAL(updateConfigReady()), this, SLOT(burnConfig()));

    initAction();
    recoverCustom();
    mMsg->appendPlainText("Current Version:V2.7.0\nUpdate date:2018.05.27\n");

}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::closeSerialPort()
{
    if (mSerialPort->isOpen())
    {
        mSerialPort->close();
    }
    QPalette pS;
    pS.setColor(QPalette::WindowText, Qt::darkRed);
    mSerialPortStatus->setPalette(pS);

    mPollUSBStatusTimer->stop();
    mSerialPortStatus->setText(tr("Disconnected"));
}


void MainWindow::downloadConfig()
{
    if (mParse->DataToSerial.isEmpty())
    {
        mMsg->appendPlainText("Error:Please compile code first");
        return;
    }

    if (mSerialPort->isOpen() == false)
    {
        mMsg->appendPlainText("Error:Please open serial port first");
        return;
    }

    sendMassData(ACT_RE_INIT_START, mParse->DataToSerial);

    ui->actionDownload->setEnabled(false);

    ui->actionDownload->setEnabled(true);
    mIsDownloadDone = true;
}

void MainWindow::sendFlashCmd()
{
    if (!mIsDownloadDone)
    {
        mMsg->appendPlainText("Error:Please download code first");
        return;
    }

    sendCmd(ACT_FLASH_PARA);
}


void MainWindow::sendCmd(quint8 t)
{
    if (!mSerialPort->isOpen())
    {
        mMsg->appendPlainText("Error:Please open serial port first");
        return;
    }

    PackageDataStruct package;
    u32 len;
    u8 data=0;
    u8 packageBuffer[64];

    package.DataID = t;
    package.DataInBuff = (u8*)&data;
    package.DataInLen = sizeof(data);
    package.DataOutBuff = packageBuffer;
    package.DataOutLen = &len;
    Package(package);

    mSerialPort->write((char*)packageBuffer,len);
}

void MainWindow::sendCmd(quint8 id , quint8 data)
{
    if (!mSerialPort->isOpen())
    {
        mMsg->appendPlainText("Error:Please open serial port first");
        return;
    }

    PackageDataStruct package;
    u32 len;
    u8 packageBuffer[64];

    package.DataID = id;
    package.DataInBuff = (u8*)&data;
    package.DataInLen = sizeof(data);
    package.DataOutBuff = packageBuffer;
    package.DataOutLen = &len;
    Package(package);

    mSerialPort->write((char*)packageBuffer,len);
}


void MainWindow::sendMassData(ActionIDTypeDef id , const QByteArray &data)
{
    PackageDataStruct package;
    u32 len;
    u8 packageBuffer[8192];

    package.DataID = id;
    package.DataInBuff = (u8*)data.data();
    package.DataInLen = data.size();
    package.DataOutBuff = packageBuffer;
    package.DataOutLen = &len;
    Package(package);

    u32 times = len/64;

    if(len%64 == 0)
        mProgress->setMaximum(times);
    else
        mProgress->setMaximum(times+1);
    mProgress->show();
    for(u32 i=0;i<times;i++)
    {
        mSerialPort->write((char *)(packageBuffer+i*64),64);
        waitSTM32Work(10);
        mProgress->setValue(i);
    }

    waitSTM32Work(10);
    if(len%64!=0)
    {
        mProgress->setValue(times+1);
        mSerialPort->write((char *)(packageBuffer+times*64),len%64);
    }
    mProgress->hide();

}

void MainWindow::sendUpgradeData(ActionIDTypeDef id , const QByteArray &data , u32 &progress)
{
    PackageDataStruct package;
    u32 len;
    u8 packageBuffer[8192];

    package.DataID = id;
    package.DataInBuff = (u8*)data.data();
    package.DataInLen = data.size();
    package.DataOutBuff = packageBuffer;
    package.DataOutLen = &len;
    Package(package);

    u32 times = len/64;

    for(u32 i=0;i<times;i++)
    {
        mSerialPort->write((char *)(packageBuffer+i*64),64);
        waitSTM32Work(10);
        mProgress->setValue(progress++);
    }

    waitSTM32Work(10);
    if(len%64!=0)
    {
        mProgress->setValue(progress++);
        mSerialPort->write((char *)(packageBuffer+times*64),len%64);
    }
}


void MainWindow::showVersion()
{
    QMessageBox::about(this, "verison", QStringLiteral("作者: Weyne Chen"));
}


void MainWindow::contactUs()
{
    QMessageBox::information(this, QStringLiteral("联系我们"),
                             QStringLiteral("请发送邮件至\ncoolsaven@coolsaven.com\n或者访问我们的网站\nwww.coolsaven.com\n获取更多帮助"));
}


void MainWindow::trImageToBin()
{
    QString fileName = QFileDialog::getOpenFileName(
                this, "Open file", mImagePath, "Image(*.bmp *.png *.jpg *.jpeg);;All file(*.*)");

    if (fileName.isEmpty())
    {
        return;
    }
    mImagePath = fileName;
    mImageToBin->show();
    mImageToBin->showImage(fileName);
}


void MainWindow::createNewFile()
{
    if (mIsFileSaved == false)
    {
        if (QMessageBox::question(this, "save file",
                                  QStringLiteral("文件未保存\n需要保存吗？")) ==
                QMessageBox::Yes)
        {
            saveConfigFile();
        }
    }
    disconnect(mCodeEdit, SIGNAL(textChanged()), this, SLOT(enableFileSave()));
    ui->actionSaveFile->setEnabled(true);
    mSavedFilePath.clear();
    this->setWindowTitle("CS Code  --  *new");
    mCodeEdit->clear();
}


void MainWindow::burnConfig()
{
    if (!mSerialPort->isOpen())
    {
        mMsg->appendPlainText("Error:Please open serial port first");
        return;
    }

    sendMassData(ACT_RE_INIT_START, mUpdateConfig->configData);

    waitSTM32Work(500);

    sendCmd(ACT_FLASH_PARA);

}

void MainWindow::pollUSBStatus()
{
    if(mHeartbeats <= 0)
    {
        mHeartbeats = 3;
        this->closeSerialPort();
    }
    else
    {
        mHeartbeats--;
    }
}

void MainWindow::openUpgradeDialog()
{
    QString fileName = QFileDialog::getOpenFileName(
                this, "Open file", mFirmwarePath, "firmware(*.cfw *.hex)");

    if (fileName.isEmpty())
    {
        return;
    }

    mFirmwarePath = fileName;

    QTextCursor cursor = this->mCommandEdit->textCursor();
    cursor.select(QTextCursor::LineUnderCursor);
    this->mCommandEdit->setTextCursor(cursor);
    this->mCommandEdit->insertPlainText("-> ");

    this->mCommandEdit->insertPlainText("upgrade "+fileName);
}

void MainWindow::clearMsg()
{
    mMsg->clear();
}


void MainWindow::upgradeFirmware(QString str)
{

    if(mSerialPort->isOpen() == false)
    {
        mMsg->appendPlainText("Error:please open serial port first");
        return;
    }

    QString filePath = str.remove(QRegExp("upgrade +"));

    mFirmwarePath = filePath;

    if(!filePath.contains(QRegExp("\\.cfw")))
    {
        if(filePath.contains(QRegExp("\\.hex")))
        {
            qDebug()<<"hex file";
            if(mHexFile->Encrypt(filePath) == false)
            {
                mMsg->appendPlainText("Error:error occurred when parse hex file,please retry");
                return;
            }

            filePath = ENCRYPT_NAME;
        }
        else
        {
            mMsg->appendPlainText("Error:wrong firmware or path");
            return;
        }
    }

    QFile upgradeFile(filePath);
    if(upgradeFile.open(QIODevice::ReadOnly) == false)
    {
        mMsg->appendPlainText("Error:permssion denied");
        return;
    }

    sendCmd(ACT_UPGRADE_FIRMWARE);

    QTime timeout;
    timeout.start();

    while(1)
    {

        QCoreApplication::processEvents();

        if(timeout.elapsed()>10000)
        {
            mMsg->appendPlainText("Error:timeout");
            return;
        }

        if(upgradeMsg == FW_UPGRADE_READY)
        {
            upgradeMsg = FW_NULL;
            break;
        }

    }

    QList<QByteArray>firmwareData;

    for(int i=0;i<upgradeFile.size();i+=2048)
    {
        firmwareData<<upgradeFile.read(2048);
    }

    //补齐最后一个QByteArray，使其为4的整数倍
    quint8 lenMod = firmwareData.last().size() % 4;
    for(int i=0;i<lenMod;i++)
    {
        firmwareData.last().append(0xff);
    }

    qint32 offset=0;

    for(int i=0;i<firmwareData.size();i++)
    {
        if(i == 0)
            offset = 0;
        else
            offset += (firmwareData[i-1].size() - sizeof(offset));

        //在前面加上offset
        firmwareData[i].prepend((char*)&offset,sizeof(offset));
    }

    //添加结束段,偏移量为-1
    char trEnd[]={0xff,0xff,0xff,0xff,  0,0,0,0, 0,0,0,0};
    QByteArray firmwareEnd(trEnd,12);
    firmwareData.append(firmwareEnd);

    mMsg->appendPlainText("Warning:"+QStringLiteral("更新中，请不要操作软件及断电"));
    mMsg->appendPlainText("Info:upgrading...");

    u32 sectionAmount = 0;

    for(int i=0;i<firmwareData.size();i++)
    {
        PackageDataStruct package;
        u32 len;
        u8 packageBuffer[8192];

        package.DataID = ACT_UPGRADE_FIRMWARE;
        package.DataInBuff = (u8*)firmwareData[i].data();
        package.DataInLen = firmwareData[i].size();
        package.DataOutBuff = packageBuffer;
        package.DataOutLen = &len;
        Package(package);

        sectionAmount += len/64;

        if(len%64 != 0)
            sectionAmount++;
    }


    ui->actionCloseSerial->setEnabled(false);
    ui->actionBurnConfig->setEnabled(false);
    ui->actionDownload->setEnabled(false);
    ui->actionFlash->setEnabled(false);
    ui->actionOpenSerial->setEnabled(false);

    mProgress->show();
    mProgress->setMaximum(sectionAmount);
    u32 progress=0;
    bool result = true;
    bool isEnd = false;
    for(int i=0;i<firmwareData.size();i++)
    {
        sendUpgradeData(ACT_UPGRADE_FIRMWARE,firmwareData[i],progress);

        timeout.restart();
        while(!isEnd)
        {
            QCoreApplication::processEvents();

            if(timeout.elapsed()>5000)
            {
                mMsg->appendPlainText("Error:timeout");
                isEnd = true;
                result = false;
                break;
            }

            if(upgradeMsg == FW_UPGRADE_READY)
            {
                upgradeMsg = FW_NULL;
                break;
            }

            if(upgradeMsg == FW_OK)
            {
                upgradeMsg = FW_NULL;
                isEnd = true;
            }

            if(upgradeMsg == FW_FLASH_ERROR)
            {
                upgradeMsg = FW_NULL;
                isEnd = true;
                result = false;
                qDebug()<<"flash error";
            }

        }
        if(isEnd == true)
            break;

    }
    mProgress->hide();

    ui->actionCloseSerial->setEnabled(true);
    ui->actionBurnConfig->setEnabled(true);
    ui->actionDownload->setEnabled(true);
    ui->actionFlash->setEnabled(true);
    ui->actionOpenSerial->setEnabled(true);

    if(result)
    {
        mMsg->appendPlainText("Warning:"+QStringLiteral("升级完毕，请不要断电，等待系统重启"));
        mMsg->appendPlainText("Warning:"+QStringLiteral("重启后请检查版本号是否正确"));
    }
    else
    {
        mMsg->appendPlainText("Error:"+QStringLiteral("升级错误，请重启后重试"));
    }
}

void MainWindow::reboot(QString)
{
    sendCmd(ACT_REBOOT);
    waitSTM32Work(50);
}

void MainWindow::getFirmwareVersion(QString)
{
    sendCmd(ACT_GET_VERSION);
    waitSTM32Work(50);
}

void MainWindow::readSSD2828(QString str)
{
    str.remove(QRegExp("read-ssd2828\\s+"));
    bool ok;
    quint8 para = str.toInt(&ok,16);
    if(ok)
    {
        sendCmd(ACT_READ_SSD2828,para);
    }
    else
    {
        mMsg->appendPlainText("Error:Wrong parameter");
    }
}

void MainWindow::toggleLcdPower(QString str)
{
    str.remove(QRegExp("set-key\\s+"));
    enum _Key
    {
        KEY_UP = 0,
        KEY_DOWN,
        KEY_POWER,
        KEY_MTP,
        KEY_TP,
        KEY_NULL = 0xff,
    } key = KEY_NULL;

    if((str == "UP") || (str =="up"))
    {
        key = KEY_UP;
    }
    else if((str == "DOWN") || (str =="down"))
    {
        key = KEY_DOWN;
    }
    else if((str == "power") || (str =="POWER"))
    {
        key = KEY_POWER;
    }
    else if((str == "MTP") || (str =="mtp"))
    {
        key = KEY_MTP;
    }
    else if((str == "TP") || (str =="tp"))
    {
        key = KEY_TP;
    }
    else
    {
        key = KEY_NULL;
    }

    qDebug()<<(quint8)key;

    sendCmd(ACT_SET_KEY,(quint8)key);
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    restoreCustom();
}


void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
    {
        closeSerialPort();
        QPalette pS;
        pS.setColor(QPalette::WindowText, Qt::darkRed);
        mSerialPortStatus->setPalette(pS);
        mSerialPortStatus->setText(QStringLiteral("串口错误"));
    }
}


void MainWindow::openSerialPort()
{
    if (mSerialPort->isOpen())
    {
        disconnect(mSerialPort, SIGNAL(readyRead()), this, SLOT(readData()));
        mSerialPort->close();
        mPollUSBStatusTimer->stop();
    }

    SettingsDialog::Settings p = mSerialPortSettingDialog->settings();
    mSerialPort->setPortName(p.name);
    mSerialPort->setBaudRate(p.baudRate);
    mSerialPort->setDataBits(p.dataBits);
    mSerialPort->setParity(p.parity);
    mSerialPort->setStopBits(p.stopBits);
    mSerialPort->setFlowControl(p.flowControl);

    QPalette pS;
    pS.setColor(QPalette::WindowText, Qt::darkBlue);
    mSerialPortStatus->setPalette(pS);

    if (mSerialPort->open(QIODevice::ReadWrite))
    {
        mSerialPortStatus->setText(tr("Connected to %1 : %2, %3, %4, %5, %6")
                                   .arg(p.name)
                                   .arg(p.stringBaudRate)
                                   .arg(p.stringDataBits)
                                   .arg(p.stringParity)
                                   .arg(p.stringStopBits)
                                   .arg(p.stringFlowControl));

        connect(mSerialPort, SIGNAL(readyRead()), this, SLOT(readData()));

        if((p.vid == VID) && ((p.pid == PID1) || (p.pid == PID2)))
        {
            mPollUSBStatusTimer->start(200);
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), mSerialPort->errorString());

        // status->setText(tr("Open error"));
    }
}

bool MainWindow::IsDataReady(QByteArray &data)
{
    QString hexStr;
    int hexData;
    bool ok;
    bool result=false;

    //转换数据到十六进制的ASCII码形式，每个data[i]表示4个bit的16进制，data[i] | data[i+1]=hex
    data=data.toHex();
    hexStr.resize(2);

    for(int i=0;i<data.size();i+=2)
    {
        hexStr[0]=data[i];
        hexStr[1]=data[i+1];
        hexData=QString(hexStr).toInt(&ok,16);

        if(ok)
        {
            mComData += hexData;
            mLidarRawData[mLidarRawDataCounter++] = hexData;
            mRecPackage.DataInBuff = mLidarRawData;
            mRecPackage.DataInLen = mLidarRawDataCounter;
            mRecPackage.DataOutLen = &mDataLen;

            if(result == false)
                if(Unpacking(&mRecPackage) == PACK_OK)
                {
                    mLidarRawDataCounter = 0;
                    mComData.clear();
                    result = true;
                }

            if(mLidarRawDataCounter == MAX_DATA_AMOUTN_PER_FRAME)
            {
                mLidarRawDataCounter = 0;
                result = false;
                break;
            }
        }
    }

    if(mComData.size() > MAX_DATA_AMOUTN_PER_FRAME * 10)
        mComData.clear();

    return result;
}

void MainWindow::waitSTM32Work(int t)
{
    //延时一段时间，让串口数据分段发送，而不是缓冲之后一起发送
    QTime time;
    time.start();
    while(time.elapsed() < t)
        QCoreApplication::processEvents();
}

void MainWindow::initSerialPort()
{
    //添加串口设置界面
    mSerialPortSettingDialog = new SettingsDialog(this);

    //添加串口
    mSerialPort = new QSerialPort(this);
    connect(ui->actionOpenSerial, SIGNAL(triggered(bool)), mSerialPortSettingDialog,
            SLOT(show()));
    connect(mSerialPortSettingDialog, SIGNAL(comReady()), this,
            SLOT(openSerialPort()));
    connect(mSerialPort, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError)));

    mHeartbeats = 3;
    mPollUSBStatusTimer = new QTimer(this);
    connect(mPollUSBStatusTimer,SIGNAL(timeout()),this,SLOT(pollUSBStatus()));
}

void MainWindow::initEdit()
{
    //添加消息显示区
    mMsg = new msgEdit(this);
    mMsgLayout = new QVBoxLayout(ui->msg_frame);
    mMsgLayout->setMargin(0);
    mMsgLayout->addWidget(mMsg);

    //添加代码编辑区
    mCodeEdit = new CodeEditor(ui->Edit_Frame);
    mCodeLayout = new QVBoxLayout(ui->Edit_Frame);
    mCodeLayout->setMargin(0);
    mCodeLayout->addWidget(mCodeEdit);
    mCodeEdit->setFocus();
    //文本改变，使能保存按钮
    connect(mCodeEdit, SIGNAL(textChanged()), this, SLOT(enableFileSave()));


    //添加命令区
    mCommandEdit = new CommandEdit(ui->command_Frame);
    mCommandEditLayout = new QVBoxLayout(ui->command_Frame);
    mCommandEditLayout->setMargin(0);
    mCommandEditLayout->addWidget(mCommandEdit);
    mCommandList << command_type("help", &MainWindow::help)
                 << command_type("clear", &MainWindow::clearCmd)
                 << command_type("pattern", &MainWindow::setPattern)
                 << command_type("upgrade", &MainWindow::upgradeFirmware)
                 << command_type("reboot",&MainWindow::reboot)
                 << command_type("read-ssd2828",&MainWindow::readSSD2828)
                 << command_type("get-version",&MainWindow::getFirmwareVersion)
                 << command_type("set-key",&MainWindow::toggleLcdPower);

    connect(mCommandEdit, SIGNAL(command(QString)), this, SLOT(parseCommand(QString)));
    connect(mCommandEdit,SIGNAL(textChanged()),this->mCommandEdit,SLOT(setFocus()));
}

void MainWindow::initStatusBar()
{
    //添加状态栏信息
    //设置状态栏背景色
    QPalette pS;
    pS.setColor(QPalette::Window, Qt::white);
    ui->statusBar->setAutoFillBackground(true);
    ui->statusBar->setPalette(pS);

    //添加状态栏信息
    mSerialPortStatus = new QLabel(ui->statusBar);
    mSerialPortStatus->setFrameStyle(QFrame::NoFrame);
    ui->statusBar->addWidget(mSerialPortStatus);

    pS.setColor(QPalette::WindowText, Qt::darkRed);
    mSerialPortStatus->setPalette(pS);

    mProgress = new QProgressBar(ui->statusBar);
    ui->statusBar->addWidget(mProgress);
    mProgress->hide();

    mSerialPortStatus->setText(QStringLiteral("串口未设置"));
    mAuthor = new QLabel(ui->statusBar);
    mAuthor->setFrameStyle(QFrame::Sunken);
    mAuthor->setText(QStringLiteral("酷赛威科技"));
    ui->statusBar->addPermanentWidget(mAuthor);

}

void MainWindow::initAction()
{
    //添加工具栏按钮
    ui->toolBar->addAction(ui->actionNew);
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
    ui->toolBar->addAction(ui->actionClear);

    //菜单信号槽
    connect(ui->actionOpenFile, SIGNAL(triggered(bool)), this,
            SLOT(openFileWithDialog(bool)));
    connect(ui->actionSaveFile, SIGNAL(triggered(bool)), this, SLOT(saveConfigFile()));
    connect(ui->actionSaveAs, SIGNAL(triggered(bool)), this, SLOT(saveFileAs()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->actionCompile, SIGNAL(triggered(bool)), this, SLOT(parseCode()));
    connect(ui->actionCloseSerial, SIGNAL(triggered(bool)), this,
            SLOT(closeSerialPort()));
    connect(ui->actionDownload, SIGNAL(triggered(bool)), this, SLOT(downloadConfig()));
    connect(ui->actionFlash, SIGNAL(triggered(bool)), this, SLOT(sendFlashCmd()));
    connect(ui->actionVersion, SIGNAL(triggered(bool)), this,
            SLOT(showVersion()));
    connect(ui->actionContactUs, SIGNAL(triggered(bool)), this,
            SLOT(contactUs()));
    connect(ui->actionImageToBin, SIGNAL(triggered(bool)), this,
            SLOT(trImageToBin()));
    connect(ui->actionNew, SIGNAL(triggered(bool)), this, SLOT(createNewFile()));
    connect(ui->actionBurnConfig, SIGNAL(triggered(bool)), mUpdateConfig, SLOT(show()));
    connect(ui->actionUpgrade,SIGNAL(triggered(bool)),this,SLOT(openUpgradeDialog()));
    connect(ui->actionClear,SIGNAL(triggered(bool)),this,SLOT(clearMsg()));

    //刚开始禁用保存
    ui->actionSaveFile->setEnabled(false);
    //禁用手动设置等功能
    ui->actionSerialSettings->setEnabled(false);
    ui->actionFont->setEnabled(false);
    ui->actionHighlightKeyWords->setEnabled(false);

}

void MainWindow::restoreCustom()
{

    //存储布局
    QSettings settings("CsCode.ini", QSettings::IniFormat);
    settings.setValue("horizantalLayout", ui->splitter->saveState());
    settings.setValue("verticalLayout", ui->splitter_2->saveState());
    settings.setValue("windowSize", this->size());

    //存储文件路径
    settings.setValue("filePath", mSavedFilePath);
    settings.setValue("mFirmwarePath", mFirmwarePath);
    settings.setValue("imagePath", mImagePath);
    settings.setValue("configFilePath", mUpdateConfig->configFilePath);

    //存储未保存的数据
    if (mIsFileSaved == false)
    {
        if (QMessageBox::question(this, "save file",
                                  QStringLiteral("文件未保存\n需要保存吗？")) ==
                QMessageBox::Yes)
        {
            saveConfigFile();
        }
    }

}

void MainWindow::recoverCustom()
{

    //窗口默认大小为电脑屏幕大小的3/4
    const QSize availableSize = QApplication::desktop()->availableGeometry(this).size();
    QVariant windowSize(availableSize / 4 * 3);

    //恢复用户布局
    QSettings settings("CsCode.ini", QSettings::IniFormat);
    bool flag = ui->splitter->restoreState(settings.value("horizantalLayout").toByteArray());
    flag = ui->splitter_2->restoreState(settings.value("verticalLayout").toByteArray());
    this->resize(settings.value("windowSize", windowSize).toSize());
    //恢复上次打开的文件
    mUpdateConfig->configFilePath = settings.value("configFilePath").toString();
    mSavedFilePath = settings.value("filePath").toString();
    mImagePath = settings.value("imagePath").toString();
    mFirmwarePath = settings.value("mFirmwarePath").toString();
    loadFile();
}


void MainWindow::readData()
{
    QByteArray data = mSerialPort->readAll();
    char buffer[512];
    QTextCursor textCursor(mMsg->document());

    if(IsDataReady(data) == false)
        return;

    switch(mRecPackage.DataID)
    {
    case ACK_STRING:
        memset(buffer,0,sizeof(buffer));
        memcpy(buffer,mRecPackage.DataOutBuff,mDataLen);


        textCursor.movePosition(QTextCursor::End);
        mMsg->setTextCursor(textCursor);
        mMsg->insertPlainText(QString(buffer));

        break;

    case ACK_UPGRADE:
        upgradeMsg = (FirmwareUpgradeType)mRecPackage.DataOutBuff[0];
        break;

    default:
        break;
    }

    mHeartbeats = 3;
    mRecPackage.DataID = ACK_NULL;
}


void MainWindow::clearCmd(QString)
{
    mCommandEdit->clear();
    mMsg->clear();
}


void MainWindow::help(QString)
{
    mCommandEdit->appendPlainText(QStringLiteral("clear      :清除文本"));
}


void MainWindow::setPattern(QString s)
{
    if (!mSerialPort->isOpen())
    {
        mMsg->appendPlainText("Error:Please open serial port first");
        return;
    }
    bool ok = false;
    quint8 frame = 0;
    s.remove(QRegExp("\\s"));
    QRegExp rx("pattern=(.*)");
    if (s.indexOf(rx) != -1)
    {
        frame = rx.cap(1).toInt(&ok, 0);
    }
    if (ok)
    {
        const quint8 a[] = { 0xAA, 0xAA, ACT_SET_FRAME, frame, 0x55, 0x55 };
        QByteArray ba((char *)a, 6);
        mSerialPort->write(ba);
    }
}


bool MainWindow::saveToFile(const QString& fileName)
{
    QFile file(fileName);

    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::critical(this, "critical", QStringLiteral("文件禁止写入"));
        return false;
    }
    else
    {
        QTextStream out(&file);
        out << mCodeEdit->toPlainText();
        mIsFileSaved = true;
        return true;
    }
}


void MainWindow::loadFile()
{
    if (mSavedFilePath.isEmpty())
    {
        return;
    }

    QRegExp rx("[^\\n]*.csc");
    if (rx.exactMatch(mSavedFilePath) == false)
    {
        return;
    }
    this->setWindowTitle("CS Code --  " + mSavedFilePath);
    QFile TextFile(mSavedFilePath);

    if (!TextFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "Open error", "Text file open failed!");
        mSavedFilePath.clear();
        this->setWindowTitle("CS Code  --  empity workspace");
        return;
    }

    disconnect(mCodeEdit, SIGNAL(textChanged()), this, SLOT(enableFileSave()));
    QTextStream ts(&TextFile);
    mCodeEdit->setPlainText(ts.readAll());
    connect(mCodeEdit, SIGNAL(textChanged()), this, SLOT(enableFileSave()));
}


void MainWindow::openFileWithDialog(bool)
{
    if (mIsFileSaved == false)
    {
        if (QMessageBox::question(this, "save file",
                                  QStringLiteral("文件未保存\n需要保存吗？")) ==
                QMessageBox::Yes)
        {
            saveConfigFile();
        }
    }

    QString fileName =
            QFileDialog::getOpenFileName(this, "Open file", mSavedFilePath,
                                         "cool saven file(*.csc);;All file(*.*)");
    if (fileName.isEmpty())
    {
        return;
    }

    mSavedFilePath = fileName;
    this->setWindowTitle("CS Code  --  " + mSavedFilePath);

    QFile TextFile(mSavedFilePath);

    if (!TextFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "Open error", "Text file open failed!");
        mSavedFilePath.clear();
        this->setWindowTitle("CS Code  --  empity workspace");
        return;
    }

    disconnect(mCodeEdit, SIGNAL(textChanged()), this, SLOT(enableFileSave()));
    QTextStream ts(&TextFile);
    mCodeEdit->setPlainText(ts.readAll());
    connect(mCodeEdit, SIGNAL(textChanged()), this, SLOT(enableFileSave()));
}


void MainWindow::saveConfigFile()
{
    if (mSavedFilePath.isEmpty())
    {
        mSavedFilePath = QFileDialog::getSaveFileName(this, QStringLiteral("保存"),
                                                      ".", "cool saven file(*.csc)");
    }
    if (mSavedFilePath.isEmpty())
    {
        return;
    }
    else
    {
        saveToFile(mSavedFilePath);
    }

    ui->actionSaveFile->setEnabled(false);
    this->setWindowTitle("CS Code  --  " + mSavedFilePath);
}


void MainWindow::saveFileAs()
{
    QString fileName =
            QFileDialog::getSaveFileName(this, QStringLiteral("另存为"), ".",
                                         "cool saven file(*.csc);;All file(*.*)");

    if (fileName.isEmpty())
    {
        return;
    }
    else
    {
        saveToFile(fileName);
        mSavedFilePath = fileName;
        this->setWindowTitle("CS Code  --  " + mSavedFilePath);
    }
}


void MainWindow::enableFileSave()
{
    if (mSavedFilePath.isEmpty())
    {
        this->setWindowTitle("CS Code  --  empity workspace");
        return;
    }
    else
    {
        if (mCodeEdit->toPlainText().isEmpty())
        {
            ui->actionSaveFile->setEnabled(false);
            this->setWindowTitle("CS Code  --  " + mSavedFilePath);
        }
        else
        {
            ui->actionSaveFile->setEnabled(true);
            this->setWindowTitle("*CS Code --  " + mSavedFilePath);
            mIsFileSaved = false;
        }
    }
}


void MainWindow::parseCode()
{
    mParse->updateStr(mCodeEdit->toPlainText());
    if (mParse->compile() == true)
    {
        QString fileName = mSavedFilePath;
        fileName.chop(3);
        fileName += "cfg";
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly))
        {
            QMessageBox::critical(this, "critical", QStringLiteral("文件禁止写入"));
            return;
        }
        else
        {
            if(mParse->DataToSerial.isEmpty())
            {
                qDebug()<<"error parse";
            }
            else
            {
                file.write(mParse->DataToSerial);
            }
        }
    }
}


void MainWindow::parseCommand(QString str)
{
    if (!str.isEmpty())
    {
        foreach(command_type c, mCommandList)
        {
            if (str.startsWith(c.commandStr))
            {
                (this->*(c.commandFun))(str);
                return;
            }
        }

        mCommandEdit->appendPlainText("Error:invalid command-->" + str);
    }
}

