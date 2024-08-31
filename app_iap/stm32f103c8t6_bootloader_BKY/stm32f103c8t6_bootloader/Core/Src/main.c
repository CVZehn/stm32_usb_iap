/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "btldr_config.h"
#include "crypt.h"
#include "ihex_parser.h"
#include "crc.h"
#include "fat32.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void SoftReset(void) 
{ 
	__set_FAULTMASK(1);      
	NVIC_SystemReset();
}

int fputc(int ch, FILE* stream)
{
	//USART_SendData(USART1, (unsigned char) ch);
	//while (!(USART1->SR & USART_FLAG_TXE));
	HAL_UART_Transmit(&huart1,(uint8_t *)&ch,1,0xFFF);
	return ch;
}


#if (BTLDR_ACT_NoAppExist > 0u)
bool is_appcode_exist(void)
{
  uint32_t *mem = (uint32_t*)APP_ADDR;
  
    return false;
  if ((mem[0] == 0x00000000 || mem[0] == 0xFFFFFFFF) && \
      (mem[1] == 0x00000000 || mem[1] == 0xFFFFFFFF) && \
      (mem[2] == 0x00000000 || mem[2] == 0xFFFFFFFF) && \
      (mem[3] == 0x00000000 || mem[3] == 0xFFFFFFFF))
  {
    return false;
  }
  else
  {
    return true;
  }
}
#endif

#if (BTLDR_ACT_CksNotVld > 0u)
bool app_cks_valid(void)
{
	uint32_t app_crc32 = 0;

	/* calculate CRC32 checksum from start of main application until CRC32 location */
	app_crc32 = crc32_calculate((const uint8_t *)APP_ADDR, (CRC_ADDR-APP_ADDR));

	/* compare self calculated CRC32 with CRC32 stored in flash */
	return (app_crc32 == *((uint32_t*)(CRC_ADDR)));
}
#endif

#if (BTLDR_ACT_ButtonPress > 0u)
bool is_button_down(void)
{
    //return !(LL_GPIO_IsInputPinSet(BTLDR_EN_GPIO_Port, BTLDR_EN_Pin) );
	return 0;
}
#endif

#if (BTLDR_ACT_BootkeyDet > 0u)

volatile __attribute__((section("._bootkey_section.btldr_act_req_key"))) uint32_t btldr_act_req_key;
bool bootkey_detected(void){
    return(btldr_act_req_key == BOOTKEY);
}
#endif

extern PCD_HandleTypeDef hpcd_USB_FS;

void SystemReset(void){
    LL_mDelay(500);

    HAL_PCD_Stop(&hpcd_USB_FS);

    /* Pull USB D+ PIN to GND so USB Host detects device disconnect */
    LL_GPIO_SetPinMode(GPIOA,LL_GPIO_PIN_12, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_12, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetPinOutputType(GPIOA,LL_GPIO_PIN_12, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_12);

    LL_mDelay(1000);

    NVIC_SystemReset();
}

static void usb_discon(void)
{
	
	LL_GPIO_SetPinMode(GPIOA,LL_GPIO_PIN_12, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_12, LL_GPIO_SPEED_FREQ_LOW);
	LL_GPIO_SetPinOutputType(GPIOA,LL_GPIO_PIN_12, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_12);

	LL_mDelay(1000);
}

typedef void (*jump_callback)(void);
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
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  static uint32_t jump_addr;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
   goto DRV_INIT;
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  
  DRV_INIT:
  {
	MX_GPIO_Init();
	MX_USART1_UART_Init();
  }
  
 if(IAP_CheckAPP() == 0 || Get_BootKey_Status() == 0)
  {
	printf("iap start \r\n");
#if(CONFIG_SUPPORT_CRYPT_MODE > 0u)
    crypt_init();//
#endif
	usb_discon();
    MX_USB_DEVICE_Init();
	 uint32_t time_cnt = 0;
    while(1)
    {
#if (CONFIG_SOFT_RESET_AFTER_IHEX_EOF > 0u)
      if (ihex_is_eof()) 
	  {
		    LL_mDelay(500);
			IAP_SetFWInfo();
		    LL_mDelay(500);
			SystemReset();
      }
	  
#endif
    }
  }
  else
  {
	  
APP_START:
    // Jump to the app code
	  
	printf("app start \r\n");
#if 1

   LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
   while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI);
 #endif
	jump_app(APP_ADDR);
	  
	printf("???? \r\n");
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_1)
  {
  }
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_6);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_SetSystemCoreClock(48000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
  LL_RCC_SetUSBClockSource(LL_RCC_USB_CLKSOURCE_PLL);
}

/* USER CODE BEGIN 4 */


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
