/**
 * @file        pro.h
 * @author      陈维
 * @version     V01
 * @date        2016.09.21
 * @brief       协议定义
 * @note
 *
 * @attention   COYPRIGHT WEYNE
 **/

#ifndef _PRO_H_
#define _PRO_H_

#define u8               unsigned char
#define u16              unsigned short
#define u32              unsigned int
#define s16              short
#define LCD_ADDRESS    0x10
#define PARSE_LEN           2048    //>1036
#define MIN_PRO_NUM      14

typedef enum
{
    PACK_FAIL,
    PACK_OK
} ResultTypeDef;

#define NULL    0


typedef struct
{
    u8  DeviceAddr;   //  cmd
    u8  FunctionCode; // sub cmd
    u16 StartAddr;    //sub sub cmd
    u32 Len;
} SdkProtocolHeaderTypeDef;


//数据包头尾、控制字
#define P_HEADER     0xAA
#define P_TAIL       0x55
#define P_CTRL       0xA5
#define P_FAIL       0
#define P_SUCCESS    1


typedef enum
{
    ACK_STRING,
        ACK_UPGRADE,
    ACK_NULL = 0xff,
}AckDataIDTypeDef;

typedef enum {
  FW_UPGRADE_READY,
  FW_CRC_ERROR,
  FW_FLASH_ERROR,
  FW_OK,
  FW_NULL = 0xff,
} FirmwareUpgradeType;


typedef struct
{
    u8               DataID;
    u8               *DataInBuff;
    u32              DataInLen;
    u8               *DataOutBuff;
    u32              *DataOutLen;
} PackageDataStruct;


#endif



/************************ (C) COPYRIGHT WEYNE *****END OF FILE****/
