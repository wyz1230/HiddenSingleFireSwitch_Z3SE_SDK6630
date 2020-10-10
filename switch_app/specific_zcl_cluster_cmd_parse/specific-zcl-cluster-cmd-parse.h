/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName   : specific-zcl-cluster-cmd-parse.h
*Author     : JimYao
*Version    : 1.0
*Date       : 2019-11-07
*Description: zcl cluster 指令的分析处理，用于SDK处理完zcl cluster 指令后，可做为处理客制指令的入口，
*             配合宏定义USE_ZCL_CLUSTER_SPECIFIC_COMMAND_PARSE_CALLBACK 开关，在SDK的process-cluster-message.c
*             加入对应的回调入口
*History:
***************************************************************************************************/
/* 防止重复引用区 -------------------------------------------------------------- */
#ifndef _SPECIFIC_ZCL_CLUSTER_CMD_PARSE_H_
#define _SPECIFIC_ZCL_CLUSTER_CMD_PARSE_H_

#ifdef USE_ZCL_CLUSTER_SPECIFIC_COMMAND_PARSE_CALLBACK  //jim 20191107 添加处理ZCL特殊指令入口回调
/* 相关包含头文件区 ------------------------------------------------------------ */
#include "app/framework/include/af.h"

/* 宏定义区 -------------------------------------------------------------------- */

/* 自定义类型区 ---------------------------------------------------------------- */
enum{
   POWERUP_CONTROL = 0, //上电控制 
   BUTTON_CONTROL,      //按键控制
   UNICAST_CONTROL,     //单播控制
   SCENE_CONTROL,       //情景控制
   MULTICAST_CONTROL,   //组播控制
   BROADCAST_CONTROL,   //广播控制
};

/* 全局函数声明区 -------------------------------------------------------------- */
/**
//函数名：getOnOffClusterCommandType
//描  述：获取最后一个OnOff cluster的指令类型，是否为广播，多播，单播，按键，上电，情景
//参  数：ways (uint8_t [输入] 第几路开关)
//返  回：uint8_t, 返回指令类型
*/
uint8_t getOnOffClusterCommandType(uint8_t ways);

/**
//函数名：setOnOffClusterCommandType
//描  述：设定onoff最后指令类型
//参  数：ways (uint8_t [输入] 第几路开关)
//        cmd_type (uint8_t [输入] 指令的类型)
//返  回：void
*/
void setOnOffClusterCommandType(uint8_t ways, uint8_t cmd_type);

#endif
#endif
/*************************************** 文 件 结 束 ******************************************/

