/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:leds-app.c
*Author : JimYao
*Version : 1.0
*Date : 2019-09-11
*Description: LED指示灯运用代码逻辑部分处理
*History:
***************************************************************************************************/

/* 防止重复引用区 ----------------------------------------------------------- */

#ifndef _LEDS_APP_H_
#define _LEDS_APP_H_

/* 相关包含头文件区 --------------------------------------------------------- */
#include "app/framework/include/af.h"

/* 宏定义区 ----------------------------------------------------------------- */

/* 自定义类型区 ------------------------------------------------------------- */
enum {
  LEDS_STATUS_POWER_ON_INIT = 0x00,
  LEDS_STATUS_NETWORK_LEAVED,
  LEDS_STATUS_NETWORK_JOINING,
  LEDS_STATUS_NETWORK_JOINED,
  LEDS_STATUS_SWITCH_1_STATUS_UPDATE,  //switch 1,2,3顺序定义
  LEDS_STATUS_SWITCH_2_STATUS_UPDATE,
  LEDS_STATUS_SWITCH_3_STATUS_UPDATE,
  LEDS_STATUS_IDENTIFY_UPDATE,
  LEDS_STATUS_CHANGE_SWITCHTYPE_UPDATA,
  LEDS_STATUS_SLEEP_END_DEVICE,
  LEDS_STATUS_ROUTE,
  LEDS_STATUS_IDLE,
  LEDS_STATUS_STOP
};
/* 全局函数声明区 ----------------------------------------------------------- */
/**
//函数名：ledsAppInit
//描述：初始化led的状态变量
//参数：无
//返回：void
*/
void ledsAppInit(void);

/**
//函数名：changeLedsStatus
//描述：切换当前系统指示灯
//参数：leds_status (uint8_t [输入]，指示灯状态)
//返回：void
*/
void ledsAppChangeLedsStatus(uint8_t leds_status);

#endif

/***************************************** 文 件 结 束 ************************************************/
