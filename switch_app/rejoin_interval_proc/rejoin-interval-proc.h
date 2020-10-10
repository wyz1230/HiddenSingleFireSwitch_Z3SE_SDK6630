/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:rejoin-interval-proc.c
*Author : JimYao
*Version : 1.0
*Date : 2019-10-16
*Description: 基于芯科end-device-move.c里的rejoin机制，通过此代码和相应的宏定义，转换为自定义的rejoin
*History:
***************************************************************************************************/

/* 防止重复引用区 ----------------------------------------------------------- */

#ifndef _REJOIN_INTERVAL_PROC_H_
#define _REJOIN_INTERVAL_PROC_H_

#ifdef USE_CUSTOM_RJOIN_CODE //此部分的code需要定义此宏
/* 相关包含头文件区 --------------------------------------------------------- */
#include "app/framework/include/af.h"

/* 宏定义区 ----------------------------------------------------------------- */

/* 自定义类型区 ------------------------------------------------------------- */

/* 全局函数声明区 ----------------------------------------------------------- */
/**
//函数名：setRejoinIntervalTableIndex
//描述：设定当前rejoin index,默认从0开始。
//参数：current_index     (uint8_t [输入]，当前rejoin间隔表的index号)
//      current_try_times (uint8_t [输入]，当前rejoin模式第几次偿试)
//返回：void
*/
void setRejoinIntervalTableIndex(uint8_t current_index, uint8_t current_try_times);

/**
//函数名：checkRejoinEventNeedToContinue
//描述：检查rejoin事件是否需要继续执行,
//参数：无
//返回：bool , true 需要继续触发; false 停止触发。
*/
bool checkRejoinEventNeedToContinue(void);

/**
//函数名：getNextRejoinDelayTimeMs
//描述：获取执行下一个rejoin的延时时间
//参数：无
//返回：uint32_t , 下一个rejoin延时的时间,单位ms。
*/
uint32_t getNextRejoinDelayTimeMs(void);

/**
//函数名：updateRejoinTableIndex
//描述：执行完当前rejoin后，相应index号的运算。
//参数：无
//返回：void
*/
void updateRejoinTableIndex(void);

/**
//函数名：getCurrentRejoinUseSecureMode
//描述：获取当前的rejoin是否需要采用加密的方式进行
//参数：无
//返回：bool , true 采用加密方式; false 采用非加密方式 
//             (此部分还和SDK里的宏义定义有关，当没有定义EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_ALLOW_REJOINS_WITH_WELL_KNOWN_LINK_KEY
//             如果此时的TC LinkKey是5a69key时，只能采用加密方式rejoin。)
*/
bool getCurrentRejoinUseSecureMode(void);

/**
//函数名：getCurrentRejoinUseAllChannelMode
//描述：获取当前rejoin是否采用全信道方式
//参数：无
//返回：bool , true 采用全信道方式; false 采用当前信道方式 
*/
bool getCurrentRejoinUseAllChannelMode(void);

/**
//函数名：checkNetworkStateAndTrigeRejoin
//描述：判断网络与设备类型，触发一特定rejoin动作。
//参数：无
//返回：void
*/
void checkNetworkStateAndTrigeRejoin(void);

#ifdef SUPPORT_EACH_CHANNELS_REJOIN    //支持每个信道单独rejoin
/**
//函数名：getCurrentRejoinChannelMask
//描述：获取当前rejoin是否采用全信道方式
//参数：无
//返回：uint32_t , rejoin的信道MASK值。0：为当前信道 当前只支持11-26信道
*/
uint32_t getCurrentRejoinChannelMask(void);
#endif

#endif

#endif

/***************************************** 文 件 结 束 ************************************************/
