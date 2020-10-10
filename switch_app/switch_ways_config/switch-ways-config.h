/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:switch-ways-config.h
*Author : JimYao
*Version : 1.0
*Date : 2019-09-10
*Description: 多路开关配置接口
*History:
***************************************************************************************************/
/* 防止重复引用区 -------------------------------------------------------------- */
#ifndef _SWITCH_WAYS_CONFIG_H_
#define _SWITCH_WAYS_CONFIG_H_
/* 相关包含头文件区 ------------------------------------------------------------ */
#include "app/framework/include/af.h"

/* 宏定义区 -------------------------------------------------------------------- */

#define SWITCH_1WAY           (1U)  //1路开关
#define SWITCH_2WAYS          (2U)  //2路开关
#define SWITCH_3WAYS          (3U)  //3路开关

/* 自定义类型区 ---------------------------------------------------------------- */


/* 全局函数声明区 -------------------------------------------------------------- */
/**
//函数名：switchWaysConfigInit
//描述：开关路数自适应初始化，按开关路路数初始化对应的endpoint。
//      此函数需要在emberAfMainInitCallback里调用
//参数：无
//返回：无
*/
void switchWaysConfigInit(void);

/**
//函数名：switchWaysConfigUpdateEndpointModelId
//描述：更新多路开关对应路数的endpoint的model id值,此函数不能在emberAfMainInitCallback里调用，需要在
//      系统进入while后调用才不会给SDK的初始化代码覆盖。
//参数：无
//返回：无
*/
void switchWaysConfigUpdateEndpointModelId(void);

/**
//函数名：switchWaysConfigGetWays
//描述：获取当前开关的路数
//参数：无
//返回：无
*/
uint8_t switchWaysConfigGetWays(void);

#endif
/*************************************** 文 件 结 束 ******************************************/

