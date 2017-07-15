#ifndef HEXPARSE_H
#define HEXPARSE_H
#include "QList"
#include "QString"
#include "QFile"
#include "QDebug"

typedef unsigned int uint32_t;

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


class EncryptHex
{

private:
    bool asciiToByte(const QByteArray &data);
    bool isHexRight();
    QList<quint8>mByte;

public:
    EncryptHex();
    bool Encrypt(QString filepath);
};

#endif // HEXPARSE_H
