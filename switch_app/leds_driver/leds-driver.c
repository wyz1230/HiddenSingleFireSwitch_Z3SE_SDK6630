/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:leds-driver.c
*Author : JimYao
*Version : 1.1
*Date : 2019-10-08
*Description: LED指示灯驱动及闪灯任务处理
*History:在1.0版本的基础上,优化时间侦测,减少频繁唤醒
***************************************************************************************************/

/* 相关包含头文件区 --------------------------------------------------------- */
//#include "leds-driver.h"
#include "common-app.h"

/* 宏定义区 ----------------------------------------------------------------- */
//debug开关设定
//#define LED_DRIVER_DEBUG_ENABLE
#ifdef LED_DRIVER_DEBUG_ENABLE
  #define DEBUG_STRING                      "Debug-LedDriver:"
  #define ledsDriverDebugPrint(...)         emberAfPrint(0xFFFF,DEBUG_STRING __VA_ARGS__)
  #define ledsDriverDebugPrintln(...)       emberAfPrintln(0xFFFF,DEBUG_STRING __VA_ARGS__)
#else
  #define ledsDriverDebugPrint(...)
  #define ledsDriverDebugPrintln(...)
#endif

//定义LED有效电平
#define LED_HIGH_ACTIVE	//高电平亮
#ifdef LED_HIGH_ACTIVE
  #define LED_ON    1
  #define LED_OFF   0
#else
  #define LED_ON    0
  #define LED_OFF   1
#endif

//定义指示灯的控制脚IO口
#define LED_MAX_NUMBER      (1U)  //LED灯的控制口数量

//以下是快速定义，对应灯号的控制，需要修改ledsDriverLedOnOff()函数，
//ledsDriverLedsControl()函数和leds-driver.h头文件中的灯号枚举。
#define LED0_PORT           (gpioPortC)
#define LED0_PIN            (1U)

         

#define SET_LED0_ONOFF(x)   do { (x)==0 ? GPIO_PinOutClear(LED0_PORT, LED0_PIN) : \
                                            GPIO_PinOutSet(LED0_PORT, LED0_PIN); }while(0)


#define LEDS_PINTS_INIT()   do { GPIO_PinModeSet(LED0_PORT,LED0_PIN,gpioModePushPull,LED_OFF);}while(0)

#define set_bit(byte, bit)          (byte |= 1 << bit)
#define reset_bit(byte, bit)        (byte &= ~(1 << bit))
#define get_bit(byte, bit)          ((byte & (1 << bit)) >> bit)

/* 自定义类型区 ------------------------------------------------------------- */
// 定义控制灯的行为，0:对应灯号闪动频率，1:对应灯号闪动次数，0xFF为常亮。
typedef struct {
  uint16_t on_delay_ms;         //延时亮灯时间
  uint16_t on_duration_ms;      //0x00表示常灭,0xFF表示常亮,其它值表示亮的周期
  uint16_t total_duration_ms;   //总周期，含亮与灭时间。
  uint16_t duration_time_ms;    //计时器
  uint8_t  repeat_counter;       //重复次数计数器,0xFF表示一直闪动,0x00表示停止闪动。
} tLedBlinkPattern;

/* 全局变量区 --------------------------------------------------------------- */
EmberEventControl ledsDriverLedTaskEventControl;  //需要定义闪灯事件和闪灯事件处理入口函数
//void ledsDriverLedTaskEventHandler(void);

/* 本地变量区 --------------------------------------------------------------- */
static uint16_t led_blink_toggle_mark;      //灯亮标志位,每路灯对应一个bit。
static uint16_t led_blinking_mark;          //闪动标志位,每路灯对应一个bit。
static tLedBlinkPattern led_blink_pattern[LED_MAX_NUMBER];
static uint16_t next_delay_ms = 0xFFFF;     //距离下一个事件延时时间,//0xFFFF表示当前没有延时，无效时间

/* 全局函数声明区 ----------------------------------------------------------- */

/* 本地函数声明区 ----------------------------------------------------------- */


/* 函数原型 ----------------------------------------------------------------- */
/**
//函数名：ledsDriverLedPinsInit
//描述：初始化各LED的IO口
//参数：无
//返回：void
*/
static void ledsDriverLedPinsInit(void)
{
  /* Enable GPIO in CMU */
#if !defined(_SILICON_LABS_32B_SERIES_2)
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
#endif
  LEDS_PINTS_INIT();
}

/**
//函数名：ledsDriverLedOnOff
//描述：点亮对应的LED灯
//参数：led_num (uint8_t[输入],要控制的对应灯号;)
//      on_off  (uint8_t[输入],对应灯号开关;)
//返回：void
*/
static void ledsDriverLedOnOff(uint8_t led_num,uint8_t on_off)
{
  if (LED_RED == led_num)
  {
    SET_LED0_ONOFF(on_off);
  }

}

/**
//函数名：ledsDriverLedTaskInit
//描述：初始化LED的IO与闪动事件处理状态变量
//参数：无
//返回：void
*/
void ledsDriverLedTaskInit(void)
{
  ledsDriverLedPinsInit();
  next_delay_ms = 0xFFFF;
  ledsDriverLedsControl(LED_ALL,0,LED_FREQUENCY_TYPE_ALWAYS_OFF,
                           LED_FREQUENCY_TYPE_ALWAYS_OFF, LED_REPEAT_TYPE_STOP_REPEAT);
  ledsDriverDebugPrintln("init");
}

/**
//函数名：ledsDriverLedTaskUpdateDelayTimeMs
//描述：更新当前任务中的累计时间
//参数：last_delay_ms (uint16_t[输入],已经延时的时间ms)
//返回：void
*/
static void ledsDriverLedTaskUpdateDelayTimeMs(uint16_t last_delay_ms)
{
  if (0xFFFF != last_delay_ms)
  {
    for (uint8_t i = 0; i < LED_MAX_NUMBER; i++)
    {
      if (get_bit(led_blinking_mark, i))
      {
        if (led_blink_pattern[i].repeat_counter)//有闪灯处理
        {
          //处理闪灯计时累计
          if (0xFFFF - last_delay_ms > led_blink_pattern[i].duration_time_ms)
          {
            led_blink_pattern[i].duration_time_ms += last_delay_ms;
          }
          else
          {
            led_blink_pattern[i].duration_time_ms = 0xFFFF;
          }
        }
      }
    }
  }
}

/**
//函数名：ledsDriverLedTaskFindTheNextDelayTimeMs
//描述：查找出下一个延时的最小时间
//参数：无
//返回：uint16_t 下一个最小的延时时间，单位ms。返回0xFFFF时表示不需要延时。
*/
static uint16_t ledsDriverLedTaskFindTheNextDelayTimeMs(void)
{
  uint8_t i;
  uint16_t mini_delay=0xFFFF, temp_delay;
  for (i = 0; i < LED_MAX_NUMBER; i++)
  {
    temp_delay = 0xFFFF;
    if (get_bit(led_blinking_mark, i))
    {
      if ((LED_FREQUENCY_TYPE_ALWAYS_OFF == led_blink_pattern[i].on_duration_ms) ||
          (LED_FREQUENCY_TYPE_ALWAYS_ON == led_blink_pattern[i].on_duration_ms))
      {
        temp_delay = 0xFFFF; //led_blink_pattern[i].duration_time_ms;
      }
      else if (led_blink_pattern[i].duration_time_ms < led_blink_pattern[i].on_delay_ms)
      {
        temp_delay = led_blink_pattern[i].on_delay_ms;
        temp_delay -= led_blink_pattern[i].duration_time_ms;
      }
      else
      {
        if (led_blink_pattern[i].total_duration_ms - led_blink_pattern[i].on_delay_ms < led_blink_pattern[i].on_duration_ms)
        {
          //出现持续亮灯比总周期长的时候，以总周期为主。特殊情况
          if (led_blink_pattern[i].duration_time_ms < led_blink_pattern[i].total_duration_ms)
          {
            temp_delay = led_blink_pattern[i].total_duration_ms - led_blink_pattern[i].duration_time_ms;
          }
          else
          {
            //异常情况
            led_blink_pattern[i].duration_time_ms = 0;
            temp_delay = 0;
          }
        }
        else
        {
          //正常情况
          if (led_blink_pattern[i].duration_time_ms < led_blink_pattern[i].on_duration_ms + led_blink_pattern[i].on_delay_ms)
          { //下一个到达on线束时间
            temp_delay = led_blink_pattern[i].on_duration_ms + led_blink_pattern[i].on_delay_ms;
            temp_delay -= led_blink_pattern[i].duration_time_ms;
          }
          else
          {
            if (led_blink_pattern[i].duration_time_ms < led_blink_pattern[i].total_duration_ms)
            {
              temp_delay = led_blink_pattern[i].total_duration_ms - led_blink_pattern[i].duration_time_ms;
            }
            else
            {
              //异常情况
              led_blink_pattern[i].duration_time_ms = 0;
              temp_delay = 0;
            }
          }
        }
      }

      if (mini_delay > temp_delay)
      {
        mini_delay = temp_delay; //更新最小延时时间。
      }
      if (0x0000 == mini_delay) //已经是最小时间，不需要遍历。
      {
        break;
      }
    }
  }
  return mini_delay;
}

/**
//函数名：ledsDriverLedTaskEventHandler
//描述：闪动事件任务处理部分
//参数：无
//返回：void
*/
void ledsDriverLedTaskEventHandler(void)
{
  uint8_t i;
  emberEventControlSetInactive(ledsDriverLedTaskEventControl);

  for (i = 0; i < LED_MAX_NUMBER; i++)
  {
    if (get_bit(led_blinking_mark, i))
    {
      if (LED_FREQUENCY_TYPE_ALWAYS_OFF == led_blink_pattern[i].on_duration_ms)  //表示常灭
      {
        ledsDriverLedOnOff(i,LED_OFF);
        led_blink_pattern[i].repeat_counter = LED_REPEAT_TYPE_STOP_REPEAT;
      }
      else if (LED_FREQUENCY_TYPE_ALWAYS_ON == led_blink_pattern[i].on_duration_ms) //表示常亮
      {
        ledsDriverLedOnOff(i,LED_ON);
        led_blink_pattern[i].repeat_counter = LED_REPEAT_TYPE_STOP_REPEAT;
      }
      else if (led_blink_pattern[i].repeat_counter)//闪灯部分处理
      {
        //处理闪灯计时累计
        led_blink_pattern[i].duration_time_ms += next_delay_ms;

        if (led_blink_pattern[i].duration_time_ms == led_blink_pattern[i].on_delay_ms) //延时亮的时间必须小于总的时间
        {
          ledsDriverLedOnOff(i,LED_ON);
          set_bit(led_blink_toggle_mark, i);
        }

        if (led_blink_pattern[i].duration_time_ms >= led_blink_pattern[i].total_duration_ms)
        {
          if (led_blink_pattern[i].repeat_counter != LED_REPEAT_TYPE_ALWAYS_REPEAT) //0xFF 表示不超时
          {
            led_blink_pattern[i].repeat_counter--;
          }
          if (led_blink_pattern[i].repeat_counter > 0) //判断是否已经结束，结束不需要重头设定灯的状态，后续流程处理。
          {
            if (led_blink_pattern[i].on_delay_ms > 0) //延时亮处理部分
            {
              ledsDriverLedOnOff(i,LED_OFF);
              reset_bit(led_blink_toggle_mark, i);
            }
            else
            {
              ledsDriverLedOnOff(i,LED_ON);
              set_bit(led_blink_toggle_mark, i);
            }
          }
          led_blink_pattern[i].duration_time_ms = 0;
        }
        else if (led_blink_pattern[i].duration_time_ms == (led_blink_pattern[i].on_duration_ms + led_blink_pattern[i].on_delay_ms))
        { //当延时亮的时间 + 持续亮灯的时间大于总时间是，表示灯最后保持亮。当此条件满足时，这部分代码不会运行到。
          ledsDriverLedOnOff(i,LED_OFF);
          reset_bit(led_blink_toggle_mark, i);
        }
      }

      if (LED_REPEAT_TYPE_STOP_REPEAT == led_blink_pattern[i].repeat_counter) //灯控制结束处理
      {
        if ((led_blink_pattern[i].on_duration_ms != LED_FREQUENCY_TYPE_ALWAYS_ON) ||
            (led_blink_pattern[i].on_duration_ms <= led_blink_pattern[i].total_duration_ms - led_blink_pattern[i].on_delay_ms))
        { //常亮时不允许关掉,亮的时间大于总时间，最后灯的状态是亮的。
          ledsDriverLedOnOff(i,LED_OFF);
        }
        reset_bit(led_blinking_mark, i);
      }
      //led_blink_pattern[i].duration_time_ms++;
      ledsDriverDebugPrintln("led=%d flash timer=%ld",i,led_blink_pattern[i].duration_time_ms);
    }

    //emberEventControlSetDelayMS(ledsDriverLedTaskEventControl,LED_TASK_BLINK_POLL_TIME); //周期闪灯轮询处理
  }
//计算最下一个最小延时时间
  next_delay_ms = ledsDriverLedTaskFindTheNextDelayTimeMs();
  if (next_delay_ms != 0xFFFF)
  {
    emberEventControlSetDelayMS(ledsDriverLedTaskEventControl,next_delay_ms);
    ledsDriverDebugPrintln("next blink delay ms=%ld",next_delay_ms);
  }
  ledsDriverDebugPrintln("blink=0x%x",led_blinking_mark);
}

/**
//函数名：ledsDriverCheckLedBlinkFinish
//描述：查看当前led灯任务处理是否完成,0表示完成
//参数：无
//返回：void
*/
uint16_t ledsDriverCheckLedBlinkFinish(void)
{
  return led_blinking_mark;
}

/**
//函数名：ledsDriverLedOnOffePatternSet
//描述：设定对应灯的闪动类型
//参数：led_num            (uint8_t[输入],要设定的对应灯号;)
//      on_delay_ms        (uint16_t[输入],每个周期延时多久亮灯;)
//      on_duration_ms     (uint16_t[输入],每个周期亮灯的时间;0x0000表示常灭,0xFFFF表示常亮;
//                                         当on_delay_ms > 0，且on_delay_ms + on_duration_ms > total_duration_ms 时，此为特殊设定，
//                                         用来处理周期闪灯后，灯保持亮的情况。)
//      total_duration_ms  (uint16_t[输入],每个周期的时间;)
//      repeat_counter     (uint8_t[输入],闪动重复次数;0xFF表示一直闪动,0x00表示停止闪动;)
//返回：void
*/
static void ledsDriverLedOnOffePatternSet(uint8_t led_num, uint16_t on_delay_ms, uint16_t on_duration_ms,
                                          uint16_t total_duration_ms, uint8_t repeat_counter)
{
  if (led_num < LED_MAX_NUMBER) //单独控制一颗灯的情况
  {
    if (total_duration_ms <= on_delay_ms) //总时间小于开延时时间，切换为常关。
    {
      led_blink_pattern[led_num].on_duration_ms = LED_FREQUENCY_TYPE_ALWAYS_OFF;
    }
    else if ((0x0000 == on_delay_ms) &&
             (on_duration_ms > total_duration_ms))//当出现延时开时间为0，而为持续时间大于总周期时间时，切换为常开。
    {
      led_blink_pattern[led_num].on_duration_ms = LED_FREQUENCY_TYPE_ALWAYS_ON;
    }
    else
    {
      led_blink_pattern[led_num].on_duration_ms = on_duration_ms;
    }

    led_blink_pattern[led_num].on_delay_ms = on_delay_ms;
    led_blink_pattern[led_num].total_duration_ms = total_duration_ms;
    led_blink_pattern[led_num].repeat_counter = repeat_counter;
    led_blink_pattern[led_num].duration_time_ms = 0;

    reset_bit(led_blink_toggle_mark, led_num);
    reset_bit(led_blinking_mark, led_num);
    if (LED_FREQUENCY_TYPE_ALWAYS_OFF == led_blink_pattern[led_num].on_duration_ms)
    {
      ledsDriverLedOnOff(led_num,LED_OFF);
    }
    else if (LED_FREQUENCY_TYPE_ALWAYS_ON == led_blink_pattern[led_num].on_duration_ms)
    {
      ledsDriverLedOnOff(led_num,LED_ON);
    }
    else
    {
      if (0x0000 == led_blink_pattern[led_num].on_delay_ms) //无延时亮处理
      {
        ledsDriverLedOnOff(led_num,LED_ON);
        set_bit(led_blink_toggle_mark, led_num);
      }
      else
      {
        ledsDriverLedOnOff(led_num,LED_OFF);
      }
      set_bit(led_blinking_mark, led_num);
    }

    ledsDriverDebugPrintln("set led=%d, on delay=%ld, on duration=%ld, total=%ld, repeat=%d",
                            led_num,led_blink_pattern[led_num].on_delay_ms,
                            led_blink_pattern[led_num].on_duration_ms,
                            led_blink_pattern[led_num].total_duration_ms,
                            led_blink_pattern[led_num].repeat_counter);
  }
}

/**
//函数名：ledsDriverLedsControl
//描述：控制对应灯的行为
//参数：led_num            (uint8_t[输入],要设定的对应灯号;)
//      on_delay_ms        (uint16_t[输入],每个周期延时多久亮灯;)
//      on_duration_ms     (uint16_t[输入],每个周期亮灯的时间;0x0000表示常灭,0xFFFF表示常亮;
//                                         当on_delay_ms > 0，且on_delay_ms + on_duration_ms > total_duration_ms 时，此为特殊设定，
//                                         用来处理周期闪灯后，灯保持亮的情况。)
//      total_duration_ms  (uint16_t[输入],每个周期的时间;)
//      repeat_counter     (uint8_t[输入],闪动重复次数;0xFF表示一直闪动,0x00表示停止闪动;)
//返回：void
*/
void ledsDriverLedsControl(uint8_t led_num, uint16_t on_delay_ms,uint16_t on_duration_ms,
                           uint16_t total_duration_ms, uint8_t repeat_counter)
{
  if (next_delay_ms != 0xFFFF) //表示触发的时候有存在其它灯的任务还在执行
  {
    //更新下一个延时的时间
    uint32_t remain_delay_time_ms = emberEventControlGetRemainingMS(ledsDriverLedTaskEventControl);
    if (remain_delay_time_ms < next_delay_ms)
    {
      next_delay_ms -= (uint16_t)remain_delay_time_ms;
    }
  }
  else
  {
    next_delay_ms = 0;
  }
  emberEventControlSetInactive(ledsDriverLedTaskEventControl);
  ledsDriverLedTaskUpdateDelayTimeMs(next_delay_ms);

  if (led_num < LED_MAX_NUMBER) //单独控制一颗灯
  {
    ledsDriverLedOnOffePatternSet(led_num,on_delay_ms,on_duration_ms,total_duration_ms,repeat_counter);
  }
  else //控制所有灯
  {
    ledsDriverLedOnOffePatternSet(LED_RED,on_delay_ms,on_duration_ms,total_duration_ms,repeat_counter);
    //ledsDriverLedOnOffePatternSet(LED_GREEN,on_delay_ms,on_duration_ms,total_duration_ms,repeat_counter);
    //ledsDriverLedOnOffePatternSet(LED_BLUE,on_delay_ms,on_duration_ms,total_duration_ms,repeat_counter);
  }

  ledsDriverDebugPrintln("pre delay time=%ld",next_delay_ms);

  next_delay_ms = 0;
  emberEventControlSetActive(ledsDriverLedTaskEventControl);
}

/***************************************** 文 件 结 束 ************************************************/
