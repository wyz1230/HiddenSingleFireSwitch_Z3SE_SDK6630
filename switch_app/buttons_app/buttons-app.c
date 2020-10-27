/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:buttons-app.c
*Author : JimYao
*Version : 1.0
*Date : 2019-10-11
*Description: 按键具体行为逻辑处理
*History:
***************************************************************************************************/
/* 相关包含头文件区 ------------------------------------------------------------ */
//#include "buttons-driver.h"
//#include "buttons-app.h"
//#include "switch-ways-config.h"
#include "common-app.h"
#include "app\framework\plugin\find-and-bind-target\find-and-bind-target.h"

/* 宏定义区 -------------------------------------------------------------------- */
//debug开关设定
//#define BUTTON_APP_DEBUG_ENABLE
#ifdef BUTTON_APP_DEBUG_ENABLE
  #define DEBUG_STRING                  "BtnApp-DB:"
  #define buttonsAppDebugPrint(...)     emberAfPrint(0xFFFF,DEBUG_STRING __VA_ARGS__)
  #define buttonsAppDebugPrintln(...)   emberAfPrintln(0xFFFF,DEBUG_STRING __VA_ARGS__)
#else
  #define buttonsAppDebugPrint(...)
  #define buttonsAppDebugPrintln(...)
#endif

//按键连击之间的最大有效时间，按产品使用习惯调整
#define BUTTON_VALID_INTERVAL_TIME_MS              (2000)

#define SWITCH_1WAY_COMBINED_BTN0_BTN2     //1路开关时，按键0 按键2 组合成一个按键，只要有一个按键按下都有作用

#define BUTTON_MAX_NUMBER    3      //按键硬件最大数

#define BUTTON_SPEECH_LIMIT  800       //按键速度限制，800ms间隔按才有效
/* 自定义类型区 ---------------------------------------------------------------- */
typedef struct{
   uint8_t  short_pressed_counter;       //短按连击次数
   uint16_t button_released_count_down_time_ms; //按键松开计时器
   uint16_t last_action_time_ms;                //最后一次有效动作时间，用来限制开关速度。20200718
   uint8_t  update_flag;                        //按键限速中有更新状态。20200718
   uint8_t  last_button_status;                 //按键最后状态
}tButtonPressedCount;

/* 全局变量区 ------------------------------------------------------------------ */
EmberEventControl buttonsAppTimerEventControl;   //两次按键之间的超时计时器
//void buttonsAppTimerEventHandler(void);
EmberEventControl app_RestartEventControl;
EmberEventControl delaySyncStatusControl;

/* 本地变量区 ------------------------------------------------------------------ */
static tButtonPressedCount buttons_counter[BUTTON_MAX_NUMBER];
static uint16_t next_delay_time_ms = 0xFFFF;
//static uint8_t buttons_number_list[BUTTON_MAX_NUMBER] = {0,1,2};   //用来记录前1,2,3路开关对应按键的对应位置.
//static uint8_t powerOnRockerStatus;	//上电时候翘板开关状态

//只允许网外切换开关类型，切换指示为60ms的间隔快闪三次
//踢网快闪切换仍然保持快闪
//加网过程中切换仍然保持慢闪
static uint8_t switch_type =SWITCH_TYPE_QIAOBAN; //默认开关类型为翘板型

/* 全局函数声明区 -------------------------------------------------------------- */
extern void unicastReportAttribute(uint16_t destination_addr,
                                   uint8_t dest_endpoint,
                                   uint8_t source_endpoint,
                                   EmberAfClusterId clusterId,
                                   EmberAfAttributeId attributeId,
                                   EmberAfAttributeType dataType,
                                   uint8_t *datas,
                                   uint8_t dataSize);
extern EmberAfStatus writeDeviceTypeAttribute(uint8_t type);
extern void setFlashSwitchType(uint8_t type);

/* 本地函数声明区 -------------------------------------------------------------- */
static void buttonsStateCallbackProcess(uint8_t num, eButtonState state);
static void buttonsPressedProcess(uint8_t num);
static void buttonsZeroShortLongPressedProcess(uint8_t num);
static void buttonsOneShortLongPressedProcess(uint8_t num);
static void buttonsTwoShortLongPressedProcess(uint8_t num);
static void buttonsLongPressedProcess(uint8_t num);
static void buttonsFourShortLongPressedProcess(uint8_t num);
static void buttonHighCallbackProcess(uint8_t num);
static void buttonLowCallbackProcess(uint8_t num);



/* 函数原型 -------------------------------------------------------------------- */
/**
//函数名：setSwitchType
//描述：设置开关类型
//参数：type (uint8_t [输入]，0:翘板型 1:点触型)
//返回：void
*/
void setSwitchType(uint8_t type)
{
    switch_type =type;
}

/**
//函数名：getSwitchType
//描述：设置开关类型
//参数：void
//返回：0:翘板型 1:点触型
*/
uint8_t getSwitchType(void)
{
	return switch_type;
}

/**
//函数名：buttonsReleasedTimerUpdate
//描述：更新按键松开延时时间
//参数：delay_time_ms (uint16_t [输入]，之前已经延时掉的时间)
//返回：void
*/
static void buttonsReleasedTimerUpdate(uint16_t delay_time_ms)
{
    if (0xFFFF != delay_time_ms)
    {
      for (uint8_t i=0; i<BUTTON_MAX_NUMBER; i++)
      {
        if (buttons_counter[i].short_pressed_counter > 0)
        {
         if (delay_time_ms > buttons_counter[i].button_released_count_down_time_ms)
         {
           buttons_counter[i].button_released_count_down_time_ms = 0;
         }
         else
         {
           buttons_counter[i].button_released_count_down_time_ms -= delay_time_ms;
         }
#if 0
         buttonsAppDebugPrintln("btn=%d,count=%d,%ld ms time out", i,
                                buttons_counter[i].short_pressed_counter,
                                buttons_counter[i].button_released_count_down_time_ms);
#endif
        }
      }
    }
}
/**
//函数名：buttonsAppInit
//描述：初始化按键运用代码相关的变量及注册按键驱动处理回调函数
//参数：无
//返回：无
*/
void buttonsAppInit(void)
{
  buttonsDriverButtonInit(buttonsStateCallbackProcess);
  for (uint8_t i=0; i<BUTTON_MAX_NUMBER; i++)
  {
    buttons_counter[i].button_released_count_down_time_ms = 0;
    buttons_counter[i].short_pressed_counter = 0;
  #ifdef BUTTON_SPEECH_LIMIT
    buttons_counter[i].last_action_time_ms = 0; //jim add 20200718
    buttons_counter[i].update_flag = 0;
  #endif
    buttons_counter[i].last_button_status = 0; //jim add 20200718
  }
  next_delay_time_ms = 0xFFFF;
//根据开关配置的路数，转换按键硬件位置到按键对应的变量位置。屏蔽掉无效的按键。
  #if 0
  switch (switchWaysConfigGetWays())
  {
    case SWITCH_3WAYS:
	  buttons_number_list[0] = 0;
      buttons_number_list[1] = 1;
      buttons_number_list[2] = 2;
      break;

    case SWITCH_2WAYS:
      buttons_number_list[0] = 0;
      buttons_number_list[1] = 2;
	  buttons_number_list[2] = 0xFF;
      break;

    default:
      buttons_number_list[0] = 1;
      buttons_number_list[1] = 0xFF;
	  buttons_number_list[2] = 0xFF;
      break;
  }

	  buttons_number_list[0] = 0;
      buttons_number_list[1] = 1;
      buttons_number_list[2] = 2;
  #endif
  buttonsAppDebugPrintln("Init");
}

/**
//函数名：buttonsStateCallbackProcess
//描述：按键驱动侦测到的按键状态变化产生的回调函数，在初始化时注册
//参数：num   (uint8_t      [输入]，按键号)
//      state (eButtonState [输入]，对应按键状态)
//返回：无
*/
static void buttonsStateCallbackProcess(uint8_t num, eButtonState state)
{
	#if 0
    uint8_t position, ways;
	ways = switchWaysConfigGetWays();
    //根据开关配置的路数，转换按键硬件位置到按键对应的变量位置。屏蔽掉无效的按键。
	for (position=0; position<ways; position++)
    {
	  if (buttons_number_list[position] < BUTTON_MAX_NUMBER)
      {
		if (buttons_number_list[position] == num)
	    {
	      break;
		}
	  }
	}
    buttonsAppDebugPrintln("Position=%d ways= %d", position, ways);

    if (position >= ways)
    {
	  return;
	}
	#endif
	uint8_t position;
	position= num;
    //以下按键长短按功能实现
    //buttonsAppDebugPrintln("Btn=%d status= %d", num, state);
    switch (state)
    {
      case BUTTON_RELEASE_SHORT_PRESS: //短按小于1秒松开
      case BUTTON_RELEASE_THAN_1S:     //长按1秒小于3秒松开
      case BUTTON_RELEASE_THAN_3S:     //长按大于3秒松开
        buttons_counter[position].last_button_status = 0; //jim add 20200718
        if (BUTTON_RELEASE_SHORT_PRESS == state)
        {
          buttons_counter[position].short_pressed_counter++; //先更新按键次数
          if (0xFFFF != next_delay_time_ms)
          {  //表示按键松开计时器还在运行中，需要更新旧的时间。算出已经运行过的时间。
             uint32_t temp_remain_time = emberEventControlGetRemainingMS(buttonsAppTimerEventControl);
             if (temp_remain_time > next_delay_time_ms)
             {
               next_delay_time_ms = 0;
             }
             else
             {
               next_delay_time_ms -= temp_remain_time; //算出已经运行过的时间
             }
          }
          else
          {
            next_delay_time_ms = 0;
          }
          emberEventControlSetInactive(buttonsAppTimerEventControl);
          buttonsReleasedTimerUpdate(next_delay_time_ms);
          next_delay_time_ms = 0;
          emberEventControlSetActive(buttonsAppTimerEventControl);

          buttons_counter[position].button_released_count_down_time_ms = BUTTON_VALID_INTERVAL_TIME_MS; //重新更新延时时间
        }
        else
        {
          buttons_counter[position].short_pressed_counter = 0;
          buttons_counter[position].button_released_count_down_time_ms = 0;
        }
        buttonLowCallbackProcess(num);
        break;

      case BUTTON_PRESSED_VALID:       //按键去抖后有效按下
        buttons_counter[position].last_button_status = 1; //jim add 20200718
        if (buttons_counter[position].short_pressed_counter)
        {
          //保证按住按键超时时间大于3秒，用来按住3秒的长按处理
          buttons_counter[position].button_released_count_down_time_ms = 0xFFFF;
        }
        buttonsPressedProcess(position); //按键按下处理
        break;

      case BUTTON_PRESSED_1S:          //按住达到1秒
        break;

      case BUTTON_PRESSED_3S:          //按住达到3秒
		if (0 == buttons_counter[position].short_pressed_counter) //0短1长条件满足
		{
			buttonsZeroShortLongPressedProcess(position);
		}
		#ifdef MODULE_CHANGE_NODETYPE
		else if(1 == buttons_counter[position].short_pressed_counter) //1短1长条件满足
		{
			//buttonsOneShortLongPressedProcess(position);
		}
		#endif
        else if (4 == buttons_counter[position].short_pressed_counter) //4短1长条件满足
        {
           buttonsFourShortLongPressedProcess(position); //按键4短1长按下处理
        }
        //buttons_counter[position].short_pressed_counter = 0;
        //buttons_counter[position].button_released_count_down_time_ms = 0xFFFF;
        break;

	  case BUTTON_PRESSED_5S:        //按住达到3秒
        if (2 == buttons_counter[position].short_pressed_counter) //2短1长5秒条件满足
        {
           buttonsTwoShortLongPressedProcess(position);  //按键2短1长按下5秒处理
        }

        if (4 != buttons_counter[position].short_pressed_counter) //处于4短1长的情况,不用处理。
        {
	      buttonsLongPressedProcess(position);
        }
	    //buttons_counter[position].short_pressed_counter = 0;
        //buttons_counter[position].button_released_count_down_time_ms = 0xFFFF;
		break;

      default: //未知，正常不会到此
        break;
    }
    //buttonsAppDebugPrintln("Btn short count=%d", buttons_counter[position].short_pressed_counter);
}

/**
//函数名：buttonsAppTimerEventHandler
//描述：按键连击之间的超时计时处理
//参数：无
//返回：void
*/
void buttonsAppTimerEventHandler(void)
{
    uint16_t mini_delay_time_ms;
	emberEventControlSetInactive(buttonsAppTimerEventControl);

    buttonsReleasedTimerUpdate(next_delay_time_ms);

    mini_delay_time_ms = 0xFFFF;
    for (uint8_t i=0; i<BUTTON_MAX_NUMBER; i++)
    {
      if (buttons_counter[i].short_pressed_counter > 0)
      {
        if (0 == buttons_counter[i].button_released_count_down_time_ms)
        {
           buttons_counter[i].short_pressed_counter = 0;
        }
        else
        {
           if (mini_delay_time_ms > buttons_counter[i].button_released_count_down_time_ms)
           {
             mini_delay_time_ms = buttons_counter[i].button_released_count_down_time_ms;
           }
        }
      }
    }

    next_delay_time_ms = mini_delay_time_ms;
    if (mini_delay_time_ms != 0xFFFF)
    {
      emberEventControlSetDelayMS(buttonsAppTimerEventControl,mini_delay_time_ms);
      buttonsAppDebugPrintln("next delay ms=%ld", mini_delay_time_ms);
    }
}

/**
//函数名：buttonsPressedProcess
//描述：按键防抖后按下的处理入口
//参数：num (uint8_t [输入]，按键号),
//返回：无
*/
static void buttonsPressedProcess(uint8_t num)
{
	uint8_t networkStatus =0;
	networkStatus =emberAfNetworkState();
#ifdef USE_ZCL_CLUSTER_SPECIFIC_COMMAND_PARSE_CALLBACK
  setOnOffClusterCommandType(num,BUTTON_CONTROL);  //切换为单播上报
#endif
  buttonsAppDebugPrintln("pressed:%d,%d",num,switch_type);

  if ((status == EMBER_NO_NETWORK) || (status == EMBER_LEAVING_NETWORK))   //按下触发加网
  {
    //if (num == 0)
    {
        //TODO:需要处理判断当前是否可以触发加网
       if (RELAY_CONTROL_TURN_ON == relayControlDriverGetCurrenStatus(MAIN_POWER_RELAY_NUMBER)) //jim 20200716 only one way on can search network
       {
	     networkStatusTrigeNetworkAction(NETWORK_ACTION_START_JOIN);
         buttonsAppDebugPrintln("start network");
       }
    }
  }
  if (num > 0)
  {
	  if(switch_type == SWITCH_TYPE_DIANCHU)
	  {
         //jim add 20200718
        #ifdef BUTTON_SPEECH_LIMIT
          if (buttons_counter[num].last_action_time_ms) //非零时，表示正锁定中
          {
             buttons_counter[num].update_flag = 1;
             emberEventControlSetDelayMS(delaySyncStatusControl,10); //延时更新锁定时间
             return;
          }
          else
          {
            buttons_counter[num].last_action_time_ms = halCommonGetInt16uMillisecondTick();
            buttons_counter[num].last_action_time_ms |= 1; //确保此值大于0
            buttons_counter[num].update_flag = 0;
            emberEventControlSetDelayMS(delaySyncStatusControl,10); //延时更新锁定时间
          }
        #endif
        //end jim
		  buttonsAppDebugPrintln("set dianchu onoff:%d,%d",emberAfEndpointFromIndex(num-1),emberAfEndpointIsEnabled(emberAfEndpointFromIndex(num-1)));
          setButtonTrigType(num-1);
		  emberAfOnOffClusterSetValueCallback(emberAfEndpointFromIndex(num-1),ZCL_TOGGLE_COMMAND_ID,false);
	  }
	  else
	  {
         //jim add 20200718
        #ifdef BUTTON_SPEECH_LIMIT
          if (buttons_counter[num].last_action_time_ms) //非零时，表示正锁定中
          {
             buttons_counter[num].update_flag = 1;
             emberEventControlSetDelayMS(delaySyncStatusControl,10); //延时更新锁定时间
             return;
          }
          else
          {
            buttons_counter[num].last_action_time_ms = halCommonGetInt16uMillisecondTick();
            buttons_counter[num].last_action_time_ms |= 1; //确保此值大于0
            buttons_counter[num].update_flag = 0;
            emberEventControlSetDelayMS(delaySyncStatusControl,10); //延时更新锁定时间
          }
        #endif
         //end jim
		  buttonHighCallbackProcess(num);
	  }
  }


  if (num+LEDS_STATUS_SWITCH_1_STATUS_UPDATE <= LEDS_STATUS_SWITCH_3_STATUS_UPDATE)
  {
	ledsAppChangeLedsStatus(num+LEDS_STATUS_SWITCH_1_STATUS_UPDATE); //更新指示灯的状态
  }

  //batteryVoltageFifoUpdate();
 //GPIO_Mode_TypeDef test= GPIO_PinModeGet(gpioPortA,5);
 //buttonsAppDebugPrintln("GPIO A5 typedef=%d", test);
 // trigerBatteryCapacityUpdate(1);  //触发一次电量更新，测试用
  checkNetworkStateAndTrigeRejoin();
}

/**
//函数名：buttonsZeroShortLongPressedProcess
//描述：按键0短1长按3秒不松手达到长按时间的处理
//参数：num (uint8_t [输入]，按键号),
//返回：无
*/
static void buttonsZeroShortLongPressedProcess(uint8_t num)
{
	EmberAfStatus status =EMBER_ZCL_STATUS_SUCCESS;
	uint8_t temp_switch_type =0;
	temp_switch_type =getSwitchType();

	if(num ==0)
	{
		if(emberAfNetworkState() !=EMBER_JOINED_NETWORK)
		{
			//只允许网外切换开关类型
			temp_switch_type = (temp_switch_type == SWITCH_TYPE_QIAOBAN? SWITCH_TYPE_DIANCHU : SWITCH_TYPE_QIAOBAN);
			setFlashSwitchType(temp_switch_type);
			setSwitchType(temp_switch_type);
			
			buttonsAppDebugPrintln("Change to current switch type:%d",temp_switch_type);
			status =writeDeviceTypeAttribute(temp_switch_type);
			if (status == EMBER_ZCL_STATUS_SUCCESS) {
				ledsAppChangeLedsStatus(LEDS_STATUS_CHANGE_SWITCHTYPE_UPDATA); //快闪三次
			}
		}
		if(emberAfNetworkState() ==EMBER_JOINED_NETWORK)
		{
			//在网络内强制发一个开关类型报告
	      unicastReportAttribute(0x0000,0x01,
	                             0x01,
	                             ZCL_BASIC_CLUSTER_ID,
	                             ZCL_GENERIC_DEVICE_TYPE_ATTRIBUTE_ID,
	                             ZCL_ENUM8_ATTRIBUTE_TYPE,
	                             (uint8_t *)&temp_switch_type,
	                             1);
		}
	}
}

/**
//函数名：buttonsTwoShortLongPressedProcess
//描述：按键1短1长按5秒不松手达到长按时间的处理
//参数：num (uint8_t [输入]，按键号),
//返回：无
*/
static void buttonsOneShortLongPressedProcess(uint8_t num)
{
	uint8_t type =0;
	type =getDeviceType();
	type = (type == EMBER_SLEEPY_END_DEVICE? EMBER_ROUTER : EMBER_SLEEPY_END_DEVICE);
	setFlashDeviceType(type);

	if(type ==EMBER_SLEEPY_END_DEVICE)
	{
		ledsAppChangeLedsStatus(LEDS_STATUS_SLEEP_END_DEVICE);
	}
	else
	{
		ledsAppChangeLedsStatus(LEDS_STATUS_ROUTE);
	}
	emberAfWriteServerAttribute(emberAfEndpointFromIndex(0),
	                    ZCL_BASIC_CLUSTER_ID,
	                    ZCL_PHYSICAL_ENVIRONMENT_ATTRIBUTE_ID,
	                    (uint8_t *)&type,
	                    ZCL_INT8U_ATTRIBUTE_TYPE);
	emberEventControlSetDelayMS(app_RestartEventControl,1000);
}

/**
//函数名：buttonsTwoShortLongPressedProcess
//描述：按键2短1长按5秒不松手达到长按时间的处理
//参数：num (uint8_t [输入]，按键号),
//返回：无
*/
static void buttonsTwoShortLongPressedProcess(uint8_t num)
{
//  if (emberAfNetworkState() == EMBER_JOINED_NETWORK)
//  {
//   emberAfPermitJoin(180, true); //Send out a broadcast pjoin
//  }
  //增加find&bind
  if (emberAfNetworkState() == EMBER_JOINED_NETWORK)
  {
    uint8_t endpoint, ways;
    ways = switchWaysConfigGetWays();
    if (num < ways)
    {
      endpoint = emberAfEndpointFromIndex(num);
      emberAfPluginFindAndBindTargetStart(endpoint);
    }
  }
}

/**
//函数名：buttonsFourShortLongPressedProcess
//描述：按键4短1长按不松手达到长按时间的处理
//参数：num (uint8_t [输入]，按键号),
//返回：无
*/
static void buttonsFourShortLongPressedProcess(uint8_t num)
{
  buttonsAppDebugPrintln("Reset");
  networkStatusTrigeNetworkAction(NETWORK_ACTION_LEAVE);
  ledsAppChangeLedsStatus(LEDS_STATUS_NETWORK_LEAVED);
}

/**
//函数名：buttonsLongPressedProcess
//描述：按键长按不松手达到5秒的处理
//参数：num (uint8_t [输入]，按键号),
//返回：无
*/
static void buttonsLongPressedProcess(uint8_t num)
{
#if 0
  if (emberAfNetworkState() == EMBER_NO_NETWORK)
  {
    networkStatusTrigeNetworkAction(NETWORK_ACTION_LEAVE_AND_JOIN);
  }
#endif
}

/**
//函数名：buttonHighCallbackProcess
//描述：按键为高电平的处理(按下)
//参数：num (uint8_t [输入]，按键号),
//返回：无
*/
static void buttonHighCallbackProcess(uint8_t num)
{
	if(getPowerOnStartingFlag())
	{
		return;
	}
  if(num ==0 || switch_type == SWITCH_TYPE_DIANCHU) return;

	buttonsAppDebugPrintln("=====high:%d,%d",num,switch_type);

    setButtonTrigType(num-1);
	emberAfOnOffClusterSetValueCallback(emberAfEndpointFromIndex(num-1),ZCL_ON_COMMAND_ID,false);

}

/**
//函数名：buttonLowCallbackProcess
//描述：按键为低电平的处理(松开)
//参数：num (uint8_t [输入]，按键号),
//返回：无
*/
static void buttonLowCallbackProcess(uint8_t num)
{
  	if(getPowerOnStartingFlag())
	{
		return;
	}
	if(num ==0 || switch_type == SWITCH_TYPE_DIANCHU) return;
     //jim add 20200718
    #ifdef BUTTON_SPEECH_LIMIT
      if (buttons_counter[num].last_action_time_ms) //非零时，表示正锁定中
      {
         buttons_counter[num].update_flag = 1;
         emberEventControlSetDelayMS(delaySyncStatusControl,10); //延时更新锁定时间
         return;
      }
      else
      {
        buttons_counter[num].last_action_time_ms = halCommonGetInt16uMillisecondTick();
        buttons_counter[num].last_action_time_ms |= 1; //确保此值大于0
        buttons_counter[num].update_flag = 0;
        emberEventControlSetDelayMS(delaySyncStatusControl,10); //延时更新锁定时间
      }
    #endif
	buttonsAppDebugPrintln("====low:%d,%d",num,switch_type);

    setButtonTrigType(num-1);
	emberAfOnOffClusterSetValueCallback(emberAfEndpointFromIndex(num-1),ZCL_OFF_COMMAND_ID,false);
    checkNetworkStateAndTrigeRejoin(); //jim add 20200717
}

/**
//函数名：app_RestartEventHandler
//描述：重启的handler
//参数：void
//返回：无
*/
void app_RestartEventHandler(void)
{
	emberEventControlSetInactive(app_RestartEventControl);
	emberLeaveNetwork();
	halReboot();
}

/**
//函数名：delaySyncStatusHandler
//描述：延时更新同步开关状态的handler
//参数：void
//返回：无
*/
void delaySyncStatusHandler(void)
{
 #ifdef BUTTON_SPEECH_LIMIT
    uint16_t currentTick,remain,minRemainMs=0xFFFF;
    emberEventControlSetInactive(delaySyncStatusControl);  
    currentTick = halCommonGetInt16uMillisecondTick();
    buttons_counter[0].last_action_time_ms = 0; //按键0没做限制
    for (uint8_t i=0; i<2;i++)
    {
      if (buttons_counter[i+1].last_action_time_ms) //非零，需要更新按键状态
      {
        remain = currentTick - buttons_counter[i+1].last_action_time_ms;
        if (remain >= BUTTON_SPEECH_LIMIT) //800ms限制
        {
          if((switch_type == SWITCH_TYPE_QIAOBAN) && (buttons_counter[i+1].update_flag))  //桥板开关需要更新最后按键状态与开关状态
          {
             buttons_counter[i+1].update_flag = 0; 
             setButtonTrigType(i);
             checkNetworkStateAndTrigeRejoin(); //jim add 20200717
             if(0 == buttons_counter[i+1].last_button_status)
             {
                emberAfOnOffClusterSetValueCallback(emberAfEndpointFromIndex(i),ZCL_OFF_COMMAND_ID,false);
             }
             else
             {
                emberAfOnOffClusterSetValueCallback(emberAfEndpointFromIndex(i),ZCL_ON_COMMAND_ID,false);
                if (i+LEDS_STATUS_SWITCH_1_STATUS_UPDATE+1 <= LEDS_STATUS_SWITCH_3_STATUS_UPDATE)
                {
	              ledsAppChangeLedsStatus(i+LEDS_STATUS_SWITCH_1_STATUS_UPDATE+1); //更新指示灯的状态
                }
             }
          }
          buttons_counter[i+1].last_action_time_ms = 0; //限制解除
          remain = 0;
        }
        else
        {
          remain = BUTTON_SPEECH_LIMIT - remain;
          if (minRemainMs >= remain)
          {
            minRemainMs = remain;
          }
        }
      }
    }
  if (minRemainMs != 0xFFFF)
  {
    emberEventControlSetDelayMS(delaySyncStatusControl,minRemainMs); //延时更新锁定时间
    buttonsAppDebugPrintln("button lock remain:%d",minRemainMs);
  }
 #endif
}

/**
//函数名：syncButtonAndSwitchStatus
//描述：同步按键与开关状态
//参数：void
//返回：无
*/
void syncButtonAndSwitchStatus(void)
{
	uint8_t networkStatus =0;
	networkStatus =emberAfNetworkState();

    if(switch_type == SWITCH_TYPE_QIAOBAN)
    {
      for (uint8_t i=0; i<2;i++)
      {
        if(0 == buttons_counter[i+1].last_button_status)
        {
           emberAfOnOffClusterSetValueCallback(emberAfEndpointFromIndex(i),ZCL_OFF_COMMAND_ID,false);
        }
        else
        {
           emberAfOnOffClusterSetValueCallback(emberAfEndpointFromIndex(i),ZCL_ON_COMMAND_ID,false);
        }
      }
    }
  if (networkStatus == EMBER_NO_NETWORK)   //按下触发加网
  {
    if (buttons_counter[1].last_button_status)  //第一路上电5s后是打开的
    {
       if (RELAY_CONTROL_TURN_ON == relayControlDriverGetCurrenStatus(MAIN_POWER_RELAY_NUMBER)) //jim 20200716 only one way on can search network
       {
	     networkStatusTrigeNetworkAction(NETWORK_ACTION_START_JOIN);
         buttonsAppDebugPrintln("power on start network");
       }
    }
  }	
}
/*************************************** 文 件 结 束 ******************************************/
