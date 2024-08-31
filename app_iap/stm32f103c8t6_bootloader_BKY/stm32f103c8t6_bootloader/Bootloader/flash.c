#include "flash.h"

extern void FLASH_PageErase(uint32_t PageAddress);//外部引用
//extern void FLASH_Program_HalfWord(uint32_t Address, uint16_t Data);//外部引用

/*********************
*mcu_flash_erase
*addr:需要擦除的地址起始位置
*count:需要擦除多少页，stm32f103c8t6的页大小为1K=1024字节
**************************/
//https://doc.embedfire.com/mcu/stm32/f103/hal_general/zh/latest/doc/chapter42/chapter42.html
//uint8_t mcu_flash_erase(uint32_t addr, uint8_t count) 
//{
//  HAL_FLASH_Unlock();// 解锁 FLASH 存储器
//	for(uint8_t i=0; i<count; i++)
//	{
//		FLASH_PageErase(addr + (i * FLASH_PAGE_SIZE));//地址add开始按页擦除
//	}
//  HAL_FLASH_Lock();	// 锁定 FLASH 存储器
//	return 1;//未验证，随便返回1
//}
uint8_t mcu_flash_erase(uint32_t addr, uint8_t count) 
{
	uint32_t SECTORError = 0;
	static FLASH_EraseInitTypeDef EraseInitStruct;
  HAL_FLASH_Unlock();// 解锁 FLASH 存储器
	/* 计算要擦除多少页 */
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;//页擦除模式
	EraseInitStruct.NbPages   = count;//要擦除的页数
	EraseInitStruct.PageAddress   = addr;//擦除开始的地址
	
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK) {
         /*擦除出错，返回，实际应用中可加入处理 */
         return 0;
     }
  HAL_FLASH_Lock();	// 锁定 FLASH 存储器
	return 1;//未验证，随便返回1
}



/*********************
*mcu_flash_write
*addr:需要写入的地址起始位置
*length:需要写入的长度，每次半字写入

https://blog.csdn.net/qq_27508477/article/details/104970097
STM32F1系列最小擦除粒度为页面（1K），最小写入粒度为半字（2byte）
https://blog.csdn.net/weixin_45003437/article/details/102846730
！！！字、半字是根据处理器的特性决定的；字节则都是8bit
！！！stm32是32bit处理器，所以它的字是32bit的(一次处理4字节长度的数据)。半字自然就是16bit(2字节)；字节是8bit。

例子：：
                for (i = 0; i < APP_ERASE_SECTORS * 8; i++)
                {
                    mcu_flash_read(down_addr, &down_buf[0], 128);
                    delay_ms(5);
                    mcu_flash_write(app_addr, &down_buf[0], 128);
                    delay_ms(5);
                    down_addr += 128;
                    app_addr += 128;
                }

假设app的内存分配为1K
mcu_flash_write(app_addr, &down_buf[0], 128);//因为down_buf是uint8_t，所以是128个字节
APP_ERASE_SECTORS假设为1，stm32f103c8t6的一个页就是1K
循环8次：8*128=1024字节=1K，刚刚好写入一个页
**************************/
////uint8_t mcu_flash_write(uint32_t addr, uint8_t *buffer, uint32_t length) 
////{
////	uint16_t i, data = 0;
////	HAL_FLASH_Unlock();// 解锁 FLASH 存储器
////	for (i = 0; i < length; i += 2) //每次写入两个8位，就是16位=两字节
////	{
////		data = (*(buffer + i + 1) << 8) + (*(buffer + i));//左移8位，和原8位组成16位=半字
////		FLASH_Program_HalfWord((uint32_t)(addr + i), data);
////		
////	}
////	HAL_FLASH_Lock();	// 锁定 FLASH 存储器
////	
////	return 1;//未验证，随便返回1
////}
//https://doc.embedfire.com/mcu/stm32/f103/hal_general/zh/latest/doc/chapter42/chapter42.html
uint8_t mcu_flash_write(uint32_t addr, uint8_t *buffer, uint32_t length) 
{
	uint16_t i, data = 0;
	HAL_FLASH_Unlock();// 解锁 FLASH 存储器
	for (i = 0; i < length; i += 2) //每次写入两个8位，就是16位=两字节
	{
		data = (*(buffer + i + 1) << 8) + (*(buffer + i));//左移8位，和原8位组成16位=半字
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)(addr + i), data) != HAL_OK)//写入半字
		{
			//某一步写入失败了
			return 0;
		}
	}
	HAL_FLASH_Lock();	// 锁定 FLASH 存储器
	
	return 1;//未验证，随便返回1
}



/************************
*addr:读取的地址
*buffer:用来保存读取的数据
*需要读取的数据长度
*************************************/
void mcu_flash_read(uint32_t addr, uint8_t *buffer, uint32_t length) 
{
	memcpy(buffer, (void *)addr, length);
}
