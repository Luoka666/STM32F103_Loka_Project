#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"

void LED_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0| GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}


void vLED1_Task(void* pvParameters) {
    (void)pvParameters;
    while (1) {
        GPIO_ResetBits(GPIOB, GPIO_Pin_0);   // PA6 低电平，LED1 亮
        vTaskDelay(pdMS_TO_TICKS(500));      // 非阻塞延时 500ms
        GPIO_SetBits(GPIOB, GPIO_Pin_0);     // PA6 高电平，LED1 灭
        vTaskDelay(pdMS_TO_TICKS(500));      // 1个tick为1ms，则为500ms
    }
}

void vLED2_Task(void* pvParameters) {
    (void)pvParameters;
    while (1) {
        GPIO_ResetBits(GPIOB, GPIO_Pin_1);   // PA7 低电平，LED2 亮
        // vTaskDelay(200);//参数为tick数
		vTaskDelay(pdMS_TO_TICKS(200));
        GPIO_SetBits(GPIOB, GPIO_Pin_1);     // PA7 高电平，LED2 灭
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}


int main(void) {
    LED_Init();  // 初始化 LED 硬件（GPIO）

    // 创建两个 LED 任务
    // 参数：任务函数、名称、栈大小、参数、优先级、句柄
    xTaskCreate(vLED1_Task, "LED1", 128, NULL, 1, NULL);
    xTaskCreate(vLED2_Task, "LED2", 128, NULL, 1, NULL);

    // 启动调度器（此函数不会返回），操作系统开始接管
    vTaskStartScheduler();

    // 调度器启动失败才会执行到这里，防止程序跑飞...
    while (1);
}
