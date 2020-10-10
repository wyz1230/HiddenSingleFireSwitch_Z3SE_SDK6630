/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName   : software-reboot-proc.h
*Author     : JimYao
*Version    : 1.0
*Date       : 2019-11-04
*Description: 软件重启处理
*History:
***************************************************************************************************/

/* 防止重复引用区 ----------------------------------------------------------- */

#ifndef _SOFTWARE_REBOOT_PROC_H_
#define _SOFTWARE_REBOOT_PROC_H_

/* 相关包含头文件区 --------------------------------------------------------- */
#include "app/framework/include/af.h"

/* 宏定义区 ----------------------------------------------------------------- */
#define SWITCH_REBOOT_REASON_OTA          0x00   //ota切换到bootloader重启
#define SWITCH_REBOOT_REASON_LONG_WAKEUP  0x01   //长时间唤醒不休眠
#define SWITCH_REBOOT_REASON_NONE         0xFF   //正常启动

/* 自定义类型区 ------------------------------------------------------------- */

/* 全局函数声明区 ----------------------------------------------------------- */
/**
//函数名：switchRebootCheckToRecoverStatus
//描述  ：检测是否是软件产生的复位，恢复复位前的状态
//参数  ：无
//返回  ：bool, true软件产生的复位，已经恢复到复位前状态; false 不是软件产生的复位，不处理
*/
bool switchRebootCheckToRecoverStatus(void);
/**
//函数名：preRebootStatusSaveProc
//描述  ：设备重启前的状态保存与重启原因处理
//参数  ：reason (uint8_t [输入]，重启的原因，供软件重启处理用)
//返回  ：void
*/
void preRebootStatusSaveProc(uint8_t reason);
#endif

/***************************************** 文 件 结 束 ************************************************/
