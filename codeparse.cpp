#include "codeparse.h"
#include <QDebug>
#include "QTextStream"
codeParse::codeParse(QObject *parent):QObject(parent),power(0),backlight(0),maxCurrent(150)
{
    titleStr<<"project name"<<"power"<<"backlight"<<"LCD parameter"<<"MIPI setting"<<"LCD initial code"<<"pattern"<<"auto run";
    powerStr<<"1.8V"<<"2.8V"<<"3.3V"<<"5V"<<"-5V";
    lcdParaStr<<"pix clock"<<"horizontol resolution"<<"vertical resolution"<<"horizontol back porch"
             <<"horizontol front porch"<<"horizontol sync pulse width"<<"vertical back porch"<<"vertical front porch"
            <<"vertical sync pulse width";
    lcdInit<<"package"<<"write"<<"delay"<<"read";
}

bool codeParse::parseProjectName(QString data)
{
    data.remove(QRegExp("\n"));
    data.remove(QRegExp("\\s+$"));
    projectName=data;
    //qDebug()<<data;
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
    power=0;
    while(!ts.atEnd())
    {
        strLine=ts.readLine();
        strLine.remove(QRegExp("\\s+"));//删除空格
        bool match=rx.exactMatch(strLine);
        if(match)
        {
            // qDebug()<<"match"<<strLine;
            switch(powerStr.indexOf(QRegExp(strLine)))
            {
            case 0:
                power |=0x01;

                break;
            case 1:
                power |=0x2;
                break;
            case 2:
                power |=0x4;
                break;
            case 3:
                power |= 0x8;
                break;
            case 4:
                power |=0x10;
                break;
            default:
                //qDebug()<<"power error!";
                emit Info("Error:power error");
                return false;
                //  break;
            }
            //    qDebug()<<power;

        }
        else
        {
            emit Info("Error:power error");
            return false;
            //            qDebug()<<"No match";
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
        // qDebug("error backlight para");
        emit Info("Error:backlight para setting error")  ;
        return false;
    }
    bool ok;

    backlight=rx.cap(0).toInt(&ok,10);

    if(ok)
    {
        if(backlight<=maxCurrent)
        {
            //  qDebug()<<backlight;
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
    lcdPara.clear();


    foreach(QString s,lcdParaStr)
    {
        if(data.indexOf(s)==-1)
        {
            // qDebug()<<"lcd parameter setting error,not found"<<s;
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

            //qDebug()<<"lcd para"<<strLine;
            //if()
            // lcdPara<<strLine.toInt();
        }
    }
    //qDebug()<<data;
    return true;
}

bool codeParse::parseLcdInit(QString data)
{
    QRegExp rxDec("\\d+");
    QRegExp rxHex("0x[0-9A-Fa-f]+");

    bool ok;
    //    lcdInit<<"package"<<"write"<<"delay"<<"read";
    // qDebug()<<data;
    lcdInitPara.clear();

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
                lcdInitPara<<0xAA<<0xAA<<MIPI_DCS<<0x55<<0x55;
            }
            else if(s.contains("GP"))
            {
                lcdInitPara<<0xAA<<0xAA<<MIPI_GP<<0x55<<0x55;
            }
            else
            {
                // qDebug()<<"error";
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
            lcdInitPara<<0xAA<<0xAA<<MIPI_WRITE<<sList.size();
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
            lcdInitPara<<0x55<<0x55;
        }
        //处理读
        if(s.contains(QRegExp("^read\\s+")))
        {
            s.remove(QRegExp("^read\\s+"));
            s.replace(QRegExp("\\s\\s+")," ");
            QStringList sList=s.split(" ",QString::SkipEmptyParts);
            if(sList.size()!=2)
            {
                // qDebug()<<"lcd read para setting error";
                emit Info("Error:lcd read para setting error")  ;
                return false;
            }
            lcdInitPara<<0xAA<<0xAA<<MIPI_READ;

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
                    //   qDebug()<<"lcd read para setting error";
                    emit Info("Error:lcd read para setting error")  ;
                    return false;
                }
            }
            lcdInitPara<<0x55<<0x55;

        }

        //处理延时
        if(s.contains(QRegExp("^delay\\s+")))
        {
            s.remove(QRegExp("^delay\\s+"));
            lcdInitPara<<0xAA<<0xAA<<MIPI_DELAY;

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
                // qDebug()<<"lcd delay para setting error";
                emit Info("Error:lcd delay para setting error");
                return false;
            }

            lcdInitPara<<0x55<<0x55;

        }



    }
    //qDebug()<<hex<<lcdPara;
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
    mipiLane=0;
    mipiSpeed=0;
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
        // qDebug()<<"mipiLane"<<mipiLane;
        // qDebug()<<"mipiSpeed"<<mipiSpeed;
        return true;
    }
}

bool codeParse::parsePattern(QString data)
{
    bool ok;
    pattern.clear();
    QStringList strList = data.split("\n",QString::SkipEmptyParts);
    foreach(QString s,strList)
    {
        if(s.contains(QRegExp("^RGB\\s*\\(.*\\)\\s*")))
        {
            pattern<<0xAA<<0xAA<<RGB;
            QRegExp rx("^RGB\\s*\\((.*),(.*),(.*)\\)");
            if(s.indexOf(rx)!=-1)
            {

                pattern<<rx.cap(1).toInt(&ok,0)<<rx.cap(2).toInt(&ok,0)<<rx.cap(3).toInt(&ok,0);

            }
            else
            {
                // qDebug()<<"pattern setting error nothing";
                emit Info("Error:pattern setting error");
                return false;
            }
            pattern<<0x55<<0x55;
        }

        if(s.contains(QRegExp("^stay\\s+")))
        {
            pattern<<0xAA<<0xAA<<PATTERN_STAY;
            quint16 p=s.remove(QRegExp("stay")).remove(QRegExp("\\s*")).toInt(&ok,0);
            pattern<<(p>>8);
            pattern<<(p&0xff);
            if(ok==false)
            {
                //  qDebug()<<"stay setting error";
                emit Info("Error:pattern setting error");
                return false;
            }
            pattern<<0x55<<0x55;
        }

        if(s.contains(QRegExp("^PIC\\s+")))
        {
            pattern<<0xAA<<0xAA<<PIC;
            s=s.remove(QRegExp("^PIC\\s+"));
            s=s.remove('"');
            pattern<<s.size();
            for(int i=0;i<s.size();i++)
            {
                pattern<<(uint)s.at(i).toLatin1();
            }
            pattern<<0x55<<0x55;
        }

        if(s.contains(QRegExp("^horizontol colorbar\\s*")))
        {
            pattern<<0xAA<<0xAA<<COLORBARH;
            pattern<<0x55<<0x55;
        }

        if(s.contains(QRegExp("^frame\\s*")))
        {
            pattern<<0xAA<<0xAA<<FRAME;
            pattern<<0x55<<0x55;
        }

        if(s.contains(QRegExp("^vertical colorbar\\s*")))
        {
            pattern<<0xAA<<0xAA<<COLORBARV;
            pattern<<0x55<<0x55;
        }

        if(s.contains(QRegExp("^flicker\\s*")))
        {
            pattern<<0xAA<<0xAA<<FLICKERH;
            pattern<<0x55<<0x55;
        }

        if(s.contains(QRegExp("^vertical gray level\\s*")))
        {
            pattern<<0xAA<<0xAA<<GRAYLEVEL_V;
            pattern<<0x55<<0x55;
        }
        if(s.contains(QRegExp("^horizontol gray level\\s*")))
        {
            pattern<<0xAA<<0xAA<<GRAYLEVEL_H;
            pattern<<0x55<<0x55;
        }
        if(s.contains(QRegExp("^crosstalk\\s*")))
        {
            pattern<<0xAA<<0xAA<<CROSSTALK;
            pattern<<0x55<<0x55;
        }
        if(s.contains(QRegExp("^null pattern\\s*")))
        {
            pattern<<0xAA<<0xAA<<NULL_PATTERN;
            pattern<<0x55<<0x55;

            if(pattern.size()!=5)
            {
                emit Info(QStringLiteral("Error:请把null pattern 放在第一个"));
                return false;
            }
            else
                true;
        }
    }
    // qDebug()<<hex<<pattern;
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
        autoRun=0;
        return true;
    }
    else if (data == "YES")
    {
autoRun=1;
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
        // qDebug()<<"empity";
        emit Info("Error:code is empity");
        return false;
    }
    //删除所有注释--删除正则表达式 "//[^\n]*" 标注的所有string
    str.remove(QRegExp("//[^\n]*"));
    //分割title和para
    QStringList segments;
    QStringList title;
    QStringList data;
    segments=str.split(QRegExp("\\["));//按标题[分割
    segments.removeFirst();//第一个是"["之前的内容，移除
    // qDebug()<<segments.size();

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
    //qDebug()<<title.size()<<data.size();

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
            //qDebug()<<data[i0];
            //  qDebug()<<"find project Name";
            emit Info("Info:find project Name");
            result[0]=parseProjectName(data[i0]);
            break;

        case 1:
            // qDebug()<<data[i0];
            // qDebug()<<"find power";
            emit Info("Info:find power");
            result[1]=parsePower(data[i0]);
            break;

        case 2:
            // qDebug()<<data[i0];
            // qDebug()<<"find backlight";
            emit Info("Info:find backlight");
            result[2]=parseBacklight(data[i0]);
            break;

        case 3:
            // qDebug()<<"find lcd parameter";
            emit Info("Info:find lcd parameter");
            result[3]=parseLcdPara(data[i0]);
            break;

        case 4:
            // qDebug()<<"MIPI setting";
            emit Info("Info:find mipi setting");
            result[4]= parseMipi(data[i0]);
            break;

        case 5:
            // qDebug()<<"LCD initial code";
            emit Info("Info:find lcd intial");
            result[5]=parseLcdInit(data[i0]);
            break;

        case 6:
            // qDebug()<<"pattern";
            emit Info("Info:find pattern setting");
            result[6]=parsePattern(data[i0]);
            break;

        case 7:
            result[7]= parseAutoRun(data[i0]);
            emit Info("Info:find auto run setting");
            //qDebug()<<"auto run";
            break;

        default:
            //  qDebug()<<"error title";
            break;
        }

        i0++;
    }

    for(int i=0;i<8;i++)
    {
        if(result[i]==false)
        {
            //  qDebug()<<"Error:compile failed";
            emit Info("Error:compile failed");
            return false;
        }
    }

    para.clear();

    //LCD initial begin
    para<<0xAA<<0xAA<<RE_INIT_START<<0x55<<0x55;

    //project name
    para<<0xAA<<0xAA<<PROJECT_NAME<<projectName.size();
    for(int i=0;i<projectName.size();i++)
    {
        para<<projectName[i].toLatin1();
    }
    para<<0x55<<0x55;

    //power
    para<<0xAA<<0xAA<<POWER<<power<<0x55<<0x55;

    //backlight
    para<<0xAA<<0xAA<<BACKLIGHT<<backlight<<0x55<<0x55;

    //lcd para
    para<<0xAA<<0xAA<<LCD_PARA;
    foreach(quint16 l,lcdPara)
    {
        para<<(quint8)(l>>8);
        para<<(quint8)(l&0xff);

    }
    para<<0x55<<0x55;

    //lcd initial code
    para<<0xAA<<0xAA<<LCD_INIT;
    quint16 amount=lcdInitPara.size();
    para<<(amount>>8)<<(amount&0xff);
    foreach(quint8 l,lcdInitPara)
    {
        para<<l;
    }
    para<<0x55<<0x55;

    //MIPI parameter
    para<<0xAA<<0xAA<<MIPI_PARA<<mipiLane<<(mipiSpeed>>8)<<(mipiSpeed&0xff)<<0x55<<0x55;

    //pattern
    para<<0xAA<<0xAA<<PATTERN;
    amount=pattern.size();
    para<<(amount>>8)<<(amount&0xff);
    foreach(quint8 l,pattern)
    {
        para<<l;
    }
    para<<0x55<<0x55;

    //pattern auto run
    para<<0xAA<<0xAA<<AUTO_RUN<<autoRun<<0x55<<0x55;

    //lcd intial end
    para<<0xAA<<0xAA<<RE_INIT_END<<0x55<<0x55;

    dataToSerial.clear();

    foreach (quint8 temp, para) {
        dataToSerial.append((char)temp);
    }


    emit Info("OK:compile success");
    return true;
}

