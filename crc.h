#ifndef CRC_H
#define CRC_H
#include <QByteArray>

class crc
{
public:
  crc();
  quint32 calculateCRC32(const QByteArray& ptr, quint32 poly = 0x04c11db7);
  quint32 calculateCRC32(const QList<quint32>& pt, quint32 poly = 0x04c11db7);
  bool appendCrc(QByteArray &data);
};

#endif // CRC_H
