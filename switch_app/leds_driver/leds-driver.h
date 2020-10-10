/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:leds-driver.c
*Author : JimYao
*Version : 1.1
*Date : 2019-10-08
*Description: LED指示灯驱动及闪灯任务处理
*History:在1.0版本的基础上,优化时间侦测,减少频繁唤醒
***************************************************************************************************/

/* 防止重复引用区 ----------------------------------------------------------- */

#ifndef _LEDS_DRIVER_H_
#define _LEDS_DRIVER_H_

/* 相关包含头文件区 --------------------------------------------------------- */
#include "app/framework/include/af.h"

/* 宏定义区 ----------------------------------------------------------------- */
#define LED_FREQUENCY_TYPE_ALWAYS_OFF     0x0000    //灯一直关
#define LED_FREQUENCY_TYPE_ALWAYS_ON      0xFFFF    //灯一直开
#define LED_REPEAT_TYPE_STOP_REPEAT       0x00      //灯停止闪
#define LED_REPEAT_TYPE_ALWAYS_REPEAT     0xFF      //灯一直闪
#define LED_REPEAT_TYPE_THREE			  3 		//灯闪三次
#define LED_REPEAT_TYPE_EIGHT			  8			//灯闪八次

/* 自定义类型区 ------------------------------------------------------------- */
enum {
  LED_RED = 0,
  LED_GREEN,
  LED_BLUE,
  LED_ALL
};

/* 全局函数声明区 ----------------------------------------------------------- */
/**
//函数名：ledsDriverLedTaskInit
//描述：初始化LED的IO与闪动事件处理状态变量
//参数：无
//返回：void
*/
void ledsDriverLedTaskInit(void);

/**
//函数名：ledsDriverCheckLedBlinkFinish
//描述：查看当前led灯任务处理是否完成,0表示完成
//参数：无
//返回：void
*/
uint16_t ledsDriverCheckLedBlinkFinish(void);

/**
//函数名：ledsDriverLedsControl
//描述：控制对应灯的行为
//参数：led_num            (uint8_t[输入],要设定的对应灯号;)
//      on_delay_ms        (uint16_t[输入],每个周期延时多久亮灯;)
//      on_duration_ms     (uint16_t[输入],每个周期亮灯的时间;0x0000表示常灭,0xFFFF表示常亮;
//                                         当on_delay_ms > 0，且on_delay_ms + on_duration_ms > total_duration_ms 时，此为特殊设定，
//                                         用来处理周期闪灯后，灯保持亮的情况。)
//      total_duration_ms  (uint16_t[输入],每个周期的时间;)
//      repeat_counter     (uint8_t[输入],闪动重复次数;0xFF表示一直闪动,0x00表示停止闪动;)
//返回：void
*/
void ledsDriverLedsControl(uint8_t led_num, uint16_t on_delay_ms,uint16_t on_duration_ms,
                           uint16_t total_duration_ms, uint8_t repeat_counter);

#endif

/***************************************** 文 件 结 束 ************************************************/
