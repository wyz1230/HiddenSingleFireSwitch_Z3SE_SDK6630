/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName   : software-reboot-proc.c
*Author     : JimYao
*Version    : 1.0
*Date       : 2019-11-04
*Description: 软件重启处理
*History:
***************************************************************************************************/
/* 相关包含头文件区 ------------------------------------------------------------ */
//#include "software-reboot-proc.h"
#include "common-app.h"

/* 宏定义区 -------------------------------------------------------------------- */
//debug开关设定
//#define SOFTWARE_REBOOT_DEBUG_ENABLE
#ifdef SOFTWARE_REBOOT_DEBUG_ENABLE
  #define DEBUG_STRING                        "SWReboot-DB:"
  #define softwareRebootDebugPrint(...)       emberAfPrint(0xFFFF, DEBUG_STRING __VA_ARGS__)
  #define softwareRebootDebugPrintln(...)     emberAfPrintln(0xFFFF, DEBUG_STRING __VA_ARGS__)
#else
  #define softwareRebootDebugPrint(...)
  #define softwareRebootDebugPrintln(...)
#endif


/* 自定义类型区 ---------------------------------------------------------------- */

/* 全局变量区 ------------------------------------------------------------------ */

/* 本地变量区 ------------------------------------------------------------------ */

/* 全局函数声明区 -------------------------------------------------------------- */

/**
//函数名：switchSaveSwitchStatusToToken
//描述  ：保存开关状态到token中
//参数  ：无
//返回  ：void
*/
static void switchSaveSwitchStatusToToken(void)
{
  uint8_t i,j;
  tTokenTypeSwitchOnOffStatus token_switch_status,temp_status;
  for (i=0; i<3; i++)
  {
    if (RELAY_CONTROL_TURN_ON == relayControlDriverGetCurrenStatus(i))
    {
      token_switch_status.onoff_status[i] = RELAY_CONTROL_TURN_ON;
    }
    else
    {
      token_switch_status.onoff_status[i] = RELAY_CONTROL_TURN_OFF;
    }
  }
  for (j=0; j<3; j++) //多次比较，提高存入token的可靠性
  {
    halCommonGetToken(temp_status.onoff_status, TOKEN_MIX_SWITCH_ONOFF_STATUS); //先获取token里的开关状态
    for (i=0; i<3; i++)  //token表与当前的状态否存在差异
    {
      if (temp_status.onoff_status[i] != token_switch_status.onoff_status[i])
      {
        //存在差异,更新token
        halCommonSetToken(TOKEN_MIX_SWITCH_ONOFF_STATUS, token_switch_status.onoff_status);
      }
    }
    if (3 == i) //相同,结束
    {
      break;
    }
  }
   softwareRebootDebugPrintln("save onoff status to token");
}

/**
//函数名：switchGetSwitchStatusFromToken
//描述  ：从token中获取开关状态
//参数  ：*status (tTokenTypeSwitchOnOffStatus * [输出]，开关状态)
//返回  ：无
*/
static void switchGetSwitchStatusFromToken(tTokenTypeSwitchOnOffStatus *status)
{
  uint8_t i,j;
   tTokenTypeSwitchOnOffStatus temp_status;
  for (j=0; j<3; j++) //多次比较，提高读取token的可靠性
  {
    halCommonGetToken(status->onoff_status, TOKEN_MIX_SWITCH_ONOFF_STATUS); //先获取token里的开关状态
    halCommonGetToken(temp_status.onoff_status, TOKEN_MIX_SWITCH_ONOFF_STATUS); //先获取token里的开关状态
    for (i=0; i<3; i++)  //两次读取的值是否一致
    {
      if (temp_status.onoff_status[i] != status->onoff_status[i])
      {
        //存在差异,重新获取
        break;
      }
    }
    if (3 == i) //相同,结束
    {
      break;
    }
  }
   softwareRebootDebugPrintln("get onoff status from token");
}

/**
//函数名：switchSaveRebootReasonToToken
//描述  ：保存重启原因到token中
//参数  ：reason (uint8_t [输入]，重启的原因，供软件重启处理用)
//返回  ：void
*/
static void switchSaveRebootReasonToToken(uint8_t reason)
{
  uint8_t i,temp;
  for (i=0; i<3; i++)   //多次比较提高可靠性
  {
    halCommonGetToken(&temp, TOKEN_MIX_SWITCH_SW_REBOOT_REASON); //先获取token里的重启原因
    if (temp != reason) //与当前的值否存在差异
    {
      halCommonSetToken(TOKEN_MIX_SWITCH_SW_REBOOT_REASON, &reason);
    }
    else
    {
      break;
    }
  }
   softwareRebootDebugPrintln("save reboot reason to token");
}
/**
//函数名：switchRebootReasonFromToken
//描述  ：从token中获取重启原因
//参数  ：无
//返回  ：uint8_t ,从token中获取到的重启原因
*/
static uint8_t switchRebootReasonFromToken(void)
{
  uint8_t temp,temp1;
  for(uint8_t i = 0; i<3; i++) //多次比较提高可靠性
  {
    halCommonGetToken(&temp, TOKEN_MIX_SWITCH_SW_REBOOT_REASON); //获取token里的重启原因
    halCommonGetToken(&temp1, TOKEN_MIX_SWITCH_SW_REBOOT_REASON); //获取token里的重启原因
    if (temp == temp1)  //两次读取的值一致
    {
      break;
    }
  }
   softwareRebootDebugPrintln("get reboot reason from token");
   return temp;
}


/**
//函数名：switchRebootCheckToRecoverStatus
//描述  ：检测是否是软件产生的复位，恢复复位前的状态
//参数  ：无
//返回  ：bool, true软件产生的复位，已经恢复到复位前状态; false 不是软件产生的复位，不处理
*/
bool switchRebootCheckToRecoverStatus(void)
{
  uint8_t reason;
  reason = switchRebootReasonFromToken();

   if ((reason == SWITCH_REBOOT_REASON_OTA) || (reason == SWITCH_REBOOT_REASON_LONG_WAKEUP))
   {
      tTokenTypeSwitchOnOffStatus status;
      uint8_t value, ways = switchWaysConfigGetWays();
      switchGetSwitchStatusFromToken(&status);
      for (uint8_t i=0; i < ways; i++)
      {
        if (status.onoff_status[i] == RELAY_CONTROL_TURN_ON)
        {
           value = 1;
        }
        else
        {
           value = 0;
        }

        emberAfWriteAttribute(emberAfEndpointFromIndex(i),
                                      ZCL_ON_OFF_CLUSTER_ID,
                                      ZCL_ON_OFF_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      (uint8_t *)&value,
                                      ZCL_BOOLEAN_ATTRIBUTE_TYPE);  //更新开关之前状态值
      }
      softwareRebootDebugPrintln("recover status ");
      switchSaveRebootReasonToToken(SWITCH_REBOOT_REASON_NONE);     //去除重启原因
      return true;
   }
   return false;
}
/**
//函数名：preRebootStatusSaveProc
//描述  ：设备重启前的状态保存与重启原因处理
//参数  ：reason (uint8_t [输入]，重启的原因，供软件重启处理用)
//返回  ：void
*/
void preRebootStatusSaveProc(uint8_t reason)
{
  switchSaveSwitchStatusToToken();                   //保存开关状态到token
  switchSaveRebootReasonToToken(reason);             //保存重启原因
}
#ifdef USER_OTA_CLIENT_CALLBACK_CODE   //jim add 20191030
/**
//函数名：otaClientPluginPreChangeBootloadCallback
//描述  ：ota下载完成后，切换到bootloader前的回调函数
//参数  ：无
//返回  ：void
*/
void otaClientPluginPreChangeBootloadCallback(void)
{
  //switchSaveSwitchStatusToToken();                         //保存开关状态到token
  //switchSaveRebootReasonToToken(SWITCH_REBOOT_REASON_OTA); //保存重启原因
  preRebootStatusSaveProc(SWITCH_REBOOT_REASON_OTA);
}
/**
//函数名：otaClientPluginPreChangeBootloadCallback
//描述  ：ota下载完成后，切换bootloader失败的回调函数
//参数  ：无
//返回  ：void
*/
void otaClientPluginChangeBootloadErrorCallback(void)
{
  switchSaveRebootReasonToToken(SWITCH_REBOOT_REASON_NONE); //去除重启原因
}
#endif  //end jim

/*************************************** 文 件 结 束 ******************************************/
