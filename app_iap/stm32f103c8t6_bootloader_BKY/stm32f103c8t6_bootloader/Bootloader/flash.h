#ifndef __FLASH_H
#define __FLASH_H
#include "main.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_flash.h"

uint8_t mcu_flash_erase(uint32_t addr, uint8_t count);
uint8_t mcu_flash_write(uint32_t addr, uint8_t *buffer, uint32_t length);
void mcu_flash_read(uint32_t addr, uint8_t *buffer, uint32_t length);

#endif
