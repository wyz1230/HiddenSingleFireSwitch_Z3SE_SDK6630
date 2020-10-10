/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:relay-control-buffer.h
*Author : JimYao
*Version : 1.0
*Date : 2019-10-17
*Description: 此代码是防止几路继电器同时控制时，产生的电流过大，电池供电的设备会产生电池撑不住的情况。
*             此部分的功能是把要执行的控制继电器的动作先放入一个缓存，再按序执行。
*History:
***************************************************************************************************/

/* 防止重复引用区 ----------------------------------------------------------- */

#ifndef _RELAY_CONTORL_BUFFER_H_
#define _RELAY_CONTORL_BUFFER_H_

/* 相关包含头文件区 --------------------------------------------------------- */
#include "app/framework/include/af.h"

/* 宏定义区 ----------------------------------------------------------------- */

/* 自定义类型区 ------------------------------------------------------------- */

/* 全局函数声明区 ----------------------------------------------------------- */
/**
//函数名：relayControlBufferInitial
//描述：初始化继电器控制动作的缓存。
//参数：无
//返回：void
*/
void relayControlBufferInitial(void);

/**
//函数名：updateAndTrigeRelayControlBufferNextAction
//描述：写继电器控制开关动作到缓存中，由调缓存调用，防止几个继电器同时开关。
//参数：way      (uint8_t[输入],需要控制第几路开关，0，1，2;)
//      action   (uint8_t[输入],需要执行的动作，on->1, off->0;)
//      delay    (uint8_t[输入],此动作延时多久ms后执行)
//      add_flag (bool   [输入],true 表示新加入的动作；false 表示已经完成的动作更新,此时参数 action, delay 无作用)
//返回：void
*/
void updateAndTrigeRelayControlBufferNextAction(uint8_t way, uint8_t action, uint8_t delay, bool add_flag);

#endif

/***************************************** 文 件 结 束 ************************************************/
