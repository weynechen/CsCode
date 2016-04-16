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


private:
    Ui::MainWindow *ui;

    CodeEditor *codeEdit;
    QVBoxLayout *codeLayout;

    QSerialPort *serial;

    commandEdit *command;
    QVBoxLayout *commandLayout;
    QList<command_type> commandList;

    QString savedFileName;

    codeParse *parse;
    QLabel *status;
    QLabel *author;
    msgEdit *msg;
    QVBoxLayout *msgLayout;

    SettingsDialog *serialSettingDialog;
    bool saveFile(const QString &fileName);
    bool isDownloadDone=false;

private slots:
    void fileOpen();
    void fileSave();
    void enableFileSave();
    void parseCode(void);
    void parseCommand(QString str);
    void openSerialPort();
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    void closeSerialPort();
    void download();
    void flash();
};


class command_type
{
public:
    command_type(QString str,void (MainWindow::*cFun)(QString) )
    {
        commandStr=str;
        commandFun=cFun;
    }
    QString commandStr;
    void (MainWindow::*commandFun)(QString);

};



#endif // MAINWINDOW_H
