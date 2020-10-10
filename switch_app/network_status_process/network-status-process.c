/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:network-status-process.c
*Author : JimYao
*Version : 1.0
*Date : 2019-09-11
*Description: 网络状态变化处理，如加网，离网，
*History:
***************************************************************************************************/
/* 相关包含头文件区 ------------------------------------------------------------ */
//#include "network-status-process.h"
#include "common-app.h"
#include "app/framework/util/attribute-storage.h"
#include "app/framework/util/util.h"
#include "app/framework/plugin/ota-client/ota-client.h"

/* 宏定义区 -------------------------------------------------------------------- */
//debug开关设定
//#define NETWORK_STATUS_PROCESS_DEBUG_ENABLE
#ifdef NETWORK_STATUS_PROCESS_DEBUG_ENABLE
  #define DEBUG_STRING                         "NwStPro-DB:"
  #define networkStatusProcDebugPrint(...)     emberAfPrint(0xFFFF,DEBUG_STRING __VA_ARGS__)
  #define networkStatusProcDebugPrintln(...)   emberAfPrintln(0xFFFF,DEBUG_STRING __VA_ARGS__)
#else
  #define networkStatusProcDebugPrint(...)
  #define networkStatusProcDebugPrintln(...)
#endif


#define NETWORK_JOINING_POLL_TIME_MS            (2000) //2秒的轮询扫描入网间隔

//#define NETWORK_JOINING_TIMEOUT_ENABLE                 //加网超时开关,没定义时加网不超时
#define NETWORK_JOINING_TIMEOUT_S               (60)   //1分钟加网超时
#define NETWORK_JOINING_TIMEOUT_COUNTS          (NETWORK_JOINING_TIMEOUT_S*1000/NETWORK_JOINING_POLL_TIME_MS)  //换算为周期个数

/* 自定义类型区 ---------------------------------------------------------------- */

/* 全局变量区 ------------------------------------------------------------------ */
EmberEventControl networkActionPollingEventControl;   //轮询触发加网处理事件
//void networkActionPollingEventHandler(void);

/* 本地变量区 ------------------------------------------------------------------ */
static uint8_t network_status_action = NETWORK_ACTION_NONE;
static uint8_t network_status_action_overtime = 0;      //动作超时计数处理
static uint8_t attributes_resetting_default_flag = 0;   //属性值恢复出厂默认值时的标志，解决有些属性不需要恢复默认值的问题。
/* 全局函数声明区 -------------------------------------------------------------- */

/* 本地函数声明区 -------------------------------------------------------------- */

/* 函数原型 -------------------------------------------------------------------- */
/**
//函数名：clearOtaStorageTempDataAll
//描述：清除ota buffer里的头尾部数据
//参数：无
//返回：void
*/
void clearOtaStorageTempDataAll(void)
{
  uint32_t current_address;
  uint8_t status = EEPROM_SUCCESS, i = 0;

  current_address = EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_STORAGE_START;

    for (i = 0; i < 3; i++) //出错时试三次
    {
      status = emberAfPluginEepromErase(current_address, 4096);
      if (status == EEPROM_SUCCESS)
      {
        break;
      }
    }
    current_address = EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_STORAGE_END - 4096;
     for (i = 0; i < 3; i++) //出错时试三次
     {
      status = emberAfPluginEepromErase(current_address, 4096);
      if (status == EEPROM_SUCCESS)
      {
        break;
      }
    }
}
#ifdef USE_ATRRIBUTE_RESET_DEFAULT_CALLBACK_CODE  //jim add 20191104
/**
//函数名：setAttributesResetDefaultFlag
//描述：属性恢复默认值前设定标志位，用来属性改变时产生回调处理是否需要恢复默认值用
//参数：flag (uint8_t [输入],需要设定的标志位值，0表示不在恢复出厂默认值，其它值表示正在恢复出厂默认值)
//返回：void
*/
void setAttributesResetDefaultFlag(uint8_t flag)
{
  attributes_resetting_default_flag = flag;
}
/**
//函数名：checkAtrributeNeedToResetDefaultCallback
//描述  ：属性恢复默认值前判断是否需要属性默认值的一个回调，方便有些属性不需要属性默认值的处理。
//参数  ：endpoint         (uint8_t            [输入],endpoint)
//        clusterId        (EmberAfClusterId   [输入],cluster id)
//        mask             (EmberAfClusterMask [输入],cluster的mask)
//        attributeId      (EmberAfAttributeId [输入],attributeId)
//        manufacturerCode (uint16_t           [输入],manufacturerCode)
//返回  ：bool true,表示需要属性默认值; false,表示不需要属性默认值，保持原来的值。
*/
bool checkAtrributeNeedToResetDefaultCallback(uint8_t endpoint,
                                              EmberAfClusterId clusterId,
                                              EmberAfClusterMask mask,
                                              EmberAfAttributeId attributeId,
                                              uint16_t manufacturerCode)
{
  if (attributes_resetting_default_flag)  //设备正在恢复出厂默认
  {
     if ((ZCL_BASIC_CLUSTER_ID == clusterId) && (CLUSTER_MASK_SERVER == mask))
     {
       if ((ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID == attributeId) ||
           (ZCL_APPLICATION_VERSION_ATTRIBUTE_ID == attributeId) ||
           (ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID == attributeId) ||
           (ZCL_DATE_CODE_ATTRIBUTE_ID == attributeId) ||
           (ZCL_SW_BUILD_ID_ATTRIBUTE_ID == attributeId))
       {
         networkStatusProcDebugPrintln(" Unreset ep=%d,cluID=%ld,attID=%ld",endpoint,clusterId,attributeId);
         return false;  //model id不需要恢复出厂默认值
       }
     }
     if ((ZCL_ON_OFF_CLUSTER_ID == clusterId) &&
         (ZCL_ON_OFF_ATTRIBUTE_ID == attributeId) &&
         (CLUSTER_MASK_SERVER == mask))
     {
       networkStatusProcDebugPrintln(" Unreset ep=%d,cluID=%ld,attID=%ld",endpoint,clusterId,attributeId);
       return false;  //on off开关状态不需要恢复出厂默认值
     }
  }
  return true;
}
#endif
/**
//函数名：nodeInfoDeafultReset
//描述：节点信息恢复默认值
//参数：无
//返回：void
*/
void nodeInfoDeafultReset(void)
{
  uint8_t endpointIndex;
  uint8_t endpoint;

  emberClearBindingTable();
  emberAfClearReportTableCallback();
  for (endpointIndex = 0; endpointIndex < emberAfEndpointCount(); endpointIndex++)
  {
    endpoint = emberAfEndpointFromIndex(endpointIndex);
    attributes_resetting_default_flag = 1;     //属性恢复默认值标志置1，表示属性正在恢复默认值
    emberAfResetAttributes(endpoint);
    attributes_resetting_default_flag = 0;     //属性恢复默认值标志置0，表示属性正常执行。
    emberAfGroupsClusterClearGroupTableCallback(endpoint);
    emberAfScenesClusterClearSceneTableCallback(endpoint);
  }
//  switchWaysConfigUpdateEndpointModelId();  //按开关路数更新endpoint的model id
//  deviceInfoAppBasicAttributeInit();        //更新basic属性相关版本信息
  clearCoordinatorManufacturerInfo();         //清除获取到的网关产商信息
  emAfOtaClientStop();
  networkStatusProcDebugPrintln("default reset");
}
/**
//函数名：networkStatusProcessInit
//描述：网络状态上电初始化处理
//参数：无
//返回：无
*/
void networkStatusProcessInit(void)
{
  attributes_resetting_default_flag = 0;
  network_status_action = NETWORK_ACTION_NONE;
  network_status_action_overtime = 0;
  emberEventControlSetInactive(networkActionPollingEventControl);
  networkStatusProcDebugPrintln("init");
  //jim moved 20200427 networkStatusTrigeNetworkAction(NETWORK_ACTION_START_JOIN); //上电触发加网处理事件
}

/**
//函数名：networkStatusUpdateShow
//描述：网络状态更新显示
//参数：无
//返回：无
*/
void networkStatusUpdateShow(void)
{
  if ((emberAfNetworkState() == EMBER_JOINED_NETWORK) ||
      (emberAfNetworkState() == EMBER_JOINED_NETWORK_NO_PARENT))
  {
    ledsAppChangeLedsStatus(LEDS_STATUS_NETWORK_JOINED);
    if (getIdentifyingFlg()) //还存在identify
    {
       ledsAppChangeLedsStatus(LEDS_STATUS_IDENTIFY_UPDATE);
    }
  }
  else
  {
    if (NETWORK_ACTION_START_JOIN == network_status_action)
    {
      ledsAppChangeLedsStatus(LEDS_STATUS_NETWORK_JOINING);
    }
    else
    {
      ledsAppChangeLedsStatus(LEDS_STATUS_IDLE);
    }
  }
}

/**
//函数名：netStatusChangeProcess
//描述：网络状态变化处理函数
//参数：无
//返回：无
*/
void networkStatusChangeProcess(EmberStatus status)
{
   switch (status)
   {
     case EMBER_NETWORK_UP: //在网
       ledsAppChangeLedsStatus(LEDS_STATUS_NETWORK_JOINED);
       break;

    // case EMBER_NETWORK_DOWN:
    //   break;

     default: //不在网，有可能是离网，也有可能是找不到父节点，对于路由设备没有找不到父节点的问题。
       if (emberAfNetworkState() == EMBER_NO_NETWORK)
       {
	     if (EMBER_NETWORK_DOWN == status)
         {
           nodeInfoDeafultReset(); //节点信息恢复出厂默认设定
	     }
         if (network_status_action != NETWORK_ACTION_START_JOIN) //没在加网过程时，更新离网指示灯，如果在加网过程，会影响加网指示灯。
         {
           ledsAppChangeLedsStatus(LEDS_STATUS_NETWORK_LEAVED);
         }
       }
       break;
   }
  // networkStatusUpdateShow();
}

/**
//函数名：networkStatusTrigeNetworkAction
//描述：触发网络动作，离网，入网，离网后重启等
//参数：action_type (uint8_t [输入]，动作类型)
//返回：无
*/
void networkStatusTrigeNetworkAction(uint8_t action_type)
{
#ifdef ORB_PRODUCTION_TEST_CODE  //产测模式判断处理
  if (inProductionTestMode()) //产测时，不允许加网，有冲突
  {
    emberAfPluginNetworkSteeringStop();
    network_status_action = action_type;
    network_status_action_overtime = 0;
    emberEventControlSetInactive(networkActionPollingEventControl);
    return;
  }
#endif
  switch (action_type)
  {
    case NETWORK_ACTION_LEAVE:            //只离网
    case NETWORK_ACTION_LEAVE_AND_JOIN:   //离网后加网
    case NETWORK_ACTION_LEAVE_AND_REBOOT: //离网后重启
        emberAfPluginNetworkSteeringStop();
        emberLeaveNetwork();
        nodeInfoDeafultReset();
        clearOtaStorageTempDataAll();
        network_status_action = action_type;
        network_status_action_overtime = 0;
        emberEventControlSetActive(networkActionPollingEventControl); //开启轮询触发加网处理事件,等待离网成功。
      break;

    case NETWORK_ACTION_START_JOIN:       //触发加网
      emberAfPluginNetworkSteeringStop();
      network_status_action = NETWORK_ACTION_START_JOIN;
      network_status_action_overtime = 0;
      //emberEventControlSetActive(networkActionPollingEventControl); //开启轮询触发加网处理事件
      emberEventControlSetDelayMS(networkActionPollingEventControl,100); //延时等待开关状态变化
      break;

    //jim add 20200717
    case NETWORK_ACTION_DELAY_AND_START_JOIN: //延时后加网
      emberAfPluginNetworkSteeringStop();
      network_status_action = NETWORK_ACTION_START_JOIN;
      network_status_action_overtime = 0;
      emberEventControlSetDelayMS(networkActionPollingEventControl,2000); //延时等待开关状态变化
      break;

    case NETWORK_ACTION_LEAVE_AND_STOP:       //离网后停止加网
      emberStopScan();
      emberAfPluginNetworkSteeringStop();
      emberLeaveNetwork();
      nodeInfoDeafultReset();
      clearOtaStorageTempDataAll();
      network_status_action = action_type;
      network_status_action_overtime = 0;
      emberEventControlSetInactive(networkActionPollingEventControl); //开启轮询触发加网处理事件,等待离网成功。
      break;

    default:
      break;
  }
  networkStatusProcDebugPrintln("Action=%d",action_type);
}

/**
//函数名：networkActionPollingEventHandler
//描述：入网扫描事件处理函数
//参数：无
//返回：无
*/
void networkActionPollingEventHandler(void)
{
  EmberNetworkStatus status = emberAfNetworkState();

  emberEventControlSetInactive(networkActionPollingEventControl);
#ifdef ORB_PRODUCTION_TEST_CODE  //产测模式判断处理
  if (inProductionTestMode()) //产测时，不允许加网，有冲突
  {
    return;
  }
#endif
  networkStatusProcDebugPrintln("poll count=%d,Action=%d",network_status_action_overtime,network_status_action);

  if (NETWORK_ACTION_LEAVE == network_status_action) //只离网
  {
    if ((EMBER_NO_NETWORK == status) || (network_status_action_overtime >= 3)) //已经离网或者3秒超时
    {
      network_status_action = NETWORK_ACTION_NONE;
      network_status_action_overtime = 0;
    }
    else //等待离网成功
    {
      network_status_action_overtime++;
      emberEventControlSetDelayMS(networkActionPollingEventControl,1000); //周期触发扫描入网轮询事件
    }
    return;
  }
  else if (NETWORK_ACTION_LEAVE_AND_JOIN == network_status_action) //离网后加网
  {
    if ((EMBER_NO_NETWORK == status) || (network_status_action_overtime >= 3)) //已经离网或者3秒超时
    {
      network_status_action = NETWORK_ACTION_START_JOIN;
      network_status_action_overtime = 0;
    }
    else //等待离网成功
    {
      network_status_action_overtime++;
      emberEventControlSetDelayMS(networkActionPollingEventControl,1000); //周期触发扫描入网轮询事件
      return;
    }
  }
  else if (NETWORK_ACTION_LEAVE_AND_REBOOT == network_status_action) //离网后重启
  {
    if ((EMBER_NO_NETWORK == status) || (network_status_action_overtime >= 3)) //已经离网或者3秒超时
    {
      network_status_action = NETWORK_ACTION_NONE;
      network_status_action_overtime = 0;
      halReboot();
      //halInternalSysReset(RESET_SOFTWARE_REBOOT);
    }
    else //等待离网成功
    {
      network_status_action_overtime++;
      emberEventControlSetDelayMS(networkActionPollingEventControl,1000); //周期触发扫描入网轮询事件
      return;
    }
  }

  //加网部分
  if (NETWORK_ACTION_START_JOIN == network_status_action)
  {
    if ((EMBER_JOINED_NETWORK == status) || (EMBER_JOINED_NETWORK_NO_PARENT == status))  //已经入网
    {
      network_status_action = NETWORK_ACTION_NONE;
      network_status_action_overtime = 0;
      ledsAppChangeLedsStatus(LEDS_STATUS_NETWORK_JOINED);
    }
    else //未完全入网
    {
    #ifdef NETWORK_JOINING_TIMEOUT_ENABLE  //加网超时开关
	  network_status_action_overtime++;
     // if ((RELAY_CONTROL_TURN_OFF == relayControlDriverGetCurrenStatus(0)) &&
     //     (RELAY_CONTROL_TURN_OFF == relayControlDriverGetCurrenStatus(1))) //jim 20200427 add make sure the relay on
      if (RELAY_CONTROL_TURN_OFF == relayControlDriverGetCurrenStatus(MAIN_POWER_RELAY_NUMBER)) //jim 20200717
      {
        network_status_action_overtime = NETWORK_JOINING_TIMEOUT_COUNTS;
      }
      if (network_status_action_overtime < NETWORK_JOINING_TIMEOUT_COUNTS)
	  {
        ledsAppChangeLedsStatus(LEDS_STATUS_NETWORK_JOINING);
       #ifdef ORB_PRODUCTION_TEST_CODE  //产测模式判断处理
        if (authorizationPass() == true)
       #endif
        {
           emberAfPluginNetworkSteeringStart(); //触发扫描入网
        }
        emberEventControlSetDelayMS(networkActionPollingEventControl,NETWORK_JOINING_POLL_TIME_MS); //周期触发扫描入网轮询事件
	  }
	  else  //超时
	  {
        emberStopScan();
		emberAfPluginNetworkSteeringStop(); //停止扫描入网
		network_status_action = NETWORK_ACTION_NONE;
		network_status_action_overtime = 0;
        //networkStatusUpdateShow();
        ledsAppChangeLedsStatus(LEDS_STATUS_STOP);
	  }
    #else
      ledsAppChangeLedsStatus(LEDS_STATUS_NETWORK_JOINING);
      #ifdef ORB_PRODUCTION_TEST_CODE  //产测模式判断处理
        if (authorizationPass() == true)
      #endif
        {
          if (RELAY_CONTROL_TURN_OFF == relayControlDriverGetCurrenStatus(MAIN_POWER_RELAY_NUMBER)) //jim 20200717
          {
            emberStopScan();
            emberAfPluginNetworkSteeringStop();
			ledsAppChangeLedsStatus(LEDS_STATUS_STOP);
            network_status_action_overtime = 0;
            return;
          }
          else
            emberAfPluginNetworkSteeringStart(); //触发扫描入网
        }
      emberEventControlSetDelayMS(networkActionPollingEventControl,NETWORK_JOINING_POLL_TIME_MS); //周期触发扫描入网轮询事件
    #endif
    }
  }
}

/*************************************** 文 件 结 束 ******************************************/
