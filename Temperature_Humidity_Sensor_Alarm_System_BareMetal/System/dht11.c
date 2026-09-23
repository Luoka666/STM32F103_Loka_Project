#include "stm32f10x.h"
#include "delay.h"

#define DHT11_TIMEOUT_US  120U

static void DATA_OUT_Mode(void);
static void DATA_INPUT_Mode(void);

static uint8_t DHT11_WaitLevel(BitAction level, uint16_t timeout_us) {
    while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) != level) {
        if (timeout_us == 0U) {
            return 0;
        }
        timeout_us--;
        Delay_us(1);
    }
    return 1;
}

static void DHT11_ReleaseBus(void) {
    DATA_OUT_Mode();
    GPIO_SetBits(GPIOA, GPIO_Pin_0);
}
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
static void DATA_OUT_Mode(void) {

    GPIO_InitTypeDef p;
    p.GPIO_Mode = GPIO_Mode_Out_PP;
    p.GPIO_Pin = GPIO_Pin_0;
    p.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&p);

}
// 设置DATA线为上拉输入模式，用于接收DHT11的响应信号
static void DATA_INPUT_Mode(void) {

    GPIO_InitTypeDef p;
    p.GPIO_Mode = GPIO_Mode_IPU;
    p.GPIO_Pin = GPIO_Pin_0;
    p.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&p);

}
// 传感器接收信号
static void DHT11_Accept(void) {

    // 拉低18ms
    GPIO_ResetBits(GPIOA,GPIO_Pin_0);
    Delay_ms(18);
    // 拉高30us
    GPIO_SetBits(GPIOA,GPIO_Pin_0);
    Delay_us(30);

}
// 检测DHT11是否成功完成传输数据准备(1成功，0失败)
static uint8_t DHT11_Send_check(void) {
    DATA_INPUT_Mode();// 切换上拉输入接收数据

    if (!DHT11_WaitLevel(Bit_RESET, DHT11_TIMEOUT_US)) return 0;
    if (!DHT11_WaitLevel(Bit_SET, DHT11_TIMEOUT_US)) return 0;
    return 1;

}
// 读取一个字节；任何位超时都立即失败，避免主循环永久卡死
static uint8_t DHT11_ReadByte(uint8_t *value) {
    uint8_t byte = 0;
    uint8_t bit;

    for (int i = 0;i < 8;i++) {

        if (!DHT11_WaitLevel(Bit_RESET, DHT11_TIMEOUT_US)) return 0;
        if (!DHT11_WaitLevel(Bit_SET, DHT11_TIMEOUT_US)) return 0;
        Delay_us(40);
        if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1) {
            bit = 1;
            if (!DHT11_WaitLevel(Bit_RESET, DHT11_TIMEOUT_US)) return 0;
        }else {
            bit = 0;
        }

        byte = (uint8_t)((byte << 1) | bit);
    }

    *value = byte;
    return 1;
}
// 数据接收检验与整合(0失败，1成功)
uint8_t data_Check(uint8_t *temp, uint8_t *humi) {

    uint8_t data[5] = {0};

    DATA_OUT_Mode();
    DHT11_Accept();
    DATA_INPUT_Mode();
    if (DHT11_Send_check() == 0) {
        DHT11_ReleaseBus();
        return 0;
    }
    // 存储数据
    for (int i = 0;i < 5;i++) {
        if (!DHT11_ReadByte(&data[i])) {
            DHT11_ReleaseBus();
            return 0;
        }
    }
    DHT11_ReleaseBus();
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
