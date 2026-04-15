## 1. 故障现象
* **硬件无响应**：使用 `TIM_SetCompare` 函数控制 CCR 寄存器后，舵机保持静止，完全不转动。
* **按键控制失效**：在通道配置正确后，尝试通过按键控制舵机转动 30°，但按下按键后舵机无任何动作。

## 2. 故障原因
* **硬件引脚与输出通道不匹配**：实际电路中舵机连接的是 PA1 引脚，但代码中错误地配置了 TIM2 的 OC1 通道（对应 PA0），并错误地使用了 `TIM_SetCompare1` 函数。
* **数据类型导致计算精度丢失**：在计算 CCR 值与角度的线性对应关系时，计算公式为 `Angle / 180 * 2000 + 500`。由于将 `Angle` 变量设置为 `uint16_t` 类型，导致 `Angle / 180` 在角度小于 180 时计算结果始终为 0，使得 CCR 始终等于 500（对应 0° 位置），因此舵机看似无响应。

## 3. 解决方案
* **更正输出通道**：将定时器通道初始化函数修改为 `TIM_OC2Init(TIM2, &oc)`，并将占空比控制函数改为 `TIM_SetCompare2(TIM2, CompareValue)`，以匹配 PA1 引脚。
* **优化变量数据类型**：将 `Angle` 变量的数据类型由 `uint16_t` 改为 `float` 类型，确保除法运算能够保留小数精度，从而得到正确的 CCR 映射值。

## 4. 核心代码
```c
// 修正后的通道初始化与控制
TIM_OCInitTypeDef oc;
    //配置结构体参数默认值
    TIM_OCStructInit(&oc);

    oc.TIM_OCMode = TIM_OCMode_PWM1;    //选择TWM模式1
    oc.TIM_OCPolarity = TIM_OCPolarity_High;    //控制REF，不反转波形，正常输出。
    oc.TIM_OutputState = TIM_OutputState_Enable ;
    oc.TIM_Pulse = 0;    //CCR的值，初始为0，逐渐递增。
    TIM_OC2Init(TIM2,&oc);


// 修正后的角度线性关系计算
void set_Angle(float Angle){

	TIM_SetCompare2(TIM2,Angle / 180 * 2000 + 500);

}
