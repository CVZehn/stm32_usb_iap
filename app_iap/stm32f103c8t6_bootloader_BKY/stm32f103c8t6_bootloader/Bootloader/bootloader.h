#ifndef __BOOTLOARDER_H
#define __BOOTLOARDER_H

#include "main.h"
#include "flash.h"

/******************************************************** 
======== flash pration table =========
| name     | offset     | size       |
--------------------------------------
| boot     | 0x08000000 | 0x00005000 |
| setting  | 0x08005000 | 0x00006000 |
| app      | 0x08006000 | 0x0000A000 |
| download | w25qxx_flash |
======================================
*********************************************************/

#define FLASH_SECTOR_SIZE       1024
#define FLASH_SECTOR_NUM        64    // 64K
#define FLASH_START_ADDR        ((uint32_t)0x8000000)
#define FLASH_END_ADDR          ((uint32_t)(0x8000000 + FLASH_SECTOR_NUM * FLASH_SECTOR_SIZE))

#define BOOT_SECTOR_ADDR        0x08000000
#define BOOT_SECTOR_SIZE        0x5000
#define SETTING_SECTOR_ADDR     0x08005000
#define SETTING_SECTOR_SIZE     0x1000
#define APP_SECTOR_ADDR         0x08006000     // APP sector start address  
#define APP_SECTOR_SIZE         0xA000         // APP sector size   


#define DOWNLOAD_SECTOR_ADDR    0x08012000     // Download sector start address
#define DOWNLOAD_SECTOR_SIZE    0xE000         // Download sector size   

#define APP_ERASE_SECTORS        (APP_SECTOR_SIZE / FLASH_SECTOR_SIZE)//40

#define MaxQueueSize            1200

#define SETTING_BOOT_STATE      0x08005000
#define UPDATE_PROGRAM_STATE    2
#define UPDATE_SUCCESS_STATE    3

typedef enum 
{
    NONE,
    START_PROGRAM,
    UPDATE_PROGRAM,
    UPDATE_SUCCESS,
} process_status;

typedef void (*boot_callback)(process_status);

typedef struct 
{
    process_status process;
    uint8_t status;
    uint8_t id;
    uint32_t addr;
} boot_t;

extern boot_t boot_parameter;

//顺序循环队列的结构体定义如下：
typedef struct
{
    uint8_t queue[MaxQueueSize];
    int rear;  //队尾指针
    int front;  //队头指针
    int count;  //计数器
} SeqCQueue; 

typedef void (*jump_callback)(void);

uint8_t jump_app(uint32_t appAddr);
void set_boot_state(process_status process);
process_status get_boot_state(void);
process_status read_setting_boot_state(void);
uint8_t write_setting_boot_state(uint8_t boot_state);

#endif
