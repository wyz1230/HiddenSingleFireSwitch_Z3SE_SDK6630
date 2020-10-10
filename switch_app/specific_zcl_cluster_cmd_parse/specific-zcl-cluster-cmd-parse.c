/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName   : specific-zcl-cluster-cmd-parse.c
*Author     : JimYao
*Version    : 1.0
*Date       : 2019-11-07
*Description: zcl cluster 指令的分析处理，用于SDK处理完zcl cluster 指令后，可做为处理客制指令的入口，
*             配合宏定义USE_ZCL_CLUSTER_SPECIFIC_COMMAND_PARSE_CALLBACK 开关，在SDK的process-cluster-message.c
*             加入对应的回调入口
*History:
***************************************************************************************************/
/* 相关包含头文件区 ------------------------------------------------------------ */
#ifdef USE_ZCL_CLUSTER_SPECIFIC_COMMAND_PARSE_CALLBACK  //jim 20191107 添加处理ZCL特殊指令入口回调

//#include "specific-zcl-cluster-cmd-parse.h"
#include "common-app.h"

/* 宏定义区 -------------------------------------------------------------------- */
//debug开关设定
//#define SPECIFIC_ZCL_CLUSTER_CMD_PARSE_DEBUG_ENABLE
#ifdef SPECIFIC_ZCL_CLUSTER_CMD_PARSE_DEBUG_ENABLE
  #define DEBUG_STRING                                    "SpeZclCmdPar-DB:"
  #define specificZclClusterCmdParseDebugPrint(...)       emberAfPrint(0xFFFF,DEBUG_STRING __VA_ARGS__)
  #define specificZclClusterCmdParseDebugPrintln(...)     emberAfPrintln(0xFFFF,DEBUG_STRING __VA_ARGS__)
#else
  #define specificZclClusterCmdParseDebugPrint(...)
  #define specificZclClusterCmdParseDebugPrintln(...)
#endif

#define MAX_SWITCH_WAYS      3
/* 自定义类型区 ---------------------------------------------------------------- */

/* 全局变量区 ------------------------------------------------------------------ */

/* 本地变量区 ------------------------------------------------------------------ */
static uint8_t last_onoff_cmd_type[MAX_SWITCH_WAYS] = {UNICAST_CONTROL,UNICAST_CONTROL,UNICAST_CONTROL};  //三路开关，对应前三个endpoint

/* 全局函数声明区 -------------------------------------------------------------- */

/* 本地函数声明区 -------------------------------------------------------------- */

/* 函数原型 -------------------------------------------------------------------- */

/**
//函数名：clusterSpecficCommandParsePreCheckCallback
//描  述：SDK处理zcl指令前的一个回调，可用来分析当前的是哪种类型的指令。
//参  数：*cmd   (EmberAfClusterCommand * [输入]，cluster指令指针)
//返  回：无
*/
void clusterSpecficCommandParsePreCheckCallback(EmberAfClusterCommand *cmd)
{
  uint8_t ways_temp = switchWaysConfigGetWays();
  switch (cmd->apsFrame->clusterId)
  {
    case ZCL_ON_OFF_CLUSTER_ID:

      for (uint8_t i=0; i<ways_temp; i++)
      {
        if (cmd->apsFrame->destinationEndpoint == emberAfEndpointFromIndex(i))
        {
          if(cmd->type == EMBER_INCOMING_MULTICAST)
          {
            last_onoff_cmd_type[i] = MULTICAST_CONTROL;
          }
          else if(cmd->type == EMBER_INCOMING_BROADCAST)
          {
            last_onoff_cmd_type[i] = BROADCAST_CONTROL;
          }
          else
          {
            last_onoff_cmd_type[i] = UNICAST_CONTROL;
          }
          specificZclClusterCmdParseDebugPrintln("ways=%d,onoff cmd type=%d",i,cmd->type);
          break;
        }
      }
      break;
    default:
      // Unrecognized cluster ID, error status will apply.
      break;
  }
}

/**
//函数名：getOnOffClusterCommandType
//描  述：获取最后一个OnOff cluster的指令类型，是否为广播，多播，单播，按键，上电，情景
//参  数：ways (uint8_t [输入] 第几路开关)
//返  回：uint8_t, 返回指令类型
*/
uint8_t getOnOffClusterCommandType(uint8_t ways)
{
  if (ways < MAX_SWITCH_WAYS)
  {
    specificZclClusterCmdParseDebugPrintln("get ways=%d,last onoff cmd type=%d",ways,last_onoff_cmd_type[ways]);
    return last_onoff_cmd_type[ways];
  }
  return UNICAST_CONTROL;
}

/**
//函数名：setOnOffClusterCommandType
//描  述：设定onoff最后指令类型
//参  数：ways (uint8_t [输入] 第几路开关)
//        cmd_type (uint8_t [输入] 指令的类型)
//返  回：void
*/
void setOnOffClusterCommandType(uint8_t ways, uint8_t cmd_type)
{
  if (ways < MAX_SWITCH_WAYS)
  {
    last_onoff_cmd_type[ways] = cmd_type;
    specificZclClusterCmdParseDebugPrintln("set way=%d last onoff cmd type=%d",ways,last_onoff_cmd_type[ways]);
  }
}

#endif
/*************************************** 文 件 结 束 ******************************************/
