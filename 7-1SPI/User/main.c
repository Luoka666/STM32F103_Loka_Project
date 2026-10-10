#include "stm32f10x.h"                  // Device header
#include "spi.h"
int main(void){
	
	volatile uint32_t g_flash_id = 0;
	volatile uint8_t g_flash_ok = 0;
	
	uint32_t id = 0;

	Flash_SPI_Init();
	g_flash_ok = Flash_ReadID(&id);
	g_flash_id = id;

	return 0;
}
