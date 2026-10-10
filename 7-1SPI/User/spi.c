#include "stm32f10x.h"
// 初始化 SPI2
void Flash_SPI_Init(void)
{
    GPIO_InitTypeDef gpio;
    SPI_InitTypeDef spi;

    // 1. 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    // 2. CS：PB12，普通推挽输出
    GPIO_SetBits(GPIOB, GPIO_Pin_12);

    gpio.GPIO_Pin = GPIO_Pin_12;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP; // PB12 由代码直接控制高低电平，实现 Flash 的片选。
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);

    // 3. SCK：PB13，MOSI：PB15
    gpio.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出使 PB13、PB15 由 SPI2 外设驱动，而不是用普通 GPIO 写高低电平。
    GPIO_Init(GPIOB, &gpio);

    // 4. MISO：PB14，浮空输入
    gpio.GPIO_Pin = GPIO_Pin_14;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &gpio);

    // 5. 配置 SPI
    SPI_StructInit(&spi);

    spi.SPI_Mode = SPI_Mode_Master; // STM32作为主机
    spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // 双线全双工模式：MOSI 负责发送，MISO 负责接收，可以同时工作。
    spi.SPI_DataSize = SPI_DataSize_8b; // 每次传输8位（一字节）
	
	// spi模式 0，SCK 空闲为低电平，数据通常在上升沿被采样，在另一个边沿附近更新。
    spi.SPI_CPOL = SPI_CPOL_Low;
    spi.SPI_CPHA = SPI_CPHA_1Edge;
	
    spi.SPI_NSS = SPI_NSS_Soft; // 使用软件管理片选
    spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    spi.SPI_FirstBit = SPI_FirstBit_MSB; // 高位先行

    SPI_Init(SPI2, &spi);

    // 软件管理 NSS 时，主机内部 NSS 置高，避免主机出现模式故障。
    SPI_NSSInternalSoftwareConfig(SPI2, SPI_NSSInternalSoft_Set);

    // 6. 启动 SPI
    SPI_Cmd(SPI2, ENABLE);
}

// 实现 SPI 收发一个字节
// STM32 发送 tx，同时接收一个字节，并把接收到的数据保存到 *rx，最后告诉我传输是否成功。
uint8_t Flash_SPI_Transfer(uint8_t tx, uint8_t *rx)
{
    uint32_t timeout = 100000;

    // 等待发送缓冲区空
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
    {
        if (--timeout == 0) return 0; // 如果等待太久，就返回 0，表示失败。
    }

    // 将 tx 写入 SPI 数据寄存器，开始传输
    SPI_I2S_SendData(SPI2, tx);

    timeout = 100000;

    // 等待收到一个字节
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
    {
        if (--timeout == 0) return 0;
    }
	
	// 当 RXNE 为 1，代表 SPI 已经收到数据，可以读取。
    // 读取接收数据
    *rx = (uint8_t)SPI_I2S_ReceiveData(SPI2);

    return 1; // 通信成功
}

// 读取芯片ID
uint8_t Flash_ReadID(uint32_t *id)
{
    uint8_t dummy = 0; // 保存发送命令时收到的无用数据。
    uint8_t mf = 0; // 保存厂商 ID。
    uint8_t type = 0; //保存设备类型 ID。
    uint8_t capacity = 0; // 保存容量 ID。
    uint8_t ok; // 记录传输是否成功。
    uint32_t timeout = 100000; // 避免无限等待。

    // CS 拉低，Flash 被选中
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);

    // 发送 0x9F，连续读取三个 ID 字节
    ok = Flash_SPI_Transfer(0x9F, &dummy) &&
         Flash_SPI_Transfer(0xFF, &mf) &&
         Flash_SPI_Transfer(0xFF, &type) &&
         Flash_SPI_Transfer(0xFF, &capacity);

    // 等待 SPI 真正完成传输
    if (ok)
    {
		// 需要确保最后一个字节已经传输结束，再释放 CS。
        while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET) // BSY 是忙碌标志
        {
            if (--timeout == 0)
            {
                ok = 0;
                break;
            }
        }
    }

    // 结束通信，无论成功失败都释放 CS(拉高CS)
    GPIO_SetBits(GPIOB, GPIO_Pin_12);

    if (!ok) return 0;

    // 合并三个字节
    *id = ((uint32_t)mf << 16) |
          ((uint32_t)type << 8) |
          capacity;

    return 1;
}
