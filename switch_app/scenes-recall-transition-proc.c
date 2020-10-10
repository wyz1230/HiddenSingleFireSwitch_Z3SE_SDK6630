/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:scenes-recall-transition-proc.c
*Author : JimYao
*Version : 1.0
*Date : 2019-09-18
*Description: 场景recall过渡时间的处理
*History:
***************************************************************************************************/
/* 相关包含头文件区 ------------------------------------------------------------ */
#include "common-app.h"

#ifdef SCENES_RECALL_SAVED_SECENE_CALLBACK_CODE
/* 宏定义区 -------------------------------------------------------------------- */
//debug开关设定
//#define SCENES_RECALL_TRAN_DEBUG_ENABLE
#ifdef SCENES_RECALL_TRAN_DEBUG_ENABLE
  #define DEBUG_STRING                               "Debug-ScenesTran:"
  #define scenesRecallTransitionDebugPrint(...)       emberAfPrint(0xFFFF,DEBUG_STRING __VA_ARGS__)
  #define scenesRecallTransitionDebugPrintln(...)     emberAfPrintln(0xFFFF,DEBUG_STRING __VA_ARGS__)
#else
  #define scenesRecallTransitionDebugPrint(...)
  #define scenesRecallTransitionDebugPrintln(...)
#endif

#define MAX_UINT32       0xFFFFFFFF

#define SCENES_TRANSITION_TABLE_SIZE     EMBER_AF_PLUGIN_SCENES_TABLE_SIZE
/* 自定义类型区 ---------------------------------------------------------------- */
typedef struct {
   uint8_t endpoint;
   uint16_t group_id;
   uint8_t scene_id;
   uint32_t transition_time_ms;
} t_scene_transition;

/* 全局变量区 ------------------------------------------------------------------ */
EmberEventControl scenesTransitionEventControl;  //需要定义场景过渡时间处理事件
//void scenesTransitionEventHandler(void);

/* 本地变量区 ------------------------------------------------------------------ */
static t_scene_transition recall_scene_trans_table[SCENES_TRANSITION_TABLE_SIZE];
static uint32_t transition_remain_time = MAX_UINT32;   //下一次执行剩余时间。0xFFFFFFF为关闭

/* 全局函数声明区 -------------------------------------------------------------- */
EmberAfStatus scenesRecallSavedSceneTimeToFinish(uint8_t endpoint,
                                                 uint16_t groupId,
                                                 uint8_t sceneId);

/* 本地函数声明区 -------------------------------------------------------------- */

/* 函数原型 -------------------------------------------------------------------- */
/**
//函数名：scenesTransitionInit
//描述：初始化场景过渡相关变量
//参数：无
//返回：void
*/
void scenesTransitionInit(void)
{
  for (uint8_t i=0; i<SCENES_TRANSITION_TABLE_SIZE; i++)
  {
    recall_scene_trans_table[i].endpoint = 0xFF;
    recall_scene_trans_table[i].group_id = 0xFFFF;
    recall_scene_trans_table[i].scene_id = 0xFF;
    recall_scene_trans_table[i].transition_time_ms = MAX_UINT32;
  }
  transition_remain_time = MAX_UINT32;
  emberEventControlSetInactive(scenesTransitionEventControl);

  scenesRecallTransitionDebugPrintln("init");
}

/**
//函数名：getScenesTransMinRemainTime
//描述：找出过渡表中最小的剩余时间
//参数：无
//返回：uint32_t, 最小的剩余时间，当为0xFFFFFFFF时，没有剩余时间，不需要倒计。
*/
static uint32_t getScenesTransMinRemainTime(void)
{
  uint32_t min_remain = MAX_UINT32;
   for (uint8_t i=0; i<SCENES_TRANSITION_TABLE_SIZE; i++)
   {
     if (recall_scene_trans_table[i].transition_time_ms < min_remain)
     {
       min_remain = recall_scene_trans_table[i].transition_time_ms;
     }
   }
   return min_remain;
}

/**
//函数名：updateScenesTransRemainTime
//描述：更新控制状态有效的剩余时间
//参数：last_delay_time (uint32_t[输入],上一个任务延时的时间;)
//返回：无
*/
static void updateScenesTransRemainTime(uint32_t last_delay_time)
{
   for (uint8_t i=0; i<SCENES_TRANSITION_TABLE_SIZE; i++)
   {
     if (recall_scene_trans_table[i].transition_time_ms < MAX_UINT32)
     {
       if (recall_scene_trans_table[i].transition_time_ms < last_delay_time)
       {
         recall_scene_trans_table[i].transition_time_ms = 0;
       }
       else
       {
         recall_scene_trans_table[i].transition_time_ms -= last_delay_time;
       }
     }
   }
}

/**
//函数名：updateScenesTransTableAndStartDelay
//描述：更新过渡时间表和算出最近剩余时间触发延时事件
//参数：无
//返回：bool ，true表示过渡加入正常，延时执行。false 表示过渡加入异常，没有延时处理。
*/
static bool updateScenesTransTableAndStartDelay(uint8_t endpoint,
                                                uint16_t group_id,
                                                uint8_t scene_id,
                                                uint32_t transition_time_ms)
{
  uint8_t i = 0;
  //计算出触发时，场景表中剩下的时间
  if (transition_remain_time < MAX_UINT32) //有其它正在执行的过渡延时
  {
    uint32_t task_remain_time = emberEventControlGetRemainingMS(scenesTransitionEventControl);
    if (transition_remain_time >= task_remain_time)
    {
      transition_remain_time -= task_remain_time;
    }
    else
    {
      transition_remain_time = 0;
    }
    //更新过渡表中正在过渡的时间
    updateScenesTransRemainTime(transition_remain_time);
  }

  //先找出表中是否已经存在，
  for (i=0; i<SCENES_TRANSITION_TABLE_SIZE; i++)
  {
    if (recall_scene_trans_table[i].endpoint == endpoint &&
        recall_scene_trans_table[i].group_id == group_id &&
        recall_scene_trans_table[i].scene_id == scene_id)
    {
      // 已经存在的更新时间。
      recall_scene_trans_table[i].transition_time_ms = transition_time_ms;
      break;
    }
  }
  //如果不存在，找出第一个endpoint为0xFF的索引号，加入
  if (SCENES_TRANSITION_TABLE_SIZE == i)
  {
    for (i=0; i<SCENES_TRANSITION_TABLE_SIZE; i++)
    {
      if (MAX_UINT32 == recall_scene_trans_table[i].transition_time_ms)
      {
        recall_scene_trans_table[i].endpoint = endpoint;
        recall_scene_trans_table[i].group_id = group_id;
        recall_scene_trans_table[i].scene_id = scene_id;
        recall_scene_trans_table[i].transition_time_ms = transition_time_ms;
        break;
      }
    }
  }
  //找出过渡表中最小的时间
  transition_remain_time = getScenesTransMinRemainTime();
  if (MAX_UINT32 == transition_remain_time)
  {
    emberEventControlSetInactive(scenesTransitionEventControl);
  }
  else
  {
    emberEventControlSetDelayMS(scenesTransitionEventControl,transition_remain_time);
    scenesRecallTransitionDebugPrintln("start,delay=%ld ms",transition_remain_time);
    scenesRecallTransitionDebugPrintln("endpoint=0x%x,groupid=0x%x,sceneid=0x%x",
                                         endpoint,
                                         group_id,
                                         scene_id);
    return true;
  }
  return false;
}

/**
//函数名：scenesTransitionEventHandler
//描述：场景过渡时间事件处理入口
//参数：无
//返回：void
*/
void scenesTransitionEventHandler(void)
{
  emberEventControlSetInactive(scenesTransitionEventControl);
  //更新过渡表中正在过渡的时间
  updateScenesTransRemainTime(transition_remain_time);
  //先找出表中已经到达执行时间的场景号，触发执行。
  for (uint8_t i=0; i<SCENES_TRANSITION_TABLE_SIZE; i++)
  {
    if (recall_scene_trans_table[i].transition_time_ms == 0) //到达执行时间
    {
      //触发执行
  #ifdef USE_ZCL_CLUSTER_SPECIFIC_COMMAND_PARSE_CALLBACK
      uint8_t temp_ways = switchWaysConfigGetWays();
      for (uint8_t i=0; i<temp_ways; i++)
      {
        if (recall_scene_trans_table[i].endpoint == emberAfEndpointFromIndex(i))
        {
          setOnOffClusterCommandType(i,SCENE_CONTROL);  //切换为场景上报
          break;
        }
      }
  #endif
      scenesRecallSavedSceneTimeToFinish(recall_scene_trans_table[i].endpoint,
                                         recall_scene_trans_table[i].group_id,
                                         recall_scene_trans_table[i].scene_id);

      scenesRecallTransitionDebugPrintln("finish,endpoint=0x%x,groupid=0x%x,sceneid=0x%x",
                                         recall_scene_trans_table[i].endpoint,
                                         recall_scene_trans_table[i].group_id,
                                         recall_scene_trans_table[i].scene_id);
      //清除执行延时场景
      recall_scene_trans_table[i].endpoint = 0xFF;
      recall_scene_trans_table[i].group_id = 0xFFFF;
      recall_scene_trans_table[i].scene_id = 0xFF;
      recall_scene_trans_table[i].transition_time_ms = MAX_UINT32;
    }
  }
  //找出过渡表中最小的时间
  transition_remain_time = getScenesTransMinRemainTime();
  if (transition_remain_time < MAX_UINT32)
  {
    emberEventControlSetDelayMS(scenesTransitionEventControl,transition_remain_time);
    scenesRecallTransitionDebugPrintln("next,delay=%ld ms",transition_remain_time);
  }
}

/**
//函数名：scenesRecallSavedSceneDelayProcCallback
//描述：recall scene回调给运用代码确认是否需要处理延时部分
//参数：endpoint (uint8_t [输入]，端点号)
//      groupId (uint16_t [输入]，组号)
//      sceneId (uint8_t [输入]，场景号)
//      transition_time_ms (uint32_t [输入]，过渡时间)
//返回：bool ， 返回true表示运用代码已经处理，需要过渡，false表示运用代码没处理，不需要过渡。
*/
bool scenesRecallSavedSceneDelayProcCallback(uint8_t endpoint,
                                             uint16_t groupId,
                                             uint8_t sceneId,
                                             uint32_t transition_time_ms)
{
  uint8_t local_endpoint,temp_ways;
  if (transition_time_ms == 0) //过渡时间为0，不需要过渡，直接跳出，由SDK处理。
  {
#ifdef USE_ZCL_CLUSTER_SPECIFIC_COMMAND_PARSE_CALLBACK
    temp_ways = switchWaysConfigGetWays();
    for (uint8_t i=0; i<temp_ways; i++)
    {
      if (endpoint == emberAfEndpointFromIndex(i))
      {
        setOnOffClusterCommandType(i,SCENE_CONTROL);  //切换为场景上报
        break;
      }
    }
#endif
    return false;
  }
  temp_ways = switchWaysConfigGetWays();
  for (uint8_t i=0; i<temp_ways; i++) //找出开关对应路数的endpoint号
  {
    local_endpoint = emberAfEndpointFromIndex(i);
    if (endpoint == local_endpoint)
    {
      return (updateScenesTransTableAndStartDelay(endpoint,groupId,sceneId,transition_time_ms));
    }
  }
  return false;
}

#endif

/*************************************** 文 件 结 束 ******************************************/
