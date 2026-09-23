#include "task_alarm.h"
#include "task_sensor.h"   // 使用 SensorData_t 和 alarmQueue 声明
#include "LED.h"
#include "Buzzer.h"
#include "USART.h"
#include "task_statemachine.h"

// 报警任务：保存最新测量值，并以 500ms 周期非阻塞闪烁
void vTask_Alarm(void* pvParameters) {
    (void)pvParameters;
    SensorData_t data = {0, 0};
    TickType_t last_toggle = xTaskGetTickCount();
    uint8_t alarm_active = 0;
    uint8_t output_on = 0;

    while (1) {
        TickType_t now;
        uint8_t should_alarm;

        // 最多等待 50ms，以便切换到 STOP 后迅速关闭报警输出
        (void)xQueueReceive(alarmQueue, &data, pdMS_TO_TICKS(50));
        now = xTaskGetTickCount();
        should_alarm = (currentState == RUN) &&
                       (data.temperature > temp_threshold || data.humidity > humi_threshold);

        if (!should_alarm) {
            alarm_active = 0;
            output_on = 0;
            LED1_OFF();
            Buzzer_off();
            last_toggle = now;
        } else if (!alarm_active) {
            alarm_active = 1;
            output_on = 1;
            LED1_ON();
            Buzzer_on();
            last_toggle = now;
        } else if ((now - last_toggle) >= pdMS_TO_TICKS(500)) {
            last_toggle = now;
            output_on = (uint8_t)!output_on;
            if (output_on) {
                LED1_ON();
                Buzzer_on();
            } else {
                LED1_OFF();
                Buzzer_off();
            }
        }
    }
}
