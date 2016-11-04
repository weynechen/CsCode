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
#include "QTimer"

#define MAX_DATA_AMOUTN_PER_FRAME 8192
#define PID 0x5739
#define VID 0x0483
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
    void upgradeFirmware(QString str);
    void reboot(QString);
    void getFirmwareVersion(QString);
    void readSSD2828(QString str);

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
    FirmwareUpgradeType upgradeMsg;
    QTimer *mPollUSBStatusTimer;
    qint8 mHeartbeats;
    QString mFirmwarePath;

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
    void sendCmd(quint8 t);
    void sendCmd(quint8 id , quint8 data);
    void sendMassData(ActionIDTypeDef id, const QByteArray &data);
    void sendUpgradeData(ActionIDTypeDef id , const QByteArray &data , u32 &progress);

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
    void pollUSBStatus();
    void openUpgradeDialog();

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
