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
#include "QProgressBar"

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

    CodeEditor *mCodeEdit;
    QVBoxLayout *mCodeLayout;
    QSerialPort *mSerialPort;
    CommandEdit *mCommandEdit;
    QVBoxLayout *mCommandEditLayout;
    QList<command_type> mCommandList;
    QString mSavedFilePath;
    CodeParse *mParse;
    QLabel *mSerialPortStatus;
    QLabel *mAuthor;
    msgEdit *mMsg;
    QVBoxLayout *mMsgLayout;
    ImageToBinDialog *mImageToBin;
    QString mImagePath;
    SettingsDialog *mSerialPortSettingDialog;
    updateConfig *mUpdateConfig;
    QList<quint8> mComData;
    quint8 mLidarRawData[MAX_DATA_AMOUTN_PER_FRAME];
    quint32 mLidarRawDataCounter;
    PackageDataStruct mRecPackage;
    quint32 mDataLen;
    QProgressBar *mProgress;
    bool mIsDownloadDone;
    bool mIsFileSaved;


    bool saveToFile(const QString& fileName);
    void loadFile();
    bool IsDataReady(QByteArray &data);
    void waitSTM32Work(int t);
    void initSerialPort();
    void initEdit();
    void initStatusBar();
    void initAction();
    void restoreCustom();
    void recoverCustom();

private slots:
    void openFileWithDialog(bool);
    void saveConfigFile();
    void saveFileAs();
    void enableFileSave();
    void parseCode(void);
    void parseCommand(QString str);
    void openSerialPort();
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    void closeSerialPort();
    void downloadConfig();
    void sendFlashCmd();
    void showVersion();
    void contactUs();
    void trImageToBin();
    void createNewFile();
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
