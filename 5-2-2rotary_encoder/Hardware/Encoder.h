//
// Created by 32663 on 2026/3/15.
//

#ifndef _ENCODER_H
#define _ENCODER_H

//中断函数后触发后由CPU自己调用，不需要再.h里声明，也不需要在.c文件里调用
void encoder_Init(void);
int32_t get_Num(void);

#endif
