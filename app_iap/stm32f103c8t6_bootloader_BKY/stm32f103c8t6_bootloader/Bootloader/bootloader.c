#include "bootloader.h"

boot_t boot_parameter = {START_PROGRAM, 0, 0, 0};


/***************
*Bootloader作为引导程序，
最重要的工作之一就是通过内存跳转进入用户程序，
下面这段代码可以跳转到任何一个内存地址
*****************************/
uint8_t jump_app(uint32_t app_addr) 
{
    uint32_t jump_addr;
    jump_callback cb;
    if (((*(__IO uint32_t*)app_addr) & 0x2FFE0000 ) == 0x20000000) {  
        jump_addr = *(__IO uint32_t*) (app_addr + 4);  
        cb = (jump_callback)jump_addr;  
        __set_MSP(*(__IO uint32_t*)app_addr);  
        cb();
        return 1;
    } 
    return 0;
}


/******************
结构体中有四种状态
*NONE,
  START_PROGRAM,程序开始
  UPDATE_PROGRAM,程序更新
  UPDATE_SUCCESS,更新成功

设置状态
******************************/
void set_boot_state(process_status process) 
{
    boot_parameter.process = process;
}


//获取boot的状态，四种之一
process_status get_boot_state(void) 
{
    process_status process;
    process = boot_parameter.process;

    return process;
}



//读取在设置内存里面的bootloader状态，大小1K，地址在08005000开始
process_status read_setting_boot_state(void)
{
	process_status process;
	uint8_t boot_state;
    mcu_flash_read(SETTING_BOOT_STATE, &boot_state, 1);
    // printf("boot_state: %d \r\n", boot_state);
 
    if(boot_state != UPDATE_PROGRAM_STATE)
    {
       process = START_PROGRAM;
    }
    else
    {
       process = UPDATE_PROGRAM;
    }
	return process;
}

//设置在配置内存里面的bootloader的状态
//result=1表示成功，返回1
uint8_t write_setting_boot_state(uint8_t boot_state)
{
	uint8_t result;
	result = mcu_flash_erase(SETTING_BOOT_STATE, 1);
	if(result)
	{
		result = mcu_flash_write(SETTING_BOOT_STATE, &boot_state, 1);
		if(result != 1)
		{
			return result;
		}
	}
    
	return result;
}
