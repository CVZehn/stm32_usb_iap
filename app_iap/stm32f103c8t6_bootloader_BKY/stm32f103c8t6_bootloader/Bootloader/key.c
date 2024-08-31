#include "key.h"

#define key1_state HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8)
#define key2_state HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3)
#define key3_state HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15)

uint8_t get_key(void)
{
	//key1
	if(key1_state == 0)
	{
		HAL_Delay(10);
		if(key1_state == 0)
		{
			return 1;
		}
	}
	
	//key2
	else if(key2_state == 0)
	{
		HAL_Delay(10);
		if(key2_state == 0)
		{
			return 2;
		}
	}
	
	//key3
	else if(key3_state == 0)
	{
		HAL_Delay(10);
		if(key3_state == 0)
		{
			return 3;
		}
	}
	return 0;
}

