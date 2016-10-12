/**
 * @file        rec.c
 * @author      陈维
 * @version     V01
 * @date        2016.09.21
 * @brief       解包
 * @note
 *
 * @attention   COYPRIGHT WEYNE
 **/
#include "rec.h"

static u8 ParseBuffer[PARSE_LEN];                    // 存放解包后的数据

/**
 * @brief  解包函数
 * @param  待解包数据指针
 * @retval 返回解包成功或者失败
  */
ResultTypeDef Unpacking(PackageDataStruct *package)
{
    u16 i = 0;

    if ((package->DataInBuff == NULL) && (package->DataInLen < MIN_PRO_NUM))
    {
        return PACK_FAIL;
    }

    if (package->DataInLen >= MIN_PRO_NUM)
    {
        if ((*(package->DataInBuff + package->DataInLen - 1) == P_TAIL) && (*(package->DataInBuff + package->DataInLen - 2) == P_TAIL))
        {
            i = MIN_PRO_NUM - 2;
            while (i++)
            {
                if (*(package->DataInBuff + package->DataInLen - i) == P_HEADER)
                {
                    if (*(package->DataInBuff + package->DataInLen - (i + 1)) == P_HEADER)
                    {
                        u8 *pbuff = package->DataInBuff + package->DataInLen - (i - 1); // pbuff指向DataInBuff有效数据起始位置(即AA AA 后面的一位)
                        u16 len = i - 3; //i 的长度为数据包(AA AA ... 55 55) 长度减一，len为有效数据长度(除去AA AA 55 55部分，包含checksum)
                        u16 j = 0;
                        u8 checksum = 0;
                        u16 data_out_count = 0;
						
						if(len > sizeof(ParseBuffer)) //ParseBuffer的size要足够大
							return PACK_FAIL;
						
                        for (j = 0 ; j < len; j++)
                        {
                            if (*(pbuff + j) == P_CTRL)
                            {
                                j++;
                            }
                            ParseBuffer[data_out_count++] = *(pbuff + j);
                            if (data_out_count == PARSE_LEN)
                            {
                                package->DataID = ACK_NULL;
                                return PACK_FAIL;
                            }
                        }

                        for(j = 0 ; j < data_out_count-1;j++)
                        {
                            checksum += ParseBuffer[j];
                        }

                        if (checksum == ParseBuffer[data_out_count-1])
                        {
                            SdkProtocolHeaderTypeDef *sdk = (SdkProtocolHeaderTypeDef *)ParseBuffer;
                            *package->DataOutLen = data_out_count - 1 - sizeof(SdkProtocolHeaderTypeDef);
                            package->DataOutBuff = ParseBuffer + sizeof(SdkProtocolHeaderTypeDef);
							
							if(sdk->DeviceAddr == LIDAR_ADDRESS)
                                package->DataID = (AckDataIDTypeDef)sdk->FunctionCode;
							else
                                package->DataID = ACK_NULL;
                            
							return PACK_OK;
                        }
                        else
                        {
                            package->DataID = ACK_NULL;
                            *package->DataOutLen = 0;
                            return PACK_FAIL;
                        }
                    }
                }
                if (i == package->DataInLen)
                {
                    package->DataID = ACK_NULL;
                    return PACK_FAIL;
                }
            }
        }
    }

    return PACK_FAIL;
}


/************************ (C) COPYRIGHT WEYNE *****END OF FILE****/
