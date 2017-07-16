#include "encrypthex.h"
#include "crc.h"
#include "QDataStream"

EncryptHex::EncryptHex(QObject *parent):QObject(parent)
{
    RandomNum[0] = 0x03020100;
    RandomNum[1] = 0x07060504;
    RandomNum[2] = 0x0B0A0908;
    RandomNum[3] = 0x0F0E0D0C;
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

void EncryptHex::calSectionInfo()
{
    uint32_t mod = 0;
    crc crc32;

    if(!mSectionData.isEmpty())
    {
        /*calculate len and real len*/
        mSectionData.last()->RealLen = mSectionData.last()->Data.size()*4;
        mod = mSectionData.last()->Data.size()%4;

        if(mod !=0)
        {
            for(quint32 i=0;i<4-mod;i++)
            {
                mSectionData.last()->Data.append(0xffffffff);
            }
        }

        mSectionData.last()->Len = (mSectionData.last()->Data.size())*4;
        //test
        //mSectionData.last()->RealLen = mSectionData.last()->Len;

        mSectionData.last()->CRC32 = crc32.calculateCRC32(mSectionData.last()->Data);
        /*calculate crc of section data*/


    }
}

void EncryptHex::newSection()
{
    uint32_t address = 0;
    mSectionData <<new HexSection();
    address = 0;
    address = (mByte[4]<<8)| mByte[5];
    address <<=16;
    mSectionData.last()->Address = address;
}

bool EncryptHex::getSectionData()
{
    uint32_t dataLen;
    uint32_t i;
    uint32_t data;
    uint32_t address;

    if(mSectionData.isEmpty())
    {
        return false;
    }

    dataLen = mByte[0];

    if(dataLen%4!=0)
    {
        qDebug()<<"hex data len error";
        return false;
    }

    if(mSectionData.last()->Data.isEmpty())
    {
        address = 0;
        address = (mByte[1]<<8)| mByte[2];
        mSectionData.last()->Address |= address;
    }

    for(i=4;i<4+dataLen;i+=4)
    {
        data = ((uint32_t)mByte[i+3]<<24)| ((uint32_t)mByte[i+2]<<16)| ((uint32_t)mByte[i+1]<<8)| (uint32_t)mByte[i];
        mSectionData.last()->Data<<data;
    }

    return true;
}

bool EncryptHex::encryptAllFile()
{
    if(mSectionData.isEmpty())
        return false;


    /*save to file */
    QFile encrypt_file(ENCRYPT_NAME);
    crc crc32;

    if(!mEncrytData.isEmpty())
        mEncrytData.clear();

    if(encrypt_file.open(QIODevice::WriteOnly) == true)
    {
        qDebug()<<"write encrypt file";

        foreach(HexSection *p,mSectionData)
        {
            mEncrytData<<p->Len<<p->RealLen<<p->Address<<p->CRC32;
            foreach(uint32_t t,p->Data)
            {
                mEncrytData<<t;
            }
        }
        mEncrytData.prepend(RandomNum[3]);
        mEncrytData.prepend(RandomNum[2]);
        mEncrytData.prepend(RandomNum[1]);
        mEncrytData.prepend(RandomNum[0]);
        mEncrytData.prepend(mSectionData.size());
        mEncrytData.prepend(0);
        mEncrytData.prepend((mEncrytData.size()+2)*4);

        uint32_t crc_data = crc32.calculateCRC32(mEncrytData);

        mEncrytData.append(crc_data);

        qDebug()<<mEncrytData.size();

        QDataStream out(&encrypt_file);
        out.setByteOrder(QDataStream::LittleEndian);
        foreach(uint32_t t,mEncrytData)
        {
            out<<t;
        }

        encrypt_file.close();

    }

    if(!mEncrytData.isEmpty())
        mEncrytData.clear();
    if(!mSectionData.isEmpty())
        mSectionData.clear();

    /*delet heap*/
    foreach(HexSection *p , mSectionData)
    {
        qDebug()<<hex<<p->Address;
        delete p;
    }

    return true;
}

bool EncryptHex::encryptSection(QString filepath)
{
    QFile file(filepath);
    if(file.open(QIODevice::ReadOnly|QFile::Text) == false)
    {
        return false;
    }

    mSectionData.clear();

    while(!file.atEnd())
    {

        QByteArray line = file.readLine();

        if(line.isEmpty())
        {
            file.close();
            return false;
        }
        if(line.at(0)!=':')
        {
            file.close();
            return false;
        }

        line.remove(0,1);

        if(line.endsWith('\n'))
            line.remove(line.size()-1,1);

        if(asciiToByte(line) == false)
        {
            file.close();
            return false;
        }

        if(isHexRight() == false)
        {
            file.close();
            return false;
        }

        switch(mByte[3])
        {
        case END_RECORD:
            calSectionInfo();
            break;

        case EXTEND_LINEAR_ADDRESS:
            calSectionInfo();
            newSection();
            break;

        case DATA_RECORD:
            if(getSectionData() == false)
            {
                file.close();
                return false;
            }
            break;

        default:
            break;
        }
    }

    file.close();
    qDebug()<<mSectionData.size();

    return true;
}


bool EncryptHex::Encrypt(QString file_path)
{

    if(encryptSection(file_path) ==false)
        return false;

    if(encryptAllFile() ==false)
        return false;

    return true;
}

