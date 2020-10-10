/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:switch-ways-config.c
*Author : JimYao
*Version : 1.0
*Date : 2019-09-10
*Description: 多路开关配置接口
*History:
***************************************************************************************************/
/* 相关包含头文件区 ------------------------------------------------------------ */
//#include "switch-ways-config.h"
#include "common-app.h"

/* 宏定义区 -------------------------------------------------------------------- */
//debug开关设定
//#define SWITCH_WAYS_CONFIG_DEBUG_ENABLE
#ifdef SWITCH_WAYS_CONFIG_DEBUG_ENABLE
  #define DEBUG_STRING                        "Debug-SwWaysCfg:"
  #define switchWaysConfigDebugPrint(...)     emberAfPrint(0xFFFF,DEBUG_STRING __VA_ARGS__)
  #define switchWaysConfigDebugPrintln(...)   emberAfPrintln(0xFFFF,DEBUG_STRING __VA_ARGS__)
#else
  #define switchWaysConfigDebugPrint(...)
  #define switchWaysConfigDebugPrintln(...)
#endif

//配置采用两个IO口的高低电平组合来辨识
#define SWITCH_WAYS_CONFIG0_PORT           (gpioPortB)
#define SWITCH_WAYS_CONFIG0_PIN            (0U)

#define SWITCH_WAYS_CONFIG1_PORT           (gpioPortB)
#define SWITCH_WAYS_CONFIG1_PIN            (1U)

/* 自定义类型区 ---------------------------------------------------------------- */

/* 全局变量区 ------------------------------------------------------------------ */

/* 本地变量区 ------------------------------------------------------------------ */
static uint8_t switch_x_ways = SWITCH_2WAYS; //jim SWITCH_3WAYS; //此变量用来记录当前配置为哪种类的开关

/* 全局函数声明区 -------------------------------------------------------------- */

/* 本地函数声明区 -------------------------------------------------------------- */

/* 函数原型 -------------------------------------------------------------------- */
#if 0 //jim
/**
//函数名：getSwitchWaysConfigSetting
//描述：获取外部配置开关路数
//参数：无
//返回：无
*/
static uint8_t getSwitchWaysConfigSetting(void)
{
  uint8_t config_val = 0, return_val = SWITCH_1WAY;
    //初始化IO口配置
  /* Enable GPIO in CMU */
#if !defined(_SILICON_LABS_32B_SERIES_2)
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
#endif

  GPIO_PinModeSet(SWITCH_WAYS_CONFIG0_PORT,
                  SWITCH_WAYS_CONFIG0_PIN,
                  gpioModeInputPull,
                  1);
  GPIO_PinModeSet(SWITCH_WAYS_CONFIG1_PORT,
                  SWITCH_WAYS_CONFIG1_PIN,
                  gpioModeInputPull,
                  1);
  GPIO_PinInGet(SWITCH_WAYS_CONFIG0_PORT, SWITCH_WAYS_CONFIG0_PIN);
  GPIO_PinInGet(SWITCH_WAYS_CONFIG1_PORT, SWITCH_WAYS_CONFIG1_PIN);

  //简单的延时处理
  volatile uint16_t delay = 0;
  while (delay < 1000)
  {
    delay++;
  }
  //再读取一次IO口状态
  if (GPIO_PinInGet(SWITCH_WAYS_CONFIG0_PORT, SWITCH_WAYS_CONFIG0_PIN))
  {
    config_val |= 0x01;
  }
  if (GPIO_PinInGet(SWITCH_WAYS_CONFIG1_PORT, SWITCH_WAYS_CONFIG1_PIN))
  {
    config_val |= 0x02;
  }

  if (0x00 == config_val)
  {
    return_val = SWITCH_1WAY;
  }
  else if (0x01 == config_val)
  {
    return_val = SWITCH_2WAYS;
  }
  else if (0x03 == config_val)
  {
    return_val = SWITCH_3WAYS;
  }
  GPIO_PinModeSet(SWITCH_WAYS_CONFIG0_PORT,
                  SWITCH_WAYS_CONFIG0_PIN,
                  gpioModeDisabled,
                  1);
  GPIO_PinModeSet(SWITCH_WAYS_CONFIG1_PORT,
                  SWITCH_WAYS_CONFIG1_PIN,
                  gpioModeDisabled,
                  1);

  switchWaysConfigDebugPrintln("get config,ways=%d",return_val);
  return return_val;
}
#endif
/**
//函数名：switchWaysConfigInit
//描述：开关路数自适应初始化，按开关路路数初始化对应的endpoint。
//      此函数需要在emberAfMainInitCallback里调用,在SDK初始化前处理
//参数：无
//返回：无
*/
void switchWaysConfigInit(void)
{
  #if 0
  //获取到当前开关的路数配置
  switch_x_ways = getSwitchWaysConfigSetting();
  //关闭掉多余的endpoints
  for (uint8_t i=switch_x_ways; i < SWITCH_3WAYS; i++)
  {
    emberAfEndpointEnableDisable(emberAfEndpointFromIndex(i), EMBER_AF_ENDPOINT_DISABLED);
  }
  #endif
  switchWaysConfigDebugPrintln("init,ways=%d",switch_x_ways);
}

/**
//函数名：switchWaysConfigUpdateEndpointModelId
//描述：更新多路开关对应路数的endpoint的model id值,此函数不能在emberAfMainInitCallback里调用，需要在
//      SDK初始化后调用，这里在系统进入while后调用。
//参数：无
//返回：无
*/
void switchWaysConfigUpdateEndpointModelId(void)
{
   uint8_t length_temp;
   uint8_t model_id_temp[33];
#if 0 //jim
   switch (switch_x_ways)
   {
     case SWITCH_3WAYS:
       length_temp = strlen(SWITCH_3_GAND_MODELID_STRING);
       if (length_temp > 32) //最大只能32个字符
       {
         length_temp = 32;
       }
       memcpy(&model_id_temp[1],SWITCH_3_GAND_MODELID_STRING,length_temp);
       break;
     case SWITCH_2WAYS:
       length_temp = strlen(SWITCH_2_GAND_MODELID_STRING);
       if (length_temp > 32) //最大只能32个字符
       {
         length_temp = 32;
       }
       memcpy(&model_id_temp[1],SWITCH_2_GAND_MODELID_STRING,length_temp);
       break;
     default:
       length_temp = strlen(SWITCH_1_GAND_MODELID_STRING);
       if (length_temp > 32) //最大只能32个字符
       {
         length_temp = 32;
       }
       memcpy(&model_id_temp[1],SWITCH_1_GAND_MODELID_STRING,length_temp);
       break;
   }
#endif
       length_temp = strlen(HIDDEN_SINGFIRE_MODELID_STRING);
       if (length_temp > 32) //最大只能32个字符
       {
         length_temp = 32;
       }
       memcpy(&model_id_temp[1],HIDDEN_SINGFIRE_MODELID_STRING,length_temp);
   model_id_temp[0] = length_temp;
   for (uint8_t i=0; i < switch_x_ways; i++)
   {
     emberAfWriteAttribute(emberAfEndpointFromIndex(i),
                           ZCL_BASIC_CLUSTER_ID,
                           ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID,
                           CLUSTER_MASK_SERVER,
                           model_id_temp,
                           ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
   }
   switchWaysConfigDebugPrintln("update model id");
}

/**
//函数名：switchWaysConfigGetWays
//描述：获取当前开关的路数
//参数：无
//返回：无
*/
uint8_t switchWaysConfigGetWays(void)
{
  //获取当前开关的路数
  return switch_x_ways;
}
/*************************************** 文 件 结 束 ******************************************/
