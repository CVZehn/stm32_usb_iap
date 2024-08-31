/*
 * @Descripttion : 
 * @version      : 
 * @Author       : Kevincoooool
 * @Date         : 2020-10-15 14:27:56
 * @LastEditors  : Kevincoooool
 * @LastEditTime : 2020-12-15 18:11:08
 * @FilePath     : \Simple_TeenyUSB_TX\BSP\w25qxx.c
 */
#include "w25qxx.h"
#include "spi.h"
#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK精英STM32开发板
//W25Q64 代码
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/9
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

uint16_t W25QXX_TYPE = W25Q256; //默认是W25Q128


uint8_t SPI_RW_2(uint8_t dat)
{
	uint8_t d_read, d_send = dat;
	if (HAL_SPI_TransmitReceive(&hspi2, &d_send, &d_read, 1, 0xFF) != HAL_OK)
	{
		d_read = 0xFF;
	}
	return d_read;

}



//4Kbytes为一个Sector扇区
//16个扇区为1个Block块
//W25Q128
//容量为16M字节,共有128个Block,4096个Sector

//初始化SPI FLASH的IO口

/****************
*W25Q256的容量为 256Mb（32MB），共有 512 个块和 8192 个扇区
*容量：256Mb（32MB）
块大小：64KB
扇区大小：4KB
块数：512
扇区数：8192
***************************/
uint8_t W25QXX_Init(void)
{

	MX_SPI2_Init(); //初始化SPI
	W25QXX_CS_SET; //SPI FLASH不选中

	W25QXX_TYPE = W25QXX_ReadID(); //读取FLASH ID.
	W25QXX_TYPE = W25QXX_ReadID(); //读取FLASH ID.
	if (W25QXX_TYPE != 0xff)
		return 0;
	else
		return 1;
}

//读取W25QXX的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
uint8_t W25QXX_ReadSR(void)
{
	uint8_t byte = 0;
	W25QXX_CS_RESET;			  //使能器件
	SPI_RW_2(W25X_ReadStatusReg); //发送读取状态寄存器命令
	byte = SPI_RW_2(0Xff);		  //读取一个字节
	W25QXX_CS_SET;				  //取消片选
	return byte;
}
//写W25QXX状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void W25QXX_Write_SR(uint8_t sr)
{
	W25QXX_CS_RESET;			   //使能器件
	SPI_RW_2(W25X_WriteStatusReg); //发送写取状态寄存器命令
	SPI_RW_2(sr);				   //写入一个字节
	W25QXX_CS_SET;				   //取消片选
}
//W25QXX写使能
//将WEL置位
void W25QXX_Write_Enable(void)
{
	W25QXX_CS_RESET;			//使能器件
	SPI_RW_2(W25X_WriteEnable); //发送写使能
	W25QXX_CS_SET;				//取消片选
}
//W25QXX写禁止
//将WEL清零
void W25QXX_Write_Disable(void)
{
	W25QXX_CS_RESET;			 //使能器件
	SPI_RW_2(W25X_WriteDisable); //发送写禁止指令
	W25QXX_CS_SET;				 //取消片选
}
//读取芯片ID
//返回值如下:
//0XEF13,表示芯片型号为W25Q80
//0XEF14,表示芯片型号为W25Q16
//0XEF15,表示芯片型号为W25Q32
//0XEF16,表示芯片型号为W25Q64
//0XEF17,表示芯片型号为W25Q128
//0xef18,W25Q256
uint16_t W25QXX_ReadID(void)
{
	uint16_t Temp = 0;
	W25QXX_CS_RESET;
	SPI_RW_2(0x90); //发送读取ID命令
	SPI_RW_2(0x00);
	SPI_RW_2(0x00);
	SPI_RW_2(0x00);
	Temp |= SPI_RW_2(0xFF) << 8;
	Temp |= SPI_RW_2(0xFF);
	W25QXX_CS_SET;
	return Temp;
}
//读取SPI FLASH
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
/*W25Q256 闪存芯片的最大寻址范围为 268,435,456 字节（256Mb）。
地址范围：
起始地址：0x000000
结束地址：0x1FFFFF
示例：
使用你之前提供的 W25QXX_Read() 函数从地址 0x100000 开始读取 100 个字节：
Copy
W25QXX_Read(data, 0x100000, 100);
这将从 W25Q256 闪存芯片的地址 0x100000 开始读取 100 个字节并将其存储在 data 缓冲区中。*/
void W25QXX_Read(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	uint16_t i;
	W25QXX_CS_RESET;					   //使能器件
	SPI_RW_2(W25X_ReadData);			   //发送读取命令
	SPI_RW_2((uint8_t)((ReadAddr) >> 16)); //发送24bit地址
	SPI_RW_2((uint8_t)((ReadAddr) >> 8));
	SPI_RW_2((uint8_t)ReadAddr);
	for (i = 0; i < NumByteToRead; i++)
	{
		pBuffer[i] = SPI_RW_2(0XFF); //循环读数
	}
	W25QXX_CS_SET;
}
//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!
void W25QXX_Write_Page(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint16_t i;
	W25QXX_Write_Enable();					//SET WEL
	W25QXX_CS_RESET;						//使能器件
	SPI_RW_2(W25X_PageProgram);				//发送写页命令
	SPI_RW_2((uint8_t)((WriteAddr) >> 16)); //发送24bit地址
	SPI_RW_2((uint8_t)((WriteAddr) >> 8));
	SPI_RW_2((uint8_t)WriteAddr);
	for (i = 0; i < NumByteToWrite; i++)
		SPI_RW_2(pBuffer[i]); //循环写数
	W25QXX_CS_SET;			  //取消片选
	W25QXX_Wait_Busy();		  //等待写入结束
}
//无检验写SPI FLASH
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void W25QXX_Write_NoCheck(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint16_t pageremain;
	pageremain = 256 - WriteAddr % 256; //单页剩余的字节数
	if (NumByteToWrite <= pageremain)
		pageremain = NumByteToWrite; //不大于256个字节
	while (1)
	{
		W25QXX_Write_Page(pBuffer, WriteAddr, pageremain);
		if (NumByteToWrite == pageremain)
			break; //写入结束了
		else	   //NumByteToWrite>pageremain
		{
			pBuffer += pageremain;
			WriteAddr += pageremain;

			NumByteToWrite -= pageremain; //减去已经写入了的字节数
			if (NumByteToWrite > 256)
				pageremain = 256; //一次可以写入256个字节
			else
				pageremain = NumByteToWrite; //不够256个字节了
		}
	};
}
//写SPI FLASH
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
uint8_t W25QXX_BUFFER[4096];
void W25QXX_Write(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;
	uint16_t i;
	uint8_t *W25QXX_BUF;
	W25QXX_BUF = W25QXX_BUFFER;
	secpos = WriteAddr / 4096; //扇区地址
	secoff = WriteAddr % 4096; //在扇区内的偏移
	secremain = 4096 - secoff; //扇区剩余空间大小
	//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
	if (NumByteToWrite <= secremain)
		secremain = NumByteToWrite; //不大于4096个字节
	while (1)
	{
		W25QXX_Read(W25QXX_BUF, secpos * 4096, 4096); //读出整个扇区的内容
		for (i = 0; i < secremain; i++)				  //校验数据
		{
			if (W25QXX_BUF[secoff + i] != 0XFF)
				break; //需要擦除
		}
		if (i < secremain) //需要擦除
		{
			W25QXX_Erase_Sector(secpos);	//擦除这个扇区
			for (i = 0; i < secremain; i++) //复制
			{
				W25QXX_BUF[i + secoff] = pBuffer[i];
			}
			W25QXX_Write_NoCheck(W25QXX_BUF, secpos * 4096, 4096); //写入整个扇区
		}
		else
			W25QXX_Write_NoCheck(pBuffer, WriteAddr, secremain); //写已经擦除了的,直接写入扇区剩余区间.
		if (NumByteToWrite == secremain)
			break; //写入结束了
		else	   //写入未结束
		{
			secpos++;	//扇区地址增1
			secoff = 0; //偏移位置为0

			pBuffer += secremain;		 //指针偏移
			WriteAddr += secremain;		 //写地址偏移
			NumByteToWrite -= secremain; //字节数递减
			if (NumByteToWrite > 4096)
				secremain = 4096; //下一个扇区还是写不完
			else
				secremain = NumByteToWrite; //下一个扇区可以写完了
		}
	};
}
//擦除整个芯片
//等待时间超长...
void W25QXX_Erase_Chip(void)
{
	W25QXX_Write_Enable(); //SET WEL
	W25QXX_Wait_Busy();
	W25QXX_CS_RESET;		  //使能器件
	SPI_RW_2(W25X_ChipErase); //发送片擦除命令
	W25QXX_CS_SET;			  //取消片选
	W25QXX_Wait_Busy();		  //等待芯片擦除结束
}
//擦除一个扇区
//Dst_Addr:扇区地址 根据实际容量设置
//擦除一个山区的最少时间:150ms
void W25QXX_Erase_Sector(uint32_t Dst_Addr)
{
	//监视falsh擦除情况,测试用
	//	printf("fe:%x\r\n", Dst_Addr);
	Dst_Addr *= 4096;
	W25QXX_Write_Enable(); //SET WEL
	W25QXX_Wait_Busy();
	W25QXX_CS_RESET;					   //使能器件
	SPI_RW_2(W25X_SectorErase);			   //发送扇区擦除指令
	SPI_RW_2((uint8_t)((Dst_Addr) >> 16)); //发送24bit地址
	SPI_RW_2((uint8_t)((Dst_Addr) >> 8));
	SPI_RW_2((uint8_t)Dst_Addr);
	W25QXX_CS_SET;		//取消片选
	W25QXX_Wait_Busy(); //等待擦除完成
}
//等待空闲
void W25QXX_Wait_Busy(void)
{
	while ((W25QXX_ReadSR() & 0x01) == 0x01)
		; // 等待BUSY位清空
}
//进入掉电模式
void W25QXX_PowerDown(void)
{
	W25QXX_CS_RESET;		  //使能器件
	SPI_RW_2(W25X_PowerDown); //发送掉电命令
	W25QXX_CS_SET;			  //取消片选
							  //	delay_us(3);			//等待TPD
}
//唤醒
void W25QXX_WAKEUP(void)
{
	W25QXX_CS_RESET;				 //使能器件
	SPI_RW_2(W25X_ReleasePowerDown); //  send W25X_PowerDown command 0xAB
	W25QXX_CS_SET;					 //取消片选
									 //	delay_us(3);				   //等待TRES1
}
