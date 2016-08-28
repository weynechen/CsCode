#include "codeparse.h"
#include <QDebug>
#include "QTextStream"
#include "crc.h"
#include "QTime"

codeParse::codeParse(QObject *parent):QObject(parent),power(0),backlight(0),maxCurrent(150)
{
    titleStr<<"project name"<<"power"<<"backlight"<<"LCD parameter"<<"MIPI setting"<<"LCD initial code"<<"pattern"<<"auto run";
    powerStr<<"1.8V"<<"2.8V"<<"3.3V"<<"5V"<<"-5V";
    lcdParaStr<<"pix clock"<<"horizontal resolution"<<"vertical resolution"<<"horizontal back porch"
             <<"horizontal front porch"<<"horizontal sync pulse width"<<"vertical back porch"<<"vertical front porch"
            <<"vertical sync pulse width";
    lcdInit<<"package"<<"write"<<"delay"<<"read";

    //初始化SystemConfig
    QTime t;
    t= QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    quint8 *p = (quint8 *)&SystemConfig;
    for(int i = 0 ; i < sizeof(SystemConfig) ; i ++)
    {
        *(p+i) = qrand()%0xff;
    }
}

bool codeParse::parseProjectName(QString data)
{
    data.remove(QRegExp("\n"));
    data.remove(QRegExp("\\s+$"));
    projectName=data;

    emit Info(QString("project Name:%1").arg(data));
    return true;
}

bool codeParse::parsePower(QString data)
{
    if(data.isEmpty())
        return false;
    data.replace(QRegExp("\\n+"),"\n");
    QRegExp rx("-?\\d.?\\d?V");
    QTextStream ts(&data);
    QString strLine;
    SystemConfig.PowerSettings=0;
    while(!ts.atEnd())
    {
        strLine=ts.readLine();
        strLine.remove(QRegExp("\\s+"));//删除空格
        bool match=rx.exactMatch(strLine);
        if(match)
        {

            switch(powerStr.indexOf(QRegExp(strLine)))
            {
            case 0:
                SystemConfig.PowerSettings |=0x01;

                break;
            case 1:
                SystemConfig.PowerSettings |=0x2;
                break;
            case 2:
                SystemConfig.PowerSettings |=0x4;
                break;
            case 3:
                SystemConfig.PowerSettings |= 0x8;
                break;
            case 4:
                SystemConfig.PowerSettings |=0x10;
                break;
            default:

                emit Info("Error:power error");
                return false;
            }


        }
        else
        {
            emit Info("Error:power error");
            return false;

        }
    }
    return true;
}

bool codeParse::parseBacklight(QString data)
{
    if(data.isEmpty())
        return false;
    data.remove("\n");
    data.remove(QRegExp("\\s+"));
    QRegExp rx("\\d+");
    int count=rx.indexIn(data);
    if(count!=0)
    {

        emit Info("Error:backlight para setting error")  ;
        return false;
    }
    bool ok;

    SystemConfig.Backlight=rx.cap(0).toInt(&ok,10);

    if(ok)
    {
        if(SystemConfig.Backlight<=maxCurrent)
        {

            return true;
        }
    }
    emit Info("Error:backliaght parameter setting error");

    return false;
}

bool codeParse::parseLcdPara(QString data)
{
    if(data.isEmpty())
    {
        emit Info("Error:no lcd parameter found");
        return false;
    }

    data.replace(QRegExp("\\n\\n+"),"\n");

    QTextStream ts(&data);
    QString strLine;
    QList<quint16> lcdPara;

    foreach(QString s,lcdParaStr)
    {
        if(data.indexOf(s)==-1)
        {
            emit Info("Error:lcd parameter setting error")  ;
            return false;
        }
        else
        {
            strLine=ts.readLine();
            strLine.remove(s);
            strLine.remove(":");
            strLine.remove(QRegExp("\\s"));

            QRegExp rxDec("\\b\\d+\\b");
            QRegExp rxHex("\\b0x[0-9A-Fa-f]+\\b");
            bool result = rxDec.exactMatch(strLine);
            if(result)
            {
                lcdPara<<strLine.toInt();
            }
            else
            {
                result=rxHex.exactMatch(strLine);
                if(result)
                {
                    bool ok;
                    strLine.remove("0x");
                    lcdPara<<strLine.toInt(&ok,16);
                }
                else
                {
                    emit Info("Error:lcd parameter setting error")  ;
                    return false;
                }
            }
        }
    }

    quint16 count = 0;
    foreach(quint16 l,lcdPara)
    {
        SystemConfig.LCDTimingPara[count++] = (quint8)(l>>8);
        SystemConfig.LCDTimingPara[count++] = (quint8)(l&0xff);

    }

    return true;
}

bool codeParse::parseLcdInit(QString data)
{
    QRegExp rxDec("\\d+");
    QRegExp rxHex("0x[0-9A-Fa-f]+");
    bool ok;

    QList<quint8> lcdInitPara;

    QStringList strList=data.split(QRegExp("\n+"),QString::SkipEmptyParts);
    foreach(QString s,strList)
    {
        QRegExp rxSpace("\\s+");
        if(rxSpace.exactMatch(s))
            continue;
        s.remove("\t");
        if((s.contains(QRegExp("\\bpackage\\s*=\\s*"))) || (s.contains(QRegExp("^write\\s+"))) \
                || (s.contains(QRegExp("^read\\s+"))) || (s.contains(QRegExp("^delay\\s+"))))
        {

        }
        else
        {
            emit Info("Error:lcd inlitial code syntax error");
        }

        //处理封包
        if(s.contains(QRegExp("\\bpackage\\s*=\\s*")))
        {
            if(s.contains("DCS"))
            {
                lcdInitPara<<MIPI_DCS;
            }
            else if(s.contains("GP"))
            {
                lcdInitPara<<MIPI_GP;
            }
            else
            {
                emit Info("Error:lcd package setting error")  ;
                return false;
            }
        }

        //处理写
        if(s.contains(QRegExp("^write\\s+")))
        {
            s.remove(QRegExp("^write\\s+"));
            s.replace(QRegExp("\\s\\s+")," ");
            QStringList sList=s.split(" ",QString::SkipEmptyParts);
            lcdInitPara<<MIPI_WRITE<<sList.size();
            foreach(QString sp,sList)
            {
                if(rxDec.exactMatch(sp))
                {
                    lcdInitPara<<sp.toInt();
                }
                else if(rxHex.exactMatch(sp))
                {
                    sp=sp.remove("0x");
                    lcdInitPara<<sp.toInt(&ok,16);
                }
                else
                {
                    emit Info("Error:lcd write para setting error")  ;
                    return false;
                }
            }
        }

        //处理读
        if(s.contains(QRegExp("^read\\s+")))
        {
            s.remove(QRegExp("^read\\s+"));
            s.replace(QRegExp("\\s\\s+")," ");
            QStringList sList=s.split(" ",QString::SkipEmptyParts);
            if(sList.size()!=2)
            {

                emit Info("Error:lcd read para setting error")  ;
                return false;
            }
            lcdInitPara<< MIPI_READ;

            foreach(QString sp,sList)
            {
                if(rxDec.exactMatch(sp))
                {
                    lcdInitPara<<sp.toInt();
                }
                else if(rxHex.exactMatch(sp))
                {
                    sp=sp.remove("0x");
                    lcdInitPara<<sp.toInt(&ok,16);
                }
                else
                {
                    emit Info("Error:lcd read para setting error")  ;
                    return false;
                }
            }
        }

        //处理延时
        if(s.contains(QRegExp("^delay\\s+")))
        {
            s.remove(QRegExp("^delay\\s+"));
            lcdInitPara<<MIPI_DELAY;

            if(rxDec.exactMatch(s.remove(QRegExp("\\s*"))))
            {
                lcdInitPara<<(quint8)(s.toInt()>>8);
                lcdInitPara<<(quint8)(s.toInt() &0xff);
            }
            else if(rxHex.exactMatch(s.remove(QRegExp("\\s*"))))
            {
                s=s.remove("0x");
                lcdInitPara<<(quint8)(s.toInt(&ok,16)>>8);
                lcdInitPara<<(quint8)(s.toInt(&ok,16) &0xff);
            }
            else
            {
                emit Info("Error:lcd delay para setting error");
                return false;
            }
        }
    }
    quint16 initCodeSize = lcdInitPara.size();
    SystemConfig.LCDInitCode[0] = initCodeSize >> 8;
    SystemConfig.LCDInitCode[1] = (quint8)initCodeSize;

    qDebug()<<initCodeSize;
    qDebug()<<lcdInitPara;
    if(initCodeSize > LCD_INIT_LEN)
    {
        emit Info("Error:too much lcd para");
        return false;
    }

    for(int i = 0 ;i<initCodeSize;i++)
    {
        SystemConfig.LCDInitCode[i+2] = lcdInitPara[i];
    }

    return true;
}

bool codeParse::parseMipi(QString data)
{
    bool ok;
    QRegExp rxDec("\\d+");
    QRegExp rxHex("0x[0-9A-Fa-f]+");
    QRegExp rxLaneDec("MIPI lane:\\s*[1-4]\\s*");
    QRegExp rxSpeedDec("MIPI speed:\\s*\\d+\\s*Mbps\\s*");
    QRegExp rxLaneHex("MIPI lane:\\s*0x[1-4]\\s*");
    QRegExp rxSpeedHex("MIPI speed:\\s*0x[0-9A-Fa-f]+\\s*Mbps\\s*");
    data.replace(QRegExp("\n\n+"),"\n");
    QStringList strList=data.split("\n",QString::SkipEmptyParts);
    uint mipiLane = 0,mipiSpeed = 0;

    foreach(QString str,strList)
    {
        if(rxLaneDec.exactMatch(str))
        {
            mipiLane=QString(str[str.indexOf(rxDec)]).toInt();

        }

        if(rxLaneHex.exactMatch(str))
        {
            str.indexOf(rxHex);
            QString s=rxHex.cap();
            mipiSpeed= s.remove("0x").toInt(&ok,16);

        }

        if(rxSpeedDec.exactMatch(str))
        {
            str.indexOf(rxDec);
            mipiSpeed=rxDec.cap().toInt();
        }

        if(rxSpeedHex.exactMatch(str))
        {
            str.indexOf(rxHex);
            QString s=rxHex.cap();
            mipiSpeed= s.remove("0x").toInt(&ok,16);
        }
    }

    if(mipiSpeed==0 || mipiLane ==0)
    {
        emit Info("Error:Mipi setting error");
        return false;
    }
    else
    {
        SystemConfig.MIPIConfig[0] = 3;
        SystemConfig.MIPIConfig[1] = mipiSpeed>>8;
        SystemConfig.MIPIConfig[2] = mipiSpeed & 0xff;
        SystemConfig.MIPIConfig[3] = mipiLane & 0xff;
        return true;
    }
}

bool codeParse::parsePattern(QString data)
{
    bool ok;
    QList<quint8> pattern;
    QStringList strList = data.split("\n",QString::SkipEmptyParts);

    foreach(QString s,strList)
    {
        if(s.contains(QRegExp("^RGB\\s*\\(.*\\)\\s*")))
        {
            pattern<<RGB;
            QRegExp rx("^RGB\\s*\\((.*),(.*),(.*)\\)");
            if(s.indexOf(rx)!=-1)
            {
                pattern<<rx.cap(1).toInt(&ok,0)<<rx.cap(2).toInt(&ok,0)<<rx.cap(3).toInt(&ok,0);
            }
            else
            {
                emit Info("Error:pattern setting error");
                return false;
            }

        }

        if(s.contains(QRegExp("^stay\\s+")))
        {
            pattern<<PATTERN_STAY;
            quint16 p=s.remove(QRegExp("stay")).remove(QRegExp("\\s*")).toInt(&ok,0);
            pattern<<(p>>8);
            pattern<<(p&0xff);
            if(ok==false)
            {

                emit Info("Error:pattern setting error");
                return false;
            }
        }

        if(s.contains(QRegExp("^PIC\\s+")))
        {
            pattern<<PIC;
            s=s.remove(QRegExp("^PIC\\s+"));
            s=s.remove('"');
            pattern<<s.size();
            for(int i=0;i<s.size();i++)
            {
                pattern<<(uint)s.at(i).toLatin1();
            }
        }

        if(s.contains(QRegExp("^horizontal colorbar\\s*")))
        {
            pattern<<COLORBARH;
        }

        if(s.contains(QRegExp("^frame\\s*")))
        {
            pattern<<FRAME;
        }

        if(s.contains(QRegExp("^vertical colorbar\\s*")))
        {
            pattern<<COLORBARV;
        }

        if(s.contains(QRegExp("^flicker\\s*")))
        {
            pattern<<FLICKERH;
        }

        if(s.contains(QRegExp("^vertical gray level\\s*")))
        {
            pattern<<GRAYLEVEL_V;
        }
        if(s.contains(QRegExp("^horizontal gray level\\s*")))
        {
            pattern<<GRAYLEVEL_H;
        }
        if(s.contains(QRegExp("^crosstalk\\s*")))
        {
            pattern<<CROSSTALK;
        }

        if(s.contains(QRegExp("^chessboard\\s*")))
        {
            pattern<<CHESSBOARD;
        }

        if(s.contains(QRegExp("^rgbbar\\s*")))
        {
            pattern<<RGBBAR;
        }

        if(s.contains(QRegExp("^rgblevel\\s*")))
        {
            pattern<<RGBLEVEL;
        }

        if(s.contains(QRegExp("^null pattern\\s*")))
        {
            pattern<<NULL_PATTERN;

            if(pattern[0]=(quint8)NULL_PATTERN)
            {
                emit Info(QStringLiteral("Error:请把null pattern 放在第一个"));
                return false;
            }
            else
                true;
        }
    }

    SystemConfig.Pattern[0] = pattern.size();
    for(int i =0 ;i<pattern.size();i++)
    {
        SystemConfig.Pattern[i+1] = pattern[i];
    }

    return true;
}

bool codeParse::parseAutoRun(QString data)
{
    if(data.isEmpty())
        return false;
    data.remove("\n");
    data.remove(QRegExp("\\s+"));
    if(data == "NO")
    {
        SystemConfig.IsAutoRun=0;
        return true;
    }
    else if (data == "YES")
    {
        SystemConfig.IsAutoRun=1;
        return true;
    }
    return false;

}

void codeParse::updateStr(QString &str)
{
    strToParse=str;
}

bool codeParse::compile()
{
    QString str=strToParse;
    if(str.isEmpty())
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
    segments=str.split(QRegExp("\\["));//按标题[分割
    segments.removeFirst();//第一个是"["之前的内容，移除

    foreach(QString s,segments)
    {
        if(s.isEmpty()||(s=="\n"))
        {
            continue;
        }
        else
        {
            title<<s.section("]\n",0,0);//提取段落头
            data<<s.section("]\n",1,1);//提取段落中的内容
        }
    }

    int i0=0;
    bool result[10];
    for(int i=0;i<10;i++)
        result[i]=false;

    //根据title选择不同的解析函数
    foreach(QString s,title)
    {
        switch(titleStr.indexOf(QRegExp(s)))
        {
        case 0:
            emit Info("Info:find project Name");
            result[0]=parseProjectName(data[i0]);
            break;

        case 1:
            emit Info("Info:find power");
            result[1]=parsePower(data[i0]);
            break;

        case 2:
            emit Info("Info:find backlight");
            result[2]=parseBacklight(data[i0]);
            break;

        case 3:
            emit Info("Info:find lcd parameter");
            result[3]=parseLcdPara(data[i0]);
            break;

        case 4:
            emit Info("Info:find mipi setting");
            result[4]= parseMipi(data[i0]);
            break;

        case 5:
            emit Info("Info:find lcd intial");
            result[5]=parseLcdInit(data[i0]);
            break;

        case 6:
            emit Info("Info:find pattern setting");
            result[6]=parsePattern(data[i0]);
            break;

        case 7:
            result[7]= parseAutoRun(data[i0]);
            emit Info("Info:find auto run setting");
            break;

        default:
            break;
        }

        i0++;
    }

    for(int i=0;i<8;i++)
    {
        if(result[i]==false)
        {
            emit Info("Error:compile failed");
            return false;
        }
    }

    CompiledPara.clear();

    //interface number
    CompiledPara<<(char)IF_UART1;

    //ID
    CompiledPara<<RE_INIT_START;

    //data
    quint8 *p = (quint8 *)&SystemConfig;
    for(int i = 0 ; i< sizeof(ConfigTypeDef) ; i++)
    {
        CompiledPara<<*p++;
    }

    dataToSerial.clear();

    foreach (quint8 temp, CompiledPara) {
        dataToSerial.append((char)temp);
    }

    //补齐
    quint8 lenMod = (dataToSerial.size() + 2) % 4;
    for(int i=0;i<4-lenMod;i++)
    {
        dataToSerial.append(0xff);
    }


    //data len
    quint16 len = dataToSerial.size() + 2;
    dataToSerial.prepend((char)(len &0xff));
    dataToSerial.prepend((char)((len >> 8) & 0xff));

    //crc32
    crc config_crc;
    quint32 crc32 = config_crc.crctablefast(dataToSerial);

    for(int i = 4 ;i >0 ;i --)
    {
        dataToSerial.append((char)(crc32>>(i-1)*8) & 0xff);
    }

    emit Info("OK:compile success\n");
    return true;
}

