#include "stm32f10x.h"
#include "delay.h"
// 初始化温湿度传感器所连DATA线
void DHT11_Init(void) {

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

    GPIO_InitTypeDef p;
    p.GPIO_Mode = GPIO_Mode_Out_PP;
    p.GPIO_Pin = GPIO_Pin_0;
    p.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&p);
    // 默认高电平（初始拉高）
    GPIO_SetBits(GPIOA,GPIO_Pin_0);

}
// 设置DATA线为推挽输出模式，用于给DHT11发起始信号
void DATA_OUT_Mode() {

    GPIO_InitTypeDef p;
    p.GPIO_Mode = GPIO_Mode_Out_PP;
    p.GPIO_Pin = GPIO_Pin_0;
    p.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&p);

}
// 设置DATA线为上拉输入模式，用于接收DHT11的响应信号
void DATA_INPUT_Mode() {

    GPIO_InitTypeDef p;
    p.GPIO_Mode = GPIO_Mode_IPU;
    p.GPIO_Pin = GPIO_Pin_0;
    p.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&p);

}
// 传感器接收信号
void DHT11_Accept() {

    // 拉低18ms
    GPIO_ResetBits(GPIOA,GPIO_Pin_0);
    Delay_ms(18);
    // 拉高30us
    GPIO_SetBits(GPIOA,GPIO_Pin_0);
    Delay_us(30);

}
// 检测DHT11是否成功完成传输数据准备(1成功，0失败)
uint8_t DHT11_Send_check() {

    uint8_t counter = 0;

    DATA_INPUT_Mode();// 切换上拉输入接收数据

    while (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1) {// 等待拉低
        if (counter > 100) {
            return 0;
        }
        Delay_us(1);
        counter++;
    }
    counter = 0;
    while (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 0) {// 等待拉高
        if (counter > 100) {
            return 0;
        }
        Delay_us(1);
        counter++;
    }

    return 1;

}
// 数据发送
uint8_t DHT11_Send() {

    uint8_t byte = 0;// 每一字节的数据
    uint8_t bit = 0;// 读取的每一位数据

    for (int i = 0;i < 8;i++) {

        // 等待拉低
        while (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1);
        // 等待拉高
        while (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 0);
        Delay_us(40);
        if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1) {

            bit = 1;
            while (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1);//等待拉低

        }else {

            bit = 0;

        }

        byte <<= 1; // 左移一位
        byte |= bit; // 放bit=0
    }

    return byte;
}
// 数据接收检验与整合(0失败，1成功)
uint8_t data_Check(uint8_t *temp, uint8_t *humi) {

    uint8_t data[5] = {0};

    DATA_OUT_Mode();
    DHT11_Accept();
    DATA_INPUT_Mode();
    if (DHT11_Send_check() == 0) {

        return 0;

    }
    // 存储数据
    for (int i = 0;i < 5;i++) {

        data[i] = DHT11_Send();

    }
    // 4. 校验数据：前4个字节相加 == 第5个字节（校验和）
    if ((data[0] + data[1] + data[2] + data[3]) == data[4])
    {
        *humi = data[0]; // 湿度整数位（小数位buf[1]一般为0，DHT11精度不高）
        *temp = data[2]; // 温度整数位（小数位buf[3]一般为0）
        return 1;       // 读取成功
    }
    else
    {
        return 0; // 校验失败
    }

}
