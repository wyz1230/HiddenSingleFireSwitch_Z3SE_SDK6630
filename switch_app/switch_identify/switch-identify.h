/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName   : switch-identify.c
*Author     : JimYao
*Version    : 1.0
*Date       : 2019-11-01
*Description: identify提示标志更新
*History:
***************************************************************************************************/
/* 防止重复引用区 -------------------------------------------------------------- */
#ifndef _SWITCH_IDENTIFY_H_
#define _SWITCH_IDENTIFY_H_
/* 相关包含头文件区 ------------------------------------------------------------ */
#include "app/framework/include/af.h"

/* 宏定义区 -------------------------------------------------------------------- */


/* 自定义类型区 ---------------------------------------------------------------- */


/* 全局函数声明区 -------------------------------------------------------------- */
/**
//函数名：identifyPollingEventHandler
//描述：获取外部配置开关路数
//参数：无
//返回：uint8_t 当前正常indentify中的标志，每个bit对应一路
*/
uint8_t getIdentifyingFlg(void);

/**
//函数名：app_IdentifyDeviceEventHandler
//描述：获取查找的时候设置的endpoint
//参数：void
//返回：endpoint
*/
uint8_t getIdentifySetEndpoint(void);

/**
//函数名：getIdentifyStatus
//描述：获取identify状态
//参数：void
//返回：startIdentifyFlg
*/
uint8_t getIdentifyStatus(void);
#endif
/*************************************** 文 件 结 束 ******************************************/

