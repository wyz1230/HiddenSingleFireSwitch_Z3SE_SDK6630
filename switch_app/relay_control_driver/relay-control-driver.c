/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:relay-control-driver.c
*Author : JimYao
*Version : 1.1
*Date : 2019-10-08
*Description: 继电器与可控硅开关控制驱动,此部分是驱动继电器控制IC，MS3111S而修改。
*History:
***************************************************************************************************/

/* 相关包含头文件区 --------------------------------------------------------- */
//#include "relay-control-driver.h"
#include "common-app.h"

/* 宏定义区 ----------------------------------------------------------------- */
//debug开关设定
//#define RELAY_CONTROL_DRIVER_DEBUG_ENABLE
#ifdef RELAY_CONTROL_DRIVER_DEBUG_ENABLE
  #define DEBUG_STRING                          "RelayDriver-DB:"
  #define relayControlDriverDebugPrint(...)     emberAfPrint(0xFFFF,DEBUG_STRING __VA_ARGS__)
  #define relayControlDriverDebugPrintln(...)   emberAfPrintln(0xFFFF,DEBUG_STRING __VA_ARGS__)
#else
  #define relayControlDriverDebugPrint(...)
  #define relayControlDriverDebugPrintln(...)
#endif


#define MAX_UINT32     0xFFFFFFFF

//定义继电器有效电平
#define CONTROL_HIGH_ACTIVE	//高电平闭合
#ifdef CONTROL_HIGH_ACTIVE
  #define TURN_ON    1
  #define TURN_OFF   0
#else
  #define TURN_ON    0
  #define TURN_OFF   1
#endif

//定义继电器的控制脚IO口
#define RELAY_MAX_NUMBER         (2U) //jim (3U)  //继电器的控制口数量


//以下是快速定义，对应继电器号的控制，需要修改relayCtlDriverRelayOnOff()函数，
//relayCtlDriverRelayOnOff()函数和relay-control-driver.h头文件中的继电器号枚举。

#define RELAY0_0_PORT            (gpioPortC)
#define RELAY0_0_PIN             (2U)

#define RELAY1_0_PORT            (gpioPortD)
#define RELAY1_0_PIN             (2U)

//#define RELAY2_0_PORT            (gpioPortC)
//#define RELAY2_0_PIN             (4U)

#define RELAY0_1_PORT            (gpioPortC)
#define RELAY0_1_PIN             (0U)

#define RELAY1_1_PORT            (gpioPortD)
#define RELAY1_1_PIN             (0U)

//#define RELAY2_1_PORT            (gpioPortC)
//#define RELAY2_1_PIN             (5U)


//开关高低电平控制定义
#define SET_RELAY0_0_ONOFF(x)       do { (x)==0 ? GPIO_PinOutClear(RELAY0_0_PORT, RELAY0_0_PIN) : \
                                                GPIO_PinOutSet(RELAY0_0_PORT, RELAY0_0_PIN); }while(0)
#define SET_RELAY1_0_ONOFF(x)       do { (x)==0 ? GPIO_PinOutClear(RELAY1_0_PORT, RELAY1_0_PIN) : \
                                                GPIO_PinOutSet(RELAY1_0_PORT, RELAY1_0_PIN); }while(0)
//#define SET_RELAY2_0_ONOFF(x)       do { (x)==0 ? GPIO_PinOutClear(RELAY2_0_PORT, RELAY2_0_PIN) : \
//                                                GPIO_PinOutSet(RELAY2_0_PORT, RELAY2_0_PIN); }while(0)

#define SET_RELAY0_1_ONOFF(x)       do { (x)==0 ? GPIO_PinOutClear(RELAY0_1_PORT, RELAY0_1_PIN) : \
                                                GPIO_PinOutSet(RELAY0_1_PORT, RELAY0_1_PIN); }while(0)
#define SET_RELAY1_1_ONOFF(x)       do { (x)==0 ? GPIO_PinOutClear(RELAY1_1_PORT, RELAY1_1_PIN) : \
                                                GPIO_PinOutSet(RELAY1_1_PORT, RELAY1_1_PIN); }while(0)
//#define SET_RELAY2_1_ONOFF(x)       do { (x)==0 ? GPIO_PinOutClear(RELAY2_1_PORT, RELAY2_1_PIN) : \
//                                            GPIO_PinOutSet(RELAY2_1_PORT, RELAY2_1_PIN); }while(0)

//控制脚初始化配置
#define CONTROL_PINTS_INIT()      do { GPIO_PinModeSet(RELAY0_0_PORT,RELAY0_0_PIN,gpioModePushPull,TURN_OFF); \
                                       GPIO_PinModeSet(RELAY1_0_PORT,RELAY1_0_PIN,gpioModePushPull,TURN_OFF); \
                                       /*GPIO_PinModeSet(RELAY2_0_PORT,RELAY2_0_PIN,gpioModePushPull,TURN_OFF);*/ \
                                       GPIO_PinModeSet(RELAY0_1_PORT,RELAY0_1_PIN,gpioModePushPull,TURN_OFF); \
                                       GPIO_PinModeSet(RELAY1_1_PORT,RELAY1_1_PIN,gpioModePushPull,TURN_OFF); \
                                       /*GPIO_PinModeSet(RELAY2_1_PORT,RELAY2_1_PIN,gpioModePushPull,TURN_OFF);*/ }while(0)

#define set_bit(byte, bit)          (byte |= 1 << bit)
#define reset_bit(byte, bit)        (byte &= ~(1 << bit))
#define get_bit(byte, bit)          ((byte & (1 << bit)) >> bit)

#define CONTROL_ON_DELAY_TO_SLEEP_TIME_MS       15    //开动作，等待继电器完成动作后，进入休眠状态。
#define CONTROL_OFF_DELAY_TO_SLEEP_TIME_MS      15    //关动作，等待继电器完成动作后，进入休眠状态。


/* 自定义类型区 ------------------------------------------------------------- */
enum {
  IDLE_OFF = 0x00,   //继电器驱动IC-MS3111S 处于休眠状态，最后一次的动作是关，休眠态时驱动IC的两个输入口都是低电平。
  IDLE_ON,           //继电器驱动IC-MS3111S 处于休眠状态，最后一次的动作是开，休眠态时驱动IC的两个输入口都是低电平。
  OFF_TO_ON_START,   //从关到开，驱动IC的两个输入口，0脚为低电平，1脚为高电平，需要持续30ms。
  WAIT_TO_ON_END,    //等待30ms完成开的动作，设定进入休眠状态。
  ON_TO_OFF_START,   //从开到关，驱动IC的两个输入口，1脚为低电平，0脚为高电平，需要持续30ms。
  WAIT_TO_OFF_END,   //等待30ms完成关的动作，设定进入休眠状态。
};
// 定义继电器开关状态
typedef struct {
  uint8_t  status;            //控制状态
  uint32_t remain_time_ms;    //下一状态执行剩下时间(ms) //0xFFFFFFFF时，表示一直处于当前状态，不做倒计时。
} tContorlStatus;

/* 全局变量区 --------------------------------------------------------------- */
EmberEventControl relayControlTaskEventControl;  //需要定义继电器控制事件和继电器控制事件处理入口函数
//void relayControlTaskEventHandler(void);

/* 本地变量区 --------------------------------------------------------------- */
static fpRelayControlCallback relayControlCallback = NULL;
static tContorlStatus relays_control_status[RELAY_MAX_NUMBER];
static uint32_t relay_control_task_remain_time = MAX_UINT32;   //控制任务下一次执行剩余时间。0xFFFFFFF为关闭


/* 全局函数声明区 ----------------------------------------------------------- */

/* 本地函数声明区 ----------------------------------------------------------- */
static void relayControlDriverControlOnOff(uint8_t pin_num,uint8_t on_off);

/* 函数原型 ----------------------------------------------------------------- */
/**
//函数名：relayControlDriverControlPinsInit
//描述：初始化各控制脚的IO口
//参数：无
//返回：void
*/
static void relayControlDriverControlPinsInit(void)
{
  /* Enable GPIO in CMU */
#if !defined(_SILICON_LABS_32B_SERIES_2)
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
#endif

  CONTROL_PINTS_INIT();
  relayControlDriverControlOnOff(CONTROL_ALL,TURN_OFF);
}

/**
//函数名：relayControlDriverControlOnOff
//描述：控制对应控制脚
//参数：pin_num (uint8_t[输入],要控制的对应引脚号;)
//      on_off  (uint8_t[输入],对应引脚开关;)
//返回：void
*/
static void relayControlDriverControlOnOff(uint8_t pin_num,uint8_t on_off)
{
  	relayControlDriverDebugPrintln("relayControlDriverControlOnOff num:%d,val %d",pin_num,on_off);
  if (CONTROL_RELAY_0_0 == pin_num)
  {
    SET_RELAY0_0_ONOFF(on_off);
  }
  else if (CONTROL_RELAY_1_0 == pin_num)
  {
    SET_RELAY1_0_ONOFF(on_off);
  }
/*  else if (CONTROL_RELAY_2_0 == pin_num)
  {
    SET_RELAY2_0_ONOFF(on_off);
  }*/
  else if (CONTROL_RELAY_0_1 == pin_num)
  {
    SET_RELAY0_1_ONOFF(on_off);
  }
  else if (CONTROL_RELAY_1_1 == pin_num)
  {
    SET_RELAY1_1_ONOFF(on_off);
  }
/*  else if (CONTROL_RELAY_2_1 == pin_num)
  {
    SET_RELAY2_1_ONOFF(on_off);
  }*/
  else if (CONTROL_ALL_RELAYS_0 == pin_num)
  {
    SET_RELAY0_0_ONOFF(on_off);
    SET_RELAY1_0_ONOFF(on_off);
   // SET_RELAY2_0_ONOFF(on_off);
  }
  else if (CONTROL_ALL_RELAYS_1 == pin_num)
  {
    SET_RELAY0_1_ONOFF(on_off);
    SET_RELAY1_1_ONOFF(on_off);
   // SET_RELAY2_1_ONOFF(on_off);
  }
  else if (CONTROL_ALL == pin_num)
  {
    SET_RELAY0_0_ONOFF(on_off);
    SET_RELAY1_0_ONOFF(on_off);
   // SET_RELAY2_0_ONOFF(on_off);
    SET_RELAY0_1_ONOFF(on_off);
    SET_RELAY1_1_ONOFF(on_off);
   // SET_RELAY2_1_ONOFF(on_off);
  }
}

/**
//函数名：relayContorlDriverTaskInit
//描述：初始化控制脚的IO与控制任务事件处理状态变量，默认过渡功能是开的
//参数：control_cb (fpRelayControlCallback [输入]，控制状态回调注册)
//返回：void
*/
void relayContorlDriverTaskInit(fpRelayControlCallback control_cb)
{
  relayControlDriverControlPinsInit();
  for (uint8_t i=0; i<RELAY_MAX_NUMBER; i++)
  {
    relays_control_status[i].status = IDLE_OFF;
    relays_control_status[i].remain_time_ms = MAX_UINT32;
  }
  relayControlCallback = control_cb;
  relay_control_task_remain_time = 0;
  emberEventControlSetActive(relayControlTaskEventControl); //开启控制状态处理任务扫描

  relayControlDriverDebugPrintln("init");
}

/**
//函数名：getRelayControlMinRemainTime
//描述：找出控制状态里最小的剩余时间
//参数：无
//返回：uint32_t, 最小的剩余时间，当为0xFFFFFFFF时，没有剩余时间，不需要倒计。
*/
static uint32_t getRelayControlMinRemainTime(void)
{
  uint32_t min_remain = MAX_UINT32;
   for (uint8_t i=0; i<RELAY_MAX_NUMBER; i++)
   {
     if (relays_control_status[i].remain_time_ms < min_remain)
     {
       min_remain = relays_control_status[i].remain_time_ms;
     }
   }
   return min_remain;
}

/**
//函数名：updateRelayControlStatusRemainTime
//描述：更新控制状态有效的剩余时间
//参数：last_delay_time (uint32_t[输入],上一个任务延时的时间;)
//返回：无
*/
static void updateRelayControlStatusRemainTime(uint32_t last_delay_time)
{
   for (uint8_t i=0; i<RELAY_MAX_NUMBER; i++)
   {
     if (relays_control_status[i].remain_time_ms < MAX_UINT32)
     {
       if (relays_control_status[i].remain_time_ms < last_delay_time)
       {
         relays_control_status[i].remain_time_ms = 0;
       }
       else
       {
         relays_control_status[i].remain_time_ms -= last_delay_time;
       }
     }
   }
}

/**
//函数名：relayControlTaskEventHandler
//描述：控制状态任务处理部分
//参数：无
//返回：void
*/
void relayControlTaskEventHandler(void)
{
  uint8_t i;
  emberEventControlSetInactive(relayControlTaskEventControl);
  //更新任务剩余时间
  updateRelayControlStatusRemainTime(relay_control_task_remain_time);

  for (i=0; i<RELAY_MAX_NUMBER; i++)
  {
    if (0 == relays_control_status[i].remain_time_ms)
    {
      relayControlDriverDebugPrintln("Way=%d,Status=%d",i,relays_control_status[i].status);
      switch (relays_control_status[i].status)
      {
        case IDLE_OFF:    //空闲关  0
        case IDLE_ON:     //空闲开  1
          //两只控制脚都设定为低的时候，进入休眠状态。
          relayControlDriverControlOnOff(CONTROL_RELAY_0_0 + i, TURN_OFF);   //关继电器控制IC脚0
          relayControlDriverControlOnOff(CONTROL_RELAY_0_1 + i, TURN_OFF);   //关继电器控制IC脚1
          relays_control_status[i].remain_time_ms = MAX_UINT32;              //关掉计时
          if (relayControlCallback != NULL)
          {
            //触发动作完成的回调函数
            if (IDLE_OFF == relays_control_status[i].status)
            {
              relayControlCallback(i, RELAY_CONTROL_TURN_OFF);
            }
            else
            {
              relayControlCallback(i, RELAY_CONTROL_TURN_ON);
            }
          }
          break;

        case OFF_TO_ON_START:   //2
          //jim add 20200718
          if (!emberOkToNap()) //RF busy
          {
            relays_control_status[i].remain_time_ms = 5; //wait rf idle
          }
          //end jim
          //触发继电器控制IC，驱动继电器打向开。需要延时30ms后进入休眠状态。
          relayControlDriverControlOnOff(CONTROL_RELAY_0_0 + i, TURN_OFF);   //关继电器控制IC脚0
          relayControlDriverControlOnOff(CONTROL_RELAY_0_1 + i, TURN_ON);    //开继电器控制IC脚1

          //设定延时进入休眠状态
          relays_control_status[i].status = WAIT_TO_ON_END;
          relays_control_status[i].remain_time_ms = CONTROL_ON_DELAY_TO_SLEEP_TIME_MS;
          break;

        case WAIT_TO_ON_END:    //3
          //切换状态为开，由开状态完成动作。
          relays_control_status[i].status = IDLE_ON;
          relays_control_status[i].remain_time_ms = 0;
          break;

        case ON_TO_OFF_START:   //4
          //jim add 20200718
          if (!emberOkToNap()) //RF busy
          {
            relays_control_status[i].remain_time_ms = 5; //wait rf idle
          }
          //end jim
          //触发继电器控制IC，驱动继电器打向开。需要延时30ms后进入休眠状态。
          relayControlDriverControlOnOff(CONTROL_RELAY_0_0 + i, TURN_ON);   //开继电器控制IC脚0
          relayControlDriverControlOnOff(CONTROL_RELAY_0_1 + i, TURN_OFF);  //关继电器控制IC脚1

          //设定延时进入休眠状态
          relays_control_status[i].status = WAIT_TO_OFF_END;
          relays_control_status[i].remain_time_ms = CONTROL_OFF_DELAY_TO_SLEEP_TIME_MS;
          break;

        case WAIT_TO_OFF_END:    //5
          //切换状态为关，由关状态完成动作。
          relays_control_status[i].status = IDLE_OFF;
          relays_control_status[i].remain_time_ms = 0;
          break;

        default:
          break;
      }
    }
  }
  //找出下一任务最小延时时间，没有延时任务时关闭。
  relay_control_task_remain_time = getRelayControlMinRemainTime();
  if (relay_control_task_remain_time != MAX_UINT32)
  {
    emberEventControlSetDelayMS(relayControlTaskEventControl,relay_control_task_remain_time);
    relayControlDriverDebugPrintln("next delay %ld",relay_control_task_remain_time);
  }
}

/**
//函数名：relayControlDriverTrigeOnOffAction
//描述：触发继电器开关动作
//参数：way    (uint8_t[输入],需要控制第几路开关，0，1，2;)
//      action (uint8_t[输入],需要执行的动作，on->1, off->0;)
//      delay  (uint8_t[输入],此动作延时多久ms后执行)
//返回：void
*/
void relayControlDriverTrigeOnOffAction(uint8_t way, uint8_t action, uint8_t delay)
{
  uint32_t task_remain_time = 0;
  if (way >= RELAY_MAX_NUMBER)
  {
    return;
  }
  //计算出触发时，控制任务剩下的时间
  if (MAX_UINT32 == relay_control_task_remain_time) //没有其它正在执行的任务
  {
    relay_control_task_remain_time = 0;
  }
  else
  {
    task_remain_time = emberEventControlGetRemainingMS(relayControlTaskEventControl);
    if (relay_control_task_remain_time >= task_remain_time)
    {
      relay_control_task_remain_time -= task_remain_time;
    }
  }
  emberEventControlSetInactive(relayControlTaskEventControl);
  //更新任务剩余时间
  updateRelayControlStatusRemainTime(relay_control_task_remain_time);

  //更新此触发路的任务动作，与延时时间。
  relayControlDriverControlOnOff(CONTROL_RELAY_0_0 + way, TURN_OFF);   //关继电器控制IC脚0
  relayControlDriverControlOnOff(CONTROL_RELAY_0_1 + way, TURN_OFF);   //关继电器控制IC脚1
  if (RELAY_CONTROL_TURN_ON == action)
  {
    relays_control_status[way].status = OFF_TO_ON_START;
  }
  else
  {
    relays_control_status[way].status = ON_TO_OFF_START;
  }
  relays_control_status[way].remain_time_ms = (uint32_t)delay;

  relayControlDriverDebugPrintln("trig,Way=%d,Status=%d",way,relays_control_status[way].status);

  relay_control_task_remain_time = 0;
  emberEventControlSetActive(relayControlTaskEventControl);
}

/**
//函数名：relayControlDriverGetCurrenStatus
//描述  ：获取当前继电器开关状态
//参数  ：way    (uint8_t[输入],需要获取第几路开关状态，0，1，2;)
//返回  ：uint8_t (返回当前此路开关的状态，0关，1开，0xFF表示有误。
*/
uint8_t relayControlDriverGetCurrenStatus(uint8_t way)
{
  if (way >= RELAY_MAX_NUMBER)
  {
    return 0xFF;
  }
  switch (relays_control_status[way].status)
  {
    case IDLE_ON:
    case OFF_TO_ON_START:
    case WAIT_TO_ON_END:
      return RELAY_CONTROL_TURN_ON;
      break;
    case IDLE_OFF:
    case ON_TO_OFF_START:
    case WAIT_TO_OFF_END:
      return RELAY_CONTROL_TURN_OFF;
      break;
    default:
      break;
  }
  return 0xFF;
}
/***************************************** 文 件 结 束 ************************************************/
