/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:z3-touch-switch-app.c
*Author : JimYao
*Version : 1.0
*Date : 2019-09-16
*Description: 触摸开关运用代码入口
*History:
***************************************************************************************************/
/* 相关包含头文件区 ------------------------------------------------------------ */
#include "common-app.h"
#include "app/framework/util/attribute-storage.h"
#include "app/framework/util/util.h"
#include "app/framework/plugin/ota-client/ota-client.h"
#include "app/framework/plugin/reporting/reporting.h"

/* 宏定义区 -------------------------------------------------------------------- */
//debug开关设定
//#define MIX_SW_APP_DEBUG_ENABLE
#ifdef MIX_SW_APP_DEBUG_ENABLE
  #define DEBUG_STRING                   "SwApp-DB:"
  #define customAppDebugPrint(...)       emberAfPrint(0xFFFF, DEBUG_STRING __VA_ARGS__)
  #define customAppDebugPrintln(...)     emberAfPrintln(0xFFFF, DEBUG_STRING __VA_ARGS__)
#else
  #define customAppDebugPrint(...)
  #define customAppDebugPrintln(...)
#endif


#define DEFAULT_NORMAL_SHORT_POLLING_INTERVAL_MS          800    //设备正常工作的polling间隔
#define START_OTA_SHORT_POLLING_INTERVAL_MS               100     //设备开始ota的polling间隔

#define POWER_ON_NETWORK_UP_TRIGREJOIN_CODE     //20200428 jim add for test
/* 自定义类型区 ---------------------------------------------------------------- */

/* 全局变量区 ------------------------------------------------------------------ */
EmberEventControl appPowerOnDelayInitEventControl;	        //上电延时初始化事件
EmberEventControl appPowerOnOperationControl;				//上电延时5s再去操作继电器
static uint8_t poweron_flg =false;									//上电标志位

//void appPowerOnDelayInitEventHandler(void);               //需要在代码中添加appPowerOnDelayInitEventHandler事件处理


/* 本地变量区 ------------------------------------------------------------------ */
static uint8_t local_ways = SWITCH_3WAYS;
static uint8_t unbind_report_enable[3] = {0,0,0};  //处理第一次入网，为欧瑞博网关时，主动上报的处理标志
static uint8_t node_type = EMBER_SLEEPY_END_DEVICE; //4; //设备类型,默认为enddevice
//jim add 20200717
enum{
    NONE_TRIG = 0x00,
    BUTTON_TRIG = 0x01,
};
static uint8_t trig_type[SWITCH_3WAYS] = {NONE_TRIG};
//end jim

#ifdef PREVENT_LONG_TIME_AWAKE_CODE
  #define PREVENT_LONG_TIME_AWAKE_TIMEOUT_MS           (20*60000)   //20分钟不休眠超时重启
  static uint32_t last_awake_time_ms;
#endif
/* 全局函数声明区 -------------------------------------------------------------- */
#ifdef REPORTING_CALLBACK_CODE
/**
//函数名：reportingPluginTrigReport
//描述：通过此函数触发上报
//参数：source_endpoint (uint8_t [输入], 本地端上报的源端点;)
//      clusterId (EmberAfClusterId [输入], 本地端上报的源端点下的cluster id;)
//      attributeId (EmberAfAttributeId [输入], 本地端上报的cluster id下的attribute id;
//                                              当为MAX_INT16U_VALUE时，表示所有配置表中的atrributes都上报)
//      fix_delay_time_s (uint8_t [输入], 上报延时时间,秒;)
//      random_delay_max_time_ms (uint16_t [输入], 上报随机时间,毫秒)
//返回：无
*/
void reportingPluginTrigReport(uint8_t source_endpoint,
                               EmberAfClusterId clusterId,
                               EmberAfAttributeId attributeId,
                               uint8_t fix_delay_time_s,
                               uint16_t random_delay_max_time_ms);
/**
//函数名：reportingPluginCheckReportConfigExist
//描述：检测对应的cluster是否在上报配置表
//参数：endpoint (uint8_t [输入], 本地端上报的源端点;)
//      clusterId (EmberAfClusterId [输入], 本地端上报的源端点下的cluster id;)
//返回：bool, true 存在；false 不存在；
*/
bool reportingPluginCheckReportConfigExist(uint8_t endpoint,
                                           EmberAfClusterId clusterId);
#endif

#ifdef SCENES_RECALL_SAVED_SECENE_CALLBACK_CODE
  void scenesTransitionInit(void);
#endif
/* 本地函数声明区 -------------------------------------------------------------- */
static void relayControlFinishedCallback(uint8_t way, uint8_t status);
void switchAppNetworkUpTrigReport(uint8_t type);
//static void batteryCapacityUpdateCallback(tBatteryWorkRecord *battery_record);
static uint8_t readDeviceTypeAttribute(void);
static void reportDeviceType(void);
static void reportSwitchType(void);
//jim add 20200717
static bool switchOffPreProc(uint8_t way);
static void switchOnDoneProc(uint8_t way);
//end jim
/* 函数原型 -------------------------------------------------------------------- */
#ifdef PREVENT_LONG_TIME_AWAKE_CODE
/**
//函数名：preventLongTimeAwakeInit
//描  述：初始化长时间不休眠变量
//参  数：无
//返  回：void
*/
void preventLongTimeAwakeInit(void)
{
  last_awake_time_ms = halCommonGetInt32uMillisecondTick();
}
/**
//函数名：preventLongTimeAwakeProc
//描  述：长时间不休眠处理入口
//参  数：无
//返  回：void
*/
void preventLongTimeAwakeProc(void)
{
  uint32_t current_time_ms;
  current_time_ms =  halCommonGetInt32uMillisecondTick();
  if (elapsedTimeInt32u(last_awake_time_ms,current_time_ms) > PREVENT_LONG_TIME_AWAKE_TIMEOUT_MS)
  {
    //已经达到持续唤醒超时时限，需要做处理。
    preRebootStatusSaveProc(SWITCH_REBOOT_REASON_LONG_WAKEUP); //重启前的状态保存处理
    // Software reset.
    halReboot();
  }
}
#endif

/**
//函数名：getDeviceType
//描述：获取设备类型，route为2，enddevice为4
//参数：无
//返回：void
*/
uint8_t getDeviceType(void)
{
	return node_type;
}

/**
//函数名：setFlashDeviceType
//描述：将设备类型存进flash，route为2，enddevice为4
//参数：无
//返回：void
*/
void setFlashDeviceType(uint8_t device_type)
{
	halCommonSetToken(TOKEN_CUSTOM_NODE_TYPE, (uint8_t *)&device_type);
}

/**
//函数名：getFlashDeviceType
//描述：从flash获取设备类型
//参数：无
//返回：void
*/
uint8_t getFlashDeviceType(void)
{
	halCommonGetToken(&node_type, TOKEN_CUSTOM_NODE_TYPE);
	customAppDebugPrintln("flash node type:%d",node_type);
	return node_type;
}

/**
//函数名：setFlashSwitchType
//描述：设置开关类型，0为翘班，1为回弹
//参数：无
//返回：void
*/
void setFlashSwitchType(uint8_t type)
{
	halCommonSetToken(TOKEN_CUSTOM_SWITCH_TYPE, (uint8_t *)&type);
}

/**
//函数名：getFlashSwitchType
//描述：从flash获取开关是属于翘板还是回弹类型
//参数：无
//返回：void
*/
uint8_t getFlashSwitchType(void)
{
	uint8_t type;
	halCommonGetToken(&type, TOKEN_CUSTOM_SWITCH_TYPE);
	customAppDebugPrintln("switch type:%d",type);
	return type;
}

void appPowerOnOperationHandler(void)
{
	emberEventControlSetInactive(appPowerOnOperationControl);
	poweron_flg = false;
    syncButtonAndSwitchStatus();
	customAppDebugPrintln("appPowerOnOperationHandler");
}

/**
//函数名：reportingPluginCheckReportConfigExist
//描述：处理onoffcluster指令前的回调，通过此可以控制是否需要每次都更新属性值。
//参数：endpoint               (uint8_t [输入], 指令控制的本地端点;)
//      currentValue           (bool    [输入], 当前属性中的开关值;)
//      command                (uint8_t [输入], 当前的指令动作;)
//      initiatedByLevelChange (bool    [输入], 开关是否与level值关联;)
//返回：bool, true SDK代码处理，当前动作与属性值相同时不写属性；false 不管动作，直接更新属性；
*/
bool onOffClusterSetValueCommandCallback(uint8_t endpoint, bool currentValue,
                                         uint8_t command,bool initiatedByLevelChange)
{
	return false;
}

/**
//函数名：appPowerOnInit
//描述：设备上电后初始化，留此接接口方便产测超时退出时使用
//参数：无
//返回：void
*/
void appPowerOnInit(void)
{
  poweron_flg = true;
//  batteryVoltageAdcGgpioPinsInit();                              //初始化电池电压采样相关引脚
  //jim switchWaysConfigInit();                                        //第一步需要先初始化开关的路数配置，这个初始化会影响到按键的功能部分。
  ledsAppInit();                                                 //初始化指示灯，指示灯驱动初始化与逻辑变量初始化。
  relayContorlDriverTaskInit(relayControlFinishedCallback);      //继电器控制脚驱动初始化,注册继电器控制完成后的一个回调。
  relayControlBufferInitial();                                   //初始化继电器控制缓存器，用来防止继电器同时动作。
  buttonsAppInit();                                              //初始化按键，驱动和按键逻辑变量初始化。
  local_ways = switchWaysConfigGetWays();

  customAppDebugPrintln("main init,ways=%d,device type:%d",local_ways,getFlashDeviceType());

#ifdef SCENES_RECALL_SAVED_SECENE_CALLBACK_CODE
  scenesTransitionInit();  //初始化场景过渡处理部分
#endif
  setRejoinIntervalTableIndex(0, 0); //rejoin的index号初始化

#ifdef PREVENT_LONG_TIME_AWAKE_CODE
  preventLongTimeAwakeInit();
#endif
#if 0 //add power on delay 500ms
  uint32_t current_time_ms;
  current_time_ms = halCommonGetInt32uMillisecondTick();
  while(1)
  {
    halResetWatchdog();
    if (halCommonGetInt32uMillisecondTick() - current_time_ms > 10*1000)
    {
      break;
    }
  }
#endif
  emberEventControlSetActive(appPowerOnDelayInitEventControl);
  emberEventControlSetDelayMS(appPowerOnOperationControl,5000); //延时5s再去根据翘板开关状态去动作继电器

}
/**
//函数名：appPowerOnDelayInitEventHandler
//描述：app上电延时初如化事件处理入口
//参数：无
//返回：void
*/
void appPowerOnDelayInitEventHandler(void)
{
  uint8_t tempType =0;
  emberEventControlSetInactive(appPowerOnDelayInitEventControl);

  emberSetRadioPower(emberAfPluginNetworkSteeringGetPowerForRadioChannelCallback(emberGetRadioChannel()));
  switchRebootCheckToRecoverStatus();          //重启检测是否为软件重启，恢复重启前状态
  identifyCoordinatorManufactureProcInit();    //获取网关处理初始化
  switchWaysConfigUpdateEndpointModelId();     //按开关路数更新endpoint的model id
  deviceInfoAppBasicAttributeInit();           //更新basic属性相关版本信息
  networkStatusProcessInit();                  //上电更新网络状态
  readDeviceTypeAttribute();				   //读开关类型
  tempType =getFlashSwitchType();
  setSwitchType(tempType);
  customAppDebugPrintln(">>app init,switch type:%d,%d",getFlashSwitchType(),readDeviceTypeAttribute());
}
/** @brief Main Init
 *
 * This function is called from the application's main function. It gives the
 * application a chance to do any initialization required at system startup.
 * Any code that you would normally put into the top of the application's
 * main() routine should be put into this function.
        Note: No callback
 * in the Application Framework is associated with resource cleanup. If you
 * are implementing your application on a Unix host where resource cleanup is
 * a consideration, we expect that you will use the standard Posix system
 * calls, including the use of atexit() and handlers for signals such as
 * SIGTERM, SIGINT, SIGCHLD, SIGPIPE and so on. If you use the signal()
 * function to register your signal handler, please mind the returned value
 * which may be an Application Framework function. If the return value is
 * non-null, please make sure that you call the returned function from your
 * handler to avoid negating the resource cleanup of the Application Framework
 * itself.
 *
 */
void emberAfMainInitCallback(void)
{
#ifdef ORB_PRODUCTION_TEST_CODE  //产测模式判断入口
  checkToEnterProductionTest();                             //上电判断是否满足进去产测条件
#endif
  appPowerOnInit();
}

/** @brief Main Tick
 *
 * Whenever main application tick is called, this callback will be called at the
 * end of the main tick execution.
 *
 */
void emberAfMainTickCallback(void)
{
#ifdef PREVENT_LONG_TIME_AWAKE_CODE
  preventLongTimeAwakeProc();  //防止长时间不休眠处理
#endif
}

/** @brief Get Power For Radio Channel
 *
 * This callback is fired when the Network Steering plugin needs to set the
 * power level. The application has the ability to change the max power level
 * used for this particular channel.
 *
 * @param channel The channel that the plugin is inquiring about the power
 * level. Ver.: always
 */
int8_t emberAfPluginNetworkSteeringGetPowerForRadioChannelCallback(uint8_t channel)
{
  //return emberAfMaxPowerLevel();
  return EMBER_AF_PLUGIN_NETWORK_STEERING_RADIO_TX_POWER;
}

/** @brief Complete
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to EMBER_SUCCESS to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt. Ver.: always
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID. Ver.: always
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network. Ver.: always
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete. Ver.: always
 */
void emberAfPluginNetworkSteeringCompleteCallback(EmberStatus status,
                                                  uint8_t totalBeacons,
                                                  uint8_t joinAttempts,
                                                  uint8_t finalState)
{

}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
bool emberAfStackStatusCallback(EmberStatus status)
{
#ifdef POWER_ON_NETWORK_UP_TRIGREJOIN_CODE //20200428 jim add for test
  //第一次上电，还认为在网，会进行一次全信道的扫描(禁止)，故加的此代码
  static uint8_t first_power_on = 1;
  if (first_power_on)
  {
   #ifdef MODULE_CHANGE_NODETYPE
	if(getDeviceType() ==EMBER_SLEEPY_END_DEVICE)
   #endif
	{
      if (EMBER_NETWORK_UP == status)
      {
        emberFindAndRejoinNetworkWithReason(true,
                                            0,
                                            EMBER_AF_REJOIN_DUE_TO_END_DEVICE_MOVE);
      }
    }
    first_power_on = 0;
    return false;
  }
#endif
  networkStatusChangeProcess(status);
  if (EMBER_NETWORK_UP == status)
  {
    switchAppNetworkUpTrigReport(1);
    checkToStartIdentifyCoordinatorManufacturer(1500);                  //判断是否需要启动获取网关信息
    //休眠设备部分的设定
    #ifdef MODULE_CHANGE_NODETYPE
	if(getDeviceType()==EMBER_SLEEPY_END_DEVICE)
	{
	#endif
	    emberAfSetWakeTimeoutMsCallback(0xFFFF);                                         //设定为最大值，处于一直polling
	    emberAfSetShortPollIntervalMsCallback(DEFAULT_NORMAL_SHORT_POLLING_INTERVAL_MS); //设定polling间隔 800 ms
	    emberAfAddToCurrentAppTasksCallback(EMBER_AF_FORCE_SHORT_POLL);                  //触发polling事件
    #ifdef MODULE_CHANGE_NODETYPE
	}
    #endif
	customAppDebugPrintln("wyz->EMBER_NETWORK_ON:%d\r\n",getDeviceType()); //wyz add
	customAppDebugPrintln("wyz chn: %d\r\n",emberAfGetRadioChannel());
	customAppDebugPrintln("wyz node ID: %2x\r\n",emberAfGetNodeId());
  }
  else
  {
	 customAppDebugPrintln("network down");
  }
  return false;
}

/** @brief On/off Cluster Server Post Init
 *
 * Following resolution of the On/Off state at startup for this endpoint, perform any
 * additional initialization needed; e.g., synchronize hardware state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginOnOffClusterServerPostInitCallback(uint8_t endpoint)
{
  // At startup, trigger a read of the attribute and possibly a toggle of the
  // LED to make sure they are always in sync.
  #if 0
  emberAfOnOffClusterServerAttributeChangedCallback(endpoint,
                                                    ZCL_ON_OFF_ATTRIBUTE_ID);
  #endif
}

/** @brief Server Attribute Changed
 *
 * On/off cluster, Server Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
void emberAfOnOffClusterServerAttributeChangedCallback(uint8_t endpoint,
                                                       EmberAfAttributeId attributeId)
{
  // When the on/off attribute changes, set the LED appropriately.  If an error
  // occurs, ignore it because there's really nothing we can do.
  if (attributeId == ZCL_ON_OFF_ATTRIBUTE_ID)
  {
    uint8_t i, on_off;
    customAppDebugPrintln("on off attribute change endpoint=%d",endpoint);
    for (i=0; i<local_ways; i++)
    {
      if (endpoint == emberAfEndpointFromIndex(i)) //找出是哪一路
      {
        if (emberAfReadServerAttribute(endpoint,
                                       ZCL_ON_OFF_CLUSTER_ID,
                                       ZCL_ON_OFF_ATTRIBUTE_ID,
                                       (uint8_t *)&on_off,
                                       sizeof(on_off)) == EMBER_ZCL_STATUS_SUCCESS)
        {
          if (0x00 == on_off)
          {
            //jim add 20200717
            uint8_t delay_ms = 50;
            if (switchOffPreProc(i))
            {
               delay_ms = 150; //延时150ms 开
            }
            //end jim
            //relayControlDriverTrigeOnOffAction(i,RELAY_CONTROL_TURN_OFF, 0); //指示灯切换在relay contorl的call back函数里处理
            updateAndTrigeRelayControlBufferNextAction(i, RELAY_CONTROL_TURN_OFF, delay_ms, true);
          }
          else
          {
            //relayControlDriverTrigeOnOffAction(i,RELAY_CONTROL_TURN_ON, 0); //指示灯切换在relay contorl的call back函数里处理
            updateAndTrigeRelayControlBufferNextAction(i, RELAY_CONTROL_TURN_ON, 10, true);
          }
          customAppDebugPrintln("get on off attribute=%d",on_off);
        }
        break;
      }
    }
  }
}

/**
//函数名：relayControlFinishedCallback
//描述：继电器开关控制完成状态回调
//参数：way    (uint8_t[输入],第几路开关完成控制，0，1，2;)
//      status (uint8_t[输入],开关最后状态，0关，1开)
//返回：void
*/
static void relayControlFinishedCallback(uint8_t way, uint8_t status)
{
  if (way < local_ways)
  {
      //闪灯触发
    networkStatusUpdateShow();
    customAppDebugPrintln("way=%d,contorl finished",way);
  }
  updateAndTrigeRelayControlBufferNextAction(way, 0, 0, false); //结束后，需要更把最后在add_flg参数填false,触发一次动作更新。
 // batteryVoltageFifoUpdate(); //jim for test voltage detect
  if (status == 0x01) //开 //jim add 20200717
  {
    switchOnDoneProc(way);
  }
}


/** @brief Reset To Factory Defaults
 *
 * This function is called by the Basic server plugin when a request to reset
 * to factory defaults is received. The plugin will reset attributes managed by
 * the framework to their default values. The application should perform any
 * other necessary reset-related operations in this callback, including
 * resetting any externally-stored attributes.
 *
 * @param endpoint   Ver.: always
 */
void emberAfPluginBasicResetToFactoryDefaultsCallback(uint8_t endpoint)
{
  emberAfIdentifyClusterServerAttributeChangedCallback(endpoint,
                                                ZCL_IDENTIFY_TIME_ATTRIBUTE_ID);
}

/** @brief Remote Set Binding Permission
 *
 * This function is called by the framework to request permission to service the
 * remote set binding request. Return EMBER_SUCCESS to allow request, anything
 * else to disallow request.
 *
 * @param entry Ember Binding Tablet Entry  Ver.: always
 */
EmberStatus emberAfRemoteSetBindingPermissionCallback(const EmberBindingTableEntry *entry)
{
#ifdef REPORTING_CALLBACK_CODE
  //绑定时触发一次上报,1秒到3秒之间的随机时间。
  if (reportingPluginCheckReportConfigExist(entry->local,entry->clusterId) == true)
  {
    reportingPluginTrigReport(entry->local,
                              entry->clusterId,
                              MAX_INT16U_VALUE,
                              1,
                              2000);
    customAppDebugPrintln("bonding report");
  }
#endif
  return EMBER_SUCCESS;
}

/**
//函数名：switchAppNetworkUpTrigReport
//描述：设备入网后触发onoff属性report
//参数：type: 1 为翘板开关类型 其他值为onoff报告
//返回：void
*/
void switchAppNetworkUpTrigReport(uint8_t type)
{
  uint8_t endpoint;

	if(type ==1)
	{
		#ifdef MODULE_CHANGE_NODETYPE
		reportDeviceType();
		#endif
		reportSwitchType();
	}
	else
	{
		for (uint8_t i=0; i<local_ways; i++)
		{
		  endpoint = emberAfEndpointFromIndex(i);
		  if (endpoint != 0xFF)
		  {
			#ifdef REPORTING_CALLBACK_CODE
			reportingPluginTrigReport(endpoint,
									  ZCL_ON_OFF_CLUSTER_ID,
									  ZCL_ON_OFF_ATTRIBUTE_ID,
									  2+i,
									  2000); //随机时间上电后每个节点后延一秒
			customAppDebugPrintln("+++network up onoff report");
			#endif
		  }
		
		}

	}
}
#ifdef REPORTING_CALLBACK_CODE
/**
//函数名：reportingPluginReportInitLoadSettingCallback
//描述：report congifg初始化时更新配置时间的回调，主要给运用代码处理和上报时间相关的任务。
//参数：*entry (EmberAfPluginReportingEntry * [输入]，更新的report config信息)
//返回：无
*/
void reportingPluginReportInitLoadSettingCallback(const EmberAfPluginReportingEntry *entry)
{
  uint8_t endpoint;
  for (uint8_t i=0; i<local_ways; i++)
  {
    endpoint = emberAfEndpointFromIndex(i);
    if (endpoint == entry -> endpoint)
    {
      if ((entry -> clusterId == ZCL_ON_OFF_CLUSTER_ID) &&
          (entry -> attributeId == ZCL_ON_OFF_ATTRIBUTE_ID))
      {
        reportingPluginTrigReport(endpoint,
                                  ZCL_ON_OFF_CLUSTER_ID,
                                  ZCL_ON_OFF_ATTRIBUTE_ID,
                                  5+i,
                                  2000); //随机时间上电后每个节点后延一秒
      }
    }
  }
}
/**
//函数名：reportingPluginReportableChangeCallback
//描  述：属性更改
//参  数：*entry               (EmberAfPluginReportingEntry * [输入]     ，属性变化达到要求的report表入口信息)
//        *report_data         (EmAfPluginReportVolatileData *[输入]     ，属性上报值的相关信息)
//返  回：无
*/
void reportingPluginReportableChangeCallback(const EmberAfPluginReportingEntry *entry,
                                             EmAfPluginReportVolatileData *report_data)
{
  //此部分用来处理设备上报条件满足后，按不同类型的数据包做延时上报，此回调是设备属性状态变化后触发的回调
  uint8_t endpoint;
  if (report_data -> reportableChange == true)  //属性变化已经达到上报条件
  {
    for (uint8_t i=0; i<local_ways; i++)
    {
      endpoint = emberAfEndpointFromIndex(i);
      if (endpoint == entry -> endpoint)
      {
        if ((entry -> clusterId == ZCL_ON_OFF_CLUSTER_ID) &&
            (entry -> attributeId == ZCL_ON_OFF_ATTRIBUTE_ID))
        {
           uint32_t minIntervalMs = (entry->data.reported.minInterval
                                * MILLISECOND_TICKS_PER_SECOND);
           uint32_t maxIntervalMs = (entry->data.reported.maxInterval
                                * MILLISECOND_TICKS_PER_SECOND);
           uint32_t elapsedMs = elapsedTimeInt32u(report_data->lastReportTimeMs,
                                             halCommonGetInt32uMillisecondTick());
           if (elapsedMs >= minIntervalMs)  //已经达到上报条件
           {
               //如果是ORB的网关，组播，多播，场景触发的上报，不做上报。
               //如果是其它的网关，按zigbee3.0标准的上报机制上报，组播，多播，场景触发的 加多一个小窗口随机延时
             switch (getOnOffClusterCommandType(i))
             {
                case SCENE_CONTROL:       //场景控制
                case MULTICAST_CONTROL:   //多播控制
                case BROADCAST_CONTROL:   //广播控制

                  if (whetherIsOrviboCoordinator() == true)
                  {
                     //ORB的网关，不上报
                     report_data->reportableChange = false;
                     report_data->lastReportTimeMs = halCommonGetInt32uMillisecondTick(); //更新最后上报时间为当前时间，去掉上报。
                     uint8_t on_off=0;
                     if (emberAfReadServerAttribute(endpoint,
                                       ZCL_ON_OFF_CLUSTER_ID,
                                       ZCL_ON_OFF_ATTRIBUTE_ID,
                                       &on_off,
                                       sizeof(on_off)) == EMBER_ZCL_STATUS_SUCCESS)
                     {
                        report_data->lastReportValue = on_off;
                     }
                     customAppDebugPrintln("on off report maxintervalms time=%ld ms",maxIntervalMs);
                  }
                  else
                  {
                     //其它网关，切换随机延时上报
                     report_data->reportableChange = false;
                     //计算出随机时间距离最大时间的最后上报时间。实现触发随机上报
                     minIntervalMs = 1000 + (emberGetPseudoRandomNumber() % 2000); //获取1-3秒随机数
                     elapsedMs = elapsedTimeInt32u(maxIntervalMs,halCommonGetInt32uMillisecondTick()); //计算出距离最大上报间隔的时间

                     if (minIntervalMs > maxIntervalMs) //不能超过最大上报时间
                     {
                        minIntervalMs = maxIntervalMs;
                     }
                     elapsedMs += minIntervalMs;
                     report_data->lastReportTimeMs = elapsedMs;
                     customAppDebugPrintln("on off report randmon time=%ld ms",minIntervalMs);
                  }
                  break;
                default:  //单播和按键触发的，按正常的上报机制处理
                  break;
             }
             setOnOffClusterCommandType(i,UNICAST_CONTROL);  //切换为单播上报
             break;
           }
        }
      }
    }
  }
}
/**
//函数名：orviboCoordinatorIdentifyFinishCallback
//描  述：第一次辨识到是ORB的网关产生的回调
//参  数：无
//返  回：void
*/
void orviboCoordinatorIdentifyFinishCallback(void)
{
   if (whetherIsOrviboCoordinator() == true) //设备辨识到是ORB的网关
   {
      for (uint8_t i=0; i<local_ways; i++)
      {
        unbind_report_enable[i] = emberAfEndpointFromIndex(i); //启动检查非绑定上报处理,上报完三路开关状态后，关闭。
        reportingPluginTrigReport(unbind_report_enable[i],
                                  ZCL_ON_OFF_CLUSTER_ID,
                                  ZCL_ON_OFF_ATTRIBUTE_ID,
                                  1+i,
                                  1000); //随机时间上电后每个节点后延一秒
      }
   }
}
/**
//函数名：unicastReportAttribute
//描述：直接上报属性值
//参数：destination_addr (uint16_t[输入],上报的目标地址;)
//      dest_endpoint    (uint8_t[输入],上报的目标地址的endpoint;)
//      source_endpoint  (uint8_t[输入],上报属性所在的endpoint;)
//      clusterId        (EmberAfClusterId[输入],上报属性所在的clusterid;)
//      attributeId      (EmberAfAttributeId[输入],上报属性的ID值;)
//      dataType         (EmberAfAttributeType[输入],上报属性值的数据类型;)
//      *datas           (uint8_t*[输入],上报属性值内容;)
//      dataSize         (uint8_t [输入],上报属性值内容长度;)
//返回：void
*/
void unicastReportAttribute(uint16_t destination_addr,
                                   uint8_t dest_endpoint,
                                   uint8_t source_endpoint,
                                   EmberAfClusterId clusterId,
                                   EmberAfAttributeId attributeId,
                                   EmberAfAttributeType dataType,
                                   uint8_t *datas,
                                   uint8_t dataSize)
{
   EmberApsFrame *apsFrame = NULL;
   apsFrame = emberAfGetCommandApsFrame();
   emberAfFillExternalManufacturerSpecificBuffer(
                            (ZCL_GLOBAL_COMMAND |
                             ZCL_FRAME_CONTROL_SERVER_TO_CLIENT |
                             ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                             clusterId,
                             EMBER_AF_NULL_MANUFACTURER_CODE,
                             ZCL_REPORT_ATTRIBUTES_COMMAND_ID,
                             "");
   apsFrame->sourceEndpoint = source_endpoint;
   apsFrame->destinationEndpoint = dest_endpoint;
   apsFrame->options = EMBER_AF_DEFAULT_APS_OPTIONS;
   // Payload is [attribute id:2] [type:1] [data:N].
   emberAfPutInt16uInResp(attributeId);
   emberAfPutInt8uInResp(dataType);
   emberAfPutBlockInResp(datas, dataSize);

   for (uint8_t i = 0; i < 3; i++)
   {
      if(emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,destination_addr) == EMBER_SUCCESS)
        break;
   }
}
/**
//函数名：reportingPluginReportWithoutBindTableProcCallback
//描述：处理无绑定表的上报，由运用层处理相应的上报。
//参数：source_endpoint (uint8_t          [输入]，本地上报的源endpoint号)
//      clusterId       (EmberAfClusterId [输入]，本地上报的ClusterId号)
//返回：void
*/
void reportingPluginReportWithoutBindTableProcCallback(uint8_t source_endpoint,EmberAfClusterId clusterId)
{
  if (whetherIsOrviboCoordinator() == true)
  {
    for (uint8_t i=0; i<local_ways; i++)
    {
      if (unbind_report_enable[i] == source_endpoint) //设备入网后主动上报一次非绑定的上报给ORB的网关。
      {
        uint8_t on_off;
        if (emberAfReadServerAttribute(source_endpoint,
                                       ZCL_ON_OFF_CLUSTER_ID,
                                       ZCL_ON_OFF_ATTRIBUTE_ID,
                                       (uint8_t *)&on_off,
                                       sizeof(on_off)) == EMBER_ZCL_STATUS_SUCCESS)
        {
          unicastReportAttribute(0x0000,0x01,
                                 source_endpoint,
                                 ZCL_ON_OFF_CLUSTER_ID,
                                 ZCL_ON_OFF_ATTRIBUTE_ID,
                                 ZCL_BOOLEAN_ATTRIBUTE_TYPE,
                                 (uint8_t *)&on_off,
                                 1);
        }
        unbind_report_enable[i] = 0;
        customAppDebugPrintln("ep:%d,cluster:%d unbind report",source_endpoint,clusterId);
        break;
      }
    }
  }
}
#endif

/** @brief Pre Message Send
 *
 * This function is called by the framework when it is about to pass a message
 * to the stack primitives for sending.   This message may or may not be ZCL,
 * ZDO, or some other protocol.  This is called prior to
        any ZigBee
 * fragmentation that may be done.  If the function returns true it is assumed
 * the callback has consumed and processed the message.  The callback must also
 * set the EmberStatus status code to be passed back to the caller.  The
 * framework will do no further processing on the message.
        If the
 * function returns false then it is assumed that the callback has not processed
 * the mesasge and the framework will continue to process accordingly.
 *
 * @param messageStruct The structure containing the parameters of the APS
 * message to be sent.  Ver.: always
 * @param status A pointer to the status code value that will be returned to the
 * caller.  Ver.: always
 */
bool emberAfPreMessageSendCallback(EmberAfMessageStruct* messageStruct,
                                   EmberStatus* status)
{
  //jim add for test
  //customAppDebugPrintln("=====Message Send======");
  checkNetworkStateAndTrigeRejoin();
  return false;
}

/** @brief Basic Cluster Server Attribute Changed
*
* Server Attribute Changed
*
* @param endpoint Endpoint that is being initialized  Ver.: always
* @param attributeId Attribute that changed  Ver.: always
*/
void emberAfBasicClusterServerAttributeChangedCallback(uint8_t endpoint,
                                                       EmberAfAttributeId attributeId)
{
	uint8_t nodetype =0;
	#ifdef MODULE_CHANGE_NODETYPE
	if(attributeId ==ZCL_PHYSICAL_ENVIRONMENT_ATTRIBUTE_ID)
	{
		if(poweron_flg)
		{
			//poweron_flg = false; //刚上电不会跑下面重启
			return;
		}
		if (emberAfReadServerAttribute(endpoint,
									   ZCL_BASIC_CLUSTER_ID,
									   ZCL_PHYSICAL_ENVIRONMENT_ATTRIBUTE_ID,
									   (uint8_t *)&nodetype,
									   sizeof(nodetype))
				== EMBER_ZCL_STATUS_SUCCESS)
		{
			customAppDebugPrintln("basic change:%d",nodetype);
			if(nodetype ==EMBER_SLEEPY_END_DEVICE || nodetype ==EMBER_ROUTER)
			{
				setFlashDeviceType(nodetype);
				if(nodetype ==EMBER_SLEEPY_END_DEVICE)
				{
					ledsAppChangeLedsStatus(LEDS_STATUS_SLEEP_END_DEVICE);
				}
				else
				{
					ledsAppChangeLedsStatus(LEDS_STATUS_ROUTE);
				}
				emberEventControlSetDelayMS(app_RestartEventControl,1000);
			}
		}

	}
	#endif
}
/** @brief Wake Up
 *
 * This function is called by the Idle/Sleep plugin after sleeping.
 *
 * @param durationMs The duration in milliseconds that the device slept.
 * Ver.: always
 */
void emberAfPluginIdleSleepWakeUpCallback(uint32_t durationMs)
{
#ifdef PREVENT_LONG_TIME_AWAKE_CODE
  if (durationMs > 5) //做个休眠时长过滤，至少休眠5ms后唤醒的才算是有效休眠
  {
     preventLongTimeAwakeInit(); //从休眠唤醒后，需要更新时间值
  }
#endif
}

/**
//函数名：readDeviceTypeAttribute
//描述：读开关类型的属性，并设置到对应的变量里
//参数：无
//返回：void
*/
static uint8_t readDeviceTypeAttribute(void)
{
	uint8_t temp =0;
	emberAfReadAttribute(emberAfEndpointFromIndex(0),
								ZCL_BASIC_CLUSTER_ID,
								ZCL_GENERIC_DEVICE_TYPE_ATTRIBUTE_ID,
								CLUSTER_MASK_SERVER,
								(uint8_t *)&temp,
								sizeof(temp),
								NULL); // data type
	customAppDebugPrintln("read switch type attri:%d",temp);
	return temp;
}

/**
//函数名：writeDeviceTypeAttribute
//描述：写开关类型的属性
//参数：type:0:翘板开关          1:轻触式开关(回弹式开关)
//返回：status: 状态
*/
EmberAfStatus writeDeviceTypeAttribute(uint8_t type)
{
	EmberAfStatus status;
	status =emberAfWriteAttribute(emberAfEndpointFromIndex(0),
			 ZCL_BASIC_CLUSTER_ID,
			 ZCL_GENERIC_DEVICE_TYPE_ATTRIBUTE_ID,
			 CLUSTER_MASK_SERVER,
			 &type,
			 ZCL_INT8U_ATTRIBUTE_TYPE);
	return status;

}

#ifdef USER_OTA_CLIENT_CALLBACK_CODE   //jim add 20191030
/**
//函数名：otaClientPluginPreImageDownloadCallback
//描述：ota下载数据包前回调，休眠设备可用此入口调整pollin间隔
//参数：无
//返回：void
*/
void otaClientPluginPreImageDownloadCallback(void)
{
  if (emberAfGetShortPollIntervalMsCallback() > START_OTA_SHORT_POLLING_INTERVAL_MS) //polling间隔大于100ms时，调整为100ms
  {
    if (emberAfNetworkState() == EMBER_JOINED_NETWORK)
    {
      emberAfSetWakeTimeoutMsCallback(0xFFFF);                                         //设定为最大值，处于一直polling
      emberAfSetShortPollIntervalMsCallback(START_OTA_SHORT_POLLING_INTERVAL_MS);      //设定polling间隔 100 ms
      emberAfAddToCurrentAppTasksCallback(EMBER_AF_FORCE_SHORT_POLL);                  //触发polling事件
    }
  }
}
/**
//函数名：otaClientPluginDownloadAbortedCallback
//描述：ota中断的回调函数，出错，或者失败时，可以通过此入口调把pollin间隔调回正常值
//参数：无
//返回：void
*/
void otaClientPluginDownloadAbortedCallback(void)
{
  if (emberAfGetShortPollIntervalMsCallback() != DEFAULT_NORMAL_SHORT_POLLING_INTERVAL_MS) //polling间隔不等于800ms时，调整为800ms
  {
    if (emberAfNetworkState() == EMBER_JOINED_NETWORK)
    {
      emberAfSetWakeTimeoutMsCallback(0xFFFF);                                               //设定为最大值，处于一直polling
      emberAfSetShortPollIntervalMsCallback(DEFAULT_NORMAL_SHORT_POLLING_INTERVAL_MS);       //设定polling间隔 100 ms
      emberAfAddToCurrentAppTasksCallback(EMBER_AF_FORCE_SHORT_POLL);                        //触发polling事件
    }
  }
}
/**
//函数名：otaClientPluginPreSendUpgradeEndRequestCallback
//描  述：ota结束前发送send upgrade end request前的回调函数
//参  数：无
//返  回：void
*/
void otaClientPluginPreSendUpgradeEndRequestCallback(void)
{
  if (emberAfGetShortPollIntervalMsCallback() != DEFAULT_NORMAL_SHORT_POLLING_INTERVAL_MS) //polling间隔不等于800ms时，调整为800ms
  {
    if (emberAfNetworkState() == EMBER_JOINED_NETWORK)
    {
      emberAfSetWakeTimeoutMsCallback(0xFFFF);                                               //设定为最大值，处于一直polling
      emberAfSetShortPollIntervalMsCallback(DEFAULT_NORMAL_SHORT_POLLING_INTERVAL_MS);       //设定polling间隔 100 ms
      emberAfAddToCurrentAppTasksCallback(EMBER_AF_FORCE_SHORT_POLL);                        //触发polling事件
    }
  }
}
#endif  //end jim
#ifdef USE_CUSTOM_PRE_IDLE_SLEEP_PROC_CODE  //jim add 20191206 for production test non sleep proc
/**
//函数名：readyToEnterIdleSleepCheckProcCallback
//描  述：设备进入休眠处理前的一个回调
//参  数：无
//返  回：bool (true 允许继续处理休眠代码；false 系统目前不允许休眠）
*/
bool readyToEnterIdleSleepCheckProcCallback(void)
{
#ifdef ORB_PRODUCTION_TEST_CODE  //产测模式判断处理
  if (inProductionTestMode()) //产测时，不允许休眠，防止串口给关闭的问题
  {
    return false;
  }
#endif
  return true;
}

/**
//函数名：reportDeviceType
//描  述：上报设备的属性，enddevice 或者route设备
//参  数：无
//返  回：void
*/
static void reportDeviceType(void)
{
	uint8_t temp;
	if(	emberAfReadAttribute(emberAfEndpointFromIndex(0),
								ZCL_BASIC_CLUSTER_ID,
								ZCL_PHYSICAL_ENVIRONMENT_ATTRIBUTE_ID,
								CLUSTER_MASK_SERVER,
								(uint8_t *)&temp,
								sizeof(temp),
								NULL) ==EMBER_ZCL_STATUS_SUCCESS)
	{
		unicastReportAttribute(0x0000,0x01,
							   emberAfEndpointFromIndex(0),
							   ZCL_BASIC_CLUSTER_ID,
							   ZCL_PHYSICAL_ENVIRONMENT_ATTRIBUTE_ID,
							   ZCL_ENUM8_ATTRIBUTE_TYPE,
							   (uint8_t *)&temp,
							   1);
		customAppDebugPrintln("network up endevice report");

	}
}

/**
//函数名：reportSwitchType
//描  述：上报开关的属性，翘板或者回弹
//参  数：无
//返  回：void
*/
static void reportSwitchType(void)
{
	uint8_t temp;
    temp =getFlashSwitchType();
	unicastReportAttribute(0x0000,0x01,
						   emberAfEndpointFromIndex(0),
						   ZCL_BASIC_CLUSTER_ID,
						   ZCL_GENERIC_DEVICE_TYPE_ATTRIBUTE_ID,
						   ZCL_ENUM8_ATTRIBUTE_TYPE,
						   (uint8_t *)&temp,
						   1);
	customAppDebugPrintln("poweron report qiaoban or huitan:%d",temp);	
	#if 0
	if(	emberAfReadAttribute(emberAfEndpointFromIndex(0),
								ZCL_BASIC_CLUSTER_ID,
								ZCL_GENERIC_DEVICE_TYPE_ATTRIBUTE_ID,
								CLUSTER_MASK_SERVER,
								(uint8_t *)&temp,
								sizeof(temp),
								NULL) ==EMBER_ZCL_STATUS_SUCCESS)
	{


	}
    #endif
}
#endif
//jim add 20200717
/**
//函数名：getPowerOnStartingFlag
//描  述：获取设备上电启动状态
//参  数：无
//返  回：bool, true:正在启动处理中; false:完成启动处理
*/
bool getPowerOnStartingFlag(void)
{
  return poweron_flg;
}

/**
//函数名：setButtonTrigType
//描  述：设定按键触发类型
//参  数：way [in] uint8_t 哪一路
//返  回：void
*/
void setButtonTrigType(uint8_t way)
{
    if (way < SWITCH_3WAYS)
    {
      trig_type[way]= BUTTON_TRIG;
    }
}
/**
//函数名：switchOffPreProc
//描  述：关灯前动作处理
//参  数：way [in] uint8_t 哪路开关动作
//返  回：bool, true:需要延时关继电器; false:不需要延时关继电器
*/
static bool switchOffPreProc(uint8_t way)
{
   if (way >= SWITCH_3WAYS)
   {
       return false;
   }
   if ((MAIN_POWER_RELAY_NUMBER == way) && (BUTTON_TRIG == trig_type[way])) //主供电继电器关 //对应按键触发的关
   {
     if (emberAfNetworkState() == EMBER_NO_NETWORK) //TODO: 无网络状态 （正在加网）按产品功能来处理
     {
		ledsAppChangeLedsStatus(LEDS_STATUS_STOP);
        //emberAfPluginNetworkSteeringStop();
        networkStatusTrigeNetworkAction(NETWORK_ACTION_LEAVE_AND_STOP);
        trig_type[way] = NONE_TRIG;
        return true; //关继电器需要延时
     }
   }
   trig_type[way] = NONE_TRIG;
   return false;
}

/**
//函数名：switchOnDoneProc
//描  述：开灯动作完成后处理
//参  数：way [in] uint8_t 哪路开关动作
//返  回：void
*/
static void switchOnDoneProc(uint8_t way)
{
   if (way >= SWITCH_3WAYS)
   {
       return;
   }
   if (RELAY_CONTROL_TURN_ON == relayControlDriverGetCurrenStatus(MAIN_POWER_RELAY_NUMBER)) //只有主供电的继电器打开，才能触发加网的动作。
   {
       if (BUTTON_TRIG == trig_type[way])
       {
          if (emberAfNetworkState() == EMBER_NO_NETWORK) //TODO: （没在加网）按产品功能来处理
          {
              //延时触发加网
              networkStatusTrigeNetworkAction(NETWORK_ACTION_DELAY_AND_START_JOIN);
          }
       }
   }
   trig_type[way] = NONE_TRIG;
}
//end jim
/*************************************** 文 件 结 束 ******************************************/
