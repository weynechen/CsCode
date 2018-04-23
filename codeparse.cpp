#include "codeparse.h"
#include <QDebug>
#include "QTextStream"
#include "crc.h"
#include "QTime"

CodeParse::CodeParse(QObject *parent) : QObject(parent), mPower(0), mBacklight(0), mMaxCurrent(150),IsLcdTimingParsed(false),
    isDefaultPowerSet(false),
    isExSystemConfig(false),
    isUserPowerSet(false)
{
    mTitleStr << "project name" << "power" << "backlight" << "LCD parameter" << "MIPI setting" << "LCD initial code" << "pattern" << "auto run"<<"lcd type"
              <<"power on sequence"<<"power off sequence"<<"font scale";
    mPowerStr << "1.8V" << "2.8V" << "3.3V" << "VSP" << "VSN"<<"5V"<<"MTP"<<"AVDD";
    mLcdParaStr << "pix clock" << "horizontal resolution" << "vertical resolution" << "horizontal back porch"
                << "horizontal front porch" << "horizontal sync pulse width" << "vertical back porch" << "vertical front porch"
                << "vertical sync pulse width";
    mLcdInit << "package" << "write" << "delay" << "read";

    powerList<< "1.8V" << "2.8V" << "3.3V" << "VSP" << "VSN"<<"5V"<<"MTP"<<"AVDD"<<"VCOM"<<"VGH"<<"VGL"<<"reset=high"<<"reset=low";
    //初始化SystemConfig
    QTime t;
    t = QTime::currentTime();
    qsrand(t.msec() + t.second() * 1000);
    quint8 *p = (quint8 *)&mSystemConfig;
    for (int i = 0; i < sizeof(mSystemConfig); i++)
    {
        *(p + i) = qrand() % 0xff;
    }
    p = (uint8_t *)&exSystemConfig;
    for (int i = 0; i < sizeof(exSystemConfig); i++)
    {
        *(p + i) = qrand() % 0xff;
    }
}


bool CodeParse::parseProjectName(QString data)
{
    data.remove(QRegExp("\n"));
    data.remove(QRegExp("\\s+$"));
    mProjectName = data;

    emit Info(QString("project Name:%1").arg(data));
    return true;
}


bool CodeParse::parsePower(QString data)
{
    if (data.isEmpty())
    {
        return false;
    }
    data.replace(QRegExp("\\n+"), "\n");
    //QRegExp rx("-?\\d.?\\d?V");
    QTextStream ts(&data);
    QString strLine;
    mSystemConfig.PowerSettings = 0;
    while (!ts.atEnd())
    {
        strLine = ts.readLine();
        strLine.remove(QRegExp("\\s+"));    //删除空格
        // bool match = rx.exactMatch(strLine);
        //if (match)
        //{
        switch (mPowerStr.indexOf(QRegExp(strLine)))
        {
        case 0:
            mSystemConfig.PowerSettings |= 0x01;

            break;

        case 1:
            mSystemConfig.PowerSettings |= 0x2;
            break;

        case 2:
            mSystemConfig.PowerSettings |= 0x4;
            break;

        case 3:
            mSystemConfig.PowerSettings |= 0x8;
            break;

        case 4:
            mSystemConfig.PowerSettings |= 0x10;
            break;

        case 5:
            mSystemConfig.PowerSettings |= 0x20;
            break;

        case 6:
            mSystemConfig.PowerSettings |= 0x40;
            break;

        case 7:
            mSystemConfig.PowerSettings |= 0x80;
            break;

        default:

            emit Info("Error:power error");
            return false;
        }
        //}
        //        else
        //        {
        //            emit Info("Error:power error");
        //            return false;
        //        }
    }
    return true;
}


bool CodeParse::parseBacklight(QString data)
{
    if (data.isEmpty())
    {
        return false;
    }
    data.remove("\n");
    data.remove(QRegExp("\\s+"));
    QRegExp rx("\\d+");
    int count = rx.indexIn(data);
    if (count != 0)
    {
        emit Info("Error:backlight para setting error");
        return false;
    }
    bool ok;

    mSystemConfig.Backlight = rx.cap(0).toInt(&ok, 10);

    if (ok)
    {
        if (mSystemConfig.Backlight <= mMaxCurrent)
        {
            return true;
        }
    }
    emit Info("Error:backliaght parameter setting error");

    return false;
}


bool CodeParse::parseLcdPara(QString data)
{
    if (data.isEmpty())
    {
        emit Info("Error:no lcd parameter found");
        return false;
    }

    data.replace(QRegExp("\\n\\n+"), "\n");

    QTextStream ts(&data);
    QString strLine;
    QList<quint16> lcdPara;

    foreach(QString s, mLcdParaStr)
    {
        if (data.indexOf(s) == -1)
        {
            emit Info("Error:lcd parameter setting error");
            return false;
        }
        else
        {
            strLine = ts.readLine();
            strLine.remove(s);
            strLine.remove(":");
            strLine.remove(QRegExp("\\s"));

            QRegExp rxDec("\\b\\d+\\b");
            QRegExp rxHex("\\b0x[0-9A-Fa-f]+\\b");
            bool result = rxDec.exactMatch(strLine);
            if (result)
            {
                lcdPara << strLine.toInt();
            }
            else
            {
                result = rxHex.exactMatch(strLine);
                if (result)
                {
                    bool ok;
                    strLine.remove("0x");
                    lcdPara << strLine.toInt(&ok, 16);
                }
                else
                {
                    emit Info("Error:lcd parameter setting error");
                    return false;
                }
            }
        }
    }

    quint16 count = 0;

    foreach(quint16 l, lcdPara)
    {
        mSystemConfig.LCDTimingPara[count++] = (quint8)(l & 0xff);
        mSystemConfig.LCDTimingPara[count++] = (quint8)(l >> 8);
    }

    memcpy((quint16 *)&mLCDTiming, (quint16 *)mSystemConfig.LCDTimingPara, sizeof(mSystemConfig.LCDTimingPara));
    IsLcdTimingParsed = true;


    return true;
}


bool CodeParse::parseMipiOr8BitRGBLcdInit(QString data)
{
    QRegExp rxDec("\\d+");
    QRegExp rxHex("0x[0-9A-Fa-f]+");
    bool ok;

    QList<quint8> lcdInitPara;

    QStringList strList = data.split(QRegExp("\n+"), QString::SkipEmptyParts);
    foreach(QString s, strList)
    {
        QRegExp rxSpace("\\s+");

        if (rxSpace.exactMatch(s))
        {
            continue;
        }
        s.remove("\t");
        if ((s.contains(QRegExp("\\bpackage\\s*=\\s*"))) || (s.contains(QRegExp("^write\\s+"))) || \
                (s.contains(QRegExp("^read\\s+"))) || (s.contains(QRegExp("^delay\\s+"))))
        {
        }
        else
        {
            emit Info("Error:lcd inlitial code syntax error");
        }

        //处理封包
        if (s.contains(QRegExp("\\bpackage\\s*=\\s*")))
        {
            if ((s.contains("DCS")) || (s.contains("RISING")))
            {
                lcdInitPara << MIPI_DCS;
            }
            else if ((s.contains("GP")) || (s.contains("FALLING")))
            {
                lcdInitPara << MIPI_GP;
            }
            else
            {
                emit Info("Error:lcd package setting error");
                return false;
            }
        }

        //处理写
        if (s.contains(QRegExp("^write\\s+")))
        {
            s.remove(QRegExp("^write\\s+"));
            s.replace(QRegExp("\\s\\s+"), " ");
            QStringList sList = s.split(" ", QString::SkipEmptyParts);
            lcdInitPara << MIPI_WRITE << sList.size();
            foreach(QString sp, sList)
            {
                if (rxDec.exactMatch(sp))
                {
                    lcdInitPara << sp.toInt();
                }
                else if (rxHex.exactMatch(sp))
                {
                    sp = sp.remove("0x");
                    lcdInitPara << sp.toInt(&ok, 16);
                }
                else
                {
                    emit Info("Error:lcd write para setting error");
                    return false;
                }
            }
        }

        //处理读
        if (s.contains(QRegExp("^read\\s+")))
        {
            s.remove(QRegExp("^read\\s+"));
            s.replace(QRegExp("\\s\\s+"), " ");
            QStringList sList = s.split(" ", QString::SkipEmptyParts);
            if (sList.size() != 2)
            {
                emit Info("Error:lcd read para setting error");
                return false;
            }
            lcdInitPara << MIPI_READ;

            foreach(QString sp, sList)
            {
                if (rxDec.exactMatch(sp))
                {
                    lcdInitPara << sp.toInt();
                }
                else if (rxHex.exactMatch(sp))
                {
                    sp = sp.remove("0x");
                    lcdInitPara << sp.toInt(&ok, 16);
                }
                else
                {
                    emit Info("Error:lcd read para setting error");
                    return false;
                }
            }
        }

        //处理延时
        if (s.contains(QRegExp("^delay\\s+")))
        {
            s.remove(QRegExp("^delay\\s+"));
            lcdInitPara << MIPI_DELAY;

            if (rxDec.exactMatch(s.remove(QRegExp("\\s*"))))
            {
                lcdInitPara << (quint8)(s.toInt() >> 8);
                lcdInitPara << (quint8)(s.toInt() & 0xff);
            }
            else if (rxHex.exactMatch(s.remove(QRegExp("\\s*"))))
            {
                s = s.remove("0x");
                lcdInitPara << (quint8)(s.toInt(&ok, 16) >> 8);
                lcdInitPara << (quint8)(s.toInt(&ok, 16) & 0xff);
            }
            else
            {
                emit Info("Error:lcd delay para setting error");
                return false;
            }
        }
    }
    quint16 initCodeSize = lcdInitPara.size();
    mSystemConfig.LCDInitCode[0] = initCodeSize >> 8;
    mSystemConfig.LCDInitCode[1] = (quint8)initCodeSize;

    if (initCodeSize > LCD_INIT_LEN)
    {
        emit Info("Error:too much lcd para");
        return false;
    }

    for (int i = 0; i < initCodeSize; i++)
    {
        mSystemConfig.LCDInitCode[i + 2] = lcdInitPara[i];
    }

    return true;
}


bool CodeParse::parseMipiSettings(QString data)
{
    QRegExp rxPara("write +0x[0-9a-fA-F]+ 0x[0-9a-fA-F]+");
    QRegExp rx("^write +(.*) +(.*)");
    data.replace(QRegExp("\n\n+"), "\n");
    data.remove("\t");
    data.remove("\r");
    data.replace(QRegExp("  +")," ");
    QStringList strList = data.split("\n", QString::SkipEmptyParts);
    bool ok;
    QList<quint8> para;

    if(strList.isEmpty())
    {
        emit Info("Error:need mipi parameter");
        return false;
    }
    QString sMode = strList[0];

    if(sMode.contains(rxPara))
    {

        if(IsLcdTimingParsed == false)
        {
            emit Info("Error:Please set Lcd timming before MIPI setting");
            return false;
        }
        else
            IsLcdTimingParsed = false;


        para<<0xb7<<0x00<<0x50;
        para<<0xb9<<0x00<<0x00;
        para<<0xb1<<(quint8)mLCDTiming.VSPW<<(quint8)mLCDTiming.HSPW;
        para<<0xb2<<(quint8)mLCDTiming.VBPD<<(quint8)(mLCDTiming.HBPD);
        para<<0xb3<<(quint8)mLCDTiming.VFPD<<(quint8)mLCDTiming.HFPD;
        para<<0xb4<<(quint8)(mLCDTiming.LCDH>>8)<<(quint8)(mLCDTiming.LCDH);
        para<<0xb5<<(quint8)(mLCDTiming.LCDV>>8)<<(quint8)(mLCDTiming.LCDV);

        foreach(QString s,strList)
        {
            if(s.contains(rxPara))
            {
                if(s.indexOf(rx) != -1)
                {
                    quint8 cmd = rx.cap(1).toInt(&ok,16);
                    quint16 parameter = rx.cap(2).toInt(&ok,16);

                    if(!ok)
                    {
                        emit Info("Error:MIPI settings error");
                        return false;
                    }
                    para<<cmd<<(quint8)(parameter>>8)<<(quint8)(parameter);

                }
            }
            else
            {
                emit Info("Error:MIPI settings error");
                return false;
            }
        }

        if(para.size()>255)
            emit Info("Error:Two much MIPI parameters");

        mSystemConfig.MIPIConfig[0] = para.size();


        for(int i = 0;i<para.size();i++)
        {
            mSystemConfig.MIPIConfig[i + 1] = para[i];
        }
    }
    else// if(sMode.contains(rxbuildin))
    {
        QRegExp rxDec("\\d+");
        QRegExp rxHex("0x[0-9A-Fa-f]+");
        QRegExp rxLaneDec("MIPI lane:\\s*[1-4]\\s*");
        QRegExp rxSpeedDec("MIPI speed:\\s*\\d+\\s*Mbps\\s*");
        QRegExp rxLaneHex("MIPI lane:\\s*0x[1-4]\\s*");
        QRegExp rxSpeedHex("MIPI speed:\\s*0x[0-9A-Fa-f]+\\s*Mbps\\s*");

        QStringList strList = data.split("\n", QString::SkipEmptyParts);
        uint mipiLane = 0, mipiSpeed = 0;

        foreach(QString str, strList)
        {
            if (rxLaneDec.exactMatch(str))
            {
                mipiLane = QString(str[str.indexOf(rxDec)]).toInt();
            }

            if (rxLaneHex.exactMatch(str))
            {
                str.indexOf(rxHex);
                QString s = rxHex.cap();
                mipiSpeed = s.remove("0x").toInt(&ok, 16);
            }

            if (rxSpeedDec.exactMatch(str))
            {
                str.indexOf(rxDec);
                mipiSpeed = rxDec.cap().toInt();
            }

            if (rxSpeedHex.exactMatch(str))
            {
                str.indexOf(rxHex);
                QString s = rxHex.cap();
                mipiSpeed = s.remove("0x").toInt(&ok, 16);
            }
        }

        if ((mipiSpeed == 0) || (mipiLane == 0))
        {
            emit Info("Error:Mipi setting error");
            return false;
        }
        else
        {
            mSystemConfig.MIPIConfig[0] = 3;
            mSystemConfig.MIPIConfig[1] = mipiSpeed >> 8;
            mSystemConfig.MIPIConfig[2] = mipiSpeed & 0xff;
            mSystemConfig.MIPIConfig[3] = mipiLane & 0xff;
        }
    }
    return true;

}


bool CodeParse::parsePattern(QString data)
{
    bool ok;

    QList<quint8> pattern;
    QStringList strList = data.split("\n", QString::SkipEmptyParts);

    foreach(QString s, strList)
    {
        if (s.contains(QRegExp("^RGB\\s*\\(.*\\)\\s*")))
        {
            pattern << RGB;
            QRegExp rx("^RGB\\s*\\((.*),(.*),(.*)\\)");
            if (s.indexOf(rx) != -1)
            {
                pattern << rx.cap(1).toInt(&ok, 0) << rx.cap(2).toInt(&ok, 0) << rx.cap(3).toInt(&ok, 0);
            }
            else
            {
                emit Info("Error:pattern setting error");
                return false;
            }
        }


        if (s.contains(QRegExp("^ShowID\\s*")))
        {
            pattern << SHOW_ID;
        }

        if (s.contains(QRegExp("^stay\\s+")))
        {
            pattern << PATTERN_STAY;
            quint16 p = s.remove(QRegExp("stay")).remove(QRegExp("\\s*")).toInt(&ok, 0);
            pattern << (p >> 8);
            pattern << (p & 0xff);
            if (ok == false)
            {
                emit Info("Error:pattern setting error");
                return false;
            }
        }



        if (s.contains(QRegExp("^PIC\\s+")))
        {
            pattern << PIC;
            s = s.remove(QRegExp("^PIC\\s+"));
            s = s.remove('"');
            for (int i = 0; i < s.size(); i++)
            {
                pattern << (uint)s.at(i).toLatin1();
            }
            pattern << 0; //结束符
        }

        if (s.contains(QRegExp("^horizontal colorbar\\s*")))
        {
            pattern << COLORBARH;
        }

        if (s.contains(QRegExp("^frame\\s*")))
        {
            pattern << FRAME;
        }

        if (s.contains(QRegExp("^vertical colorbar\\s*")))
        {
            pattern << COLORBARV;
        }

        if (s.contains(QRegExp("^flicker\\s*")))
        {
            pattern << FLICKERH;
        }

        if (s.contains(QRegExp("^column flicker\\s*")))
        {
            pattern << FLICKERH;
        }

        if (s.contains(QRegExp("^row flicker\\s*")))
        {
            pattern << FLICKERV;
        }

        if (s.contains(QRegExp("^dot flicker\\s*")))
        {
            pattern << FLICKER_DOT;
        }

        if (s.contains(QRegExp("^vertical gray level +\\d+")))
        {
            pattern << GRAYLEVEL_V_USER;
            QRegExp rx("^vertical gray level +(.*)");
            if (s.indexOf(rx) != -1)
            {
                uint16_t level = rx.cap(1).toInt(&ok,0);
                qDebug()<<level;
                if(level > 256)
                {
                    emit Info("Error:gray level shold not big than 256");
                    return false;
                }
                pattern << (quint8)level << (quint8)(level>>8);
            }
            else
            {
                emit Info("Error:pattern setting error");
                return false;
            }
        }
        else if (s.contains(QRegExp("^vertical gray level\\s*")))
        {
             qDebug()<<"fix level";
            pattern << GRAYLEVEL_V;
        }

        if (s.contains(QRegExp("^horizontal gray level +\\d+")))
        {
            pattern << GRAYLEVEL_H_USER;
            QRegExp rx("^horizontal gray level +(.*)");
            if (s.indexOf(rx) != -1)
            {
                uint16_t level = rx.cap(1).toInt(&ok,0);
                qDebug()<<level;
                if(level > 256)
                {
                    emit Info("Error:gray level shold not big than 256");
                    return false;
                }
                pattern << (quint8)level << (quint8)(level>>8);
            }
            else
            {
                emit Info("Error:pattern setting error");
                return false;
            }
        }
        else if (s.contains(QRegExp("^horizontal gray level\\s*")))
        {
            pattern << GRAYLEVEL_H;
        }

        if (s.contains(QRegExp("^crosstalk\\s*")))
        {
            pattern << CROSSTALK;
        }

        if (s.contains(QRegExp("^chessboard\\s*")))
        {
            pattern << CHESSBOARD;
        }

        if (s.contains(QRegExp("^rgbbar\\s*")))
        {
            pattern << RGBBAR;
        }

        if (s.contains(QRegExp("^rgblevel\\s*")))
        {
            pattern << RGBLEVEL;
        }

        if (s.contains(QRegExp("^sleep in\\s*")))
        {
            pattern << SLEEP_IN;
        }

        if (s.contains(QRegExp("^sleep out\\s*")))
        {
            pattern << SLEEP_OUT;
        }

        if (s.contains(QRegExp("^null pattern\\s*")))
        {
            pattern << NULL_PATTERN;

            if (pattern[0] = (quint8)NULL_PATTERN)
            {
                emit Info(QStringLiteral("Error:请把null pattern 放在第一个"));
                return false;
            }
            else
            {
                true;
            }
        }
    }

    mSystemConfig.Pattern[0] = pattern.size() >> 8;
    mSystemConfig.Pattern[1] = pattern.size();
    for (int i = 0; i < pattern.size(); i++)
    {
        mSystemConfig.Pattern[i + 2] = pattern[i];
    }
    //qDebug() << pattern.size();
    //qDebug() << hex << pattern;
    return true;
}


bool CodeParse::parseAutoRun(QString data)
{
    if (data.isEmpty())
    {
        return false;
    }
    data.remove("\n");
    data.remove(QRegExp("\\s+"));
    if (data == "NO")
    {
        mSystemConfig.IsAutoRun = 0;
        return true;
    }
    else if (data == "YES")
    {
        mSystemConfig.IsAutoRun = 1;
        return true;
    }
    return false;
}


bool CodeParse::parseFontScale(QString data)
{
    if (data.isEmpty())
    {
        return false;
    }
    data.remove("\n");
    data.remove(QRegExp("\\s+"));

    bool ok = true;
    int scale = data.toInt(&ok,10);

    qDebug()<< "scale:"<<scale;
    if(ok)
    {
        exSystemConfig.FontScale = scale;
        return true;
    }
    return false;
}

bool CodeParse::parseLcdType(QString data)
{
    if (data.isEmpty())
    {
        return false;
    }
    data.remove("\n");
    data.remove(QRegExp("\\s+"));
    if (data == "MIPI")
    {
        mSystemConfig.LcdType = MIPI_LCD;
        return true;
    }
    else if (data == "RGB_SPI16BIT")
    {
        mSystemConfig.LcdType = RGB_SPI16BIT;
        return true;
    }
    else if (data == "RGB_SPI8BIT")
    {
        mSystemConfig.LcdType = RGB_SPI8BIT;
        return true;
    }
    else if (data == "RGB_SPI9BIT")
    {
        mSystemConfig.LcdType = RGB_SPI9BIT;
        return true;
    }
    else if (data == "SPI_2_Data_Lane")
    {
        mSystemConfig.LcdType = SPI_2_Data_Lane;
        return true;
    }
    else if (data == "LVDS_666_VESA")
    {
        mSystemConfig.LcdType = LVDS_666_VESA;
        return true;
    }
    else if (data == "LVDS_666_JEIDA")
    {
        mSystemConfig.LcdType = LVDS_666_JEIDA;
        return true;
    }
    else if (data == "LVDS_888_VESA")
    {
        mSystemConfig.LcdType = LVDS_888_VESA;
        return true;
    }
    else if (data == "LVDS_888_JEIDA")
    {
        mSystemConfig.LcdType = LVDS_888_JEIDA;
        return true;
    }
    return false;
}

bool CodeParse::parseRGBLcdInit(QString data)
{
    QRegExp rxDec("\\d+");
    QRegExp rxHex("0x[0-9A-Fa-f]+");
    bool ok;

    QList<quint8> lcdInitPara;

    QStringList strList = data.split(QRegExp("\n+"), QString::SkipEmptyParts);
    foreach(QString s, strList)
    {
        QRegExp rxSpace("\\s+");

        if (rxSpace.exactMatch(s))
        {
            continue;
        }
        s.remove("\t");
        if ((s.contains(QRegExp("\\bpackage\\s*=\\s*"))) || (s.contains(QRegExp("^write\\s+"))) || \
                (s.contains(QRegExp("^read\\s+"))) || (s.contains(QRegExp("^delay\\s+"))))
        {
        }
        else
        {
            emit Info("Error:lcd inlitial code syntax error");
        }

        //处理封包
        if (s.contains(QRegExp("\\bpackage\\s*=\\s*")))
        {
            if (s.contains("RISING"))
            {
                lcdInitPara << RGB_SPI_RISING;
            }
            else if (s.contains("FALLING"))
            {
                lcdInitPara << RGB_SPI_FALLING;
            }
            else
            {
                emit Info("Error:lcd package setting error");
                return false;
            }
        }

        //处理写
        if (s.contains(QRegExp("^write\\s+")))
        {
            s.remove(QRegExp("^write\\s+"));
            s.replace(QRegExp("\\s\\s+"), " ");
            QStringList sList = s.split(" ", QString::SkipEmptyParts);

            if(sList.size()!=2)
            {
                emit Info("Error:Unsupported RGB Initial type");
                return false;
            }

            lcdInitPara << RGB_WRITE;

            quint16 data = sList[0].toInt(&ok,16);
            lcdInitPara << (data & 0xff) << (data >> 8);
            lcdInitPara << sList[1].toInt(&ok,16);

        }

        //处理读
        if (s.contains(QRegExp("^read\\s+")))
        {
            s.remove(QRegExp("^read\\s+"));
            s.replace(QRegExp("\\s\\s+"), " ");
            QStringList sList = s.split(" ", QString::SkipEmptyParts);
            if (sList.size() != 1)
            {
                emit Info("Error:lcd read para setting error");
                return false;
            }
            lcdInitPara << RGB_READ;

            quint16 data = sList[0].toInt(&ok,16);
            lcdInitPara << (data & 0xff) << (data >> 8);
        }

        //处理延时
        if (s.contains(QRegExp("^delay\\s+")))
        {
            s.remove(QRegExp("^delay\\s+"));
            lcdInitPara << MIPI_DELAY;

            if (rxDec.exactMatch(s.remove(QRegExp("\\s*"))))
            {
                lcdInitPara << (quint8)(s.toInt() >> 8);
                lcdInitPara << (quint8)(s.toInt() & 0xff);
            }
            else if (rxHex.exactMatch(s.remove(QRegExp("\\s*"))))
            {
                s = s.remove("0x");
                lcdInitPara << (quint8)(s.toInt(&ok, 16) >> 8);
                lcdInitPara << (quint8)(s.toInt(&ok, 16) & 0xff);
            }
            else
            {
                emit Info("Error:lcd delay para setting error");
                return false;
            }
        }
    }
    quint16 initCodeSize = lcdInitPara.size();
    mSystemConfig.LCDInitCode[0] = initCodeSize >> 8;
    mSystemConfig.LCDInitCode[1] = (quint8)initCodeSize;

    if (initCodeSize > LCD_INIT_LEN)
    {
        emit Info("Error:too much lcd para");
        return false;
    }

    for (int i = 0; i < initCodeSize; i++)
    {
        mSystemConfig.LCDInitCode[i + 2] = lcdInitPara[i];
    }

    return true;
}

bool CodeParse::parseUserPower(QString &data,QList<uint8_t>&power)
{
    if (data.isEmpty())
    {
        return false;
    }
    data.replace(QRegExp("\\n+"), "\n");
    QTextStream ts(&data);
    QString strLine;
    mSystemConfig.PowerSettings = 0;



    bool isPower = false;
    uint8_t powerAmount = 0;

    while (!ts.atEnd())
    {
        strLine = ts.readLine();

        //处理延时
        if (strLine.contains(QRegExp("^delay\\s+")))
        {
            //必须有电源在前面
            if(!isPower)
            {
                emit Info("Error:No power settings before delay");
                return false;
            }
            isPower = false;

            bool ok = false;
            strLine.remove(QRegExp("^delay\\s+"));

            uint16_t delay = strLine.toInt(&ok);

            if(!ok)
            {
                emit Info("Error:power delay error");
                return false;
            }

            power<<(uint8_t)(delay>>8)<<(uint8_t)delay;

        }
        else
        {
            if(isPower)
            {
                power<<0<<0;
            }
            isPower = true;
            strLine.remove(QRegExp("\\s+"));    //删除空格

            int index = powerList.indexOf(QRegExp(strLine));
            if(index == -1)
            {
                emit Info("Error: power parameters error");
                return false;
            }
            else
            {
                power<<index;
            }
            powerAmount++;
        }
    }
    power<<0<<0;

    power.prepend(powerAmount);

    qDebug()<<power;
    return true;
}

bool CodeParse::parsePowerOnSequence(QString data)
{
    QList<uint8_t>power;
    if(parseUserPower(data,power))
    {
        if(power.size()>POWER_LEN)
        {
            emit Info("Error: power parameter error");
            return false;
        }
        for(int i=0;i<power.size();i++)
        {
            exSystemConfig.PowerOnSequence[i] = power[i];
        }
        return true;
    }
    return false;
}

bool CodeParse::parsePowerOffSequence(QString data)
{
    QList<uint8_t>power;
    if(parseUserPower(data,power))
    {
        if(power.size()>POWER_LEN)
        {
            emit Info("Error: power parameter error");
            return false;
        }
        for(int i=0;i<power.size();i++)
        {
            exSystemConfig.PowerOffSequence[i] = power[i];
        }
        return true;
    }
    return false;
}


void CodeParse::updateStr(QString& str)
{
    mStrToParse = str;
}


bool CodeParse::compile()
{
    QString str = mStrToParse;

    if (str.isEmpty())
    {
        emit Info("Error:code is empity");
        return false;
    }
    //删除所有注释
    str.remove(QRegExp("/\\*[^\\*]*[^/]*\\*/"));
    str.remove(QRegExp("//[^\n]*"));
    //分割title和para
    QStringList segments;
    QStringList title;
    QStringList data;
    segments = str.split(QRegExp("\\[")); //按标题[分割
    segments.removeFirst();               //第一个是"["之前的内容，移除

    foreach(QString s, segments)
    {
        if (s.isEmpty() || (s == "\n"))
        {
            continue;
        }
        else
        {
            title << s.section("]\n", 0, 0); //提取段落头
            data << s.section("]\n", 1, 1);  //提取段落中的内容
        }
    }

    int i0 = 0;
    QList<bool>result;

    isDefaultPowerSet = false;
    isExSystemConfig = false;
    isUserPowerSet = false;

    exSystemConfig.FontScale = 0;
    //根据title选择不同的解析函数
    foreach(QString s, title)
    {
        switch (mTitleStr.indexOf(QRegExp(s)))
        {
        case 0:
            emit Info("Info:find project name");
            result<< parseProjectName(data[i0]);
            memset(mSystemConfig.ProjectName,0,MAX_NAME_LEN);
            if(mProjectName.size()<MAX_NAME_LEN)
            {
                quint32 counter = 0;
                foreach(QChar c,mProjectName)
                {
                    mSystemConfig.ProjectName[counter++] = c.toLatin1();
                }
                qDebug()<<(char*)mSystemConfig.ProjectName;
            }
            break;

        case 1:
            if(isUserPowerSet)
            {
                emit Info("Error:power settings and User-Defined power settings are NOT allowed at the same time");
                result<<false;
                break;
            }
            emit Info("Info:find power");
            result<<parsePower(data[i0]);
            isExSystemConfig = false;
            isDefaultPowerSet = true;
            break;

        case 2:
            emit Info("Info:find backlight");
            result<<parseBacklight(data[i0]);
            break;

        case 3:
            emit Info("Info:find lcd parameter");
            result<<parseLcdPara(data[i0]);
            break;

        case 4:
            if(mSystemConfig.LcdType != MIPI_LCD)
            {
                emit Info("Warning:NOT MIPI LCD");
                result<<true;
                break;
            }

            emit Info("Info:find mipi setting");
            result<<parseMipiSettings(data[i0]);
            break;

        case 5:
            emit Info("Info:find lcd intial");
            if((mSystemConfig.LcdType == MIPI_LCD) || (mSystemConfig.LcdType == RGB_SPI8BIT) || (mSystemConfig.LcdType == RGB_SPI9BIT)|| (mSystemConfig.LcdType == SPI_2_Data_Lane))
                result<<parseMipiOr8BitRGBLcdInit(data[i0]);
            else if(mSystemConfig.LcdType == RGB_SPI16BIT)
                result<<parseRGBLcdInit(data[i0]);
            else
                result<<true;
            break;

        case 6:
            emit Info("Info:find pattern setting");
            result<<parsePattern(data[i0]);
            break;

        case 7:
            result<<parseAutoRun(data[i0]);
            emit Info("Info:find auto run setting");
            break;

        case 8:
            result<<parseLcdType(data[i0]);
            emit Info("Info:find lcd type");
            break;

        case 9:
            if(isDefaultPowerSet)
            {
                emit Info("Error:power settings and power on settings are NOT allowed at the same time");
                result<<false;
                break;
            }
            result<<parsePowerOnSequence(data[i0]);
            isExSystemConfig = true;
            isUserPowerSet = true;
            emit Info("Info:find power on sequence");
            break;

        case 10:
            if(isDefaultPowerSet)
            {
                emit Info("Error:power settings and power off settings are NOT allowed at the same time");
                result<<false;
                break;
            }
            result<<parsePowerOffSequence(data[i0]);
            isExSystemConfig = true;
            isUserPowerSet = true;
            emit Info("Info:find power off sequence");
            break;

        case 11:
            isExSystemConfig = true;
            result<<parseFontScale(data[i0]);
            emit Info("Info:find font scale");
            break;

        default:
            break;
        }

        i0++;
    }


    uint16_t parameters = isExSystemConfig?(mTitleStr.size()-1):(mTitleStr.size()-2);
    int reserveBytes = 0;//兼容以前的配置。模板中没有配置的，从exsystemconfig移除这些字节。
    if(isExSystemConfig)
    {
        if(exSystemConfig.FontScale==0)
        {
            reserveBytes = 1;
        }
        result<<true;
    }

    if(result.size()<parameters)
    {
        emit Info("Error:compile failed,missing parameters");
        return false;
    }

    foreach(bool r,result)
    {
        if (r == false)
        {
            emit Info("Error:compile failed");
            return false;
        }
    }

    mCompiledPara.clear();


    //data
    quint8 *p = (quint8 *)&mSystemConfig;
    for (int i = 0; i < sizeof(ConfigTypeDef); i++)
    {
        mCompiledPara << *p++;
    }

    if(isExSystemConfig)
    {
        p = (quint8 *)exSystemConfig.PowerOnSequence;
        for(int i=0;i<sizeof(UserConfigTypeDef) - sizeof(ConfigTypeDef)-reserveBytes;i++)
        {
            mCompiledPara<<*p++;
        }
    }
    qDebug()<<mCompiledPara.size();

    DataToSerial.clear();

    foreach(quint8 temp, mCompiledPara)
    {
        DataToSerial.append((char)temp);
    }

    //补齐
    quint8 lenMod = (DataToSerial.size() + 2) % 4;
    for (int i = 0; i < 4 - lenMod; i++)
    {
        DataToSerial.append(0xff);
    }


    emit Info("OK:compile success\n");
    return true;
}
