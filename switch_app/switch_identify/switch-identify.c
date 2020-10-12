/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName   : switch-identify.c
*Author     : JimYao
*Version    : 1.0
*Date       : 2019-11-01
*Description: identify提示标志更新
*History:
***************************************************************************************************/
/* 相关包含头文件区 ------------------------------------------------------------ */
//#include "switch-identify.h"
#include "common-app.h"

/* 宏定义区 -------------------------------------------------------------------- */
//debug开关设定
//#define SWITCH_IDENTIFY_DEBUG_ENABLE
#ifdef SWITCH_IDENTIFY_DEBUG_ENABLE
  #define DEBUG_STRING                        "Debug-SwIdentify:"
  #define switchIdentifyDebugPrint(...)       emberAfPrint(0xFFFF,DEBUG_STRING __VA_ARGS__)
  #define switchIdentifyDebugPrintln(...)     emberAfPrintln(0xFFFF,DEBUG_STRING __VA_ARGS__)
#else
  #define switchIdentifyDebugPrint(...)
  #define switchIdentifyDebugPrintln(...)
#endif

#define set_bit(byte, bit)          (byte |= 1 << bit)
#define reset_bit(byte, bit)        (byte &= ~(1 << bit))
#define get_bit(byte, bit)          ((byte & (1 << bit)) >> bit)
/* 自定义类型区 ---------------------------------------------------------------- */

/* 全局变量区 ------------------------------------------------------------------ */

/* 本地变量区 ------------------------------------------------------------------ */
static uint8_t ways_identifying_flag = 0x00;  //每个bit对应一路正在identify的标志。
EmberEventControl app_IdentifyDeviceControl;
static uint8_t relay_action_cnt; //继电器动作次数，用于开/关
static uint8_t setEndpointNum; //identify操作哪个endpoint的序号
static uint8_t startIdentifyFlg =false; //identify开始标志

/* 全局函数声明区 -------------------------------------------------------------- */

/* 本地函数声明区 -------------------------------------------------------------- */
void operationRelay(uint8_t endpoint);

/* 函数原型 -------------------------------------------------------------------- */
/*
1、智能开关的查找都是三个led闪烁。
2 、灯的查找，就分成两种情况：
情况1：绑定在其他设备或者无绑定，点击灯的查找，只有灯负载闪烁。 
情况2：本地绑定，就是一个led和负载同时闪烁。
*/
/** @brief Start Feedback
 *
 * This function is called by the Identify plugin when identification begins.
 * It informs the Identify Feedback plugin that it should begin providing its
 * implemented feedback functionality (e.g. LED blinking, buzzer sounding,
 * etc.) until the Identify plugin tells it to stop. The identify time is
 * purely a matter of informational convenience; this plugin does not need to
 * know how long it will identify (the Identify plugin will perform the
 * necessary timekeeping.)
 *
 * @param endpoint The identifying endpoint Ver.: always
 * @param identifyTime The identify time Ver.: always
 */
void emberAfPluginIdentifyStartFeedbackCallback(uint8_t endpoint,
                                                uint16_t identifyTime)
{
  uint8_t ways_temp = switchWaysConfigGetWays();
  for (uint8_t i=0; i<ways_temp; i++)
  {
    if (endpoint == emberAfEndpointFromIndex(i))
    {
      set_bit(ways_identifying_flag,i);
	  if(identifyTime)
	  	emberEventControlSetDelayMS(app_IdentifyDeviceControl,2000);		  
      switchIdentifyDebugPrintln("start,ep=%d,time=%ld",endpoint,identifyTime);
    }
  }
  startIdentifyFlg =true;
  setEndpointNum =endpoint;
  relay_action_cnt =0; 
  if(identifyTime)
  	operationRelay(endpoint);  
  ledsAppChangeLedsStatus(LEDS_STATUS_IDENTIFY_UPDATE);
}

/** @brief Stop Feedback
 *
 * This function is called by the Identify plugin when identification is
 * finished. It tells the Identify Feedback plugin to stop providing its
 * implemented feedback functionality.
 *
 * @param endpoint The identifying endpoint Ver.: always
 */
void emberAfPluginIdentifyStopFeedbackCallback(uint8_t endpoint)
{
  uint8_t ways_temp = switchWaysConfigGetWays();
  for (uint8_t i=0; i<ways_temp; i++)
  {
    if (endpoint == emberAfEndpointFromIndex(i))
    {
      reset_bit(ways_identifying_flag,i);
	  emberEventControlSetInactive(app_IdentifyDeviceControl);
	  relay_action_cnt =0;	  
      switchIdentifyDebugPrintln("stop,ep=%d",endpoint);
    }
  }
  setEndpointNum =endpoint;
  startIdentifyFlg =false;
  ledsAppChangeLedsStatus(LEDS_STATUS_IDENTIFY_UPDATE);
}

/**
//函数名：identifyPollingEventHandler
//描述：获取外部配置开关路数
//参数：无
//返回：uint8_t 当前正常indentify中的标志，每个bit对应一路
*/
uint8_t getIdentifyingFlg(void)
{
  return ways_identifying_flag;
}

/**
//函数名：operationRelay
//描述：根据现有继电器状态去开关继电器
//参数：endpoint
//返回：void
*/
void operationRelay(uint8_t endpoint)
{
	uint8_t zclCmdType =0,i;
	uint8_t ways_temp = switchWaysConfigGetWays();
    for (i=0; i<ways_temp; i++)
    {
      if (endpoint == emberAfEndpointFromIndex(i)) //找出是哪一路
      {
    	  //非本地绑定不执行继电器和指示灯逻辑
    	  #if 0
    	  if(isLocalBindType(endpoint) != true)
    	  {
    		  continue;
    	  }
		  #endif

		if (relayControlDriverGetCurrenStatus(i) == RELAY_CONTROL_TURN_ON) //开的状态，这次是关
		{
		  zclCmdType =ZCL_OFF_COMMAND_ID;

		}
		else //关的状态，指示灯亮
		{
		  zclCmdType =ZCL_ON_COMMAND_ID;
		}
		emberAfOnOffClusterSetValueCallback(endpoint, zclCmdType, false);
		switchIdentifyDebugPrintln("operationRelay:%d,%d,%d",zclCmdType,i,emberAfEndpointFromIndex(i));
      }
    }
}

/**
//函数名：app_IdentifyDeviceEventHandler
//描述：查找操作负载执行的handler.2s的周期开关，动作三次
//参数：void
//返回：void
*/
void app_IdentifyDeviceEventHandler(void)
{
    if(relay_action_cnt >=2)
   	{
		emberEventControlSetInactive(app_IdentifyDeviceControl);
	}
	else
	{
		emberEventControlSetDelayMS(app_IdentifyDeviceControl,2000);
	}
	
    operationRelay(setEndpointNum);

	relay_action_cnt ++;		
}

/**
//函数名：app_IdentifyDeviceEventHandler
//描述：获取查找的时候设置的endpoint
//参数：void
//返回：endpoint
*/
uint8_t getIdentifySetEndpoint(void)
{
	return setEndpointNum;
}

/**
//函数名：getIdentifyStatus
//描述：获取identify状态
//参数：void
//返回：startIdentifyFlg
*/
uint8_t getIdentifyStatus(void)
{
	return startIdentifyFlg;
}
/*************************************** 文 件 结 束 ******************************************/
