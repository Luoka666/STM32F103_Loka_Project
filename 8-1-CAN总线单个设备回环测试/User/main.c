#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MyCAN.h"
#include "Key.h"
int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	MYCAN_Init();		//CAN初始化
	Key_Init();		//按键初始化
	uint8_t data[8] = {0x12, 0x77}; // 要发送的数据
	while (1)
	{
		OLED_ShowString(1, 1, "TxData");
		OLED_ShowHexNum(2, 1, data[0], 2);
		OLED_ShowHexNum(2, 5, data[1], 2);

		uint8_t key = Key_GetNum(); // 获取按键状态
		if (key == 1) {
			// 发送数据
			Delay_ms(1000); // 延时传送展现，展现传送过程。
			MyCAN_SendMessage(0x123, data, 2);
		
			// 接收数据
			uint32_t id;
			uint8_t len;
			if (MyCAN_ReceiveMessage(&id, data, &len) != 0) {
				// 处理接收到的数据
				OLED_ShowString(3, 1, "RxData");
				OLED_ShowHexNum(4, 1, data[0], 2);
				OLED_ShowHexNum(4, 5, data[1], 2);

			}else {
				OLED_ShowString(3, 1, "No Data");
			}
		}
	}
}
