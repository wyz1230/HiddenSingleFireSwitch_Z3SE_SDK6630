/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:network-status-process.h
*Author : JimYao
*Version : 1.0
*Date : 2019-09-11
*Description: 网络状态变化处理，如加网，离网，
*History:
***************************************************************************************************/
/* 防止重复引用区 -------------------------------------------------------------- */
#ifndef _NETWORK_STATUS_PROCESS_H_
#define _NETWORK_STATUS_PROCESS_H_
/* 相关包含头文件区 ------------------------------------------------------------ */
#include "app/framework/include/af.h"

/* 宏定义区 -------------------------------------------------------------------- */

/* 自定义类型区 ---------------------------------------------------------------- */
enum{
  NETWORK_ACTION_NONE = 0x00,      //无其它动作
  NETWORK_ACTION_LEAVE,            //只离网
  NETWORK_ACTION_START_JOIN,       //触发加网
  NETWORK_ACTION_LEAVE_AND_JOIN,   //离网后加网
  NETWORK_ACTION_LEAVE_AND_REBOOT, //离网后重启
  //jim add 20200717
  NETWORK_ACTION_DELAY_AND_START_JOIN, //延时后加网
  NETWORK_ACTION_LEAVE_AND_STOP,       //离网后停止加网
};

/* 全局函数声明区 -------------------------------------------------------------- */
/**
//函数名：clearOtaStorageTempDataAll
//描述：清除ota buffer里的头尾部数据
//参数：无
//返回：void
*/
void clearOtaStorageTempDataAll(void);
/**
//函数名：nodeInfoDeafultReset
//描述：节点信息恢复默认值
//参数：无
//返回：void
*/
void nodeInfoDeafultReset(void);
/**
//函数名：networkStatusProcessInit
//描述：网络状态上电初始化处理
//参数：无
//返回：无
*/
void networkStatusProcessInit(void);
/**
//函数名：networkStatusUpdateShow
//描述：网络状态更新显示
//参数：无
//返回：无
*/
void networkStatusUpdateShow(void);
/**
//函数名：netStatusChangeProcess
//描述：网络状态变化处理函数
//参数：无
//返回：无
*/
void networkStatusChangeProcess(EmberStatus status);

/**
//函数名：networkStatusTrigeNetworkAction
//描述：触发网络动作，离网，入网，离网后重启等
//参数：action_type (uint8_t [输入]，动作类型)
//返回：无
*/
void networkStatusTrigeNetworkAction(uint8_t action_type);

#endif
/*************************************** 文 件 结 束 ******************************************/

