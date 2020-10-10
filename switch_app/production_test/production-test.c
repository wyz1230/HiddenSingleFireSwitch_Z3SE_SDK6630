/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName:production-test.c
*Author : JimYao
*Version : 1.0
*Date : 2019-11-13
*Description: efr32方案产测代码处理入口
*History:
***************************************************************************************************/

/* 相关包含头文件区 --------------------------------------------------------- */
//#include "production-test.h"
#include "tiny_cli/tiny_cli.h"
#include "tiny_printf/tiny_printf.h"
#include "common-app.h"
#include "stack/include/mfglib.h"
#include "app/framework/security/af-security.h"
#include "md5_encryption/WjCryptLib_MD5.h"

#ifdef ORB_PRODUCTION_TEST_CODE
/* 宏定义区 ----------------------------------------------------------------- */
//debug开关设定
//#define PRODUCTION_TEST_DEBUG_ENABLE
#ifdef PRODUCTION_TEST_DEBUG_ENABLE
  #define DEBUG_STRING                          "ProductionTest-DB:"
  #define productionTestDebugPrint(...)         emberAfPrint(0xFFFF,DEBUG_STRING __VA_ARGS__)
  #define productionTestDebugPrintln(...)       emberAfPrintln(0xFFFF,DEBUG_STRING __VA_ARGS__)
#else
  #define productionTestDebugPrint(...)
  #define productionTestDebugPrintln(...)
#endif

#define RSP_OK_STRING                      "OK\r\n"
#define RSP_FAIL_STRING                    "Fail\r\n"
#define productionTestResponds(...)        CLI_printf("Rsp " __VA_ARGS__)//; CLI_printf("\r\n")

//#define AWAYS_IN_TEST_MODE              //使设备一直处于产测模式。为了方便测试产测功能用，正常功能不打开。正常产品一定需要关闭
//#define CACULATE_AUTHORIZATION_ENABLE   //是否使用授权码运算返回，自测获取授权码用。正常产品一定需要关闭
//#define AUTHORISATION_TEST_KEY          //验证制具授权公式用的特别key，验证后去掉此宏，变为产品的运算Key。正常产品一定需要关闭
#define PASS_AUTHORISATION_CHECK

//#define AUTO_INSTALL_AUTHORIZATION_CODE_AFTER_OTA   //打开此宏定义，支持设备上电在网时自动安装授权码，解决旧产品没授权码，通过ota后能正常使用
#ifdef AUTO_INSTALL_AUTHORIZATION_CODE_AFTER_OTA
  #define VALID_MODEL_ID_LIST_SIZE                  7   //特定支持上电在网自动授权的 model id

  #define VALID_MODEL_ID_LIST_TABLE_INIT          {{"2ae011fb6d0542f58705d6861064eb5f"},/*MixSwitch-LN 1路*/\
                                                   {"b11c438ea86f416b9026b2526b7abe84"},/*MixSwitch-LN 2路*/\
                                                   {"e8d667cb184b4a2880dd886c23d00976"},/*MixSwitch-LN 3路*/\
                                                   {"83b9b27d5ffb4830bf35be5b1023623e"},/*隐藏式智能开关（单火情景）*/\
                                                   {"9ea4d5d8778d4f7089ac06a3969e784b"},/*入墙式单火开关二路*/\
                                                   {"6c9bf098efce4fa097bd9eb1eac9d304"},/*零火情景开关*/\
                                                   {"fa45841808e54e9588e3c8e9a954208d"}/*MixSwitch系列情景面板*/};
#endif

//设定产测串口
#define PRODUCTION_TEST_SERIAL_PORT    BSP_SERIAL_APP_PORT
#define PRODUCTION_TEST_UART_BAUDRATE  (115200UL)

//设定进入产测按键口
#define TEST_PORT0                     gpioPortD  //(gpioPortA)
#define TEST_PIN0                      1   //5

#define TEST_PORT0_ACTIVE_LEVEL        (0) //(0) // //0低电平进入，1高电平进入

//设定产测空闲超时时间
#define PRODUCTION_MODE_TIMEOUT_MS     (60*1000)         //1分钟超时退出

#define MAX_POWER_VALUE                (10)              //RF最大发射功率

#define RF_TEST_DEFAULT_POWER_VALUE    (10)              //RF测试默认发射功率为最大值
#define RF_TEST_DEFAULT_CHANNEL        (19)              //RF测试默认信道

//频偏参数默认值定义，注意按产品的RF参数来设定，没定义默认值和产测没修改ctune值时，会按SDK里的默认设定处理。
#ifdef USED_CUSTOM_HFX_CTUNE_CODE
   #define CUSTOM_HFX_CTUNE_DEFAULT_VALUE        (0x75) //定义频偏参数默认值
#endif

// The max packet size for 802.15.4 is 128, minus 1 byte for the length, and 2 bytes for the CRC.
#define MAX_BUFFER_SIZE   125                             //RF测试发送包buffer

#define HEADER0  0x05
#define HEADER1  0xAA
#define HEADER2  0x05
#define HEADER3  0xAA

#define RSSI_PACKETS_MODE_ANY_PKTS      0        //任何数据包
#define RSSI_PACKETS_MODE_SPECIFIC_PKTS 1        //特定数据包

#define FILL_PACKETS_TYPE_SPECIFIC      0        //特定数据包填充
#define FILL_PACKETS_TYPE_RANDOM        1        //随机数填充发送包
#define FILL_PACKETS_TYPE_ACK_DONGLE_RF 2        //响应Dongle RF测试特定数据包填充

#define ADD_CLI_ITEMS(name, func, arg, argDesc, funcDesc) \
                     {name , func, arg, argDesc, funcDesc}\

#define ADD_SECTION(sectionName) \
                   {sectionName , NULL, NULL, NULL, NULL}\


#define TINY_CLI_END() 	{NULL, NULL, NULL, NULL, NULL}

//在production_test_mfglib_rf_flag变量用不同的bit表示rf测试不同的模式
#define MFGLIB_RF_RUNNING_BIT               0         //mfglib rf测试启动标志位，用bit0表示
#define MFGLIB_RF_TONE_SENDING_BIT          1         //mfglib rf测试TONE发送,非调制模式志位，用bit1表示
#define MFGLIB_RF_STREAM_SENDING_BIT        2         //mfglib rf测试Stream，调制模式志位，用bit2表示
#define MFGLIB_RF_RX_RSSI_TESTING_BIT       3         //mfglib rf测试接收RSSI，接收RSSI模式志位，用bit3表示
#define MFGLIB_RF_RX_TX_AUTO_TESTING_BIT    4         //mfglib rf测试收发自动动测试，收发测试模式志位，用bit4表示
#define MFGLIB_RF_JOINING_NW_BIT            7         //测试加网中，此时，其它bit需要关掉

#define set_bit(byte, bit)          (byte |= 1 << bit)
#define reset_bit(byte, bit)        (byte &= ~(1 << bit))
#define get_bit(byte, bit)          ((byte & (1 << bit)) >> bit)
/* 自定义类型区 ------------------------------------------------------------- */
enum {
  UNAUTHORIZATION = 0,    //非授权
  AUTHORIZATION,          //授权
};
enum {
  CHECK_AUTHORIZATION = 0,    //检验授权码
  CACULATE_AUTHORIZATION,     //运算授权码
};
/* 全局变量区 --------------------------------------------------------------- */
/**
//函数名：startJoinSpecificNetwork
//描  述：开始加入指定网络
//参  数：channel (uint8_t  [输入]，需要加入网络的信道)
//        pand_id (uint16_t [输入]，需要加入网络的pand id)
//返  回：EmberStatus EMBER_SUCCESS：正常启动；其它值：启动异常
*/
EmberStatus startJoinSpecificNetwork(uint8_t channel, uint16_t pand_id);

/* 本地变量区 --------------------------------------------------------------- */
static uint8_t  production_test_mode = 0;      //产测模式标志，0正常模式，1产测模式
static uint32_t production_test_last_time_ms; //产测模式下操作的最后一次时间，用来处理进入产测模式后超时退出

static uint8_t 	production_test_current_channel = RF_TEST_DEFAULT_CHANNEL;       //默认RF测试使用信道
static uint8_t 	production_test_current_power = RF_TEST_DEFAULT_POWER_VALUE;     //默认RF测试使用最大发射功率
static uint8_t  production_test_mfglib_rf_flag = 0x00;               //mfglib rf测试模式标志

static int32_t  rssi_total= 0;                                       //收到的累计rssi值
static uint16_t received_total_packets = 0;                          //收到的总包数
static uint16_t last_received_total_packets = 0;                     //最后处理超时时收到的总包数
static uint32_t last_received_time_ms;                               //最后处理超时时的时间，用于超时处理用。
static uint8_t  rssi_packet_mode = RSSI_PACKETS_MODE_ANY_PKTS;       //测试RF时，rssi接收的数据包的模式

static uint8_t  dut_mac[8];                           //被测产品的mac地址
static uint8_t  received_packets_data_length = 0;     //所收数据包的数据区的数据区长度值。
static uint16_t dut_need_to_send_packets_number = 0;  //DUT所需发包数

static uint8_t  authorization_state = UNAUTHORIZATION;              //授权状态，0为非授权，其它值已经授权

// Add 1 for the length byte which is at the start of the buffer.
ALIGNMENT(2)
static uint8_t   send_buff[MAX_BUFFER_SIZE + 1];

#ifdef AUTO_INSTALL_AUTHORIZATION_CODE_AFTER_OTA   //设备上电在网时自动安装授权码
static uint8_t auto_authorization_valid_model_id_list[VALID_MODEL_ID_LIST_SIZE][33] = VALID_MODEL_ID_LIST_TABLE_INIT;
#endif
/* 全局函数声明区 ----------------------------------------------------------- */
#ifdef USED_CUSTOM_HFX_CTUNE_CODE
void customRetryInitHFXO(void);
#endif

/* 本地函数声明区 ----------------------------------------------------------- */
#ifdef AUTO_INSTALL_AUTHORIZATION_CODE_AFTER_OTA   //设备上电在网时自动安装授权码，
 static void checkToInstallAuthorizationAfterOta(void);
 static void saveInvalidAuthorization(void);
#endif
static void enterProductionTestInit(void);
static void exitProductionTestProc(void);
static void CLI_printHexValue(uint8_t *data, uint8_t length, uint8_t mode);
static bool validateAuthorisationCode(bool check_or_get,uint8_t *pbuffer, uint16_t length);
static void saveCustomInstallCodeToToken(tTokenTypeCustomInstallCode *code);
static void getCustomInstallCodeFromToken(tTokenTypeCustomInstallCode *code);
static void saveCustomAuthorisationCodeToToken(tTokenTypeCustomAuthorisation *code);
static void getCustomAuthorisationCodeFromToken(tTokenTypeCustomAuthorisation *code);

static bool stopMfglibRfTestMode(void);
static bool startMfglibRfTestMode(void (*mfglibRxCallback)(uint8_t *packet, uint8_t linkQuality, int8_t rssi));
static bool mfgRfTestSendFillBufferCallback(uint8_t* buff, uint8_t length, uint8_t packet_type, uint16_t packets_number);
static void mfgRfTestRxHandler(uint8_t *packet,uint8_t linkQuality,int8_t rssi);
static void mfglibRfTestRxTimeoutProc(void);
///
static void tinyCLI_SwVersion(void);              //获取软件版本号
static void tinyCLI_HwVersion(void);              //获取硬件版本号
static void tinyCLI_SetGPIO(void);                //Gpio口输出设定与高低电平控制
static void tinyCLI_GetGPIO(void);                //Gpio口输入设定与状态获取
static void tinyCLI_SetHFXCtune(void);            //设定外部高速晶振频偏值
static void tinyCLI_GetHFXCtune(void);            //读取外部高速晶振频偏值
static void tinyCLI_SetTxPower(void);             //设定发射功率
static void tinyCLI_GetTxPower(void);             //获取发射功率
static void tinyCLI_SetChannel(void);             //设定RF信道
static void tinyCLI_TxTune(void);                 //开始未调制信号发送
static void tinyCLI_TxStream(void);               //开始发送调制信号
static void tinyCLI_StopTx(void);                 //停止发送
static void tinyCLI_TxPkt(void);                  //发送一定数量的数据包
static void tinyCLI_RxRssi(void);                 //rx接收测试
static void tinyCLI_JoinNWK(void);                //加入到特定Zigbee网络
static void tinyCLI_RfAutoTest(void);             //RF自动测试指令
static void tinyCLI_Sleep(void);                  //休眠测试
static void tinyCLI_GetMAC(void);                 //获取设备MAC地址
static void tinyCLI_GetModelID(void);             //获取ModelID
static void tinyCLI_InstallAuthorisation(void);   //设备授权
static void tinyCLI_GetAuthorisation(void);       //获取授权码
static void tinyCLI_DeleteAuthorisation(void);    //删除授权
static void tinyCLI_WriteInstallcode(void);       //写入安装码
static void tinyCLI_ReadInstallcode(void);        //读取安装码
static void tinyCLI_DeleteInstallcode(void);      //删除安装码
static void tinyCLI_WriteSerialNumber(void);      //写入序列号
static void tinyCLI_ReadSerialNumber(void);       //读取序列号
static void tinyCLI_DeleteSerialNumber(void);     //删除序列号
static void tinyCLI_FactoryReset(void);           //恢复出厂设置
//MixSwitch部分的测试
static void tinyCLI_GetWaysConfig(void);          //获取开关路数配置结果
static void tinyCLI_RelayControl(void);           //继电器控制


//命令表定义：命令名(不能有空格) - 命令函数名 - 参数类型 - 参数说明 - 命令功能描述
//---------------参数类型定义-----------------
//1 - 1字节整型，比如char 、uint8、 int8
//2 - 2字节整型，比如int16 、uint16
//4 - 4字节整型，比如int32 、uint32
//f - 浮点类型数据，比如3.14
//s - 字符串数据类型，结尾以\0结束符
//b - buffer HEX数据类型,第一个字节为buffer长度(CLI输入时不需要这个长度字段)
const CLI_cmdTable_t CLI_cmdTable[] =
{
	//模块分隔字符串
	ADD_SECTION("General Test"),

	//ADD_CLI_ITEMS("命令名(不能有空格)", 命令函数名, "参数类型", "参数说明", "命令功能描述"),
	ADD_CLI_ITEMS("Test-SwVersion",            tinyCLI_SwVersion,             NULL,   \
                  "null",                                "Get software version"),
    ADD_CLI_ITEMS("Test-HwVersion",            tinyCLI_HwVersion,             NULL,   \
                  "null",                                "Get hardware version"),
    ADD_CLI_ITEMS("Test-SetGPIO",              tinyCLI_SetGPIO,               "1111", \
                  "OuputMode(u8) 0:push-pull 1:open-drain; HighLow(u8) 1:high 0:low;" \
                  "GPIO-Port(u8) 0:A 1:B 2:C 3:D 4:E 5:F; GPIO-Pin(u8) 0-15",  "Gpio output control"),
    ADD_CLI_ITEMS("Test-GetGPIO",              tinyCLI_GetGPIO,               "111",  \
                  "InputMode(u8) 0:pull-up 1:pull-down 2:float 3:analog;"             \
                  "GPIO-Port(u8) 0:A 1:B 2:C 3:D 4:E 5:F; GPIO-Pin(u8) 0-15",  "Gpio state get"),
    ADD_CLI_ITEMS("Test-SetHFXCtune",          tinyCLI_SetHFXCtune,           "2",    \
                  "HFXCtune Value(u16)",                 "Set the HFX ctune value"),
    ADD_CLI_ITEMS("Test-GetHFXCtune",          tinyCLI_GetHFXCtune,           NULL,   \
                  "null",                                "Get the HFX ctune value"),
    ADD_CLI_ITEMS("Test-SetTxPower",           tinyCLI_SetTxPower,            "1",    \
                  "TxPower(int8) dBm",                   "Set the Tx Power"),
    ADD_CLI_ITEMS("Test-GetTxPower",           tinyCLI_GetTxPower,            NULL,   \
                  "null",                                "Get the Tx Power dBm"),
    ADD_CLI_ITEMS("Test-SetChannel",           tinyCLI_SetChannel,            "1",    \
                  "Channel(u8) 11-26",                   "Set the RF channel"),
    ADD_CLI_ITEMS("Test-TxTune",               tinyCLI_TxTune,                NULL,   \
                  "null",                                "Start Tx tune mode"),
    ADD_CLI_ITEMS("Test-TxStream",             tinyCLI_TxStream,              NULL,   \
                  "null",                                "Start TX stream mode"),
    ADD_CLI_ITEMS("Test-StopTx",               tinyCLI_StopTx,                NULL,   \
                  "null",                                "Stop TX"),
    ADD_CLI_ITEMS("Test-TxPkt",                tinyCLI_TxPkt,                 "2",    \
                  "PacketsNumber(u16)",                  "Send packets"),
    ADD_CLI_ITEMS("Test-RxRssi",               tinyCLI_RxRssi,                "1",    \
                  "PacketsMode(u8) 0:any 1:specifical",  "Get the packets average rssi"),
    ADD_CLI_ITEMS("Test-JoinNWK",              tinyCLI_JoinNWK,               "12",   \
                  "Channel(u8) PanId(u16)",              "Join to the specifical NW"),
    ADD_CLI_ITEMS("Test-RfAutoTest",           tinyCLI_RfAutoTest,            NULL,   \
                  "null",                                "Start RF auto RX/TX test"),
    ADD_CLI_ITEMS("Test-Sleep",                tinyCLI_Sleep,                "1",     \
                  "SleepMode(u8)",                       "Enter to sleep mode"),
    ADD_CLI_ITEMS("Test-GetMAC",               tinyCLI_GetMAC,                NULL,   \
                  "null",                                "Get the MAC"),
    ADD_CLI_ITEMS("Test-GetModelID",           tinyCLI_GetModelID,            NULL,   \
                  "null",                                "Get the model id"),
    ADD_CLI_ITEMS("Test-InstallAuthorisation", tinyCLI_InstallAuthorisation,  "b",    \
                  "AuthorisationCode(16B)",              "Install authorisation code"),
    ADD_CLI_ITEMS("Test-GetAuthorisation",     tinyCLI_GetAuthorisation,      NULL,   \
                  "null",                                "Get the authorisation result"),
    ADD_CLI_ITEMS("Test-DeleteAuthorisation",  tinyCLI_DeleteAuthorisation,   NULL,   \
                  "null",                                "Delete the authorisation code"),
    ADD_CLI_ITEMS("Test-WriteInstallcode",     tinyCLI_WriteInstallcode,      "b",    \
                  "InstallCode(16B+2B-CRC)",             "Write Installcode"),
    ADD_CLI_ITEMS("Test-ReadInstallcode",      tinyCLI_ReadInstallcode,       NULL,   \
                  "null",                                "Read Installcode"),
    ADD_CLI_ITEMS("Test-DeleteInstallcode",    tinyCLI_DeleteInstallcode,     NULL,   \
                  "null",                                "Delete Installcode"),
    ADD_CLI_ITEMS("Test-WriteSerialNumber",    tinyCLI_WriteSerialNumber,     "s",    \
                  "SerialNumber(16B String)",            "Write SerialNumber"),
    ADD_CLI_ITEMS("Test-ReadSerialNumber",     tinyCLI_ReadSerialNumber,      NULL,   \
                  "null",                                "Read SerialNumber"),
    ADD_CLI_ITEMS("Test-DeleteSerialNumber",   tinyCLI_DeleteSerialNumber,    NULL,   \
                  "null",                                "Delete SerialNumber"),
    ADD_CLI_ITEMS("Test-FactoryReset",         tinyCLI_FactoryReset,          NULL,   \
                  "null",                                "Reset to the factory default"),

    ADD_SECTION("MixSwitch Test"),

    ADD_CLI_ITEMS("Test-GetWaysConfig",        tinyCLI_GetWaysConfig,         NULL,   \
                  "null",                                "Get the ways config"),
    ADD_CLI_ITEMS("Test-RelayControl",         tinyCLI_RelayControl,          "11",   \
                  "RelayWay(u8)0-2; OnOff(u8) 0:Off 1:On", "Control the relay"),

	TINY_CLI_END()
};
/* 函数原型 ----------------------------------------------------------------- */
/**
//函数名：customeDelay(uint8_t time_ms)
//描述：延时函数
//参数：无
//返回：void
*/
static void customeDelay(uint8_t time_ms)
{
  uint32_t current_time_ms;
  current_time_ms = halCommonGetInt32uMillisecondTick();
  while(1)
  {
    halResetWatchdog();
    if (halCommonGetInt32uMillisecondTick() - current_time_ms > time_ms)
    {
      return;
    }
  }
}
/**
//函数名：deviceInfoPowerOnPrint
//描述：上电打印设备相关信息。
//参数：无
//返回：void
*/
static void deviceInfoPowerOnPrint(void)
{
#define PRINT_DELAY_EACH_LINE_TIME_MS  20
  EmberEUI64 eui64;
  uint8_t delay_flag = 0; //由于产测时，方便上位机解析，修改为每一个换行回车指令延时发送
  CLI_printf("\r\n");  //换行
//测试模式时，需要打印测试模式的提示
  if (inProductionTestMode())
  {
    customeDelay(PRINT_DELAY_EACH_LINE_TIME_MS);
    CLI_printf("TEST MODE \r\n");
    delay_flag = 1;
  }
  if (delay_flag) customeDelay(PRINT_DELAY_EACH_LINE_TIME_MS);
//固件命名规则
//产品名称_V主版本号.次版本号.修订版本号_日期_其他信息.后缀
  CLI_printf("FW  : %s_V%s_%s_%s.hex\r\n",
                    ORB_PRODUCT_NAME_STRING,
                    ORB_SW_BUILD_ID_STRING,
                    ORB_DATE_CODE_STRING,
                    ORB_OTHER_INFO_STRING);
  if (delay_flag) customeDelay(PRINT_DELAY_EACH_LINE_TIME_MS);
  //换行
//打印MAC地址
  emberAfGetEui64(eui64);
  CLI_printf("MAC : ");
  CLI_printHexValue(eui64, 8, 0);
  CLI_printf("\r\n");  //换行

  if (delay_flag) customeDelay(PRINT_DELAY_EACH_LINE_TIME_MS);

//打印是否受权，此功能当前还没完成。
  tTokenTypeCustomAuthorisation temp;
  getCustomAuthorisationCodeFromToken(&temp);

  if (validateAuthorisationCode(CHECK_AUTHORIZATION,temp.code, 16) == true)
  {
    //校验正确
    authorization_state = AUTHORIZATION;
  }
  else
  {
    //校验失败
    authorization_state = UNAUTHORIZATION;
  }
  CLI_printf("AUTH: %d\r\n",authorization_state == AUTHORIZATION ? 1:0);
}

/**
//函数名：checkToEnterProductionTest
//描  述：检测是否需要进入产测模式
//参  数：无
//返  回：void
*/
void checkToEnterProductionTest(void)
{
    GPIO_Mode_TypeDef pin0_mode;//jim GPIO_Mode_TypeDef pin0_mode,pin1_mode;
    unsigned int pin0_dout;//jim unsigned int pin0_dout,pin1_dout;
#ifdef USED_CUSTOM_HFX_CTUNE_CODE
   customRetryInitHFXO();  //重新初始化晶振参数
#endif
    //初始化IO口配置
  /* Enable GPIO in CMU */
  #if !defined(_SILICON_LABS_32B_SERIES_2)
    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_GPIO, true);
  #endif

    // 寄存器备份
    pin0_mode = GPIO_PinModeGet(TEST_PORT0, TEST_PIN0);
    pin0_dout = GPIO_PinInGet(TEST_PORT0, TEST_PIN0);

    GPIO_PinModeSet(TEST_PORT0, TEST_PIN0,
                    gpioModeInputPull,HAL_GPIO_DOUT_HIGH);
    GPIO_PinInGet(TEST_PORT0, TEST_PIN0);


    production_test_mode = 0;

    if (GPIO_PinInGet(TEST_PORT0, TEST_PIN0) == TEST_PORT0_ACTIVE_LEVEL)
    {
       //简单的延时处理
       volatile uint16_t delay = 0;
       while (delay < 100)
       {
         delay++;
       }
       if (GPIO_PinInGet(TEST_PORT0, TEST_PIN0) == TEST_PORT0_ACTIVE_LEVEL)
       {
         enterProductionTestInit();
       }
    }

    // 寄存器还原
    GPIO_PinModeSet(TEST_PORT0,TEST_PIN0,pin0_mode,pin0_dout);

  #ifdef AWAYS_IN_TEST_MODE //for test
    if (production_test_mode == 0)
    {
      enterProductionTestInit();
    }
  #endif

    deviceInfoPowerOnPrint();  //上电打印设备信息
}

/**
//函数名：enterProductionTestInit
//描  述：进入产测模式初始化
//参  数：无
//返  回：void
*/
static void enterProductionTestInit(void)
{
   production_test_mode = 1;    //进入产测模式
   production_test_last_time_ms = halCommonGetInt32uMillisecondTick();  //更新最后操作时间
   //初始化串口
   emberSerialInit(PRODUCTION_TEST_SERIAL_PORT, PRODUCTION_TEST_UART_BAUDRATE, PARITY_NONE, 1);
   networkStatusTrigeNetworkAction(NETWORK_ACTION_NONE);
   ledsAppChangeLedsStatus(LEDS_STATUS_POWER_ON_INIT);
}

/**
//函数名：exitProductionTestProc
//描  述：退出产测模式处理
//参  数：无
//返  回：void
*/
static void exitProductionTestProc(void)
{
   //还原串口初始化，按产品的波特率调整
   emberSerialInit(PRODUCTION_TEST_SERIAL_PORT, PRODUCTION_TEST_UART_BAUDRATE, PARITY_NONE, 1);
   stopMfglibRfTestMode();      //停止rf测试
   production_test_mode = 0;    //退出产测模式
   appPowerOnInit();            //设备重新初始化
   deviceInfoPowerOnPrint();    //打印设备信息
}

/**
//函数名：inProductionTestMode
//描  述：判断是否在产测模式
//参  数：无
//返  回：uint8_t , 0正常模式; 其它值 产测模式
*/
uint8_t inProductionTestMode(void)
{
  return production_test_mode;
}
/**
//函数名：authorizationPass
//描  述：判断是否授权通过
//参  数：无
//返  回：bool , true 已经授权; false 非授权
*/
bool authorizationPass(void)
{
#ifdef PASS_AUTHORISATION_CHECK
  return true;
#endif
  return authorization_state == AUTHORIZATION ? true:false;
}

/**
//函数名：_putchar
//描述：调用tiny print的输出入口函数，转换为efr32平台的串口输出
//参数：character (char [输入]，需要打印的字符)
//返回：void
*/
void _putchar(char character)
{
  emberSerialWriteData(PRODUCTION_TEST_SERIAL_PORT, (uint8_t*)&character, 1);
}

/**
//函数名：productionTestProc
//描述：产测处理入口函数
//参数：无
//返回：bool,
//           false 表示产测代码没处理，可以由SDK的CLI处理；
//           true  表示产测代码已经处理，跳过SDK的CLI处理。
*/
bool productionTestProc(void)
{
  uint32_t current_time_ms;
  uint8_t data,count;
  //EmberStatus status;
#ifdef AUTO_INSTALL_AUTHORIZATION_CODE_AFTER_OTA   //设备上电在网时自动安装授权码，
  static bool poweron_init = true;
  if (poweron_init == true)
  {
    checkToInstallAuthorizationAfterOta();       //上电在网自动安装授权码
    poweron_init = false;
  }
#endif
  if (production_test_mode == 0) //非产测模式，直接返回false，由SDK处理
  {
    return false;
  }

  current_time_ms = halCommonGetInt32uMillisecondTick();

  //获取串口数据
  count = 0;
  while (emberSerialReadByte(PRODUCTION_TEST_SERIAL_PORT, &data) == EMBER_SUCCESS)
  {
     CLI_writeRingBuffer(&data, 1);
     //CLI_stringInput(&data, 1);

     production_test_last_time_ms = current_time_ms;  //更新最后一次操作时间
     if ( ++count > 10)    //最大连续取串口数据10个
     {
       break;
     }
  }

  //处理收到的串口数据
  CLI_main(current_time_ms);

  //处理RF在rx rssi和rxtx自动测试时的超时处理
  mfglibRfTestRxTimeoutProc();

#ifndef AWAYS_IN_TEST_MODE //for test
  //产测模式超时退出
  if (elapsedTimeInt32u(production_test_last_time_ms,current_time_ms) > PRODUCTION_MODE_TIMEOUT_MS)
  {
    exitProductionTestProc();
  }
#endif
  return true;
}
/**
//函数名：CLI_printHexValue
//描述  ：打印hex数据组，按大写打印
//参数  ：*data (uint8_t * [输入]，需要打印的数组指针)
//        length (uint8_t  [输入]，需要打印的数组长度)
//        mode   (uint8_t  [输入]，需要打印的模式，0顺序，其它值倒序)
//返回  ：void
*/
static void CLI_printHexValue(uint8_t *data, uint8_t length, uint8_t mode)
{
  uint8_t i,j;
  for (i=0; i < length; i++)
  {
    if (mode == 0) //顺序
    {
      j = i;
    }
    else //倒序
    {
      j = length -1 - i;
    }
    if (data[j] < 0x10) //个位数补0
      CLI_printf("0");
    CLI_printf("%X",data[j]);
  }
}
#if 0
// Reverse the bits in a byte
static uint8_t reverse(uint8_t b)
{
#if defined(EZSP_HOST) || defined(BOARD_SIMULATION)
  return ((b * 0x0802UL & 0x22110UL) | (b * 0x8020UL & 0x88440UL)) * 0x10101UL >> 16;
#else
  return (__RBIT((uint32_t)b) >> 24); // Cortex function __RBIT uses uint32_t
#endif // EZSP_HOST
}
/**
//函数名：caculateBufferCrc16
//描  述：计算buffer数据的crc32值 POLYNOMIAL (0xEDB88320UL)
//参  数：*pbuffer     (uint8_t* [输入] 计算crc16的数据指针)
//        length       (uint8_t  [输入] 计算crc16的数据长度)
//返  回：uint16_t 运算后的crc16值
*/
// CRC-16/X-25  POLYNOMIAL 0x1021
static uint16_t caculateBufferCrc16(uint8_t *pbuffer, uint8_t length)
{
  uint16_t crc = 0xFFFF;

  for (uint8_t i = 0; i < length; i++)
  {
    crc = halCommonCrc16(reverse(pbuffer[i]), crc);
  }
  crc = ~HIGH_LOW_TO_INT(reverse(LOW_BYTE(crc)), reverse(HIGH_BYTE(crc)));

  return crc;
}
#endif
/**
//函数名：validateAuthorisationCode
//描  述：检验授权码处理
//参  数：check_or_get (bool     [输入]      0：检验授权码；1：运算授权码)
//        *pbuffer     (uint8_t* [输入/输出] 授权码数组指针)
//        length       (uint16_t [输入] 授权码数组长度)
//返  回：bool true:有效; false:无效
*/
static bool validateAuthorisationCode(bool check_or_get,uint8_t *pbuffer, uint16_t length)
{
//采用MD5码产生授权码，
//产生步骤:
//1-> 4F525649424F2D415554483A + MAC值(低端模式) 运算出md5值 MAC_MD5
//2-> 第一步运算出来的 MAC_MD5 每一个byte 和 固定的16字节数组FIX_ARRAY对应的byte做异或 得到新的数组 MAC_MD5_XOR
//3-> 用第二步得到的 MAC_MD5_XOR + 4F525649424F + FIX_ARRAY 再次运算出md5值，得到最终的授权码
#ifdef AUTHORISATION_TEST_KEY
  uint8_t fix_hex_array[16] = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
#else
  uint8_t fix_hex_array[16] = {0x11,0xD2,0x60,0x43,0x29,0x58,0x57,0x1A,0x67,0x45,0x15,0xD3,0x05,0xF1,0x08,0x07};
#endif
  uint8_t auth_array[48];
  char auth_prefix[]="ORVIBO-AUTH:";  //4F525649424F2D415554483A
  uint8_t length_temp=0;
  MD5Context      MD5Context;
  MD5_HASH        MD5Hash;
  EmberEUI64 eui64;

  if (length != 16) //长度固定16字节
    return false;

  emberAfGetEui64(eui64);
//第一步
  //在"ORVIBO-AUTH:"字串后面加上mac地址，十六进制小端模式，进行第一次md5运算
  length_temp = strlen(auth_prefix);
  memcpy(auth_array,auth_prefix,length_temp);
  //加上MAC值
  for (uint8_t i=0; i<8; i++)
  {
    auth_array[length_temp++] = eui64[i];
  }

 //运算出md5码值
  MD5Initialise( &MD5Context );
  MD5Update( &MD5Context, auth_array, (uint32_t)length_temp );
  MD5Finalise( &MD5Context, &MD5Hash );
//  CLI_printHexValue(&MD5Hash.bytes[0], sizeof(MD5Hash), 0);
//  CLI_printf("\r\n");

//第二步
  //在第一次运算出来的md5码值与固定数组fix_hex_array对应的byte做异或
  length_temp = sizeof(MD5Hash);
  for (uint8_t i=0; i<length_temp; i++)
  {
    auth_array[i] = MD5Hash.bytes[i] ^ fix_hex_array[i];
  }

//第三步
  //在第二次运算出来的数组值后面加上产商名再加上固定数组fix_hex_array，再做一次md5运算

  //加入产商名到运算后的数组后面
  memcpy(&auth_array[length_temp],"ORVIBO",6);  //4F525649424F
  length_temp += 6;
  //再加入固件的数组fix_hex_array
  memcpy(&auth_array[length_temp],fix_hex_array,sizeof(fix_hex_array));
  length_temp += sizeof(fix_hex_array);
  //运算出md5码值
  MD5Initialise( &MD5Context );
  MD5Update( &MD5Context, auth_array, (uint32_t)length_temp );
  MD5Finalise( &MD5Context, &MD5Hash );
//  CLI_printHexValue(&MD5Hash.bytes[0], sizeof(MD5Hash), 0);
//  CLI_printf("\r\n");


  if (check_or_get) //获取授权码
  {
    //复制授权码输出
    memcpy(pbuffer,&MD5Hash.bytes[0],length);
  }
  else  //检验授权码
  {
    //比较是否和授权码一致
    if (memcmp(pbuffer,&MD5Hash.bytes[0],length))
      return false;
  }
  return true;
}
#ifdef AUTO_INSTALL_AUTHORIZATION_CODE_AFTER_OTA   //设备上电在网时自动安装授权码，
/**
//函数名：checkToInstallAuthorizationAfterOta
//描  述：固件ota后，判断是否之前已经有安装授权码的，没有授权码的，需要依据特定的model id安装授权码
//        此函数需要在上电初始化协议栈后调用一次，上电马上判断是否已经在网
//参  数：无
//返  回：void
*/
static void checkToInstallAuthorizationAfterOta(void)
{
  if (UNAUTHORIZATION == authorization_state)
  {
    EmberNetworkStatus status = emberAfNetworkState(); //获取当前的网络状态
    if ((EMBER_JOINED_NETWORK == status) ||
        (EMBER_JOINED_NETWORK_NO_PARENT == status))
    {
      tTokenTypeCustomAuthorisation temp;
      getCustomAuthorisationCodeFromToken(&temp);
      //判断token里的授权码是否保存着特别的数组，000102030405060708090A0B0C0D0E0F,
      //如果是此值表示这个设备之前是通过产测方式入网的，不符合ota自动安装授权码条件
      uint8_t i;
      for (i=0; i<16; i++)
      {
        if(temp.code[i] != i)
          break;
      }
      if (i == 16)
      { //token中保存着特别的授权码值，非法方式，不允许自动授权。
        return;
      }

      //判断是否为有效的model id，有效的model id才允许自动授权。
      //先获取当前model id
      uint8_t model_id_temp[34];
      if (emberAfReadAttribute(emberAfEndpointFromIndex(0),
                                    ZCL_BASIC_CLUSTER_ID,
                                    ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID,
                                    CLUSTER_MASK_SERVER,
                                    model_id_temp,
                                    33,
                                    NULL) == EMBER_ZCL_STATUS_SUCCESS)
      {
        if (model_id_temp[0] < 33)
        {
          model_id_temp[model_id_temp[0]+1] = '\0';
        }
      }
      //比较是否为有效的model id
      uint8_t length=0;
      for (i=0; i<VALID_MODEL_ID_LIST_SIZE; i++)
      {
        length = strlen((const char*)&auto_authorization_valid_model_id_list[i][0]);
        if (length == model_id_temp[0])
        {
          if (memcmp(&model_id_temp[1],&auto_authorization_valid_model_id_list[i][0],length) == 0)
            break;
        }
      }
      if (i == VALID_MODEL_ID_LIST_SIZE) //没在有效的model id列表里 不允许自动授权。
         return;

      //运算授权码，自动授权
      if (validateAuthorisationCode(CACULATE_AUTHORIZATION,temp.code, 16) == true)
      {
        //运算授权码后，保存
        saveCustomAuthorisationCodeToToken((tTokenTypeCustomAuthorisation*)&temp);
        authorization_state = AUTHORIZATION;
      }
    }
  }
}
/**
//函数名：saveInvalidAuthorization
//描  述：在非授权模式下加入网络的，写入非法的授权码，以示区别。防止设备上电在网自动授权。
//参  数：无
//返  回：void
*/
static void saveInvalidAuthorization(void)
{
  if (UNAUTHORIZATION == authorization_state) //在非授权模式下加网的，写入非法的授权码。
  {
    tTokenTypeCustomAuthorisation temp;
    //判断token里的授权码是否保存着特别的数组，000102030405060708090A0B0C0D0E0F,
    //如果是此值表示这个设备之前是通过产测方式入网的，不符合ota自动安装授权码条件
    for (uint8_t i=0; i<16; i++)
    {
      temp.code[i] = i;
    }
    //保存非法的授权码值
    saveCustomAuthorisationCodeToToken((tTokenTypeCustomAuthorisation*)&temp);
  }
}
#endif
/**
//函数名：saveCustomInstallCodeToToken
//描述  ：保存客制的InstallCode到token中
//参数  ：*code (tTokenTypeCustomInstallCode *[输入]，需要存入的InstallCode数据指针)
//返回  ：void
*/
static void saveCustomInstallCodeToToken(tTokenTypeCustomInstallCode *code)
{
  tTokenTypeCustomInstallCode temp;
  for (uint8_t i=0; i<3; i++)   //多次比较提高可靠性
  {
    halCommonGetToken(&temp, TOKEN_CUSTOM_INSTALLCODE); //先获取token里的值
    if ((temp.length == code->length) &&
        (memcmp(&temp.code[0],&code->code[0],temp.length) == 0))
    {
      break; //数值一致
    }
    else
    {
       halCommonSetToken(TOKEN_CUSTOM_INSTALLCODE, code);  //更新值
    }
  }
}
/**
//函数名：getCustomInstallCodeFromToken
//描述  ：从token中获取InstallCode
//参数  ：*code (tTokenTypeCustomInstallCode *[输出]，需要读取的InstallCode数据存放指针)
//返回  ：void
*/
static void getCustomInstallCodeFromToken(tTokenTypeCustomInstallCode *code)
{
  tTokenTypeCustomInstallCode temp;
  for (uint8_t i=0; i<3; i++)   //多次比较提高可靠性
  {
    halCommonGetToken(&temp, TOKEN_CUSTOM_INSTALLCODE); //先获取token里的值
    halCommonGetToken(code, TOKEN_CUSTOM_INSTALLCODE);  //再次获取token里的值
    if ((temp.length == code->length) &&
        (memcmp(&temp.code[0],&code->code[0],code->length) == 0))
    {
      break; //数值一致 退出
    }
  }
}
/**
//函数名：saveSerialNumberToToken
//描述  ：保存序列号到token中
//参数  ：*sn (tTokenTypeSerialNumber *[输入]，需要存入的SN数据指针)
//返回  ：void
*/
static void saveSerialNumberToToken(tTokenTypeSerialNumber *sn)
{
  tTokenTypeSerialNumber temp;
  for (uint8_t i=0; i<3; i++)   //多次比较提高可靠性
  {
    halCommonGetToken(&temp, TOKEN_CUSTOM_SERIAL_NUMBER); //先获取token里的值
    if (memcmp(temp.sn,sn,17) == 0)  //16+结束符'\0'
    {
      break; //数值一致
    }
    else
    {
       halCommonSetToken(TOKEN_CUSTOM_SERIAL_NUMBER, sn);  //更新值
    }
  }
}
/**
//函数名：getSerialNumberToToken
//描述  ：从token中获取序列号
//参数  ：*sn (tTokenTypeSerialNumber *[输出]，需要读取的序列号数据存放指针)
//返回  ：void
*/
static void getSerialNumberFromToken(tTokenTypeSerialNumber *sn)
{
  tTokenTypeSerialNumber temp;
  for (uint8_t i=0; i<3; i++)   //多次比较提高可靠性
  {
    halCommonGetToken(&temp, TOKEN_CUSTOM_SERIAL_NUMBER); //先获取token里的值
    halCommonGetToken(sn, TOKEN_CUSTOM_SERIAL_NUMBER);    //再次获取token里的值
    if (memcmp(temp.sn,sn,17) == 0)   //16+结束符'\0'
    {
      break; //数值一致 退出
    }
  }
}
#ifdef USE_CUSTOM_INSTALLCODE_CODE
/**
//函数名：checkCustomInstallCodeAndChangeToKey
//描述  ：检验token中的InstallCode并转换为key
//参数  ：*key (EmberKeyData *[输出]，转换后的key值存放指针)
//返回  ：EmberStatus EMBER_SUCCESS：存在；其它值：不存在，或者不正确
*/
EmberStatus checkCustomInstallCodeAndChangeToKey(EmberKeyData *key)
{
  tTokenTypeCustomInstallCode temp;
  getCustomInstallCodeFromToken(&temp);
  return emAfInstallCodeToKey(&temp.code[0], temp.length, key);
}
#endif
/**
//函数名：saveCustomAuthorisationCodeToToken
//描述  ：保存客制的授权码到token中
//参数  ：*code (tTokenTypeCustomAuthorisation *[输入]，需要存入的授权数据指针)
//返回  ：void
*/
static void saveCustomAuthorisationCodeToToken(tTokenTypeCustomAuthorisation *code)
{
  tTokenTypeCustomAuthorisation temp;
  for (uint8_t i=0; i<3; i++)   //多次比较提高可靠性
  {
    halCommonGetToken(&temp, TOKEN_CUSTOM_AUTHORISATION_CODE); //先获取token里的值
    if (memcmp(&temp.code[0],&code->code[0],16) == 0)
    {
      break; //数值一致
    }
    else
    {
       halCommonSetToken(TOKEN_CUSTOM_AUTHORISATION_CODE, code);  //更新值
    }
  }
}
/**
//函数名：getCustomAuthorisationCodeFromToken
//描述  ：从token中获取授权码
//参数  ：*code (tTokenTypeCustomAuthorisation *[输出]，需要读取的授权数据存放指针)
//返回  ：void
*/
static void getCustomAuthorisationCodeFromToken(tTokenTypeCustomAuthorisation *code)
{
  tTokenTypeCustomAuthorisation temp;
  for (uint8_t i=0; i<3; i++)   //多次比较提高可靠性
  {
    halCommonGetToken(&temp, TOKEN_CUSTOM_AUTHORISATION_CODE); //先获取token里的值
    halCommonGetToken(code, TOKEN_CUSTOM_AUTHORISATION_CODE);  //再次获取token里的值
    if (memcmp(&temp.code[0],&code->code[0],16) == 0)
    {
      break; //数值一致 退出
    }
  }
}
#ifdef USED_CUSTOM_HFX_CTUNE_CODE
/**
//函数名：saveCustomHfxCtuneToToken
//描述  ：保存客制的高频外部晶振HFX的频偏值Ctune到token中
//参数  ：ctune (uint16_t [输入]，需要存入的高频外部晶振HFX的频偏值)
//返回  ：void
*/
static void saveCustomHfxCtuneToToken(uint16_t ctune)
{
  uint16_t temp;
  for (uint8_t i=0; i<3; i++)   //多次比较提高可靠性
  {
    halCommonGetToken(&temp, TOKEN_CUSTOM_HFX_CTUNE); //先获取token里的值
    if (temp == ctune)
    {
      break; //数值一致
    }
    else
    {
       halCommonSetToken(TOKEN_CUSTOM_HFX_CTUNE, &ctune);  //更新值
    }
  }
}
/**
//函数名：getCustomHfxCtuneFromToken
//描述  ：从token中获取客制的高频外部晶振HFX的频偏值Ctune
//参数  ：*ctune (uint16_t *[输出]，需要读取的高频外部晶振HFX的频偏值数据存放指针)
//返回  ：void
*/
static void getCustomHfxCtuneFromToken(uint16_t *ctune)
{
  uint16_t temp;
  for (uint8_t i=0; i<3; i++)   //多次比较提高可靠性
  {
    halCommonGetToken(&temp, TOKEN_CUSTOM_HFX_CTUNE); //先获取token里的值
    halCommonGetToken(ctune, TOKEN_CUSTOM_HFX_CTUNE);  //再次获取token里的值
    if (temp == *ctune)
    {
      break; //数值一致 退出
    }
  }
}
/**
//函数名：customGetHfxCtuneFromProductionTestToken
//描  述：获取产测自定的HFX的设定频偏值
//参  数：*ready_ctune (uint16_t * [输出] 频偏值)
//返  回：bool true,存在频偏值；false,不存在。
*/
bool customGetHfxCtuneFromProductionTestToken(uint16_t *ready_ctune)
{
  uint16_t HFX_ctune;
  getCustomHfxCtuneFromToken(&HFX_ctune);
  if (HFX_ctune != 0xFFFF)
  {
    *ready_ctune = HFX_ctune;
    return true;
  }
#ifdef CUSTOM_HFX_CTUNE_DEFAULT_VALUE  //如果有定义CTUNE默认值，没设定频偏值时，按默认值设定。
  if (CUSTOM_HFX_CTUNE_DEFAULT_VALUE != HFX_ctune) //默认值不是0xFFFF时
  {
    *ready_ctune = CUSTOM_HFX_CTUNE_DEFAULT_VALUE;
    return true;
  }
#endif
  return false;
}
#endif
/**
//函数名：stopMfglibRfTestMode
//描  述：停止Mfglib的RF测试模式
//参  数：无
//返  回：bool，true 成功停止；false 停止失败
*/
static bool stopMfglibRfTestMode(void)
{
  uint8_t i;
  bool ret_result = true;
  //停止加网检测模式
  reset_bit(production_test_mfglib_rf_flag,MFGLIB_RF_JOINING_NW_BIT);
  //停止RSSI测试模式
  reset_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_RSSI_TESTING_BIT);
  //停止RX TX自动测试模式
  reset_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_TX_AUTO_TESTING_BIT);
  //停止tone模式
  for(i=0; i<3; i++)
  {
    if (get_bit(production_test_mfglib_rf_flag, MFGLIB_RF_TONE_SENDING_BIT) == 0)
    {
      break;
    }
    if (mfglibStopTone() == EMBER_SUCCESS)
    {
      reset_bit(production_test_mfglib_rf_flag, MFGLIB_RF_TONE_SENDING_BIT);
      break;
    }
  }
  if (i == 3)
  {
    ret_result = false;
  }
  //停止Stream模式
  for(i=0; i<3; i++)
  {
    if (get_bit(production_test_mfglib_rf_flag, MFGLIB_RF_STREAM_SENDING_BIT) == 0)
    {
      break;
    }
    if (mfglibStopStream() == EMBER_SUCCESS)
    {
      reset_bit(production_test_mfglib_rf_flag, MFGLIB_RF_STREAM_SENDING_BIT);
      break;
    }
  }
  if (i == 3)
  {
    ret_result = false;
  }
  //结束mfglib的rf测试模式
  for(i=0; i<3; i++)
  {
    if (get_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RUNNING_BIT) == 0)
    {
      break;
    }
    if (mfglibEnd() == EMBER_SUCCESS)
    {
      reset_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RUNNING_BIT);
      break;
    }
  }
  if (i == 3)
  {
    ret_result = false;
  }
  networkStatusTrigeNetworkAction(NETWORK_ACTION_NONE);//需要先停止加网的部分
  return ret_result;
}
/**
//函数名：startMfglibRfTestMode
//描  述：启动Mfglib的RF测试模式
//参  数：void(*mfglibRxCallback) (void(*)(u8*,u8*,i8) [输出] RF测试接收回调函数)
//返  回：bool，true 成功启动；false 启动失败
*/
static bool startMfglibRfTestMode(void (*mfglibRxCallback)(uint8_t *packet, uint8_t linkQuality, int8_t rssi))
{
  bool ret_result = true;
  uint8_t i;
  networkStatusTrigeNetworkAction(NETWORK_ACTION_NONE);//需要先停止加网的部分
  //停止加网检测模式
  reset_bit(production_test_mfglib_rf_flag,MFGLIB_RF_JOINING_NW_BIT);
  for(i=0; i<3; i++)
  {
    if (get_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RUNNING_BIT))
    {
      break;
    }
    if (mfglibStart(mfglibRxCallback) == EMBER_SUCCESS)
    {
      set_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RUNNING_BIT);
      break;
    }
  }
  if (i == 3)
  {
    ret_result = false;
  }
  //设定信道
  if (mfglibSetChannel(production_test_current_channel) != EMBER_SUCCESS)
  {
    ret_result = false;
  }
  //设定发射功率
  if (mfglibSetPower(EMBER_TX_POWER_MODE_DEFAULT, production_test_current_power) != EMBER_SUCCESS)
  {
    ret_result = false;
  }
  return ret_result;
}
/**
//函数名：mfgRfTestSendFillBufferCallback
//描述：RF测试发送数据包填充的数据包函数
//参数：*buff (uint8_t* [输出],要发送的数据包buff);
//      length (uint8_t [输入],要发送的数据区的长度，默认的数据包长度需要加上15个bytes);
//      packet_type (uint8_t [输入],0特定数据; 1为随机; 2响应dongleRF测试包);
//      packets_number (uint16_t [输入],需要发送的数据包数);
//返回：bool 返回0时没有填充处理，返回1时填充处理完成
*/
static bool mfgRfTestSendFillBufferCallback(uint8_t* buff, uint8_t length, uint8_t packet_type, uint16_t packets_number)
{
  uint8_t i = 0,length_temp = 0;
  EmberEUI64 eui64;

  if (packet_type == FILL_PACKETS_TYPE_RANDOM)
  {
    buff[0] = length;
    for (i = 1; i < length; i += 2)
    {
      uint16_t randomNumber = emberGetPseudoRandomNumber();
      buff[i] = (uint8_t)(randomNumber & 0xFF);
      buff[i + 1] = (uint8_t)((randomNumber >> 8)) & 0xFF;
    }
    //return 1; //由SDK原代码处理
  }
  else
  {
   //----数据包格式----//
   //--数据包长度(1byte)--|--帧头0x05AA05AA(4bytes)--|--MAC(8Bytes)--|
   //--DUT所需发包数(2bytes)--|--DUT有效收包数(2bytes)--|--DUT有效接收数据包平均RSSI(1byte)--|
   //--数据区长度(1byte)--|--数据区(0-105bytes)--|--CRC(2bytes)--//
   //最小数据包有20个字节
    length_temp = length;
    if (length_temp > 105)
    {
      length_temp = 105;
    }
    buff[0] = length_temp + 20;
    //填充帧头
    buff[1] = HEADER0;
    buff[2] = HEADER1;
    buff[3] = HEADER2;
    buff[4] = HEADER3;
    //填充本地MAC地址
    emberAfGetEui64(eui64);
    for (i=0; i<8; i++)
    {
      buff[5+i] = eui64[i];
    }

    //填充DUT所需发包数,和Dongle发射端采用同样的发包数。
      buff[13] = 0x00;
      buff[14] = 0x00;
    if (packet_type == FILL_PACKETS_TYPE_SPECIFIC) //特定数据包
    {
    //填充DUT有效收包数
      buff[15] = 0x00;
      buff[16] = 0x00;
    }
    else //Dongle RF测试响应包
    {
    //填充DUT有效收包数
      buff[15] = received_total_packets & 0x00FF;
      buff[16] = (received_total_packets >> 8) & 0x00FF;
    }

    //填充DUT有效接收数据包平均RSSI
      buff[17] = (int8_t)rssi_total;

    //填充数据包长度
    buff[18] = length_temp;
    //填充数据区
    length_temp++;
    for (i=1; i<length_temp; i++)
    {
      buff[i+18] = i;
    }
    //crc不需要填充,sdk底层填充。
  }
  return 1;
}

/**
//函数名：mfgRfTestRxHandler
//描述：在manufacturing-library-cli-soc.c里的添加接收callback,用来处理相关的数据包接收打印。
//参数：*packet     (uint8_t*[输入],接收到的数据包，第一个字节是长度;)
//      linkQuality (uint8_t[输入], LQI)
//      rssi        (int8_t[输入], RSSI)
//返回：void
*/
static void mfgRfTestRxHandler(uint8_t *packet,uint8_t linkQuality,int8_t rssi)
{
  if (get_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_RSSI_TESTING_BIT) &&
      (rssi_packet_mode == RSSI_PACKETS_MODE_ANY_PKTS))
  {
    rssi_total += rssi;
    if (received_total_packets < 0xFFFF)
    {
      received_total_packets++;
    }
  }
  else if (get_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_TX_AUTO_TESTING_BIT) ||
           (get_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_RSSI_TESTING_BIT) &&
            (rssi_packet_mode == RSSI_PACKETS_MODE_SPECIFIC_PKTS)))
  {
    uint8_t i = 0;
    //----数据包格式----//
     //--数据包长度(1byte)--|--帧头0x05AA05AA(4bytes)--|--MAC(8Bytes)--|
     //--DUT所需发包数(2bytes)--|--DUT有效收包数(2bytes)--|--DUT有效接收数据包平均RSSI(1byte)--|
     //--数据区长度(1byte)--|--数据区(0-105bytes)--|--CRC(2bytes)--//
    if (packet[0] >= 20) //最小数据包有20个字节
    {
      if ((HEADER0 == packet[1]) && (HEADER1 == packet[2]) &&
          (HEADER2 == packet[3]) && (HEADER3 == packet[4])) //有效数据帧头
      {
        if (0x00 == received_total_packets) //第一包数据，保存MAC地址
        {
          memcpy(dut_mac,packet+5,8);
        }
        if (memcmp(dut_mac,packet+5,8) == 0x00) //相同的MAC
        {
          for(i=0; i<packet[18]; i++) //检测数据区是否正常
          {
            if ((i+1) != packet[i+19])
              break;
          }
          if (i == packet[18]) //接收到正确的数据包
          {
            if (0x00 == received_total_packets)
            {
              received_packets_data_length = packet[18];
              dut_need_to_send_packets_number = ((uint16_t)packet[14] << 8) + (uint16_t)packet[13];
            }
            rssi_total += (int32_t)rssi;
            if (received_total_packets < 0xFFFF)
            {
              received_total_packets++;
            }
          }
        }
      }
    }
  }
}
/**
//函数名：mfglibRfTestRxTimeoutProc
//描  述：RF的rx测试rssi和rxtx自动测试模式超时处理
//参  数：无
//返  回：void
*/
static void mfglibRfTestRxTimeoutProc(void)
{
  uint32_t current_time_ms;
  uint16_t timeout_ms = 100;  //两包数据间隔默认时间100ms
  //正在RX的RSSI测试模式 //正在RX-TX的自动测试模式
  if (get_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_RSSI_TESTING_BIT) ||
      get_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_TX_AUTO_TESTING_BIT))
  {
    current_time_ms = halCommonGetInt32uMillisecondTick();
    if (last_received_total_packets != received_total_packets)
    { //有收到新数据包，更新处理数据包时间。
       last_received_time_ms = current_time_ms;
       last_received_total_packets = received_total_packets;
    }
    else
    { //没有收到新数据包，判断数据包接收是否超时。

       if (last_received_total_packets == 0)
       {
         timeout_ms = 3000;  //没数据包，超时时间3秒
       }
       if (elapsedTimeInt32u(last_received_time_ms,current_time_ms) > timeout_ms) //两数据包100ms超时
       {
         if (received_total_packets)
         {
           rssi_total = rssi_total/(int32_t)received_total_packets;
           if (get_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_RSSI_TESTING_BIT))
           { //正在RX的RSSI测试模式
             productionTestResponds("%ld %ld\r\n",received_total_packets,rssi_total);
           }
           else if (get_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_TX_AUTO_TESTING_BIT))
           { //正在RX-TX的自动测试模式
             mfgRfTestSendFillBufferCallback(send_buff, received_packets_data_length, FILL_PACKETS_TYPE_ACK_DONGLE_RF, dut_need_to_send_packets_number);
             if (mfglibSendPacket(send_buff, dut_need_to_send_packets_number - 1) == EMBER_SUCCESS)
             {
               productionTestResponds(RSP_OK_STRING);
             }
             else
             {
               productionTestResponds(RSP_FAIL_STRING);
             }
           }
         }
         else
         {
           //无数据包处理，需要响应错误
           productionTestResponds(RSP_FAIL_STRING);
         }
         reset_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_RSSI_TESTING_BIT);
         reset_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_TX_AUTO_TESTING_BIT);
         stopMfglibRfTestMode();
       }
    }
  }
  else if (get_bit(production_test_mfglib_rf_flag,MFGLIB_RF_JOINING_NW_BIT) &&
           get_bit(production_test_mfglib_rf_flag,MFGLIB_RF_RUNNING_BIT) == 0)
  {
    //正在加网过程，判断是否已经成功加入网络，做消息反馈
    current_time_ms = halCommonGetInt32uMillisecondTick();
    if (elapsedTimeInt32u(last_received_time_ms,current_time_ms) > 3000)
    {
       reset_bit(production_test_mfglib_rf_flag,MFGLIB_RF_JOINING_NW_BIT);
       productionTestResponds(RSP_FAIL_STRING);
    }
    else
    {
      if (emberAfNetworkState() == EMBER_JOINED_NETWORK)
      {
        reset_bit(production_test_mfglib_rf_flag,MFGLIB_RF_JOINING_NW_BIT);
        productionTestResponds("%d 0x%4X\r\n",emberGetRadioChannel(),emberAfGetPanId());
        //emberAfGetNodeId();
        //emberAfGetPanId();
        #ifdef AUTO_INSTALL_AUTHORIZATION_CODE_AFTER_OTA   //设备上电在网时自动安装授权码，
          saveInvalidAuthorization();  //设备加入网络后，没授权的需要写入非法的授权码，防止上电自动授权。
        #endif
      }
    }
  }
}

/**
//函数名：tinyCLI_SwVersion
//描  述：获取软件版本号，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_SwVersion(void)
{
  //回复参数1 ：3段式版本号
  //成功:Rsp 1.0.0
  //失败:Rsp Fail
  productionTestResponds(ORB_SW_BUILD_ID_STRING "\r\n");

}

/**
//函数名：tinyCLI_HwVersion
//描  述：获取硬件版本号，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_HwVersion(void)
{
  //回复参数1 ：2段式版本号
  //成功:Rsp 1.1
  //失败:Rsp Fail
  productionTestResponds(RSP_FAIL_STRING);
}

/**
//函数名：tinyCLI_SetGPIO
//描  述：Gpio口输出控制，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_SetGPIO(void)
{
  //参数1：u8 GPIO模式(0-推挽输出；1-开漏输出)；
  //参数2：u8 高(1)低(0)电平
  //参数3：u8 GPIO PORT(0-GPIO_A;1-GPIO_B;2-GPIO_C;3-GPIO_D;4-GPIO_E;5-GPIO_F)
  //参数4：u8 GPIO PIN(0-15, 引脚号)
  //成功:Rsp OK
  //失败:Rsp Fail
  uint8_t	control_mode = *(uint8_t *)CLI_getArguments(1);
  uint8_t gpio_value = *(uint8_t *)CLI_getArguments(2);
  uint8_t gpio_port = *(uint8_t *)CLI_getArguments(3);
  uint8_t	gpio_pin = *(uint8_t *)CLI_getArguments(4);
  GPIO_Mode_TypeDef	config = gpioModePushPull;

  if (GPIO_PORT_PIN_VALID(gpio_port,gpio_pin) == 0 ||
      gpio_value > 1 ||
      control_mode > 1)
  {
    //超过引脚定义范围
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }

  if (control_mode == 0)	   //推挽输出
  {
    config = gpioModePushPull;
  }
  else                       //开漏输出
  {
    config = gpioModeWiredAnd;
  }
#if !defined(_SILICON_LABS_32B_SERIES_2)
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
#endif
  //配置gpio口
  GPIO_PinModeSet((GPIO_Port_TypeDef)gpio_port, gpio_pin,
                  config, gpio_value);
  //控制gpio口高低电平
  if (gpio_value == 0)
  {
     GPIO_PinOutClear((GPIO_Port_TypeDef)gpio_port, gpio_pin);
  }
  else
  {
     GPIO_PinOutSet((GPIO_Port_TypeDef)gpio_port, gpio_pin);
  }
  productionTestResponds(RSP_OK_STRING);
}

/**
//函数名：tinyCLI_GetGPIO
//描  述：Gpio口输入设定及状态获取，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_GetGPIO(void)
{
  //参数1：u8 GPIO模式(0-上拉输入；1-下拉输入；2-浮空输入；3-模拟输入)；
  //参数3：u8 GPIO PORT(0-GPIO_A;1-GPIO_B;2-GPIO_C;3-GPIO_D;4-GPIO_E;5-GPIO_F)
  //参数4：u8 GPIO PIN(0-15, 引脚号)
  //回复参数1 ：高(1)低(0)电平
  //成功:Rsp 1
  //失败:Rsp Fail
  uint8_t	control_mode = *(uint8_t *)CLI_getArguments(1);
  uint8_t gpio_port = *(uint8_t *)CLI_getArguments(2);
  uint8_t	gpio_pin = *(uint8_t *)CLI_getArguments(3);
  GPIO_Mode_TypeDef	config = gpioModeInput;
  uint8_t gpio_value = 0;

  if (GPIO_PORT_PIN_VALID(gpio_port,gpio_pin) == 0 ||
      control_mode > 3)
  {
    //超过引脚定义范围
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
#if !defined(_SILICON_LABS_32B_SERIES_2)
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
#endif
  if (control_mode == 0)	       //上拉输入 gpioModeInputPull
  {
    config = gpioModeInputPull;
    gpio_value = 1;              //上拉
  }
  else if (control_mode == 1)	   //下拉输入 gpioModeInputPull
  {
    config = gpioModeInputPull;
    gpio_value = 0;              //下拉
  }
  else if (control_mode == 2)    //浮空输入 gpioModeInput
  {
    config = gpioModeInput;
    gpio_value = GPIO_PinOutGet((GPIO_Port_TypeDef)gpio_port, gpio_pin);
  }
  else                           //模拟输入 gpioModeDisabled
  {
    config = gpioModeDisabled;
    gpio_value = GPIO_PinOutGet((GPIO_Port_TypeDef)gpio_port, gpio_pin);
  }

  //配置gpio口
  GPIO_PinModeSet((GPIO_Port_TypeDef)gpio_port, gpio_pin,
                  config, gpio_value);
  //获取gpio口高低电平
  gpio_value = GPIO_PinInGet((GPIO_Port_TypeDef)gpio_port, gpio_pin);

  productionTestResponds("%d\r\n",gpio_value);
}

/**
//函数名：tinyCLI_SetHFXCtune
//描  述：设定高频外部晶振HFX的频偏值Ctune，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_SetHFXCtune(void)
{
  uint16_t HFX_ctune;
  HFX_ctune = *(uint16_t *)CLI_getArguments(1);
#ifdef USED_CUSTOM_HFX_CTUNE_CODE
  saveCustomHfxCtuneToToken(HFX_ctune);
  customRetryInitHFXO();
  productionTestResponds(RSP_OK_STRING);
#else
  productionTestResponds(RSP_FAIL_STRING);
#endif
}
/**
//函数名：tinyCLI_GetHFXCtune
//描  述：获取高频外部晶振HFX的频偏值Ctune，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_GetHFXCtune(void)
{
#ifdef USED_CUSTOM_HFX_CTUNE_CODE
  uint16_t HFX_ctune;
  getCustomHfxCtuneFromToken(&HFX_ctune);
  #ifdef CUSTOM_HFX_CTUNE_DEFAULT_VALUE  //如果有定义CTUNE默认值，没设定频偏值时，按默认值设定。
  if (0xFFFF == HFX_ctune)
  {
    productionTestResponds("default 0x%X\r\n", CUSTOM_HFX_CTUNE_DEFAULT_VALUE);
  }
  else
  #endif
  {
    productionTestResponds("0x%X\r\n", HFX_ctune);
  }

#else
  productionTestResponds(RSP_FAIL_STRING);
#endif
}
/**
//函数名：tinyCLI_SetTxPower
//描  述：设定发射功率，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_SetTxPower(void)
{
  //参数1：int8 发射功率
  //成功:Rsp OK
  //失败:Rsp Fail
  int8_t power;
  uint8_t mfglib_rf_flag_temp;
  power = *(int8_t *)CLI_getArguments(1);
  if (power > MAX_POWER_VALUE)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //正在测试RX和RX-TX自动测试时，不能调整功率
  if (get_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_RSSI_TESTING_BIT) ||
      get_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_TX_AUTO_TESTING_BIT))
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //暂存之前模式标志
  mfglib_rf_flag_temp = production_test_mfglib_rf_flag;
  //更新功率变量
  production_test_current_power = power;

  //停止MfgRf测试
  if (stopMfglibRfTestMode() == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //启动MfgRf测试，设定好相应信道，发射功率
  if (startMfglibRfTestMode(NULL) == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //之前正在发送stream，重新开始发送stream
  if (get_bit(mfglib_rf_flag_temp, MFGLIB_RF_STREAM_SENDING_BIT))
  {
    if (mfglibStartStream() == EMBER_SUCCESS)
    {
      set_bit(production_test_mfglib_rf_flag, MFGLIB_RF_STREAM_SENDING_BIT);
    }
    else
    {
      productionTestResponds(RSP_FAIL_STRING);
      return;
    }
  }
  //之前正在发送tone，重新开始发送tone
  else if (get_bit(mfglib_rf_flag_temp, MFGLIB_RF_TONE_SENDING_BIT))
  {
    if (mfglibStartTone() == EMBER_SUCCESS)
    {
      set_bit(production_test_mfglib_rf_flag, MFGLIB_RF_TONE_SENDING_BIT);
    }
    else
    {
      productionTestResponds(RSP_FAIL_STRING);
      return;
    }
  }
  productionTestResponds(RSP_OK_STRING);
}
/**
//函数名：tinyCLI_GetTxPower
//描  述：获取发射功率，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_GetTxPower(void)
{
  //成功:Rsp int8 发射功率
  //失败:Rsp Fail
  int8_t power;
  power = emberGetRadioPower();
  productionTestResponds("%d\r\n",power);
}
/**
//函数名：tinyCLI_SetChannel
//描  述：设定RF信道，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_SetChannel(void)
{
  //参数1：uint8 信道
  //成功:Rsp OK
  //失败:Rsp Fail
  uint8_t channel,mfglib_rf_flag_temp;
  channel = *(uint8_t *)CLI_getArguments(1);
  if (channel < 11 || channel > 26)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }

  //正在测试RX和RX-TX自动测试时，不能调整功率
  if (get_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_RSSI_TESTING_BIT) ||
      get_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_TX_AUTO_TESTING_BIT))
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //暂存之前模式标志
  mfglib_rf_flag_temp = production_test_mfglib_rf_flag;
  //更新当前信道变量
  production_test_current_channel = channel;

  //停止MfgRf测试
  if (stopMfglibRfTestMode() == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //启动MfgRf测试，设定好相应信道，发射功率
  if (startMfglibRfTestMode(NULL) == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //之前正在发送stream，重新开始发送stream
  if (get_bit(mfglib_rf_flag_temp, MFGLIB_RF_STREAM_SENDING_BIT))
  {
    if (mfglibStartStream() == EMBER_SUCCESS)
    {
      set_bit(production_test_mfglib_rf_flag, MFGLIB_RF_STREAM_SENDING_BIT);
    }
    else
    {
      productionTestResponds(RSP_FAIL_STRING);
      return;
    }
  }
  //之前正在发送tone，重新开始发送tone
  else if (get_bit(mfglib_rf_flag_temp, MFGLIB_RF_TONE_SENDING_BIT))
  {
    if (mfglibStartTone() == EMBER_SUCCESS)
    {
      set_bit(production_test_mfglib_rf_flag, MFGLIB_RF_TONE_SENDING_BIT);
    }
    else
    {
      productionTestResponds(RSP_FAIL_STRING);
      return;
    }
  }
  productionTestResponds(RSP_OK_STRING);
}

/**
//函数名：tinyCLI_TxTune
//描  述：开始发送未调制信号，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_TxTune(void)
{
  //成功:Rsp OK
  //失败:Rsp Fail
  //停止MfgRf测试
  if (stopMfglibRfTestMode() == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //启动MfgRf测试，设定好相应信道，发射功率
  if (startMfglibRfTestMode(NULL) == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //开始发送tone
  if (mfglibStartTone() == EMBER_SUCCESS)
  {
    set_bit(production_test_mfglib_rf_flag, MFGLIB_RF_TONE_SENDING_BIT);
    productionTestResponds(RSP_OK_STRING);
  }
  else
  {
    productionTestResponds(RSP_FAIL_STRING);
  }
}

/**
//函数名：tinyCLI_TxStream
//描  述：开始发送调制信号，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_TxStream(void)
{
  //成功:Rsp OK
  //失败:Rsp Fail
  //停止MfgRf测试
  if (stopMfglibRfTestMode() == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //启动MfgRf测试，设定好相应信道，发射功率
  if (startMfglibRfTestMode(NULL) == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //开始发送stream
  if (mfglibStartStream() == EMBER_SUCCESS)
  {
    set_bit(production_test_mfglib_rf_flag, MFGLIB_RF_STREAM_SENDING_BIT);
    productionTestResponds(RSP_OK_STRING);
  }
  else
  {
    productionTestResponds(RSP_FAIL_STRING);
  }
}

/**
//函数名：tinyCLI_StopTx
//描  述：停止发送，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_StopTx(void)
{
  //成功:Rsp OK
  //失败:Rsp Fail
  //停止MfgRf测试
  if (stopMfglibRfTestMode() == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  productionTestResponds(RSP_OK_STRING);
}

/**
//函数名：tinyCLI_TxPkt
//描  述：发送一定数量的数据包，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_TxPkt(void)
{
  //参数1：uint16 数据包数量
  //成功:Rsp OK
  //失败:Rsp Fail
  uint16_t number_packets;
  number_packets = *(uint16_t *)CLI_getArguments(1);
  //判断参数是否有效
  if (number_packets == 0)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //停止MfgRf测试
  if (stopMfglibRfTestMode() == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //启动MfgRf测试，设定好相应信道，发射功率
  if (startMfglibRfTestMode(NULL) == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }

  //开始发送数据包,包大小50Bytes，特定数据包封包
  mfgRfTestSendFillBufferCallback(send_buff, 50, FILL_PACKETS_TYPE_SPECIFIC, number_packets);

  number_packets--;
  if (mfglibSendPacket(send_buff, number_packets) == EMBER_SUCCESS)
  {
    productionTestResponds(RSP_OK_STRING);
  }
  else
  {
    productionTestResponds(RSP_FAIL_STRING);
  }
  stopMfglibRfTestMode();
}

/**
//函数名：tinyCLI_RxRssi
//描  述：rx接收测试，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_RxRssi(void)
{
  //参数1：uint8 接收数据包的格式: 0任何数据包; 1特定数据包数量;（超100ms无新数据包视为结束）
  //回复参数1 ：接收到的包数
  //回复参数2 ：接收到的数据包平均RSSI
  //成功:Rsp 99 -50
  //失败:Rsp Fail
  uint8_t packets_mode;
  packets_mode = *(uint8_t *)CLI_getArguments(1);
  if (packets_mode >= 2)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  rssi_packet_mode = packets_mode;

  //停止MfgRf测试
  if (stopMfglibRfTestMode() == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //启动MfgRf测试，设定好相应信道，发射功率
  if (startMfglibRfTestMode(mfgRfTestRxHandler) == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //初始化相应的变量
  rssi_total= 0;                                       //收到的累计rssi值
  received_total_packets = 0;                          //收到的总包数
  last_received_total_packets = 0;                     //最后一次更新的总包数
  last_received_time_ms = halCommonGetInt32uMillisecondTick(); //更新最后接收时间
  set_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_RSSI_TESTING_BIT);
}

/**
//函数名：tinyCLI_JoinNWK
//描  述：加入到特定Zigbee网络，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_JoinNWK(void)
{
  //参数1：信道
  //参数2：PANID
  //回复参数1 ：加入的网络信道
  //回复参数2 ：网络PANID
  //成功:Rsp 16 0x1234
  //失败:Rsp Fail
  uint8_t channel;
  uint16_t pand_id;
  //停止MfgRf测试
  if (stopMfglibRfTestMode() == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  channel = *(uint8_t *)CLI_getArguments(1);
  pand_id = *(uint16_t *)CLI_getArguments(2);

  if (startJoinSpecificNetwork(channel, pand_id) == EMBER_SUCCESS)
  {
    //启动加网状态判断反馈处理
    last_received_time_ms = halCommonGetInt32uMillisecondTick();
    set_bit(production_test_mfglib_rf_flag, MFGLIB_RF_JOINING_NW_BIT);
  }
  else
  {
    productionTestResponds(RSP_FAIL_STRING);
  }
}

/**
//函数名：tinyCLI_RfAutoTest
//描  述：RF自动测试指令（固定流程的rx tx测试），通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_RfAutoTest(void)
{
  //成功:Rsp OK
  //失败:Rsp Fail
  //停止MfgRf测试
  if (stopMfglibRfTestMode() == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //启动MfgRf测试，设定好相应信道，发射功率
  if (startMfglibRfTestMode(mfgRfTestRxHandler) == false)
  {
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //初始化相关变量
  rssi_total= 0;                                       //收到的累计rssi值
  received_total_packets = 0;                          //收到的总包数
  last_received_total_packets = 0;                     //最后一次更新的总包数
  dut_need_to_send_packets_number = 0;                 //清除DUT需要发包数
  last_received_time_ms = halCommonGetInt32uMillisecondTick(); //更新最后接收时间
  set_bit(production_test_mfglib_rf_flag, MFGLIB_RF_RX_TX_AUTO_TESTING_BIT);
}

/**
//函数名：tinyCLI_Sleep
//描  述：休眠测试，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_Sleep(void)
{
  //参数1: (u8) 休眠模式(可选)
  //成功:Rsp OK
  //失败:Rsp Fail
  uint8_t sleep_mode;
  sleep_mode = *(uint8_t *)CLI_getArguments(1);
 // productionTestResponds("%d\r\n",sleep_mode);

  productionTestResponds(RSP_FAIL_STRING);
}

/**
//函数名：tinyCLI_GetMAC
//描  述：获取设备MAC地址，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_GetMAC(void)
{
  //成功:Rsp 112233445566AABB
  //失败:Rsp Fail
  EmberEUI64 eui64;
  emberAfGetEui64(eui64);

  CLI_printf("Rsp ");
  CLI_printHexValue(eui64, 8, 0);
  CLI_printf("\r\n");
}

/**
//函数名：tinyCLI_GetModelID
//描  述：获取ModelID，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_GetModelID(void)
{
  //成功:Rsp 1122334455667788900aabbccddeeff
  //失败:Rsp Fail
  uint8_t model_id_temp[34];
  if (emberAfReadAttribute(emberAfEndpointFromIndex(0),
                                ZCL_BASIC_CLUSTER_ID,
                                ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                model_id_temp,
                                33,
                                NULL) == EMBER_ZCL_STATUS_SUCCESS)
  {
    if (model_id_temp[0] < 33)
    {
      model_id_temp[model_id_temp[0]+1] = '\0';
      productionTestResponds("%s\r\n", &model_id_temp[1]);
      return;
    }
  }

  productionTestResponds(RSP_FAIL_STRING);
}

/**
//函数名：tinyCLI_InstallAuthorisation
//描  述：设备授权，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_InstallAuthorisation(void)
{
  //参数1：授权码 aabbccddee1122
  //成功:Rsp OK
  //失败:Rsp Fail
  uint8_t *p_authorisation_code;
  p_authorisation_code = (uint8_t *)CLI_getArguments(1);
#ifdef CACULATE_AUTHORIZATION_ENABLE
  if (8 == p_authorisation_code[0]) //输入MAC地址，给出授权结果
  {
    uint8_t temp[16];
    if (validateAuthorisationCode(CACULATE_AUTHORIZATION,temp, 16) == true)
    {
      //运算授权码后，打印
      CLI_printf("Rsp ");
      CLI_printHexValue(temp, 16, 0);
      CLI_printf("\r\n");
      return;
    }
  }
#endif
  //authorisation code 长度16
  if (validateAuthorisationCode(CHECK_AUTHORIZATION,&p_authorisation_code[1], p_authorisation_code[0]) == true)
  {
    //校验通过，保存数据
    saveCustomAuthorisationCodeToToken((tTokenTypeCustomAuthorisation*)&p_authorisation_code[1]);
    authorization_state = AUTHORIZATION;
    productionTestResponds(RSP_OK_STRING);
    return;
  }
  productionTestResponds(RSP_FAIL_STRING);
}

/**
//函数名：tinyCLI_GetAuthorisation
//描  述：获取授权码，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_GetAuthorisation(void)
{
  //回复参数1 ：机器授权码
  //成功:Rsp aabbccddee1122
  //失败:Rsp Fail
  tTokenTypeCustomAuthorisation temp;
  getCustomAuthorisationCodeFromToken(&temp);

  if (validateAuthorisationCode(CHECK_AUTHORIZATION,temp.code, 16) == true)
  {
    //校验正确，打印字串
    authorization_state = AUTHORIZATION;
    CLI_printf("Rsp ");
    CLI_printHexValue(temp.code, 16, 0);
    CLI_printf("\r\n");
  }
  else
  {
    authorization_state = UNAUTHORIZATION;
    productionTestResponds(RSP_FAIL_STRING);
  }
}

/**
//函数名：tinyCLI_DeleteAuthorisation
//描  述：删除授权，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_DeleteAuthorisation(void)
{
  //成功:Rsp OK
  //失败:Rsp Fail
  tTokenTypeCustomAuthorisation temp;
  memset(&temp.code[0],0x00,16);
  saveCustomAuthorisationCodeToToken(&temp);
  authorization_state = UNAUTHORIZATION;

  productionTestResponds(RSP_OK_STRING);
}

/**
//函数名：tinyCLI_WriteInstallcode
//描  述：写入安装码，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_WriteInstallcode(void)
{
  //参数1：安装码 11223344556677889900AABB 带crc
  //成功:Rsp OK
  //失败:Rsp Fail
  uint8_t *p_install_code;
  p_install_code = (uint8_t *)CLI_getArguments(1);
  uint8_t length,i;
  length = p_install_code[0];
#if 0 //不带CRC
  //install code 只用此四种长度，6/8/12/16
  if (length == 6  || length == 8  || length == 12 || length == 16)
  {
    tTokenTypeCustomInstallCode temp;
    temp.length = length + 2;
    for (i = 0; i < length; i++)
    {
      temp.code[i] = p_install_code[i+1];
    }

    uint16_t crc = caculateBufferCrc16(&temp.code[0], length);

    temp.code[i++] = LOW_BYTE(crc);
    temp.code[i++] = HIGH_BYTE(crc);

    //校验install code是否正确
    EmberKeyData test_key;
    if (emAfInstallCodeToKey(&temp.code[0], temp.length,&test_key) == EMBER_SUCCESS)
    {
      //保存数据到token
      saveCustomInstallCodeToToken(&temp);
      productionTestResponds(RSP_OK_STRING);
      return;
    }
  }
#endif
  //install code 只用此四种长度，6/8/12/16
  if (length == 8  || length == 10  || length == 14 || length == 18)
  {
    tTokenTypeCustomInstallCode temp;
    temp.length = length;
    for (i = 0; i < length; i++)
    {
      temp.code[i] = p_install_code[i+1];
    }
    //校验install code是否正确
    EmberKeyData test_key;
    if (emAfInstallCodeToKey(&temp.code[0], temp.length,&test_key) == EMBER_SUCCESS)
    {
      //保存数据到token
      saveCustomInstallCodeToToken(&temp);
      productionTestResponds(RSP_OK_STRING);
      return;
    }
  }
  productionTestResponds(RSP_FAIL_STRING);
}

/**
//函数名：tinyCLI_ReadInstallcode
//描  述：读取安装码，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_ReadInstallcode(void)
{
  //回复参数1 ：机器安装码
  //成功:Rsp 11223344556677889900AABB
  //失败:Rsp Fail
  tTokenTypeCustomInstallCode temp;
  getCustomInstallCodeFromToken(&temp);
  EmberKeyData test_key;
  if (EMBER_SUCCESS != emAfInstallCodeToKey(&temp.code[0], temp.length, &test_key))
  { //install code检验失败
    productionTestResponds(RSP_FAIL_STRING);
    return;
  }
  //校验正确，打印字串
  CLI_printf("Rsp ");
  CLI_printHexValue(temp.code, temp.length, 0);
//  CLI_printf(" CRC ");
//  CLI_printHexValue(&temp.code[temp.length-2], 2, 0);
  CLI_printf("\r\n");
}

/**
//函数名：tinyCLI_DeleteInstallcode
//描  述：删除安装码，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_DeleteInstallcode(void)
{
  //成功:Rsp OK
  //失败:Rsp Fail
  tTokenTypeCustomInstallCode temp;
  temp.length = 0;
  memset(&temp.code[0],0x00,18);
  saveCustomInstallCodeToToken(&temp);

  productionTestResponds(RSP_OK_STRING);
}

/**
//函数名：tinyCLI_WriteSerialNumber
//描  述：写入序列号，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_WriteSerialNumber(void)
{
  //参数1：序列号字串 D120216007000001 长度固定16字节
  //成功:Rsp OK
  //失败:Rsp Fail
  tTokenTypeSerialNumber temp;
  uint8_t *p_serial_number;
  uint8_t length;

  p_serial_number = (uint8_t *)CLI_getArguments(1);
  length = strlen((char const *)p_serial_number);

  //序列号字串,固定16字节长度。
  if (16 == length)
  {
    memcpy(&temp.sn[0],p_serial_number,17);
    //保存数据到token
    saveSerialNumberToToken(&temp);
    productionTestResponds(RSP_OK_STRING);
    return;
  }
  productionTestResponds(RSP_FAIL_STRING);
}

/**
//函数名：tinyCLI_ReadSerialNumber
//描  述：读取序列号，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_ReadSerialNumber(void)
{
  //回复参数1 ：序列号
  //成功:Rsp D120216007000001
  //失败:Rsp Fail
  tTokenTypeSerialNumber temp;
  getSerialNumberFromToken(&temp);
  if (strlen((char const *)temp.sn) == 16)
  {
    productionTestResponds("%s\r\n",temp.sn);
    return;
  }
  productionTestResponds(RSP_FAIL_STRING);
}

/**
//函数名：tinyCLI_DeleteSerialNumber
//描  述：删除序列号，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_DeleteSerialNumber(void)
{
  //成功:Rsp OK
  //失败:Rsp Fail
  tTokenTypeSerialNumber temp;
  temp.sn[0] = '\0';
  saveSerialNumberToToken(&temp);
  productionTestResponds(RSP_OK_STRING);
}

/**
//函数名：tinyCLI_FactoryReset
//描  述：恢复出厂设置，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_FactoryReset(void)
{
  //成功:Rsp OK
  //失败:Rsp Fail
  productionTestResponds(RSP_OK_STRING);
  emberAfPluginNetworkSteeringStop();
  emberLeaveNetwork();
  nodeInfoDeafultReset();
  clearOtaStorageTempDataAll();
  halReboot(); //重启
}

//MixSwitch部分的测试
/**
//函数名：tinyCLI_GetWaysConfig
//描  述：获取开关路数配置结果，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_GetWaysConfig(void)
{
  //回复参数1 ：(u8)开关路数 1-3
  //成功:Rsp 1/2/3
  //失败:Rsp Fail
  switchWaysConfigInit();
  uint8_t ways = switchWaysConfigGetWays();
  if (ways >= 1 && ways <= 3)
  {
    productionTestResponds("%d\r\n",ways);
  }
  else
  {
    productionTestResponds(RSP_FAIL_STRING);
  }
}

/**
//函数名：tinyCLI_RelayControl
//描  述：继电器控制测试，通过串口响应结果
//参  数：无
//返  回：void
*/
static void tinyCLI_RelayControl(void)
{
  //参数1：(u8)断电器路数 0-2
  //参数2：(u8)开关 0关 1开
  //成功:Rsp OK
  //失败:Rsp Fail
  uint8_t relay_number,on_off;
  relay_number = *(uint8_t *)CLI_getArguments(1);
  on_off = *(uint8_t *)CLI_getArguments(2);

  if (relay_number > 2 || on_off > 1)
  {
    productionTestResponds(RSP_FAIL_STRING);
  }
  else
  {
    if (on_off)
    {
      relayControlDriverTrigeOnOffAction(relay_number,RELAY_CONTROL_TURN_ON, 0);
    }
    else
    {
      relayControlDriverTrigeOnOffAction(relay_number,RELAY_CONTROL_TURN_OFF, 0);
    }
    productionTestResponds(RSP_OK_STRING);
  }
}

#endif //end ORB_PRODUCTION_TEST_CODE
/***************************************** 文 件 结 束 ************************************************/
