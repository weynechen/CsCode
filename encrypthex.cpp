#include "encrypthex.h"


EncryptHex::EncryptHex()
{

}


bool EncryptHex::asciiToByte(const QByteArray &data)
{
    mByte.clear();
    bool ok;

    if(data.size()%2!=0)
        return false;

    int i=0;
    while(i<data.size())
    {
        QString s = QString(data[i])+QString(data[i+1]);
        quint16 b = s.toInt(&ok,16);
        if(!ok)
            return false;
        mByte<<b;
        i+=2;
    }

    return true;
}

bool EncryptHex::isHexRight()
{
    quint8 crc = mByte[0];

    if(mByte.size()<2)
        return false;

    for(int i = 1;i<mByte.size() - 1;i++)
    {
        crc += mByte[i];
    }

    crc = 0x100 - crc;

    if(crc == mByte.last())
        return true;
    else
        return false;
}


bool EncryptHex::Encrypt(QString filepath)
{
    QFile file(filepath);
    if(file.open(QIODevice::ReadOnly|QFile::Text) == false)
    {
        return false;
    }

    while(!file.atEnd())
    {

        QByteArray line = file.readLine();

        if(line.isEmpty())
            return false;

        if(line.at(0)!=':')
            return false;


        line.remove(0,1);

        if(line.endsWith('\n'))
            line.remove(line.size()-1,1);

        if(asciiToByte(line) == false)
            return false;

        if(isHexRight() == false)
        {
            return false;
        }

        QList<HexSection*>section;
        int dataLen;
        int i;
        uint32_t crc32 = 0;
        switch(mByte[3])
        {
        case START_ADDRESS:
            section <<new HexSection();
            crc32 = (mByte[4]<<8)| mByte[5];
            crc32 <<=16;
            section.last()->Address = crc32;
            break;

        case DATA_RECORD:
            if(section.isEmpty())
                return false;

            dataLen = mByte[0];
            for(i=4;i<4+dataLen;i++)
            {
                section.last()->Data<<mByte[i];
            }
            break;

         default:
            break;
        }

        qDebug()<<line;
    }


    return true;
}

