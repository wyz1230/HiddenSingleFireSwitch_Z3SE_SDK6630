/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:buttons-driver.h
*Author : JimYao
*Version : 1.0
*Date : 2019-08-26
*Description: 按键驱动及按键扫描任务处理
*History:
***************************************************************************************************/
/* 防止重复引用区 -------------------------------------------------------------- */
#ifndef _BUTTONS_APP_H_
#define _BUTTONS_APP_H_
/* 相关包含头文件区 ------------------------------------------------------------ */
#include "app/framework/include/af.h"

/* 宏定义区 -------------------------------------------------------------------- */


/* 自定义类型区 ---------------------------------------------------------------- */


extern EmberEventControl app_RestartEventControl;
/* 全局函数声明区 -------------------------------------------------------------- */
/**
//函数名：buttonsAppInit
//描述：初始化按键运用代码相关的变量及注册按键驱动处理回调函数
//参数：无
//返回：无
*/
void buttonsAppInit(void);

/**
//函数名：setSwitchType
//描述：设置开关类型
//参数：type (uint8_t [输入]，0:翘板型 1:点触型)
//返回：void
*/
void setSwitchType(uint8_t type);

/**
//函数名：getSwitchType
//描述：设置开关类型
//参数：void
//返回：0:翘板型 1:点触型
*/
uint8_t getSwitchType(void);

/**
//函数名：setRockerSwitchActionType
//描述：设置翘板开关动作类型
//参数：type (uint8_t [输入]，0:固定方向开关 1:翻转)
//返回：void
*/
void setRockerSwitchActionType(uint8_t type);

/**
//函数名：getRockerSwitchActionType
//描述：获取翘板开关动作类型
//参数：void
//返回：0:固定方向开关 1:翻转
*/
uint8_t getRockerSwitchActionType(void);


/**
//函数名：getPowerOnRockerStatus
//描述：获取上电时翘板开关实际的状态
//参数：void
//返回：无
*/
uint8_t getPowerOnRockerStatus(void);

/**
//函数名：syncButtonAndSwitchStatus
//描述：同步按键与开关状态
//参数：void
//返回：无
*/
void syncButtonAndSwitchStatus(void);

#endif
/*************************************** 文 件 结 束 ******************************************/

