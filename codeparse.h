#ifndef CODEPARSE_H
#define CODEPARSE_H
#include"QString.h"
#include "QObject"

typedef enum
{
    RE_INIT_START, //重新初始化开始标志
    POWER,//初始化电源
    BACKLIGHT,//初始化背光
    LCD_PARA,//初始化lcd pclk，前后肩等参数
    LCD_INIT,//初始化lcd
    MIPI_PARA,//初始化SSD2828
    PATTERN,//初始化pattern
    PROJECT_NAME, // 项目名称
    AUTO_RUN, //自动切换画面
    RE_INIT_END,//重新初始化结束
    LCD_READ,//回读LCD寄存器，高速
    LCD_WRITE,//写LCD寄存器，高速模式
    CHOOSE_FRAME,//选择显示的画面
    TO_FLASH,//固化初始化参数到Flash
    FLASH_CONFIG
}com_id;

class codeParse:public QObject
{
    Q_OBJECT

private:

    const uint maxCurrent;

    QString strToParse;
    QStringList titleStr;
    QStringList powerStr;
    QStringList lcdParaStr;
    QStringList lcdInit;

    QString projectName;
    uint power;
    uint backlight;
    uint mipiLane,mipiSpeed;
    quint8 autoRun;
    QList<quint16> lcdPara;
    QList<quint8> lcdInitPara;
    QList<quint8> pattern;

    QList<quint8> para;

    typedef enum
    {
        NO_PACKAGE,
        GP,
        DCS
    }mipi_package;

    typedef enum
    {
        MIPI_START,
        MIPI_DCS,
        MIPI_GP,
        MIPI_DELAY,
        MIPI_WRITE,
        MIPI_READ,
        MIPI_END
    }mipi_type;
    typedef enum
    {
        PATTERN_START,
        RGB,
        FLICKERV,
        FLICKERH,
        COLORBARV,
        COLORBARH,
        GRAYLEVEL_V,
        GRAYLEVEL_H,
        CROSSTALK,
        PIC,
        FRAME,
        CHESSBOARD,
        RGBBAR,
        RGBLEVEL,
        PATTERN_STAY,
        NULL_PATTERN,
        PATTERN_END
    } pattern_type;


public:


    codeParse(QObject *parent);

    bool parseProjectName(QString data);
    bool parsePower(QString data);
    bool parseBacklight(QString data);
    bool parseLcdPara(QString data);
    bool parseLcdInit(QString data);
    bool parseMipi(QString data);
    bool parsePattern(QString data);
    bool parseAutoRun(QString data);

    bool compile(void);
    void updateStr(QString &str);
    QByteArray dataToSerial;

public slots:

signals:
    void Info(QString str);

};

#endif // CODEPARSE_H
