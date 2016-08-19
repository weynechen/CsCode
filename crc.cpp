#include "crc.h"
#include "QList.h"
crc::crc()
{

}


quint32 crc::crctablefast(const QByteArray &pt, quint32 poly)
{
    quint32 xbit;
    quint32 data;
    quint32 dwPolynomial = poly;
    quint32 CRC = 0xFFFFFFFF;    // init

    //转换QByteArray to uint
    QList<quint32> ptr;
    quint32 temp[4]={0,0,0,0};
    for(int i=0;i<pt.size();i+=4)
    {
        for(int j=0;j<4;j++)
        {
            temp[j] = (pt[i+j]<<j*8)&(0xff<<(j*8));
        }

        ptr<<(quint32)(temp[0] |temp[1] | temp[2] | temp[3]);
    }

    int len = ptr.size();
    int pos = 0;
    while (len-- > 0)
    {
        xbit = (quint32)0x01 << 31;

        data = ptr[pos++];
        for (int bits = 0; bits < 32; bits++)
        {
            if ((CRC & 0x80000000) != 0)
            {
                CRC <<= 1;
                CRC ^= dwPolynomial;
            }
            else
                CRC <<= 1;
            if ((data & xbit) != 0)
                CRC ^= dwPolynomial;

            xbit >>= 1;
        }
    }
    return CRC;
}
