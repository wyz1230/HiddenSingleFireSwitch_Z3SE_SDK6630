/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:buttons-driver.c
*Author : JimYao
*Version : 1.1
*Date : 2019-10-8
*Description: 按键驱动及按键扫描任务处理
*History:在1.0版本的基础上,优化时间侦测,减少频繁唤醒
***************************************************************************************************/
/* 相关包含头文件区 ------------------------------------------------------------ */
//#include "buttons-driver.h"
#include "gpiointerrupt.h"
#include "common-app.h"

/* 宏定义区 -------------------------------------------------------------------- */
//debug开关设定
//#define BUTTON_DRIVER_DEBUG_ENABLE
#ifdef BUTTON_DRIVER_DEBUG_ENABLE
  #define DEBUG_STRING                     "Debug-BtnDriver:"
  #define buttonsDriverDebugPrint(...)     emberAfPrint(0xFFFF,DEBUG_STRING __VA_ARGS__)
  #define buttonsDriverDebugPrintln(...)   emberAfPrintln(0xFFFF,DEBUG_STRING __VA_ARGS__)
#else
  #define buttonsDriverDebugPrint(...)
  #define buttonsDriverDebugPrintln(...)
#endif

#define BUTTONS_MAX_NUM                HAL_BUTTON_COUNT //(5U)  //定义最大按键数

#if !defined(HAL_BUTTON_COUNT) || (BUTTONS_MAX_NUM == 0)
#error "no button defined!"
#endif

//0 低电平有效，低电平按键按下,1 高电平有效

#define BUTTON_PRESS_ACTIVE_STATE      (1) //高电平有效

#define BUTTON_PRESSED_VALID_TIME      (50) //(10)
#define BUTTON_PRESSED_TIME_1S         (1000)
#define BUTTON_PRESSED_TIME_2S         (2000)
#define BUTTON_PRESSED_TIME_3S         (3000)
#define BUTTON_PRESSED_TIME_4S         (4000)
#define BUTTON_PRESSED_TIME_5S         (5000)
#define BUTTON_PRESSED_TIME_10S        (10000)
#define BUTTON_PRESSED_REPEAT_TIME	   (300)

#define set_bit(byte, bit)             (byte |= 1 << bit)
#define reset_bit(byte, bit)           (byte &= ~(1 << bit))
#define get_bit(byte, bit)             ((byte & (1 << bit)) >> bit)

/* 自定义类型区 ---------------------------------------------------------------- */
typedef struct {
  uint16_t pressed_time_ms;          //按键按下的时间
  uint8_t  next_action_time_index;   //下一个按键动作时间索引号
} tButtonPressedStatus;

//此部分是按键回调处理接口所用
typedef struct {
  uint16_t button_pressed_time;
  eButtonState button_pressed_state;
  eButtonState button_released_state;
} tButtonPressedTimeMsActions;

//

//采用芯科的button.c定义的驱动部分
typedef struct {
  GPIO_Port_TypeDef   port;
  unsigned int        pin;
} tButtonArray;
static const tButtonArray buttonsArray[BUTTONS_MAX_NUM] = BSP_BUTTON_INIT;


/* 全局变量区 ------------------------------------------------------------------ */
EmberEventControl buttonsDriverButtonTaskEventControl;	//按键任务扫描事件
//void buttonsDriverButtonTaskEventHandler(void);       //需要在代码中添加buttonsDriverButtonTaskEventControl事件处理

/* 本地变量区 ------------------------------------------------------------------ */
static uint8_t button_intterupt_counter = 0;                //此变量用来累计当前中断触发的次数，解决事件频繁触发的问题。
static uint16_t current_pressed_buttons_bitmask = 0x0000;	//当前按键按下的位表
static uint16_t previous_pressed_buttons_bitmask = 0x0000;	//前一次按键按下的位表
//增加此变量，用来侦测按键事件是否需要关闭，解决出现按键坏掉按死后一直无法关掉按键事件的问题。
static uint16_t buttons_released_bitmask = 0x0000;

static fpButtonsHandle buttonsAppHandle = NULL;


static tButtonPressedStatus buttons_status[BUTTONS_MAX_NUM];
//在以下的部分添加需要用到的按键时间，及状态，callback函数会按表格中设定的参数返回。
#define BUTTON_MAX_CALLBACK_NUMBER  4  //注意此部分定义的时间需要从小到大
static const tButtonPressedTimeMsActions button_action_callback_table[BUTTON_MAX_CALLBACK_NUMBER] =
											{{BUTTON_PRESSED_VALID_TIME,  BUTTON_PRESSED_VALID, BUTTON_RELEASE_SHORT_PRESS},
                                             {BUTTON_PRESSED_TIME_1S,	  BUTTON_PRESSED_1S,    BUTTON_RELEASE_THAN_1S},
											 {BUTTON_PRESSED_TIME_3S,	  BUTTON_PRESSED_3S,    BUTTON_RELEASE_THAN_3S},
											 {BUTTON_PRESSED_TIME_5S,	  BUTTON_PRESSED_5S,    BUTTON_RELEASE_THAN_3S}};

/* 全局函数声明区 -------------------------------------------------------------- */

/* 本地函数声明区 -------------------------------------------------------------- */
#ifdef USED_CUSTOM_BUTTON_DRIVER_CODE
  void halButtonIsrCallback(uint8_t pin);
#else
  void halButtonIsrCallback(uint8_t pin, uint8_t state);
#endif
/* 函数原型 -------------------------------------------------------------------- */
#ifdef USED_CUSTOM_BUTTON_DRIVER_CODE //jim 20190826 add for the new button driver
void halInternalInitButton(void)
{
#if (BUTTONS_MAX_NUM > 0)
  /* Initialize GPIO interrupt dispatcher */
  GPIOINT_Init();

  /* Enable GPIO in CMU */
#if !defined(_SILICON_LABS_32B_SERIES_2)
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
#endif

  uint8_t i;
  for ( i = 0; i < BUTTONS_MAX_NUM; i++ ) {
    /* Configure pin as input */
    GPIO_PinModeSet(buttonsArray[i].port,
                    buttonsArray[i].pin,
                    BSP_BUTTON_GPIO_MODE,
                    BSP_BUTTON_GPIO_DOUT);
    /* Register callbacks before setting up and enabling pin interrupt. */
    GPIOINT_CallbackRegister(buttonsArray[i].pin,
                             halButtonIsrCallback);
    /* Set rising and falling edge interrupts */
    GPIO_ExtIntConfig(buttonsArray[i].port,
                      buttonsArray[i].pin,
                      buttonsArray[i].pin,
                      true,
                      true,
                      true);
  }
#endif
}
#endif //jim add
/**
//函数名：halButtonIsrCallback
//描述：按键中断入口，需要在芯科的BUTTON.c里加代码，当前采用加宏定义BUTTONS_ISR_CALLBACK_CODE的方式。
//参数：pin (uint8_t [输入]，按键脚号)
//      state (uint8_t [输入]，按键脚状态位)
//返回：无
*/
#ifdef USED_CUSTOM_BUTTON_DRIVER_CODE
  void halButtonIsrCallback(uint8_t pin)
#else
  void halButtonIsrCallback(uint8_t pin, uint8_t state)
#endif
{
    emberEventControlSetActive(buttonsDriverButtonTaskEventControl);
	if (button_intterupt_counter < 0xFF)
    {
	  button_intterupt_counter++; //按键事件是否需要再次处理，通过此变量来判断。
	}
   // buttonsDriverDebugPrintln("pin=%d,state=%d",pin,state);
}

/**
//函数名：getButtonIndexState
//描述：获取按键对应的引脚电平，按索引查找。需要对应芯科生成的hal_config.h文件。
//参数：index (uint8_t [输入]，按键表的索引号)
//返回：uint16_t [输出],按键接下的键位表
*/
static uint8_t getButtonIndexState(uint8_t index)
{
	return (GPIO_PinInGet(buttonsArray[index].port, buttonsArray[index].pin));
}

/**
//函数名：getPressedButtonsBitmask
//描述：获取按键对应的按键位表
//参数：无
//返回：uint16_t [输出],按键接下的键位表
*/
uint16_t getPressedButtonsBitmask(void)
{
	//当前暂不支持组合按键
	 uint16_t buttons_bit_mask = 0x0000;
	 for (uint8_t index=0; index < BUTTONS_MAX_NUM; index++)
	 {
	   if(getButtonIndexState(index) == BUTTON_PRESS_ACTIVE_STATE)
	   {
		   buttons_bit_mask |= 1<<index;
	   }
	 }
	 return buttons_bit_mask;
}

/**
//函数名：buttonsDriverButtonInit
//描述：按键任务事件初始化，初始化按键扫描的相关变量。
//参数：handle    (fpButtonsHandle [输入]，按键回调注册)
//返回：无
*/
void buttonsDriverButtonInit(fpButtonsHandle handle)
{
  //#ifdef USED_CUSTOM_BUTTON_DRIVER_CODE //jim 20190826 add for the new button driver
    halInternalInitButton();
  //#endif
	current_pressed_buttons_bitmask = 0x0000;
	previous_pressed_buttons_bitmask = 0x0000;
    buttons_released_bitmask = 0x0000;

	for (uint8_t i=0; i<BUTTONS_MAX_NUM; i++)
	{
		buttons_status[i].pressed_time_ms = 0;
		buttons_status[i].next_action_time_index = 0xFF;
	}
	buttonsAppHandle = handle;
    button_intterupt_counter = 0;
	emberEventControlSetActive(buttonsDriverButtonTaskEventControl);
    buttonsDriverDebugPrintln("init");
}

/**
//函数名：buttonsDriverButtonTaskEventHandler
//描述：按键扫描事件任务处理部分
//参数：无
//返回：void
*/
void buttonsDriverButtonTaskEventHandler(void)
{
	uint8_t i;
    static uint16_t last_time_ms;
    uint16_t current_time_ms,delta_time_ms,mini_next_time_ms;

	emberEventControlSetInactive(buttonsDriverButtonTaskEventControl);

	if (NULL == buttonsAppHandle) //当运用代码没注册回调函数时，此任务不需要处理，所有相关变量初始化。
	{
		current_pressed_buttons_bitmask = 0x0000;
		previous_pressed_buttons_bitmask = 0x0000;
        buttons_released_bitmask = 0x0000;
		for (i=0; i<BUTTONS_MAX_NUM; i++)
		{
			buttons_status[i].pressed_time_ms = 0;
			buttons_status[i].next_action_time_index = 0xFF;
		}
		return;
	}

    current_time_ms = halCommonGetInt16uMillisecondTick();
	if (0x0000 == buttons_released_bitmask)
    {
       last_time_ms = current_time_ms; //第一次触发时，最后的时间和当前的时间是一致的。
    }
	delta_time_ms = current_time_ms - last_time_ms; //算出两次事件触发的时差
	last_time_ms = current_time_ms;

	current_pressed_buttons_bitmask = getPressedButtonsBitmask();
    buttons_released_bitmask = current_pressed_buttons_bitmask;
	for (i=0; i<BUTTONS_MAX_NUM; i++) //遍历所有键位
	{
		if (get_bit(current_pressed_buttons_bitmask,i)) //当前此位的按键是按下的
		{
         #ifdef AWAYS_PRESSED_PROTECT
            if (buttons_status[i].pressed_time_ms == 0xFFFF) //出现一直按着的异常情况
            {
                reset_bit(buttons_released_bitmask, i); //异常按键清除掉按下标志位
            }
            else
         #endif
            {
			  if (buttons_status[i].pressed_time_ms < (0xFFFF - delta_time_ms))
		      {
                buttons_status[i].pressed_time_ms += delta_time_ms; //处理时间累加
			  }
			  else
			  {
				buttons_status[i].pressed_time_ms = 0xFFFF;
			  }
            }

            if (get_bit(previous_pressed_buttons_bitmask,i) == 0)//第一次按下
            {
            	buttons_status[i].pressed_time_ms = 0;
				buttons_status[i].next_action_time_index = 0;
            }
			else //if (get_bit(previous_pressed_buttons_bitmask,i)) //按键一直按着
			{
		      if (buttons_status[i].next_action_time_index < BUTTON_MAX_CALLBACK_NUMBER)
              {
                if (buttons_status[i].pressed_time_ms >= button_action_callback_table[buttons_status[i].next_action_time_index].button_pressed_time)
			    {
                  //判断按下的时间，触发相应的动作。
				  buttonsAppHandle(i,button_action_callback_table[buttons_status[i].next_action_time_index].button_pressed_state);
				  buttons_status[i].next_action_time_index++;
			    }
              }
			}
		}
		else  //按键松开
		{
			if (get_bit(previous_pressed_buttons_bitmask,i)) //第一次按键松开
			{
			   if ((buttons_status[i].next_action_time_index > 0) && (buttons_status[i].next_action_time_index <= BUTTON_MAX_CALLBACK_NUMBER))
			   {
                  //时间由按下来累计，松开只需要通过index号来分析即可。
				  buttonsAppHandle(i,button_action_callback_table[buttons_status[i].next_action_time_index-1].button_released_state);
			   }
			}
			buttons_status[i].pressed_time_ms = 0;
			buttons_status[i].next_action_time_index = 0xFF;  //无效
            reset_bit(buttons_released_bitmask, i); //清除掉按下标志位
		}
	}

    previous_pressed_buttons_bitmask = current_pressed_buttons_bitmask; //更新前一次按键的位表

//计算出下次最短的时间,触发延时.
    mini_next_time_ms = 0xFFFF;
	if (buttons_released_bitmask) //当有按键按下，需要算出下次最短的处理延时时间。
	{
       for (i=0; i<BUTTONS_MAX_NUM; i++)
       {
         if (buttons_status[i].next_action_time_index < BUTTON_MAX_CALLBACK_NUMBER)
         {
            if (buttons_status[i].pressed_time_ms <= button_action_callback_table[buttons_status[i].next_action_time_index].button_pressed_time)
            {
               delta_time_ms = button_action_callback_table[buttons_status[i].next_action_time_index].button_pressed_time - buttons_status[i].pressed_time_ms;
               if (delta_time_ms < mini_next_time_ms)
               {
                  mini_next_time_ms = delta_time_ms; //算出下次延时的最小时间
               }
            }
         }
         buttonsDriverDebugPrintln("btn=%d,next action id=%d",i,buttons_status[i].next_action_time_index);
       }
       if (0xFFFF != mini_next_time_ms)
       {
	     emberEventControlSetDelayMS(buttonsDriverButtonTaskEventControl,mini_next_time_ms); //开启定时扫描
         buttonsDriverDebugPrintln("next delay ms=%ld",mini_next_time_ms);
       }
	}
	if (button_intterupt_counter) //有按键中断未处理完，需要立即处理。
    {
       button_intterupt_counter--;
	   emberEventControlSetActive(buttonsDriverButtonTaskEventControl);
	}
    buttonsDriverDebugPrintln("key=0x%x",current_pressed_buttons_bitmask);
}
/*************************************** 文 件 结 束 ******************************************/
