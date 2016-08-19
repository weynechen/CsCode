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


MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), ui(new Ui::MainWindow),isDownloadDone(false),isFileSaved(true) {
    ui->setupUi(this);

    //添加状态栏信息
    //设置状态栏背景色
    QPalette pS;
    pS.setColor(QPalette::Window, Qt::white);
    ui->statusBar->setAutoFillBackground(true);
    ui->statusBar->setPalette(pS);

    //添加状态栏信息
    status = new QLabel(ui->statusBar);
    status->setFrameStyle(QFrame::NoFrame);
    ui->statusBar->addWidget(status);

    pS.setColor(QPalette::WindowText, Qt::darkRed);
    status->setPalette(pS);

    status->setText(QStringLiteral("串口未设置"));
    author = new QLabel(ui->statusBar);
    author->setFrameStyle(QFrame::Sunken);
    author->setText(QStringLiteral("酷赛威科技"));
    ui->statusBar->addPermanentWidget(author);
    //添加图片转BIn
    imageToBin = new ImageToBinDialog(this);

    //添加串口设置界面
    serialSettingDialog = new SettingsDialog(this);

    //添加消息显示区
    msg = new msgEdit(this);
    msgLayout = new QVBoxLayout(ui->msg_frame);
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
    connect(parse, SIGNAL(Info(QString)), msg, SLOT(appendPlainText(QString)));

    //添加命令区
    command = new commandEdit(ui->command_Frame);
    commandLayout = new QVBoxLayout(ui->command_Frame);
    commandLayout->setMargin(0);
    commandLayout->addWidget(command);
    commandList << command_type("help", &MainWindow::help)
        << command_type("clear", &MainWindow::clearCmd)
        << command_type("pattern", &MainWindow::setPattern);

    connect(command, SIGNAL(command(QString)), this, SLOT(parseCommand(QString)));

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

    //添加串口
    serial = new QSerialPort(this);
    connect(ui->actionOpenSerial, SIGNAL(triggered(bool)), serialSettingDialog,
        SLOT(show()));
    connect(serialSettingDialog, SIGNAL(comReady()), this,
        SLOT(openSerialPort()));
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
        SLOT(handleError(QSerialPort::SerialPortError)));

    //初始化更新配置文件窗口
    mUpdateConfig = new updateConfig(this);

    //菜单信号槽
    connect(ui->actionOpenFile, SIGNAL(triggered(bool)), this,
        SLOT(fileOpenWithDialog(bool)));
    connect(ui->actionSaveFile, SIGNAL(triggered(bool)), this, SLOT(fileSave()));
    connect(ui->actionSaveAs, SIGNAL(triggered(bool)), this, SLOT(fileSaveAs()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->actionCompile, SIGNAL(triggered(bool)), this, SLOT(parseCode()));
    connect(ui->actionCloseSerial, SIGNAL(triggered(bool)), this,
        SLOT(closeSerialPort()));
    connect(ui->actionDownload, SIGNAL(triggered(bool)), this, SLOT(download()));
    connect(ui->actionFlash, SIGNAL(triggered(bool)), this, SLOT(flash()));
    connect(ui->actionVersion, SIGNAL(triggered(bool)), this,
        SLOT(showVersion()));
    connect(ui->actionContactUs, SIGNAL(triggered(bool)), this,
        SLOT(contactUs()));
    connect(ui->actionImageToBin, SIGNAL(triggered(bool)), this,
        SLOT(ImageToBin()));
    connect(ui->actionNew, SIGNAL(triggered(bool)), this, SLOT(fileNew()));
    //文本改变，使能保存按钮
    connect(codeEdit, SIGNAL(textChanged()), this, SLOT(enableFileSave()));
    connect(ui->actionBurnConfig, SIGNAL(triggered(bool)), mUpdateConfig, SLOT(show()));
    connect(mUpdateConfig,SIGNAL(updateConfigReady()),this,SLOT(burnConfig()));

    //刚开始禁用保存
    ui->actionSaveFile->setEnabled(false);
    //禁用手动设置等功能
    ui->actionSerialSettings->setEnabled(false);
    ui->actionFont->setEnabled(false);
    ui->actionHighlightKeyWords->setEnabled(false);


    //设置窗口默认大小为电脑屏幕大小的3/4
    const QSize availableSize = QApplication::desktop()->availableGeometry(this).size();
    QVariant var(availableSize / 4 * 3);

    //恢复用户布局
    QSettings settings("CsCode.ini", QSettings::IniFormat);
    bool flag = ui->splitter->restoreState(settings.value("horizantalLayout").toByteArray());
    flag = ui->splitter_2->restoreState(settings.value("verticalLayout").toByteArray());
    this->resize(settings.value("windowSize", var).toSize());
    //恢复上次打开的文件
    mUpdateConfig->configFilePath = settings.value("configFilePath").toString();
    savedFilePath = settings.value("filePath").toString();
    imagePath = settings.value("imagePath").toString();
    loadFile();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::closeSerialPort() {
    if (serial->isOpen())
        serial->close();
    QPalette pS;
    pS.setColor(QPalette::WindowText, Qt::darkRed);
    status->setPalette(pS);

    status->setText(tr("Disconnected"));
}

void MainWindow::download() {
    if (parse->dataToSerial.isEmpty()) {
        msg->appendPlainText("Error:Please compile code first");
        return;
    }
    if (serial->isOpen()) {
        serial->write(parse->dataToSerial);
        isDownloadDone = true;
    }
    else {
        msg->appendPlainText("Error:Please open serial port first");
        return;
    }
}

void MainWindow::flash() {
    if (!serial->isOpen()) {
        msg->appendPlainText("Error:Please open serial port first");
        return;
    }

    if (!isDownloadDone) {
        msg->appendPlainText("Error:Please download code first");
        return;
    }

    const quint8 a[] = { 0xAA, 0xAA, FLASH_PARA, 0x55, 0x55 };
    QByteArray ba((char *)a);
    serial->write(ba);
}

void MainWindow::showVersion() {
    QMessageBox::about(this, "verison", QStringLiteral("当前版本\nV1.2.0"));
}

void MainWindow::contactUs() {
    QMessageBox::information(this, QStringLiteral("联系我们"),
        QStringLiteral("请发送邮件至\ncoolsaven@coolsaven.com\n或者访问我们的网站\nwww.coolsaven.com\n获取更多帮助"));
}

void MainWindow::ImageToBin() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "Open file", imagePath, "Image(*.bmp *.png *.jpg *.jpeg);;All file(*.*)");
    if (fileName.isEmpty()) {
        return;
    }
    imagePath = fileName;
    imageToBin->show();
    imageToBin->showImage(fileName);
}

void MainWindow::fileNew() {
    if (isFileSaved == false) {
        if (QMessageBox::question(this, "save file",
            QStringLiteral("文件未保存\n需要保存吗？")) ==
            QMessageBox::Yes) {
            fileSave();
        }
    }
    disconnect(codeEdit, SIGNAL(textChanged()), this, SLOT(enableFileSave()));
    ui->actionSaveFile->setEnabled(true);
    savedFilePath.clear();
    this->setWindowTitle("CS Code  --  *new");
    codeEdit->clear();
}

void MainWindow::burnConfig()
{

    if (!serial->isOpen()) {
        msg->appendPlainText("Error:Please open serial port first");
        return;
    }

    char a[] = {0xAA,0xAA,FLASH_CONFIG_FILE,0x55,0x55};
    QByteArray data(a,5);
    data.prepend(mUpdateConfig->configData);

    serial->write(data);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event);

    //存储布局
    QSettings settings("CsCode.ini", QSettings::IniFormat);
    settings.setValue("horizantalLayout", ui->splitter->saveState());
    settings.setValue("verticalLayout", ui->splitter_2->saveState());
    settings.setValue("windowSize", this->size());

    //存储文件路径
    settings.setValue("filePath", savedFilePath);
    settings.setValue("imagePath",imagePath);
    settings.setValue("configFilePath",mUpdateConfig->configFilePath);

    //存储未保存的数据
    if (isFileSaved == false) {
        if (QMessageBox::question(this, "save file",
            QStringLiteral("文件未保存\n需要保存吗？")) ==
            QMessageBox::Yes) {
            fileSave();
        }
    }
}

void MainWindow::handleError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {
        closeSerialPort();
        QPalette pS;
        pS.setColor(QPalette::WindowText, Qt::darkRed);
        status->setPalette(pS);
        status->setText(QStringLiteral("串口错误"));
    }
}

void MainWindow::openSerialPort() {
    if (serial->isOpen()) {
        disconnect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
        serial->close();
    }

    SettingsDialog::Settings p = serialSettingDialog->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    QPalette pS;
    pS.setColor(QPalette::WindowText, Qt::darkBlue);
    status->setPalette(pS);

    if (serial->open(QIODevice::ReadWrite)) {
        status->setText(tr("Connected to %1 : %2, %3, %4, %5, %6")
            .arg(p.name)
            .arg(p.stringBaudRate)
            .arg(p.stringDataBits)
            .arg(p.stringParity)
            .arg(p.stringStopBits)
            .arg(p.stringFlowControl));
    }
    else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        // status->setText(tr("Open error"));
    }
}

void MainWindow::readData() {
    QTextCursor textCursor(msg->document());
    textCursor.movePosition(QTextCursor::End);
    msg->setTextCursor(textCursor);
    msg->insertPlainText(serial->readAll());
}

void MainWindow::clearCmd(QString) {
    command->clear();
    msg->clear();
}

void MainWindow::help(QString) {
    command->appendPlainText(QStringLiteral("clear      :清除文本"));
}

void MainWindow::setPattern(QString s) {
    if (!serial->isOpen()) {
        msg->appendPlainText("Error:Please open serial port first");
        return;
    }
    bool ok = false;
    quint8 frame = 0;
    s.remove(QRegExp("\\s"));
    QRegExp rx("pattern=(.*)");
    if (s.indexOf(rx) != -1) {

        frame = rx.cap(1).toInt(&ok, 0);
    }
    if (ok) {
        const quint8 a[] = { 0xAA, 0xAA, SET_FRAME, frame, 0x55, 0x55 };
        QByteArray ba((char *)a, 6);
        serial->write(ba);
    }
}

bool MainWindow::saveToFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::critical(this, "critical", QStringLiteral("文件禁止写入"));
        return false;
    }
    else {
        QTextStream out(&file);
        out << codeEdit->toPlainText();
        isFileSaved = true;
        return true;
    }
}

void MainWindow::loadFile() {
    if (savedFilePath.isEmpty()) {
        return;
    }

    QRegExp rx("[^\\n]*.csc");
    if (rx.exactMatch(savedFilePath) == false) {
        return;
    }
    this->setWindowTitle("CS Code --  " + savedFilePath);
    QFile TextFile(savedFilePath);

    if (!TextFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Open error", "Text file open failed!");
        savedFilePath.clear();
        this->setWindowTitle("CS Code  --  empity workspace");
        return;
    }

    disconnect(codeEdit, SIGNAL(textChanged()), this, SLOT(enableFileSave()));
    QTextStream ts(&TextFile);
    codeEdit->setPlainText(ts.readAll());
    connect(codeEdit, SIGNAL(textChanged()), this, SLOT(enableFileSave()));
}

void MainWindow::fileOpenWithDialog(bool) {
    if (isFileSaved == false) {
        if (QMessageBox::question(this, "save file",
            QStringLiteral("文件未保存\n需要保存吗？")) ==
            QMessageBox::Yes) {
            fileSave();
        }
    }

    QString fileName =
        QFileDialog::getOpenFileName(this, "Open file", savedFilePath,
        "cool saven file(*.csc);;All file(*.*)");
    if (fileName.isEmpty()) {
        return;
    }

    savedFilePath = fileName;
    this->setWindowTitle("CS Code  --  " + savedFilePath);

    QFile TextFile(savedFilePath);

    if (!TextFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Open error", "Text file open failed!");
        savedFilePath.clear();
        this->setWindowTitle("CS Code  --  empity workspace");
        return;
    }

    disconnect(codeEdit, SIGNAL(textChanged()), this, SLOT(enableFileSave()));
    QTextStream ts(&TextFile);
    codeEdit->setPlainText(ts.readAll());
    connect(codeEdit, SIGNAL(textChanged()), this, SLOT(enableFileSave()));
}

void MainWindow::fileSave() {
    if (savedFilePath.isEmpty())
        savedFilePath = QFileDialog::getSaveFileName(this, QStringLiteral("保存"),
        ".", "cool saven file(*.csc)");
    if (savedFilePath.isEmpty()) {
        return;
    }
    else {
        saveToFile(savedFilePath);
    }

    ui->actionSaveFile->setEnabled(false);
    this->setWindowTitle("CS Code  --  " + savedFilePath);
}

void MainWindow::fileSaveAs() {
    QString fileName =
        QFileDialog::getSaveFileName(this, QStringLiteral("另存为"), ".",
        "cool saven file(*.csc);;All file(*.*)");
    if (fileName.isEmpty()) {
        return;
    }
    else {
        saveToFile(fileName);
        savedFilePath = fileName;
        this->setWindowTitle("CS Code  --  " + savedFilePath);
    }
}

void MainWindow::enableFileSave() {
    if (savedFilePath.isEmpty()) {
        this->setWindowTitle("CS Code  --  empity workspace");
        return;
    }
    else {

        if (codeEdit->toPlainText().isEmpty()) {
            ui->actionSaveFile->setEnabled(false);
            this->setWindowTitle("CS Code  --  " + savedFilePath);
        }
        else {
            ui->actionSaveFile->setEnabled(true);
            this->setWindowTitle("*CS Code --  " + savedFilePath);
            isFileSaved = false;
        }
    }
}

void MainWindow::parseCode() {
    parse->updateStr(codeEdit->toPlainText());
    if(parse->compile() == true)
    {
        QString fileName = savedFilePath;
        fileName.chop(3);
        fileName += "cfg";
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly))
        {
            QMessageBox::critical(this, "critical", QStringLiteral("文件禁止写入"));
            return ;
        }
        else
        {
            file.write(parse->dataToSerial);
        }

    }
}

void MainWindow::parseCommand(QString str) {
    if (!str.isEmpty()) {
        foreach(command_type c, commandList) {
            if (str.startsWith(c.commandStr)) {
                (this->*(c.commandFun))(str);
                return;
            }
        }

        command->appendPlainText("Error:invalid command-->" + str);
    }
}
