#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "codeedit.h"
#include "QSerialPort"
#include "QVBoxLayout"
#include "commandedit.h"
#include "codehlighter.h"
#include "codeparse.h"
#include "msgedit.h"
#include "settingsdialog.h"
#include "QLabel"
#include "imagetobindialog.h"
#include "updateconfig.h"
#include "protocol/pro.h"

#define MAX_DATA_AMOUTN_PER_FRAME 8192
namespace Ui {
class MainWindow;
}

class command_type;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void clearCmd(QString);
    void help(QString);
    void setPattern(QString s);

private:
    Ui::MainWindow *ui;

    CodeEditor *codeEdit;
    QVBoxLayout *codeLayout;

    QSerialPort *serial;

    commandEdit *command;
    QVBoxLayout *commandLayout;
    QList<command_type> commandList;

    QString savedFilePath;

    codeParse *parse;
    QLabel *status;
    QLabel *author;
    msgEdit *msg;
    QVBoxLayout *msgLayout;
    ImageToBinDialog *imageToBin;
    QString imagePath;
    SettingsDialog *serialSettingDialog;
    updateConfig *mUpdateConfig;
    QList<quint8> comData;
    quint8 mLidarRawData[MAX_DATA_AMOUTN_PER_FRAME];
    quint32 mLidarRawDataCounter;
    PackageDataStruct mRecPackage;
    quint32 mDataLen;

    bool saveToFile(const QString& fileName);

    bool isDownloadDone;
    bool isFileSaved;
    void loadFile();
    bool IsDataReady(QByteArray &data);
    void waitSTM32Work(int t);


private slots:
    void fileOpenWithDialog(bool);
    void fileSave();
    void fileSaveAs();
    void enableFileSave();
    void parseCode(void);
    void parseCommand(QString str);
    void openSerialPort();
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    void closeSerialPort();
    void download();
    void flash();
    void showVersion();
    void contactUs();
    void ImageToBin();
    void fileNew();
    void burnConfig();

protected:
    void closeEvent(QCloseEvent *event);
};


class command_type
{
public:
    command_type(QString str, void(MainWindow::*cFun)(QString))
    {
        commandStr = str;
        commandFun = cFun;
    }

    QString commandStr;
    void (MainWindow::*commandFun)(QString);
};



#endif // MAINWINDOW_H
