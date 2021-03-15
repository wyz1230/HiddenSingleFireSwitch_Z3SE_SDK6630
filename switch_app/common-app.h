/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:common-app.h
*Author : JimYao
*Version : 1.0
*Date : 2019-09-16
*Description: 通用头文件包含
*History:
***************************************************************************************************/
/* 防止重复引用区 -------------------------------------------------------------- */
#ifndef _COMMON_APP_H_
#define _COMMON_APP_H_
/* 相关包含头文件区 ------------------------------------------------------------ */
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include PLATFORM_HEADER

#include "buttons_driver/buttons-driver.h"
#include "buttons_app/buttons-app.h"
#include "leds_driver/leds-driver.h"
#include "leds_app/leds-app.h"
#include "network_status_process/network-status-process.h"
#include "relay_control_driver/relay-control-driver.h"
#include "switch_ways_config/switch-ways-config.h"
#include "device_app_infomation/device-app-infomation.h"
#include "relay_control_buffer/relay-control-buffer.h"
#include "rejoin_interval_proc/rejoin-interval-proc.h"
#include "switch_identify/switch-identify.h"
#include "software_reboot_proc/software-reboot-proc.h"
#include "specific_zcl_cluster_cmd_parse/specific-zcl-cluster-cmd-parse.h"
#include "identify_coordinator_manufacturer/identify-coordinator-manufacturer.h"
#include "production_test/tiny_printf/tiny_printf.h"
#include "production_test/production-test.h"

/* 宏定义区 -------------------------------------------------------------------- */
#define MIX_SW_APP_DEBUG_ENABLE
//#define LED_APP_DEBUG_ENABLE
//#define LED_DRIVER_DEBUG_ENABLE
//#define BUTTON_DRIVER_DEBUG_ENABLE
#define BUTTON_APP_DEBUG_ENABLE
//#define RELAY_CONTROL_DRIVER_DEBUG_ENABLE
//#define SCENES_RECALL_TRAN_DEBUG_ENABLE
#define NETWORK_STATUS_PROCESS_DEBUG_ENABLE
//#define SWITCH_WAYS_CONFIG_DEBUG_ENABLE
//#define DEVICE_INFO_APP_DEBUG_ENABLE
//#define REJOIN_INTERVAL_PROC_DEBUG_ENABLE
//#define RELAY_CONTROL_BUFFER_DEBUG_ENABLE
//#define BATTERY_VOLTAGE_ADC_DEBUG_ENABLE
//#define BATTERY_CAPACITY_CACULATE_DEBUG_ENABLE
//#define SPECIFIC_ZCL_CLUSTER_CMD_PARSE_DEBUG_ENABLE
//#define IDENTIFY_COORDINATOR_MANUFACTURE_DEBUG_ENABLE
//#define RELAY_CONTROL_DRIVER_DEBUG_ENABLE

#define LIVE_LINE_ONLY         1  //单火线版本
#define LIVE_NAUGHT_LINE       2  //零火线版本
#define HIDDEN_SINGFIRE_GENERAL  3 //隐藏式单火版本.通用版本，默认上电关闭，默认是翘板开关，控制翘板开关的方式是固定方式
#define HIDDEN_SINGFIRE_THAILAND  4 //泰国，默认上电是跟随翘板开关状态，默认是翘板开关，控制翘板开关的方式是固定方式

#define POWER_LINE_SUPPLY      HIDDEN_SINGFIRE_GENERAL		

#define SWITCH_TYPE_QIAOBAN			0//默认为翘板型
#define SWITCH_TYPE_DIANCHU         1//点触型

#define ROCKERSWITCH_FIXED_ACTION	0 //翘板开关固定方向开关
#define ROCKERSWITCH_TOGGLE_ACTION	1 //翘板开关动作翻转

#if(POWER_LINE_SUPPLY == LIVE_LINE_ONLY)
#if 0
  #define SWITCH_1_GAND_MODELID_STRING   "93489b897f41465f89e11114cc8ed2d8"
  #define SWITCH_2_GAND_MODELID_STRING   "c70a330ef37442a3b1f9259f8abe5c47"
  #define SWITCH_3_GAND_MODELID_STRING   "daf5e207ee974081aa995336293f41e6"
  #pragma message("ORVIBO MixSwitch 单火版")
#endif
  #define SWITCH_1_GAND_MODELID_STRING   "2ae011fb6d0542f58705d6861064eb5f"
  #define SWITCH_2_GAND_MODELID_STRING   "b11c438ea86f416b9026b2526b7abe84"
  #define SWITCH_3_GAND_MODELID_STRING   "9ea4d5d8778d4f7089ac06a3969e784b"//"e8d667cb184b4a2880dd886c23d00976"
  #pragma message("ORVIBO MixSwitch 单火版采用零火的model id")
#elif(POWER_LINE_SUPPLY == LIVE_NAUGHT_LINE)

  #define SWITCH_1_GAND_MODELID_STRING   "2ae011fb6d0542f58705d6861064eb5f"
  #define SWITCH_2_GAND_MODELID_STRING   "b11c438ea86f416b9026b2526b7abe84"
  #define SWITCH_3_GAND_MODELID_STRING   "e8d667cb184b4a2880dd886c23d00976"
  #pragma message("ORVIBO MixSwitch 零火版")
#elif(POWER_LINE_SUPPLY == HIDDEN_SINGFIRE_GENERAL)  
  #define HIDDEN_SINGFIRE_MODELID_STRING "2810c2403b9c4a5db62cc62d1030d95e" 
#elif(POWER_LINE_SUPPLY == HIDDEN_SINGFIRE_THAILAND)  
  #define HIDDEN_SINGFIRE_MODELID_STRING "733f264889b948c198c5f8bf2ba3b28a" 
#endif


//定义版本信息，此部分会在上电打印和更新basic属性值。
#ifdef EMBER_AF_PLUGIN_OTA_CLIENT_POLICY_FIRMWARE_VERSION
  #define ORB_APP_VERSION_UINT8           EMBER_AF_PLUGIN_OTA_CLIENT_POLICY_FIRMWARE_VERSION
#else
  #define ORB_APP_VERSION_UINT8           1
  #warning "ota file version undefine!"
#endif
#define ORB_MANUFACTURE_NAME_STRING      "ORVIBO"
#define ORB_DATE_CODE_STRING             "20210315"
#define ORB_SW_BUILD_ID_STRING           "2.0.2"

#define ORB_PRODUCT_NAME_STRING          "SingleFireSwitch-MG21"
#define ORB_OTHER_INFO_STRING            "EFR32"


#define MAIN_POWER_RELAY_NUMBER   0 //jim add 202007116
#define	ONOFF_TYPE			 0
#define SWITCH_ALL_SET_TYPE	 1

#define POWERON_STATUS_ON			0x0
#define POWERON_STATUS_OFF			0x01
#define POWERON_STATUS_MEMORY		0x02
#define POWERON_STATUS_FOLLOW		0x03

#define RELAY_ON		1
#define RELAY_OFF		0

#define SWITCH_TYPE_BIT 		  4
#define ROCKERSWITCH_ACTION_TYPE  5

/* 自定义类型区 ---------------------------------------------------------------- */

/* 全局函数声明区 -------------------------------------------------------------- */
/**
//函数名：appPowerOnInit
//描述：设备上电后初始化，留此接接口方便产测超时退出时使用
//参数：无
//返回：void
*/
void appPowerOnInit(void);

/**
//函数名：setFlashDeviceType
//描述：将设备类型存进flash，route为2，enddevice为4
//参数：无
//返回：void
*/
void setFlashDeviceType(uint8_t device_type);

/**
//函数名：getFlashDeviceType
//描述：从flash获取设备类型
//参数：无
//返回：void
*/
uint8_t getFlashDeviceType(void);

/**
//函数名：getDeviceType
//描述：获取设备类型，route为2，enddevice为4
//参数：无
//返回：void
*/
uint8_t getDeviceType(void);

/**
//函数名：getPowerOnStartingFlag
//描  述：获取设备上电启动状态
//参  数：无
//返  回：bool, true:正在启动处理中; false:完成启动处理
*/
bool getPowerOnStartingFlag(void);

/**
//函数名：setButtonTrigType
//描  述：设定按键触发类型
//参  数：way [in] uint8_t 哪一路
//返  回：void
*/
void setButtonTrigType(uint8_t way);

/**
//函数名：resetAddNetProcess
//描述：启动复位加网
//参数：无
//返回：void
*/
void resetAddNetProcess(void);

/**
 //函数名：writeAttributeCallBack
 //描述：写相关属性值
 //参数：endpoint: 节点
 // 	 type: 0为上电状态类型 1为开关类型
 // 	 value: 待写入的onoff或亮度或色温值
 //返回：void
 */
 void writeAttributeCallBack(uint8_t type, uint16_t value);

 /**
 //函数名：readStorageCallBack
 //描述：获取相关token值
 // 	 上电初始化调用
 //参数：type: 0为开关状态 1为上电状态类型 2为开关类型
 //返回：从存储区读出的存储值
 */
 uint16_t readStorageCallBack(uint8_t type,uint8_t ep);


#endif
/*************************************** 文 件 结 束 ******************************************/
