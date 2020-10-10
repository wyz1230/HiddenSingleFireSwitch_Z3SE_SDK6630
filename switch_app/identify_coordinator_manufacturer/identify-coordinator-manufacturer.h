/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName   : identify-coordinator-manufacturer.c
*Author     : JimYao
*Version    : 1.0
*Date       : 2019-11-08
*Description: 设备入网后，主动识别网关产商名；主要用来识别ORB主自网关，处理自主的特定行为相关事项。
*History:
***************************************************************************************************/
/* 防止重复引用区 -------------------------------------------------------------- */
#ifndef _IDENTIFY_COORDINATOR_MANUFACTURE_H_
#define _IDENTIFY_COORDINATOR_MANUFACTURE_H_

/* 相关包含头文件区 ------------------------------------------------------------ */
#include "app/framework/include/af.h"

/* 宏定义区 -------------------------------------------------------------------- */

/* 自定义类型区 ---------------------------------------------------------------- */

/* 全局函数声明区 -------------------------------------------------------------- */
/**
//函数名：identifyCoordinatorManufactureProcInit
//描  述：初始化网关产商信息
//参  数：无
//返  回：void
*/
void identifyCoordinatorManufactureProcInit(void);

/**
//函数名：checkToStartIdentifyCoordinatorManufacturer
//描  述：获取网关产商信息处理步骤
//参  数：delay_ms (uint16_t [输入]延时多少ms启动获取网关信息)
//返  回：void
*/
void checkToStartIdentifyCoordinatorManufacturer(uint16_t delay_ms);

/**
//函数名：whetherIsOrviboCoordinator
//描  述：判断是否为ORB的网关
//参  数：无
//返  回：bool; true ORB网关，false 其它网关
*/
bool whetherIsOrviboCoordinator(void);

/**
//函数名：clearCoordinatorManufacturerInfo
//描  述：清除网关产商信息
//参  数：无
//返  回：void
*/
void clearCoordinatorManufacturerInfo(void);
/**
//函数名：orviboCoordinatorIdentifyFinishCallback
//描  述：第一次辨识到是ORB的网关产生的回调
//参  数：无
//返  回：void
*/
void orviboCoordinatorIdentifyFinishCallback(void);
#endif
/*************************************** 文 件 结 束 ******************************************/

