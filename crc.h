#ifndef CRC_H
#define CRC_H
#include <QByteArray>

class crc
{
public:
    crc();
    quint32 crctablefast(const QByteArray &ptr, quint32 poly = 0x04c11db7);
};

#endif // CRC_H
