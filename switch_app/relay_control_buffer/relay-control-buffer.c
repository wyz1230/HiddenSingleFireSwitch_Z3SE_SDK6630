/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:relay-control-buffer.c
*Author : JimYao
*Version : 1.0
*Date : 2019-10-17
*Description: 此代码是防止几路继电器同时控制时，产生的电流过大，电池供电的设备会产生电池撑不住的情况。
*             此部分的功能是把要执行的控制继电器的动作先放入一个缓存，再按序执行。
*History:
***************************************************************************************************/

/* 相关包含头文件区 --------------------------------------------------------- */
//#include "relay-control-buffer.h"
#include "common-app.h"

/* 宏定义区 ----------------------------------------------------------------- */
//debug开关设定
//#define RELAY_CONTROL_BUFFER_DEBUG_ENABLE
#ifdef RELAY_CONTROL_BUFFER_DEBUG_ENABLE
  #define DEBUG_STRING                          "RlCtrlBuffer-DB:"
  #define relayControlBufferDebugPrint(...)     emberAfPrint(0xFFFF,DEBUG_STRING __VA_ARGS__)
  #define relayControlBufferDebugPrintln(...)   emberAfPrintln(0xFFFF,DEBUG_STRING __VA_ARGS__)
#else
  #define relayControlBufferDebugPrint(...)
  #define relayControlBufferDebugPrintln(...)
#endif



/* 自定义类型区 ------------------------------------------------------------- */
enum {
  RELAY_CONTORL_NONE_ACTION,
  RELAY_CONTORL_WAITING,
  RELAY_CONTORL_RUNNING,
};

typedef struct {
    uint8_t status;   //缓冲中的动作状态，0 无动作，1 等待， 2 运行中
    uint8_t action;   //缓冲中的动作
    uint8_t delay;    //缓冲动作的延时时间
} tRelayCtlCmdBuf;

/* 全局变量区 --------------------------------------------------------------- */

/* 本地变量区 --------------------------------------------------------------- */
static tRelayCtlCmdBuf relay_contorl_buffer[SWITCH_3WAYS] = {{RELAY_CONTORL_NONE_ACTION,RELAY_CONTROL_TURN_OFF,0},
                                                             {RELAY_CONTORL_NONE_ACTION,RELAY_CONTROL_TURN_OFF,0},
                                                             {RELAY_CONTORL_NONE_ACTION,RELAY_CONTROL_TURN_OFF,0}};
 
/* 全局函数声明区 ----------------------------------------------------------- */

/* 本地函数声明区 ----------------------------------------------------------- */

/* 函数原型 ----------------------------------------------------------------- */

/**
//函数名：relayControlBufferInitial
//描述：初始化继电器控制动作的缓存。
//参数：无
//返回：void
*/
void relayControlBufferInitial(void)
{
  for (uint8_t i=0; i<SWITCH_3WAYS; i++)
  {
    relay_contorl_buffer[i].status = RELAY_CONTORL_NONE_ACTION;
    relay_contorl_buffer[i].action = RELAY_CONTROL_TURN_OFF;
    relay_contorl_buffer[i].delay = 0;
  }
  relayControlBufferDebugPrintln("init");
}

/**
//函数名：updateAndTrigeRelayControlBufferNextAction
//描述：写继电器控制开关动作到缓存中，由调缓存调用，防止几个继电器同时开关。
//参数：way      (uint8_t[输入],需要控制第几路开关，0，1，2;)
//      action   (uint8_t[输入],需要执行的动作，on->1, off->0;)
//      delay    (uint8_t[输入],此动作延时多久ms后执行)
//      add_flag (bool   [输入],true 表示新加入的动作；false 表示已经完成的动作更新,此时参数 action, delay 无作用)
//返回：void
*/
void updateAndTrigeRelayControlBufferNextAction(uint8_t way, uint8_t action, uint8_t delay, bool add_flag)
{
  //算出写控制动作时，正在执行中的动作已经延时多久
  static uint16_t last_action_tirge_time_ms = 0;
  uint16_t delta_time_ms;
  uint8_t i;
  
  //算出触发时，如果存在有控制动作没执行完，算出距离上次执行动作的时间差。
  delta_time_ms = 0;
  if (add_flag == true) //如果是新加入的动作
  {
    for (i=0; i<SWITCH_3WAYS; i++) //找出触发时，是否有正常在执行的动作。
    {
      if (RELAY_CONTORL_RUNNING == relay_contorl_buffer[i].status)
      {
        delta_time_ms = halCommonGetInt16uMillisecondTick() - last_action_tirge_time_ms; //算出与上次执行之间的时间差
        relayControlBufferDebugPrintln("way=%d, runing",i);
      }
    }
  }
  else //动作执行完的更新
  {
    if (way < SWITCH_3WAYS)
    {
      if (RELAY_CONTORL_RUNNING == relay_contorl_buffer[way].status) //此更新是当前执行的路数触发。
      {
        relay_contorl_buffer[way].status = RELAY_CONTORL_NONE_ACTION;
        delta_time_ms = relay_contorl_buffer[way].delay;
        relay_contorl_buffer[way].delay = 0;
        relayControlBufferDebugPrintln("way=%d, finished",way);
      }
    }
  }
  
  //更新对应的时间
  if (delta_time_ms > 0)
  {
    if (delta_time_ms > 255)
    {
      delta_time_ms = 255;
    }
    for (i=0; i<SWITCH_3WAYS; i++) //更新对应的时间
    {
      if (relay_contorl_buffer[i].status != RELAY_CONTORL_NONE_ACTION) //有动作的任务需要更新时间
      {
        if (relay_contorl_buffer[i].delay < delta_time_ms)
        {
          relay_contorl_buffer[i].delay = 0;
        }
        else
        {
          relay_contorl_buffer[i].delay -= (uint8_t)delta_time_ms;
        }
      }
    }
  }
  relayControlBufferDebugPrintln("update delta time %ld ms",delta_time_ms);
  
  //填入新的执行动作
  if (add_flag == true) //如果是新加入的动作
  {
    if (way < SWITCH_3WAYS) //分析更新缓存状态
    {
      relay_contorl_buffer[way].status = RELAY_CONTORL_WAITING;
      relay_contorl_buffer[way].action = action;
      relay_contorl_buffer[way].delay = delay;
    }
    relayControlBufferDebugPrintln("add way=%d new",way);
  }
  
  //找出下一个执行动作并执行
  uint8_t mini_time = 0xFF; //找出下一个最短的待执行的时间
  uint8_t index = 0xFF; //找出下一个最短的待执行的时间
  for (i=0; i<SWITCH_3WAYS; i++)
  {
    if (RELAY_CONTORL_RUNNING == relay_contorl_buffer[i].status) //如果存在正在执行的，不需要触发，等上次的动作执行完后再触发。
    {
      break;
    }
    if (RELAY_CONTORL_WAITING == relay_contorl_buffer[i].status)
    {
      if (mini_time > relay_contorl_buffer[i].delay)
      {
        mini_time = relay_contorl_buffer[i].delay;
        index = i;
      }
    }
  }
  //重新触发新的动作
  if (index < SWITCH_3WAYS)
  {
    relay_contorl_buffer[index].status = RELAY_CONTORL_RUNNING;
    relayControlDriverTrigeOnOffAction(index,relay_contorl_buffer[index].action, relay_contorl_buffer[index].delay);
    relayControlBufferDebugPrintln("trige way=%d running, delay=%ld ms",index,relay_contorl_buffer[index].delay);
  }
  
  last_action_tirge_time_ms = halCommonGetInt16uMillisecondTick();
}

/***************************************** 文 件 结 束 ************************************************/
