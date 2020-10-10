/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName   : device-app-infomation.h
*Author     : JimYao
*Version    : 1.0
*Date       : 2019-10-12
*Description: 重定义设备的相关基本信息
*History    :
***************************************************************************************************/
/* 防止重复引用区 -------------------------------------------------------------- */
#ifndef _DEVICE_APP_INFO_H_
#define _DEVICE_APP_INFO_H_

/* 相关包含头文件区 ------------------------------------------------------------ */

/* 宏定义区 -------------------------------------------------------------------- */
//以下的宏定义信息放到通用的头文件，方便管理。
// #define ORB_APP_VERSION_UINT8            00
// #define ORB_MANUFACTURE_NAME_STRING      "ORVIBO"
// #define ORB_DATE_CODE_STRING             "20190919"
// #define ORB_SW_BUILD_ID_STRING           "0.0.1"

// #define ORB_PRODUCT_NAME_STRING          "MixSwitch"
// #define ORB_OTHER_INFO_STRING            "efr32mg21f768"

//固件命名规则
//产品名称_v主版本号.次版本号.修订版本号_日期_其他信息.后缀
//如 switch1_v1.2.16_20170303_JiGuang.hex

/* 自定义类型区 ---------------------------------------------------------------- */

/* 全局函数声明区 -------------------------------------------------------------- */
/**
//函数名：deviceInfoAppBasicAttributeInit
//描述：初始化设备信息，按宏定义的版本与日期信息更新basic里的相应属性值。
//参数：无
//返回：void
*/
void deviceInfoAppBasicAttributeInit(void);
#if 0 //放到产测代码里
/**
//函数名：deviceInfoAppPowerOnPrint
//描述：上电打印设备相关信息。
//参数：无
//返回：void
*/
void deviceInfoAppPowerOnPrint(void);
#endif
#endif
/*************************************** 文 件 结 束 ******************************************/
