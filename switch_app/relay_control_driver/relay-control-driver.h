/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:relay-control-driver.h
*Author : JimYao
*Version : 1.1
*Date : 2019-10-08
*Description: 继电器与可控硅开关控制驱动,此部分是驱动继电器控制IC，MS3111S而修改。
*History:
***************************************************************************************************/

/* 防止重复引用区 ----------------------------------------------------------- */

#ifndef _RELAY_CONTORL_DRIVER_H_
#define _RELAY_CONTORL_DRIVER_H_

/* 相关包含头文件区 --------------------------------------------------------- */
#include "app/framework/include/af.h"

/* 宏定义区 ----------------------------------------------------------------- */

/* 自定义类型区 ------------------------------------------------------------- */
enum{
  CONTROL_RELAY_0_0 = 0x00,   //relay0脚控制号
  CONTROL_RELAY_1_0,
  CONTROL_RELAY_2_0,
  CONTROL_RELAY_0_1,          //relay1脚控制号
  CONTROL_RELAY_1_1,
  CONTROL_RELAY_2_1,
  CONTROL_ALL_RELAYS_0,       //所有继电器0脚控制号
  CONTROL_ALL_RELAYS_1,       //所有继电器1脚控制号
  CONTROL_ALL,                //所有控制脚控制号
};

enum{
  RELAY_CONTROL_TURN_OFF = 0x00, //关控制
  RELAY_CONTROL_TURN_ON,         //开控制
};

typedef void (*fpRelayControlCallback)(uint8_t way, uint8_t status); //继电器控制回调函数原型

/* 全局函数声明区 ----------------------------------------------------------- */

/**
//函数名：relayContorlDriverTaskInit
//描述：初始化控制脚的IO与控制任务事件处理状态变量，默认过渡功能是开的
//参数：control_cb (fpRelayControlCallback [输入]，控制状态回调注册)
//返回：void
*/
void relayContorlDriverTaskInit(fpRelayControlCallback control_cb);

/**
//函数名：relayControlDriverTrigeOnOffAction
//描述：触发继电器开关动作
//参数：way    (uint8_t[输入],需要控制第几路开关，0，1，2;)
//      action (uint8_t[输入],需要执行的动作，on->1, off->0;)
//      delay  (uint8_t[输入],此动作延时多久ms后执行)
//返回：void
*/
void relayControlDriverTrigeOnOffAction(uint8_t way, uint8_t action, uint8_t delay);

/**
//函数名：relayControlDriverGetCurrenStatus
//描述：触发继电器开关动作
//参数：way    (uint8_t[输入],需要获取第几路开关状态，0，1，2;)
//返回：uint8_t (返回当前此路开关的状态，0关，1开，0xFF表示有误。
*/
uint8_t relayControlDriverGetCurrenStatus(uint8_t way);
#endif

/***************************************** 文 件 结 束 ************************************************/
