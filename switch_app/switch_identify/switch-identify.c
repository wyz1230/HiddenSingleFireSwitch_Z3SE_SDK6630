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

/* 全局函数声明区 -------------------------------------------------------------- */

/* 本地函数声明区 -------------------------------------------------------------- */

/* 函数原型 -------------------------------------------------------------------- */
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
      switchIdentifyDebugPrintln("start,ep=%d,time=%ld",endpoint,identifyTime);
    }
  }
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
      switchIdentifyDebugPrintln("stop,ep=%d",endpoint);
    }
  }
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
/*************************************** 文 件 结 束 ******************************************/
