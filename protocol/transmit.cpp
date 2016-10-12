/**
  * @file        transmit.c
  * @author      陈维
  * @version     V01
  * @date        2016.09.21
  * @brief       打包
  * @note        
  *
  * @attention   COYPRIGHT WEYNE
  **/
#include "transmit.h"  
#include "pro.h"


/**
  * @brief  数据打包函数
  * @param  package :待打包数据信息
  * @note   需要确保输入的数据信息正确，特别是out buffer最好要是待打包数据长度的两倍
  * @retval None
  */
ResultTypeDef Package(PackageDataStruct package)
{
	u32 j = 0;
	u32 i = 0;
	SdkProtocolHeaderTypeDef sdk_header;
	u8 *psdk = (u8 *)&sdk_header;
	u8 checksum = 0;
	
	if((package.DataInBuff == NULL) || (package.DataOutBuff == NULL) || (package.DataOutLen == NULL))
		return PACK_FAIL;
	
	sdk_header.DeviceAddr = LIDAR_ADDRESS;
	sdk_header.FunctionCode = package.DataID;
	sdk_header.StartAddr = 0;
	sdk_header.Len = package.DataInLen;

	*(package.DataOutBuff+i ++) = P_HEADER;
	*(package.DataOutBuff+i ++) = P_HEADER;

	for(j = 0 ; j<sizeof(SdkProtocolHeaderTypeDef);j++)
	{
		if((*(psdk+j) == P_CTRL) || (*(psdk+j) == P_HEADER) || (*(psdk+j) == P_TAIL))
		{
			*(package.DataOutBuff+i ++) = P_CTRL;
		}
		*(package.DataOutBuff+i ++) = *(psdk+j);
		checksum += *(psdk+j);
	}
	
	for(j = 0 ; j<package.DataInLen; j++)
	{
		if((*(package.DataInBuff+j) == P_CTRL) || (*(package.DataInBuff+j) == P_HEADER) || (*(package.DataInBuff+j) == P_TAIL))
		{
			*(package.DataOutBuff+i++) = P_CTRL;
		}
		checksum += *(package.DataInBuff+j);
		*(package.DataOutBuff+i++) = *(package.DataInBuff+j);
	}
	
	if((checksum == P_CTRL) || (checksum == P_HEADER) || (checksum == P_TAIL))
	{
		*(package.DataOutBuff+i++) = P_CTRL;
	}
	*(package.DataOutBuff+i++) = checksum;
	
	*(package.DataOutBuff+i++) = P_TAIL;
	*(package.DataOutBuff+i++) = P_TAIL;
	
	*package.DataOutLen = i;
	
	return PACK_OK;
}

  
/************************ (C) COPYRIGHT WEYNE *****END OF FILE****/

