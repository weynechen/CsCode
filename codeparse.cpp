#include "codeparse.h"
#include <QDebug>
#include "QTextStream"
#include "crc.h"
#include "QTime"

CodeParse::CodeParse(QObject *parent) : QObject(parent), mPower(0), mBacklight(0), mMaxCurrent(150)
{
    mTitleStr << "project name" << "power" << "backlight" << "LCD parameter" << "MIPI setting" << "LCD initial code" << "pattern" << "auto run"<<"lcd type";
    mPowerStr << "1.8V" << "2.8V" << "3.3V" << "VSP" << "VSN"<<"5V"<<"MTP";
    mLcdParaStr << "pix clock" << "horizontal resolution" << "vertical resolution" << "horizontal back porch"
               << "horizontal front porch" << "horizontal sync pulse width" << "vertical back porch" << "vertical front porch"
               << "vertical sync pulse width";
    mLcdInit << "package" << "write" << "delay" << "read";

    //初始化SystemConfig
    QTime t;
    t = QTime::currentTime();
    qsrand(t.msec() + t.second() * 1000);
    quint8 *p = (quint8 *)&mSystemConfig;
    for (int i = 0; i < sizeof(mSystemConfig); i++)
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

    return true;
}


bool CodeParse::parseMipiLcdInit(QString data)
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
            if (s.contains("DCS"))
            {
                lcdInitPara << MIPI_DCS;
            }
            else if (s.contains("GP"))
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


bool CodeParse::parseMipi(QString data)
{
    bool ok;
    QRegExp rxDec("\\d+");
    QRegExp rxHex("0x[0-9A-Fa-f]+");
    QRegExp rxLaneDec("MIPI lane:\\s*[1-4]\\s*");
    QRegExp rxSpeedDec("MIPI speed:\\s*\\d+\\s*Mbps\\s*");
    QRegExp rxLaneHex("MIPI lane:\\s*0x[1-4]\\s*");
    QRegExp rxSpeedHex("MIPI speed:\\s*0x[0-9A-Fa-f]+\\s*Mbps\\s*");

    data.replace(QRegExp("\n\n+"), "\n");
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
        return true;
    }
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
            pattern << 0;
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

        if (s.contains(QRegExp("^vertical gray level\\s*")))
        {
            pattern << GRAYLEVEL_V;
        }
        if (s.contains(QRegExp("^horizontal gray level\\s*")))
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
    else if (data == "RGB")
    {
        mSystemConfig.LcdType = RGB_LCD;
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
    bool result[10];
    for (int i = 0; i < 10; i++)
    {
        result[i] = false;
    }

    //根据title选择不同的解析函数
    foreach(QString s, title)
    {
        switch (mTitleStr.indexOf(QRegExp(s)))
        {
        case 0:
            emit Info("Info:find project Name");
            result[0] = parseProjectName(data[i0]);
            break;

        case 1:
            emit Info("Info:find power");
            result[1] = parsePower(data[i0]);
            break;

        case 2:
            emit Info("Info:find backlight");
            result[2] = parseBacklight(data[i0]);
            break;

        case 3:
            emit Info("Info:find lcd parameter");
            result[3] = parseLcdPara(data[i0]);
            break;

        case 4:
            if(mSystemConfig.LcdType != MIPI_LCD)
            {
                result[4] = true;
                break;
            }

            emit Info("Info:find mipi setting");
            result[4] = parseMipi(data[i0]);
            break;

        case 5:
            emit Info("Info:find lcd intial");
            if(mSystemConfig.LcdType == MIPI_LCD)
                result[5] = parseMipiLcdInit(data[i0]);
            else if(mSystemConfig.LcdType == RGB_LCD)
                result[5] = parseRGBLcdInit(data[i0]);
            else
                result[5] = false;
            break;

        case 6:
            emit Info("Info:find pattern setting");
            result[6] = parsePattern(data[i0]);
            break;

        case 7:
            result[7] = parseAutoRun(data[i0]);
            emit Info("Info:find auto run setting");
            break;

        case 8:
            result[8] = parseLcdType(data[i0]);
            emit Info("Info:find lcd type");

        default:
            break;
        }

        i0++;
    }

    for (int i = 0; i < mTitleStr.size(); i++)
    {
        if (result[i] == false)
        {
            emit Info("Error:compile failed");
            return false;
        }
    }

    mCompiledPara.clear();

    qDebug()<<hex<<sizeof(ConfigTypeDef);

    //data
    quint8 *p = (quint8 *)&mSystemConfig;
    for (int i = 0; i < sizeof(ConfigTypeDef); i++)
    {
        mCompiledPara << *p++;
    }

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

    qDebug()<<DataToSerial.size();

    emit Info("OK:compile success\n");
    return true;
}
