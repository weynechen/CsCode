#ifndef HEXPARSE_H
#define HEXPARSE_H
#include "QList"
#include "QString"
#include "QFile"
#include "QDebug"

typedef unsigned int uint32_t;

#define ENCRYPT_NAME "tmp.cfw"

struct HexSection
{
    uint32_t Len;
    uint32_t RealLen;
    uint32_t Address;
    uint32_t CRC32;
    QList<uint32_t>Data;
};


typedef enum
{
    DATA_RECORD = 0,
    END_RECORD,
    EXTEND_ADDRESS,
    START_ADDRESS,
    EXTEND_LINEAR_ADDRESS,
    START_LINEAR_ADDRESS,
}HexDataType;


class EncryptHex : public QObject
{
      Q_OBJECT

private:
    bool asciiToByte(const QByteArray &data);
    bool isHexRight();

    void calSectionInfo();
    void newSection();
    bool getSectionData();
    bool encryptAllFile();
    bool encryptSection(QString filepath);

    QList<quint8>mByte;
    uint32_t RandomNum[4];
    QList<quint32>mEncrytData;
    QList<HexSection*>mSectionData;
public:
    EncryptHex(QObject *parent);
    bool Encrypt(QString filepath);
};

#endif // HEXPARSE_H
