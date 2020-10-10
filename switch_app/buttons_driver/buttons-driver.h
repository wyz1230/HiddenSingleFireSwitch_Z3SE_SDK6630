/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:buttons-driver.h
*Author : JimYao
*Version : 1.1
*Date : 2019-10-10
*Description: 按键驱动及按键扫描任务处理
*History:在1.0版本的基础上,优化时间侦测,减少频繁唤醒
***************************************************************************************************/
/* 防止重复引用区 -------------------------------------------------------------- */
#ifndef _BUTTONS_DRIVER_H_
#define _BUTTONS_DRIVER_H_
/* 相关包含头文件区 ------------------------------------------------------------ */
#include "app/framework/include/af.h"

/* 宏定义区 -------------------------------------------------------------------- */

/* 自定义类型区 ---------------------------------------------------------------- */
typedef enum{
    BUTTON_NONE = 0x00,
    BUTTON_PRESSED_VALID = 0x01,
	BUTTON_PRESSED_1S,
	BUTTON_PRESSED_2S,
	BUTTON_PRESSED_3S,
	BUTTON_PRESSED_4S,
	BUTTON_PRESSED_5S,
	BUTTON_PRESSED_10S,
	BUTTON_RELEASE_SHORT_PRESS = 0x80,
	BUTTON_RELEASE_THAN_1S,
	BUTTON_RELEASE_THAN_2S,
	BUTTON_RELEASE_THAN_3S,
	BUTTON_RELEASE_THAN_4S,
	BUTTON_RELEASE_THAN_5S,
	BUTTON_RELEASE_THAN_10S,
}eButtonState;

typedef void (*fpButtonsHandle)(uint8_t num, eButtonState state); //按键的回调函数类型

/* 全局函数声明区 -------------------------------------------------------------- */
/**
//函数名：getPressedButtonsBitmask
//描述：获取按键对应的按键位表
//参数：无
//返回：uint16_t [输出],按键接下的键位表
*/
uint16_t getPressedButtonsBitmask(void);

/**
//函数名：buttonsDriverButtonInit
//描述：按键任务事件初始化，初始化按键扫描的相关变量。
//参数：handle    (fpButtonsHandle [输入]，按键回调注册)
//返回：无
*/
void buttonsDriverButtonInit(fpButtonsHandle handle);
#endif
/*************************************** 文 件 结 束 ******************************************/

