/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:leds-app.c
*Author : JimYao
*Version : 1.0
*Date : 2019-09-11
*Description: LED指示灯运用代码逻辑部分处理
*History:
***************************************************************************************************/

/* 相关包含头文件区 --------------------------------------------------------- */
//#include "leds-app.h"
//#include "leds-driver.h"
//#include "switch-ways-config.h"
#include "common-app.h"

/* 宏定义区 ----------------------------------------------------------------- */
//debug开关设定
//#define LED_APP_DEBUG_ENABLE
#ifdef LED_APP_DEBUG_ENABLE
  #define DEBUG_STRING               "LedApp-DB:"
  #define ledsAppDebugPrint(...)     emberAfPrint(0xFFFF,DEBUG_STRING __VA_ARGS__)
  #define ledsAppDebugPrintln(...)   emberAfPrintln(0xFFFF,DEBUG_STRING __VA_ARGS__)
#else
  #define ledsAppDebugPrint(...)
  #define ledsAppDebugPrintln(...)
#endif

#define LEDS_LIST_NUMBER          1

#define set_bit(byte, bit)          (byte |= 1 << bit)
#define reset_bit(byte, bit)        (byte &= ~(1 << bit))
#define get_bit(byte, bit)          ((byte & (1 << bit)) >> bit)

/* 自定义类型区 ------------------------------------------------------------- */

/* 全局变量区 --------------------------------------------------------------- */

/* 本地变量区 --------------------------------------------------------------- */
static uint8_t leds_current_status = LEDS_STATUS_IDLE;
static uint8_t leds_number_list[LEDS_LIST_NUMBER] = {0};
static uint8_t leds_current_status_lock = 0;                    //锁定当前灯的状态，锁定后灯的状态无法切换，等待超时解锁
static uint8_t leds_current_identifying_flag = 0;               //led灯当前是否在执行indentify，每一个bit对应一路。

/* 全局函数声明区 ----------------------------------------------------------- */
EmberEventControl ledsAppTimerEventControl;   //指示灯状态显示延时计时器
//void ledsAppTimerEventHandler(void);

/* 本地函数声明区 ----------------------------------------------------------- */

/* 函数原型 ----------------------------------------------------------------- */

/**
//函数名：ledsAppInit
//描述：初始化led的状态变量
//参数：无
//返回：void
*/
void ledsAppInit(void)
{
    ledsDriverLedTaskInit();
  //根据开关配置的路数，转换LED灯硬件位置到LED对应的变量位置。屏蔽掉无效的LED。
    #if 0
    switch (switchWaysConfigGetWays())
    {
      case SWITCH_3WAYS:
        leds_number_list[0] = 0;
        leds_number_list[1] = 1;
        leds_number_list[2] = 2;
        break;

      case SWITCH_2WAYS:
        leds_number_list[0] = 0;
        leds_number_list[1] = 2;
        leds_number_list[2] = 0xFF;
        break;

      default:
        leds_number_list[0] = 1;
        leds_number_list[1] = 0xFF;
        leds_number_list[2] = 0xFF;
        break;
    }
	#endif
    //TODO: 需要关掉不用灯号，设定为disable模式，具体按硬件实际情况处理。
    leds_current_status_lock = 0;
    leds_current_identifying_flag = 0;
    ledsAppChangeLedsStatus(LEDS_STATUS_POWER_ON_INIT);
    ledsAppDebugPrintln("init");
}

/**
//函数名：ledsAppChangeLedsStatus
//描述：切换当前系统指示灯
//参数：leds_status (uint8_t [输入]，指示灯状态)
//返回：void
*/
void ledsAppChangeLedsStatus(uint8_t leds_status)
{
   uint8_t i, ways, new_identifying_flag; //jim uint8_t i, led_way, ways, new_identifying_flag;
#ifdef ORB_PRODUCTION_TEST_CODE  //产测模式判断处理
  if (inProductionTestMode()) //产测时，关掉所有灯，灯的动作不处理
  {
    if (leds_status == LEDS_STATUS_POWER_ON_INIT) //上电初始化一次后，关灯
    {
	  ledsDriverLedsControl(leds_number_list[0],0,500,500, 1);

    }
    return;
  }
#endif
  ways = switchWaysConfigGetWays();

  if (leds_current_status_lock)
  {
     if (LEDS_STATUS_POWER_ON_INIT == leds_current_status || LEDS_STATUS_NETWORK_LEAVED == leds_current_status)
     {
      //状态为开机状态显示时，会锁定2秒，用来显示开机状态。
      return;
     }
     else if ((leds_status < LEDS_STATUS_SWITCH_1_STATUS_UPDATE) ||
              (leds_status > LEDS_STATUS_SWITCH_3_STATUS_UPDATE))
     {
      //状态更新，锁定500ms显示效果
      return;
     }
  }
  switch (leds_status)
  {
    case LEDS_STATUS_POWER_ON_INIT:         //上电初始化状态，灯亮2秒后灭掉
#define POWER_ON_LED_SHOW_TIME_MS   500    //上电指示灯显示时长
#ifdef ORB_PRODUCTION_TEST_CODE  //产测模式判断处理
		if (authorizationPass() == true)
		{
		  ledsDriverLedsControl(leds_number_list[0], 0,POWER_ON_LED_SHOW_TIME_MS,POWER_ON_LED_SHOW_TIME_MS, 1);  //灯亮2秒后灭掉
		}
		else  //没授权的时候上电显示做区别
		{
		  ledsDriverLedsControl(leds_number_list[0], 0,100,200, 8);  //没授权上电快闪
		}
#else
		ledsDriverLedsControl(leds_number_list[0], 0,POWER_ON_LED_SHOW_TIME_MS,POWER_ON_LED_SHOW_TIME_MS, 1);  //灯亮2秒后灭掉
#endif

      leds_current_status_lock = 1;
	  emberEventControlSetDelayMS(ledsAppTimerEventControl,POWER_ON_LED_SHOW_TIME_MS);      //开机亮2秒,锁定2秒后解除.
      break;

    case LEDS_STATUS_NETWORK_LEAVED:        //离网状态

	  ledsDriverLedsControl(leds_number_list[0], 0,100,200, LED_REPEAT_TYPE_ALWAYS_REPEAT); //离网状态快闪，100ms亮，100ms灭。
      //leds_current_status_lock = 1;
      leds_current_identifying_flag = 0;
	  //emberEventControlSetDelayMS(ledsAppTimerEventControl,3000);      //锁定3秒后解除.
	  ledsAppDebugPrintln("leds indicate leaved");
      break;

    case LEDS_STATUS_NETWORK_JOINING:       //加网状态
      if (leds_current_status != LEDS_STATUS_NETWORK_JOINING)      //不重复触发
      {
		//加网慢闪,500ms亮500ms灭周期
		ledsDriverLedsControl(leds_number_list[0], 0,500,1000, LED_REPEAT_TYPE_ALWAYS_REPEAT);
		ledsAppDebugPrintln("LEDS_STATUS_NETWORK_JOINING");
      }

      leds_current_identifying_flag = 0;
      break;

    case LEDS_STATUS_NETWORK_JOINED:        //在网状态
	  //所有灯灭

	  ledsDriverLedsControl(leds_number_list[0],0,LED_FREQUENCY_TYPE_ALWAYS_OFF,
                                  LED_FREQUENCY_TYPE_ALWAYS_OFF, LED_REPEAT_TYPE_STOP_REPEAT);
      leds_current_identifying_flag = 0;
	  break;

    case LEDS_STATUS_SWITCH_1_STATUS_UPDATE:  //更新开关状态
	case LEDS_STATUS_SWITCH_2_STATUS_UPDATE:
	case LEDS_STATUS_SWITCH_3_STATUS_UPDATE:
		//开关状态变化时，对应的指示灯快闪一下，300ms
		ledsDriverLedsControl(leds_number_list[0], 0,200,500, 1);//闪一次，300ms
		leds_current_status_lock = 1;
		emberEventControlSetDelayMS(ledsAppTimerEventControl,1000); 		//闪一次

      break;

    case LEDS_STATUS_IDENTIFY_UPDATE:

      new_identifying_flag = getIdentifyingFlg();
      for (i=0; i<ways; i++)
      {
		if (leds_number_list[i] < LEDS_LIST_NUMBER)
	    {
          if (get_bit(leds_current_identifying_flag,i) != get_bit(new_identifying_flag,i))  //不重复触发
          {
            if (get_bit(new_identifying_flag,i)) //开启identify
            {
               ledsDriverLedsControl(leds_number_list[0], 0,1000,2000, LED_REPEAT_TYPE_ALWAYS_REPEAT);
               set_bit(leds_current_identifying_flag,i);
            }
            else  //停止identify
            {
			   ledsDriverLedsControl(leds_number_list[0],0,LED_FREQUENCY_TYPE_ALWAYS_OFF,
									   LED_FREQUENCY_TYPE_ALWAYS_OFF, LED_REPEAT_TYPE_STOP_REPEAT);

               reset_bit(leds_current_identifying_flag,i);
            }
          }
		}
	  }

	  if(getIdentifySetEndpoint() ==4) //智能开关的查找，三个led同时闪烁
	  {
		  if(getIdentifyStatus())
	   	  {			
			 ledsDriverLedsControl(leds_number_list[0], 0,1000,2000, LED_REPEAT_TYPE_ALWAYS_REPEAT);			
		  }
		  else //停止identify
		  {
			  ledsDriverLedsControl(leds_number_list[0],0,LED_FREQUENCY_TYPE_ALWAYS_OFF,
									  LED_FREQUENCY_TYPE_ALWAYS_OFF, LED_REPEAT_TYPE_STOP_REPEAT);

		      leds_current_identifying_flag = 0;			 
		  }
	  }	  
      break;

	  case LEDS_STATUS_CHANGE_SWITCHTYPE_UPDATA:
	  //开关状态变化时，对应的指示灯快闪一下，300ms
	  ledsDriverLedsControl(leds_number_list[0], 0,60,120, 3);//闪一次，300ms
	  leds_current_status_lock = 1;
	  emberEventControlSetDelayMS(ledsAppTimerEventControl,1000);		  //闪一次
	  break;

	case LEDS_STATUS_SLEEP_END_DEVICE:
	ledsDriverLedsControl(leds_number_list[0], 0,60,120, LED_REPEAT_TYPE_THREE);
	leds_current_identifying_flag = 0;
	break;

	case LEDS_STATUS_ROUTE:
	ledsDriverLedsControl(leds_number_list[0], 0,60,120, LED_REPEAT_TYPE_EIGHT);
	leds_current_identifying_flag = 0;
	break;

	case LEDS_STATUS_STOP:
    ledsDriverLedsControl(leds_number_list[0],0,LED_FREQUENCY_TYPE_ALWAYS_OFF,
                              LED_FREQUENCY_TYPE_ALWAYS_OFF, LED_REPEAT_TYPE_STOP_REPEAT);
	leds_current_identifying_flag = 0;
	break;

    default:  //LEDS_STATUS_IDLE 状态或者其它
	  //所有灯灭
      ledsDriverLedsControl(leds_number_list[0],0,LED_FREQUENCY_TYPE_ALWAYS_OFF,
                              LED_FREQUENCY_TYPE_ALWAYS_OFF, LED_REPEAT_TYPE_STOP_REPEAT);
      break;
  }
  leds_current_status = leds_status; //更新当前状态
  ledsAppDebugPrintln("status=%d",leds_current_status);
}

/**
//函数名：ledsAppTimerEventHandler
//描述：led灯的状态显示延时处理
//参数：无
//返回：void
*/
void ledsAppTimerEventHandler(void)
{
  emberEventControlSetInactive(ledsAppTimerEventControl);
  leds_current_status_lock = 0;
  leds_current_status = LEDS_STATUS_IDLE; //设定为LEDS_STATUS_IDLE, 解除显示锁定。
  networkStatusUpdateShow();              //更新网络状态显示

  ledsAppDebugPrintln("led show timeout");
}

/***************************************** 文 件 结 束 ************************************************/
