#include "stm32f10x.h"                  // Device header

// CAN初始化函数
void MYCAN_Init(void)
{
    // CAN初始化代码
    // 这里可以根据具体的CAN配置进行初始化
    // 例如设置波特率、工作模式等
    // CAN默认使用PA11作为CAN_TX，PA12作为CAN_RX，所以需要配置GPIOA的时钟和CAN1的时钟
    // CAN 的发送引脚需要由 CAN 外设控制输出，而“复用推挽”正是让外设驱动引脚输出高、低电平的模式。

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // 使能GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE); // 使能CAN1时钟

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 设置为复用推挽模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 设置为上拉输入模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // CAN初始化
    CAN_InitTypeDef CAN_InitStructure;
    CAN_InitStructure.CAN_TTCM = DISABLE;
    CAN_InitStructure.CAN_ABOM = DISABLE;
    CAN_InitStructure.CAN_AWUM = DISABLE;
    CAN_InitStructure.CAN_NART = DISABLE;
    CAN_InitStructure.CAN_RFLM = DISABLE;
    CAN_InitStructure.CAN_TXFP = DISABLE;
    CAN_InitStructure.CAN_Mode = CAN_Mode_LoopBack; // 环回模式,自发自收
    CAN_InitStructure.CAN_SJW = CAN_SJW_2tq;
    CAN_InitStructure.CAN_BS1 = CAN_BS1_2tq;
    CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;
    CAN_InitStructure.CAN_Prescaler = 48; // 设置波特率为125kbps
    CAN_Init(CAN1, &CAN_InitStructure);

    // 初始化并配置CAN过滤器
    CAN_FilterInitTypeDef CAN_FilterInitStructure;
    CAN_FilterInitStructure.CAN_FilterNumber = 0; // 选择过滤器编号0--13
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask; //使用掩码匹配模式
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; // 过滤器规模，32位过滤器
    // 全通过滤器，接收所有ID的报文，32位过滤器模式下，ID和Mask都设置为0x0000
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000; // 过滤匹配值的高 16 位
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000; // 过滤匹配值的低 16 位
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000; // 标志该检查高16位的哪几位
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000; // 标志该检查低16位的哪几位
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0; // 过滤器 FIFO 分配（0或1）
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE; // 过滤器激活（开关）
    CAN_FilterInit(&CAN_FilterInitStructure);

}

void MyCAN_SendMessage(uint32_t id, uint8_t *data, uint8_t len)
{
    CanTxMsg TxMessage;
    TxMessage.StdId = id; // 设置标准ID，11 位标准标识符
    TxMessage.ExtId = 0x00; // 扩展ID不使用，29 位扩展标识符
    TxMessage.IDE = CAN_Id_Standard; // 使用标准ID，标识符长度为11位，只表示"这帧用的是标准 ID 还是扩展 ID"。
    TxMessage.RTR = CAN_RTR_Data; // 数据帧还是远程帧，这里使用数据帧
    TxMessage.DLC = len; // 数据长度，数据长度码
    for (uint8_t i = 0; i < len; i++)
    {
        TxMessage.Data[i] = data[i]; // 填充数据
    }
    uint8_t result = CAN_Transmit(CAN1, &TxMessage); // 发送CAN消息。返回值为邮箱编号
    uint32_t Timeout = 100000; // 设置超时时间
    while (CAN_TransmitStatus(CAN1, result) != CAN_TxStatus_Ok) {
        if (Timeout-- == 0) {
            return; // 超时返回
        }
    } // 等待发送完成

}
// 判断接收 FIFO 里是否有数据
uint8_t MyCAN_ReceiveMessageFlag(CanRxMsg *RxMessage)
{
    if (CAN_MessagePending(CAN1, CAN_FIFO0) == 0) {
        return 0; // 没有接收到数据
    }
    return 1; // 有数据待接收
}
// // 非阻塞接收：FIFO 里有报文就取一条出来，没有就直接返回 0
// // 成功返回 1，并把收到的数据填入 RxMessage；失败返回 0
// uint8_t MyCAN_ReceiveMessage(CanRxMsg *RxMessage)
// {
//     if (CAN_MessagePending(CAN1, CAN_FIFO0) == 0) {
//         return 0; // FIFO 空，直接返回，不阻塞主循环
//     }
//     CAN_Receive(CAN1, CAN_FIFO0, RxMessage); // 从 FIFO0 弹出一条报文
//     return 1;
// }

// 定义三个指针，可视为函数传出了三个参数，分别是报文ID、数据和数据长度
uint8_t MyCAN_ReceiveMessage(uint32_t *id, uint8_t *data, uint8_t *len) {
    if (CAN_MessagePending(CAN1, CAN_FIFO0) != 0) {
        CanRxMsg RxMessage;
        CAN_Receive(CAN1, CAN_FIFO0, &RxMessage); // 从 FIFO0 弹出一条报文
        if (RxMessage.IDE == CAN_Id_Standard) {
            *id = RxMessage.StdId; // 获取报文ID,说明是标准ID
        }else {
            *id = RxMessage.ExtId; // 获取报文ID,说明是扩展ID
        }
        if (RxMessage.RTR == CAN_RTR_Data) {
            // 数据帧
            *len = RxMessage.DLC; // 获取数据长度
            for (uint8_t i = 0; i < *len; i++) {
                data[i] = RxMessage.Data[i]; // 获取数据
            }
        }else {
            // 遥控帧不做处理......
        }
        return 1; // 成功接收到数据
    }
    return 0; // 没有接收到数据
}
