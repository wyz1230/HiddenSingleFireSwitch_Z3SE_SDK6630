/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:rejoin-interval-proc.c
*Author : JimYao
*Version : 1.0
*Date : 2019-10-16
*Description: 基于芯科end-device-move.c里的rejoin机制，通过此代码和相应的宏定义，转换为自定义的rejoin
*History:
***************************************************************************************************/

/* 相关包含头文件区 --------------------------------------------------------- */
//#include "rejoin-interval-proc.h"
#include "common-app.h"

#ifdef USE_CUSTOM_RJOIN_CODE //此部分的code需要定义此宏
/* 宏定义区 ----------------------------------------------------------------- */
//debug开关设定
//#define REJOIN_INTERVAL_PROC_DEBUG_ENABLE
#ifdef REJOIN_INTERVAL_PROC_DEBUG_ENABLE
  #define DEBUG_STRING                  "RejoinProc-DB:"
  #define rejoinProcDebugPrint(...)     emberAfPrint(0xFFFF,DEBUG_STRING __VA_ARGS__)
  #define rejoinProcDebugPrintln(...)   emberAfPrintln(0xFFFF,DEBUG_STRING __VA_ARGS__)
#else
  #define rejoinProcDebugPrint(...)
  #define rejoinProcDebugPrintln(...)
#endif
#define SUPPORT_EACH_CHANNELS_REJOIN
#ifdef SUPPORT_EACH_CHANNELS_REJOIN    //支持每个信道单独rejoin
  #define ALL_CHANNELS_MASK  EMBER_ALL_802_15_4_CHANNELS_MASK
  #define MIN_CHANNEL        EMBER_MIN_802_15_4_CHANNEL_NUMBER
  #define MAX_CHANNEL        EMBER_MAX_802_15_4_CHANNEL_NUMBER
#endif

#if 0
#define REJOIN_TYPE_CURRENT_CHANNEL            (0U)   //当前信道模式
#define REJOIN_TYPE_ALL_CHANNELS               (1U)   //全信道模式
#define REJOIN_TYPE_GOTO_TABLE_INDEX           (2U)   //跳转到其它索引，此时用retry_max_times当跳转索引号
#define REJOIN_TYPE_REPEAT_COUNT_SET           (3U)   //设定后续rejoin重复次数
#define REJOIN_TYPE_CHECK_REPEAT_COUNT_GOTO    (4U)   //检测重复次数，跳转到相应的index,如果超出重复次数，则不跳转，顺序执行。
#define REJOIN_TYPE_STOP                       (0xFF) //结束模式，结束时，
#endif

enum{
    REJOIN_TYPE_CURRENT_CHANNEL=0x00,        //当前信道模式
#ifdef SUPPORT_EACH_CHANNELS_REJOIN    //支持每个信道单独rejoin
    REJOIN_TYPE_EACH_CHANNEL_START_SET,      //每个信道间隔模式起点设定，设定起始信道
    REJOIN_TYPE_EACH_CHANNEL,                //设定的信道回连执行，
    REJOIN_TYPE_EACH_CHANNEL_STOP_CHECK,     //判断是否达到停止条件，即信道11-26是否都执行完成，
#endif
    REJOIN_TYPE_ALL_CHANNELS,                //全信道模式
    REJOIN_TYPE_GOTO_TABLE_INDEX,            //跳转到其它索引，此时用retry_max_times当跳转索引号
    REJOIN_TYPE_REPEAT_COUNT_SET,            //设定后续rejoin重复次数
    REJOIN_TYPE_CHECK_REPEAT_COUNT_GOTO,     //检测重复次数，跳转到相应的index,如果超出重复次数，则不跳转，顺序执行。
    REJOIN_TYPE_STOP=0xFF                    //结束模式，结束时，
};
#define REJOIN_LIST_MAX_NUMBER                 (17U)
#define REJOIN_INTERVAL_TABLE_DEFAULT_SETTING  {{REJOIN_TYPE_CURRENT_CHANNEL,            0,16, 4000},/*0 第一次掉网*/     \
                                                {REJOIN_TYPE_EACH_CHANNEL_START_SET,     0,11,    0},/*1 从11信道开始*/   \
                                                {REJOIN_TYPE_EACH_CHANNEL,            2000, 2, 4000},/*2 每个信道试2次*/  \
                                                {REJOIN_TYPE_EACH_CHANNEL_STOP_CHECK,    0, 2,    0},/*3 判断是否跳转到2*/\
                                                {REJOIN_TYPE_REPEAT_COUNT_SET,           0, 6,    0},/*4 设定后续动作重复6次*/\
                                                {REJOIN_TYPE_CURRENT_CHANNEL,        30000,16,40000},/*5 */               \
                                                {REJOIN_TYPE_EACH_CHANNEL_START_SET,     0,11,    0},/*6 从11信道开始*/   \
                                                {REJOIN_TYPE_EACH_CHANNEL,            2000, 2, 4000},/*7 每个信道试2次*/  \
                                                {REJOIN_TYPE_EACH_CHANNEL_STOP_CHECK,    0, 7,    0},/*8 判断是否跳转到2*/\
                                                {REJOIN_TYPE_CHECK_REPEAT_COUNT_GOTO,    0, 5,    0},/*9 重复不到6次跳转到5*/ \
                                                {REJOIN_TYPE_CURRENT_CHANNEL,        60000,16,60000},/*10 */               \
                                                {REJOIN_TYPE_EACH_CHANNEL_START_SET,     0,11,    0},/*11 从11信道开始*/   \
                                                {REJOIN_TYPE_EACH_CHANNEL,            2000, 2, 4000},/*12 每个信道试2次*/  \
                                                {REJOIN_TYPE_EACH_CHANNEL_STOP_CHECK,    0,12,    0},/*13 判断是否跳转到2*/\
                                                {REJOIN_TYPE_GOTO_TABLE_INDEX,           0, 5,    0},/*14 */               \
                                                {REJOIN_TYPE_CURRENT_CHANNEL,          800, 2, 4000},/*15 */               \
                                                {REJOIN_TYPE_GOTO_TABLE_INDEX,           0,10,    0}}/*16 */

#define REJOIN_TRIGER_START_INDEX               (15U)   //特定触发时，开始执行rejoin表格的index起点号，用来做按键立即触发rejoin之类的动作。
#define SAVE_TRIGE_REJOIN_BREAKPOINT            //开要回连手动触发断点保存功能

#define USE_SECURY_REJOIN_ONLY                  //打开时只用加密方式rejoin，关闭时，会安全与非安全方式交替进行
/* 自定义类型区 ------------------------------------------------------------- */
typedef struct {
    uint8_t  type;                   //rejoin的类型，0当前信道，1全信道，
    uint32_t delay_ms;               //此rejoin延时多久执行，单位ms。
    uint8_t  retry_max_times;        //此种类型重试最大次数，
                                     //当type为 REJOIN_TYPE_GOTO_TABLE_INDEX 模式时，此值用来当跳转的索引号。
                                     //当type为REJOIN_TYPE_REPEAT_COUNT_SET时，此值为最大的重复次数。
                                     //当type为REJOIN_TYPE_EACH_CHANNEL_START_SET时，此值为起始信道设定值。
                                     //当type为REJOIN_TYPE_EACH_CHANNEL_STOP_CHECK时，此值用来当跳转的索引号。
    uint32_t retry_interval_time_ms; //此种类型重试间隔时间，单位ms。
} tRejoinInterval;

/* 全局变量区 --------------------------------------------------------------- */

/* 本地变量区 --------------------------------------------------------------- */
const static tRejoinInterval rejoin_interval_table[REJOIN_LIST_MAX_NUMBER] = REJOIN_INTERVAL_TABLE_DEFAULT_SETTING;
static uint8_t rejoin_table_current_index = 0;
static uint8_t rejoin_table_current_rejoin_try_times = 0;
static uint8_t rejoin_repeat_counter = 0;
#ifdef SAVE_TRIGE_REJOIN_BREAKPOINT   //保存触发回连断点
  static uint8_t rejoin_break_flag = 0;  //回连中断标志
  static uint8_t rejoin_table_break_index = 0;
  static uint8_t rejoin_table_break_rejoin_try_times = 0;
  static uint8_t rejoin_break_repeat_counter = 0;
#endif
#ifdef SUPPORT_EACH_CHANNELS_REJOIN    //支持每个信道单独rejoin
  static uint8_t current_channel = MIN_CHANNEL; //11-26信道，
  static uint8_t channel_counter = 0;           //记录已经偿试了多少个信道
#endif
/* 全局函数声明区 ----------------------------------------------------------- */

/* 本地函数声明区 ----------------------------------------------------------- */

/* 函数原型 ----------------------------------------------------------------- */

/**
//函数名：setRejoinIntervalTableIndex
//描述：设定当前rejoin index,默认从0开始。
//参数：current_index     (uint8_t [输入]，当前rejoin间隔表的index号)
//      current_try_times (uint8_t [输入]，当前rejoin模式第几次偿试)
//返回：void
*/
void setRejoinIntervalTableIndex(uint8_t current_index, uint8_t current_try_times)
{
    if (current_index >= REJOIN_LIST_MAX_NUMBER) //避免出错，正常不会有此情况出现
    {
        rejoin_table_current_index = 0;
        rejoin_table_current_rejoin_try_times = 0;
        rejoin_repeat_counter = 0;
    }
    else
    {
      rejoin_table_current_index = current_index;
      rejoin_table_current_rejoin_try_times = current_try_times;
      rejoin_repeat_counter = 0;
   #ifdef SAVE_TRIGE_REJOIN_BREAKPOINT   //保存触发回连断点
      if (0 == current_index) //初始化时的index号，初始化对应变量
      {
        rejoin_break_flag = 0;
        rejoin_table_break_index = rejoin_table_current_index;
        rejoin_table_break_rejoin_try_times = rejoin_table_current_rejoin_try_times;
        rejoin_break_repeat_counter = rejoin_repeat_counter;
      }
   #endif
    }
}

/**
//函数名：checkRejoinEventNeedToContinue
//描述：检查rejoin事件是否需要继续执行,
//参数：无
//返回：bool , true 需要继续触发; false 停止触发。
*/
bool checkRejoinEventNeedToContinue(void)
{
uint8_t flag_break = 0;
    if (rejoin_table_current_index >= REJOIN_LIST_MAX_NUMBER)
    {
        return false;
    }
    else
    {
      for (uint8_t i=0; i<REJOIN_LIST_MAX_NUMBER; i++)
      {
         flag_break = 1;
         if (rejoin_table_current_rejoin_try_times >= rejoin_interval_table[rejoin_table_current_index].retry_max_times)
         {
             rejoin_table_current_rejoin_try_times = 0;
             rejoin_table_current_index++;
         }
         if (rejoin_table_current_index >= REJOIN_LIST_MAX_NUMBER) //避免出错，正常不会有此情况出现
         {
           rejoin_table_current_index = 0;
           rejoin_table_current_rejoin_try_times = 0;
           rejoin_repeat_counter = 0;
         }
         if (REJOIN_TYPE_REPEAT_COUNT_SET == rejoin_interval_table[rejoin_table_current_index].type)
         {
          //此是设定重复断开始，与REJOIN_TYPE_CHECK_REPEAT_COUNT_GOTO配合使用。
          rejoin_repeat_counter = rejoin_interval_table[rejoin_table_current_index].retry_max_times;
          rejoin_table_current_index++; //接着从下一个index 开始执行
          rejoin_table_current_rejoin_try_times = 0;

          if (rejoin_table_current_index >= REJOIN_LIST_MAX_NUMBER)
          {//避免出错
             rejoin_table_current_index = 0;
             rejoin_repeat_counter = 0;
          }
          flag_break = 0; //需要此模式不能触发rejoin，继续分析下一模式。
         }
         if (REJOIN_TYPE_CHECK_REPEAT_COUNT_GOTO == rejoin_interval_table[rejoin_table_current_index].type)
         {
           //此是判断是否重复执行段还没完成，与REJOIN_TYPE_REPEAT_COUNT_SET配合使用。
           if (rejoin_repeat_counter)
           {
             rejoin_repeat_counter--;
           }
           if (rejoin_repeat_counter)
           {
             rejoin_table_current_index = rejoin_interval_table[rejoin_table_current_index].retry_max_times;
           }
           else
           {
             rejoin_table_current_index++; //接着从下一个index 开始执行
           }
           if (rejoin_table_current_index >= REJOIN_LIST_MAX_NUMBER)
           {//避免出错
              rejoin_table_current_index = 0;
              rejoin_repeat_counter = 0;
           }
           rejoin_table_current_rejoin_try_times = 0;
           flag_break = 0; //需要此模式不能触发rejoin，继续分析下一模式。
         }
       #ifdef SUPPORT_EACH_CHANNELS_REJOIN    //支持每个信道单独rejoin
         if (REJOIN_TYPE_EACH_CHANNEL_START_SET == rejoin_interval_table[rejoin_table_current_index].type)
         {
           //此是设定开始信道，与REJOIN_TYPE_EACH_CHANNEL_STOP_CHECK配合使用。
           current_channel = rejoin_interval_table[rejoin_table_current_index].retry_max_times;
           if ((current_channel < MIN_CHANNEL) || (current_channel > MAX_CHANNEL))
           {
              current_channel = MIN_CHANNEL;
           }
           channel_counter = 0; //单独指定信道记数归零
           rejoin_table_current_index++; //接着从下一个index 开始执行
           rejoin_table_current_rejoin_try_times = 0;

           if (rejoin_table_current_index >= REJOIN_LIST_MAX_NUMBER)
           {//避免出错
              rejoin_table_current_index = 0;
              rejoin_repeat_counter = 0;
           }
           flag_break = 0; //需要此模式不能触发rejoin，继续分析下一模式。
         }
         if (REJOIN_TYPE_EACH_CHANNEL_STOP_CHECK == rejoin_interval_table[rejoin_table_current_index].type)
         {
           //此是单独每个信道回连结束判断分析，与REJOIN_TYPE_EACH_CHANNEL_START_SET配合使用。
           current_channel++;
           if ((current_channel < MIN_CHANNEL) || (current_channel > MAX_CHANNEL))
           {
              current_channel = MIN_CHANNEL;
           }
           channel_counter++; //信道累计
           if (channel_counter > (MAX_CHANNEL - MIN_CHANNEL)) //所有信道都遍历，结束
           {
             rejoin_table_current_index++; //接着从下一个index 开始执行
             rejoin_table_current_rejoin_try_times = 0;
           }
           else
           {
             //跳转到指定的index执行单独每个信道回连
             rejoin_table_current_index = rejoin_interval_table[rejoin_table_current_index].retry_max_times;
             rejoin_table_current_rejoin_try_times = 0;
           }
           if (rejoin_table_current_index >= REJOIN_LIST_MAX_NUMBER)
           {//避免出错
              rejoin_table_current_index = 0;
              rejoin_repeat_counter = 0;
           }
           flag_break = 0; //需要此模式不能触发rejoin，继续分析下一模式。
         }
       #endif
         if (REJOIN_TYPE_GOTO_TABLE_INDEX == rejoin_interval_table[rejoin_table_current_index].type)
         {
            //当是跳转类型时，需要按设定的指针跳转到相应的index号。此类型对rejoin_repeat_counter不做处理。
         #ifdef SAVE_TRIGE_REJOIN_BREAKPOINT   //保存触发回连断点
           if ((rejoin_table_current_index >= REJOIN_TRIGER_START_INDEX) && (rejoin_break_flag != 0))
           {
             rejoin_break_flag = 0;
             rejoin_table_current_index = rejoin_table_break_index;
             rejoin_table_current_rejoin_try_times = rejoin_table_break_rejoin_try_times;
             rejoin_repeat_counter = rejoin_break_repeat_counter;
           }
           else
         #endif
           {
             rejoin_table_current_index = rejoin_interval_table[rejoin_table_current_index].retry_max_times;
             rejoin_table_current_rejoin_try_times = 0;
           }
             if (rejoin_table_current_index >= REJOIN_LIST_MAX_NUMBER)
             {//避免出错
                rejoin_table_current_index = 0;
                rejoin_repeat_counter = 0;
             }
             //rejoin_table_current_rejoin_try_times = 0;
             flag_break = 0; //需要此模式不能触发rejoin，继续分析下一模式。
         }
         if (REJOIN_TYPE_STOP == rejoin_interval_table[rejoin_table_current_index].type)
         {
             rejoin_table_current_index = REJOIN_LIST_MAX_NUMBER;
             rejoin_table_current_rejoin_try_times = 0;
             rejoin_repeat_counter = 0;
             return false; //结束
         }
         if (flag_break)
         {
            rejoinProcDebugPrintln("index=%d,retry=%d",rejoin_table_current_index,rejoin_table_current_rejoin_try_times);
            return true;
         }
      }
    }
    return true;
}

/**
//函数名：getNextRejoinDelayTimeMs
//描述：获取执行下一个rejoin的延时时间
//参数：无
//返回：uint32_t , 下一个rejoin延时的时间,单位ms。
*/
uint32_t getNextRejoinDelayTimeMs(void)
{
    if (rejoin_table_current_index >= REJOIN_LIST_MAX_NUMBER) //避免出错，正常不会有此情况出现
    {
        rejoin_table_current_index = 0;
        rejoin_repeat_counter = 0;
    }
    if (rejoin_table_current_rejoin_try_times > rejoin_interval_table[rejoin_table_current_index].retry_max_times)
    {
        rejoin_table_current_rejoin_try_times = 0;  //避免出错，正常不会有此情况出现
    }

    if(getPowerOnStartingFlag()) //jim add 20200716
    {
       return 5000; //delay 5 second
    }

    if (0 == rejoin_table_current_rejoin_try_times) //当前类型第一次触发，按类型切换时间执行。
    {
        return rejoin_interval_table[rejoin_table_current_index].delay_ms;
    }
    else
    {
        return rejoin_interval_table[rejoin_table_current_index].retry_interval_time_ms;
    }
}

/**
//函数名：updateRejoinTableIndex
//描述：执行完当前rejoin后，相应index号的运算。
//参数：无
//返回：void
*/
void updateRejoinTableIndex(void)
{
  rejoin_table_current_rejoin_try_times++;
}

/**
//函数名：getCurrentRejoinUseSecureMode
//描述：获取当前的rejoin是否需要采用加密的方式进行
//参数：无
//返回：bool , true 采用加密方式; false 采用非加密方式
//             (此部分还和SDK里的宏义定义有关，当没有定义EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_ALLOW_REJOINS_WITH_WELL_KNOWN_LINK_KEY
//             如果此时的TC LinkKey是5a69key时，只能采用加密方式rejoin。)
*/
bool getCurrentRejoinUseSecureMode(void)
{
#ifdef USE_SECURY_REJOIN_ONLY
  return true;
#else
    if (rejoin_table_current_rejoin_try_times % 2 == 0) //secure 和 unsecure交叉进行
    {
        return true;
    }
    else
    {
        return false;
    }
#endif
}

/**
//函数名：getCurrentRejoinUseAllChannelMode
//描述：获取当前rejoin是否采用全信道方式
//参数：无
//返回：bool , true 采用全信道方式; false 采用当前信道方式
*/
bool getCurrentRejoinUseAllChannelMode(void)
{
    if (rejoin_table_current_index >= REJOIN_LIST_MAX_NUMBER) //避免出错，正常不会有此情况出现
    {
        rejoin_table_current_index = 0;
        rejoin_table_current_rejoin_try_times = 0;
        rejoin_repeat_counter = 0;
    }
    if (REJOIN_TYPE_ALL_CHANNELS == rejoin_interval_table[rejoin_table_current_index].type)
    {
      return true;
    }
    else
    {
      return false;
    }
}

#ifdef SUPPORT_EACH_CHANNELS_REJOIN    //支持每个信道单独rejoin
/**
//函数名：getCurrentRejoinChannelMask
//描述：获取当前rejoin是否采用全信道方式
//参数：无
//返回：uint32_t , rejoin的信道MASK值。0：为当前信道 当前只支持11-26信道
*/
uint32_t getCurrentRejoinChannelMask(void)
{
    uint32_t mask = 0;
    if (rejoin_table_current_index >= REJOIN_LIST_MAX_NUMBER) //避免出错，正常不会有此情况出现
    {
        rejoin_table_current_index = 0;
        rejoin_table_current_rejoin_try_times = 0;
        rejoin_repeat_counter = 0;
    }
    if (REJOIN_TYPE_ALL_CHANNELS == rejoin_interval_table[rejoin_table_current_index].type)
    {
      mask = ALL_CHANNELS_MASK;
    }
    else if (REJOIN_TYPE_EACH_CHANNEL == rejoin_interval_table[rejoin_table_current_index].type)
    {
      mask = (1 << current_channel) & ALL_CHANNELS_MASK;
    }
    return mask;
}
#endif

/**
//函数名：checkNetworkStateAndTrigeRejoin
//描述：判断网络与设备类型，触发一特定rejoin动作。
//参数：无
//返回：void
*/
void checkNetworkStateAndTrigeRejoin(void)
{
  rejoinProcDebugPrintln("need to rejoin?");
  if (EMBER_END_DEVICE <= emAfCurrentZigbeeProNetwork->nodeType)    //判断设备是否为end device设备
  {
     if (!emberStackIsPerformingRejoin()) //设备没在执行rejoin
     {
       EmberNetworkStatus state = emberAfNetworkState();
       if (state == EMBER_JOINED_NETWORK_NO_PARENT)
       {
        #ifdef SAVE_TRIGE_REJOIN_BREAKPOINT   //保存触发回连断点
         if (rejoin_break_flag == 0)
         {
          rejoin_break_flag = 1;
          rejoin_table_break_index = rejoin_table_current_index;
          rejoin_table_break_rejoin_try_times = rejoin_table_current_rejoin_try_times;
          rejoin_break_repeat_counter = rejoin_repeat_counter;
         }
        #endif
         emberAfStopMoveCallback();
         setRejoinIntervalTableIndex(REJOIN_TRIGER_START_INDEX,0);  //rejoin 重定向到指定的rejoin列表的index号。
         emberAfStartMoveCallback();
       }
     }
  }
}
#endif
/***************************************** 文 件 结 束 ************************************************/
