#include "SPI2_Flash.h"
#include "stm32f10x.h"

/* 这是有限次数的轮询保护，不是精确的微秒计时。
 * 正常 SPI 字节在很短时间内就完成；硬件异常时不能永远卡在 while 中。
 */
#define SPI2_POLL_LIMIT 100000UL

void SPI2_Flash_Init(void) {
    GPIO_InitTypeDef gpio;
    SPI_InitTypeDef spi;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    SPI_I2S_DeInit(SPI2);

    /* 先把输出寄存器置高，再配置 CS 输出，避免初始化时误选中 Flash。
     * CS 是普通 GPIO，由程序拉低/拉高，不使用 SPI 硬件自动片选。
     */
    GPIO_SetBits(GPIOB, GPIO_Pin_12);
    gpio.GPIO_Pin = GPIO_Pin_12;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &gpio);
    gpio.GPIO_Pin = GPIO_Pin_14;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &gpio);

    SPI_StructInit(&spi);
    spi.SPI_Mode = SPI_Mode_Master;
    spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi.SPI_DataSize = SPI_DataSize_8b;
    /* 模式 0：SCK 空闲为低电平，在第一个时钟边沿采样。 */
    spi.SPI_CPOL = SPI_CPOL_Low;
    spi.SPI_CPHA = SPI_CPHA_1Edge;
    spi.SPI_NSS = SPI_NSS_Soft;
    /* 本项目 APB1=36MHz，32 分频后 SCK 约 1.125MHz，先以低速联调。 */
    spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    spi.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI2, &spi);
    SPI_NSSInternalSoftwareConfig(SPI2, SPI_NSSInternalSoft_Set);
    SPI_Cmd(SPI2, ENABLE);
}

void SPI2_Flash_Select(void) {
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
}

uint8_t SPI2_Flash_ExchangeByte(uint8_t tx, uint8_t *rx) {
    uint32_t remaining = SPI2_POLL_LIMIT;
    if (rx == 0) return 0;

    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) {
        if (--remaining == 0) return 0;
    }
    SPI_I2S_SendData(SPI2, tx);

    /* SPI 是全双工：发一个字节时也收一个字节。
     * 即使只想“发送命令”，也要读走收到的字节，避免接收溢出。
     * 只想读取时，发送 0xFF 来产生时钟，Flash 才能从 MISO 输出数据。
     */
    remaining = SPI2_POLL_LIMIT;
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) {
        if (--remaining == 0) return 0;
    }
    *rx = (uint8_t)SPI_I2S_ReceiveData(SPI2);
    return 1;
}

uint8_t SPI2_Flash_Deselect(void) {
    uint32_t remaining = SPI2_POLL_LIMIT;
    uint8_t ok = 1;
    /* TXE 只表示发送寄存器空，BSY 清零才表示总线传输真正结束。 */
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET) {
        if (--remaining == 0) {
            ok = 0;
            break;
        }
    }
    /* 成功或失败都释放 CS，不能让下一条指令接在半条指令后面。 */
    GPIO_SetBits(GPIOB, GPIO_Pin_12);
    return ok;
}
