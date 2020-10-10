/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName   : identify-coordinator-manufacturer.c
*Author     : JimYao
*Version    : 1.0
*Date       : 2019-11-08
*Description: 设备入网后，主动识别网关产商名；主要用来识别ORB主自网关，处理自主的特定行为相关事项。
*History:
***************************************************************************************************/
/* 相关包含头文件区 ------------------------------------------------------------ */
//#include "identify-coordinator-manufacturer.h"
#include "common-app.h"
#include "app/framework/plugin/ota-client/ota-client.h"

/* 宏定义区 -------------------------------------------------------------------- */
//debug开关设定
//#define IDENTIFY_COORDINATOR_MANUFACTURE_DEBUG_ENABLE
#ifdef IDENTIFY_COORDINATOR_MANUFACTURE_DEBUG_ENABLE
  #define DEBUG_STRING                                    "IdentifyCoorManuf-DB:"
  #define identifyCoorManufactureDebugPrint(...)          emberAfPrint(0xFFFF,DEBUG_STRING __VA_ARGS__)
  #define identifyCoorManufactureDebugPrintln(...)        emberAfPrintln(0xFFFF,DEBUG_STRING __VA_ARGS__)
#else
  #define identifyCoorManufactureDebugPrint(...)
  #define identifyCoorManufactureDebugPrintln(...)
#endif

#define DEVICE_BASIC_CLIENT_ENDPOINT                0x01       //设备本地端basic client的endpoint号目前固定0x01
#define ORG_COORDINATOR_BASIC_SERVER_ENDPOINT       0x01       //ORB网关basic server的endpoint号，固定0x01

#define OTA_START_RANDOM_MAX_TIME_MS          10000      //10秒内随机启动+10秒固定时间

/* 自定义类型区 ---------------------------------------------------------------- */
enum {
  COOR_IDENTIFY_NONE = 0x00,
  COOR_IDENTIFY_GET_MANUFACUTRE_INFO,
  COOR_IDENTIFY_WAIT_MANUFACUTRE_INFO,
  COOR_IDENTIFY_FINISH,
};

enum {
  ORIVIBO_COORDINATOR = 0x00,
  OTHER_COORDINATOR = 0xFE,
  UNKNOW_COORDINATOR = 0xFF,
};

/* 全局变量区 ------------------------------------------------------------------ */
EmberEventControl coordinatorManufaIdentifyStepProEventControl;  //需要定义场景过渡时间处理事件
//void coordinatorManufaIdentifyStepProEventHandler(void);

EmberEventControl autoStartOtaRandomDelayEventControl;//设备入网后随机开始ota延时事件
//void autoStartOtaRandomDelayEventHandler(void);

/* 本地变量区 ------------------------------------------------------------------ */
static uint8_t coordinator_manufacture_identify_step = COOR_IDENTIFY_NONE;  //获取网关信息步骤
static uint8_t coordinator_manufacutre_info = UNKNOW_COORDINATOR;           //获取到的网关信息标志
static uint8_t command_request_count = 0;                                   //请求指令次数

/* 全局函数声明区 -------------------------------------------------------------- */


/* 本地函数声明区 -------------------------------------------------------------- */

/* 函数原型 -------------------------------------------------------------------- */

/**
//函数名：getCoorManufacInfoFromToken
//描述  ：从token中获取网关的产商信息
//参数  ：无
//返回  ：void
*/
static void getCoorManufacInfoFromToken(void)
{
  uint8_t temp;
  for (uint8_t j=0; j<3; j++) //多次比较，提高获取token的可靠性
  {
    halCommonGetToken(&temp, TOKEN_MIX_SWITCH_COOR_MANUFACTOR_INFO); //先获取token里的网关产商信息
    if (temp != coordinator_manufacutre_info)
    {
      coordinator_manufacutre_info = temp;
    }
    else
    {
      break;
    }
  }
  identifyCoorManufactureDebugPrintln("get coor manufac info=%d from token",coordinator_manufacutre_info);
}

/**
//函数名：setCoorManufacInfoToToken
//描述  ：保存网关的产商信息到token中
//参数  ：info (uint8_t [输入]，网关产商类别)
//返回  ：void
*/
static void setCoorManufacInfoToToken(uint8_t info)
{
  uint8_t temp;
  for (uint8_t j=0; j<3; j++) //多次比较，提高写入token的可靠性
  {
    halCommonGetToken(&temp, TOKEN_MIX_SWITCH_COOR_MANUFACTOR_INFO); //先获取token里的网关产商信息
    if (temp != info)
    {
      //存在差异,更新token
      halCommonSetToken(TOKEN_MIX_SWITCH_COOR_MANUFACTOR_INFO, &info);
    }
    else
    {
      break;
    }
  }
  identifyCoorManufactureDebugPrintln("set coor manufac info=%d to token",info);
}

/**
//函数名：identifyCoordinatorManufactureProcInit
//描  述：初始化网关产商信息
//参  数：无
//返  回：void
*/
void identifyCoordinatorManufactureProcInit(void)
{
  coordinator_manufacture_identify_step = COOR_IDENTIFY_NONE;
  command_request_count = 0;
  getCoorManufacInfoFromToken();
  if (emberAfNetworkState() == EMBER_JOINED_NETWORK)
  {
    checkToStartIdentifyCoordinatorManufacturer(1000);
  }
  identifyCoorManufactureDebugPrintln("init,Coor Info=%d",coordinator_manufacutre_info);
}

static void coordinatorManufactureInfoRequest(void)
{
   uint16_t attribute_id;
   EmberApsFrame *apsFrame = NULL;
   apsFrame = emberAfGetCommandApsFrame();
   attribute_id = ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID;
   emberAfFillCommandGlobalClientToServerReadAttributes(ZCL_BASIC_CLUSTER_ID,
                                                        &attribute_id,
                                                        2);
   apsFrame->sourceEndpoint      = DEVICE_BASIC_CLIENT_ENDPOINT;
   apsFrame->destinationEndpoint = ORG_COORDINATOR_BASIC_SERVER_ENDPOINT;
   apsFrame->options = EMBER_AF_DEFAULT_APS_OPTIONS;
   emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,0x0000);
}


/**
//函数名：coordinatorManufaIdentifyStepProEventHandler
//描  述：获取网关产商信息处理步骤
//参  数：无
//返  回：void
*/
void coordinatorManufaIdentifyStepProEventHandler(void)
{
  emberEventControlSetInactive(coordinatorManufaIdentifyStepProEventControl);

  if (coordinator_manufacutre_info != UNKNOW_COORDINATOR) //已经获取到网关信息，不需要处理
  {
    coordinator_manufacture_identify_step = COOR_IDENTIFY_FINISH;
  }

  switch (coordinator_manufacture_identify_step)
  {
    case COOR_IDENTIFY_GET_MANUFACUTRE_INFO:  //发送获取网关产商信息的指令
      coordinatorManufactureInfoRequest();
      coordinator_manufacture_identify_step = COOR_IDENTIFY_WAIT_MANUFACUTRE_INFO;
      emberEventControlSetDelayMS(coordinatorManufaIdentifyStepProEventControl,1000); //等待响应一秒
      identifyCoorManufactureDebugPrintln("Get Coor Info step=%d",coordinator_manufacture_identify_step);
      break;

    case COOR_IDENTIFY_WAIT_MANUFACUTRE_INFO: //等待网关产商信息响应超时
      command_request_count++;
      if (command_request_count > 3) //超时三次，无效
      {
        command_request_count = 0;
        coordinator_manufacture_identify_step = COOR_IDENTIFY_FINISH;
      }
      else
      {
        coordinator_manufacture_identify_step = COOR_IDENTIFY_GET_MANUFACUTRE_INFO;
        emberEventControlSetActive(coordinatorManufaIdentifyStepProEventControl);
      }
      identifyCoorManufactureDebugPrintln("Get Coor Info step=%d, try count=%d",
                                           coordinator_manufacture_identify_step,
                                           command_request_count);
      break;
    default:  //其它
      break;
  }

  if (COOR_IDENTIFY_FINISH == coordinator_manufacture_identify_step)
  { //网关厂商信息获取完成，按网关厂商分类处理ota的启动
     if (coordinator_manufacutre_info != ORIVIBO_COORDINATOR)
     {
       //非ORB的网关，需要设备主动启动ota
       uint32_t random_delay_ms = 10000;  //10秒+10秒随机
       random_delay_ms += emberGetPseudoRandomNumber() % OTA_START_RANDOM_MAX_TIME_MS;
       emberEventControlSetDelayMS(autoStartOtaRandomDelayEventControl,random_delay_ms); //随机启动ota
     }
  }
}

/**
//函数名：checkToStartIdentifyCoordinatorManufacturer
//描  述：获取网关产商信息处理步骤
//参  数：delay_ms (uint16_t [输入]延时多少ms启动获取网关信息)
//返  回：void
*/
void checkToStartIdentifyCoordinatorManufacturer(uint16_t delay_ms)
{
  if (coordinator_manufacutre_info == UNKNOW_COORDINATOR)
  {
    command_request_count = 0;
    coordinator_manufacture_identify_step = COOR_IDENTIFY_GET_MANUFACUTRE_INFO;
  }
  else
  { //已知道网关厂商的情况下，触发一下finish的事件处理，处理ota的请求部分。
     coordinator_manufacture_identify_step = COOR_IDENTIFY_FINISH;
  }
  if (delay_ms > 60000)
  {
     delay_ms = 60000;
  }
  delay_ms += emberGetPseudoRandomNumber() % 500;
  emberEventControlSetDelayMS(coordinatorManufaIdentifyStepProEventControl,delay_ms);
}

/**
//函数名：whetherIsOrviboCoordinator
//描  述：判断是否为ORB的网关
//参  数：无
//返  回：bool; true ORB网关，false 其它网关
*/
bool whetherIsOrviboCoordinator(void)
{
  if (coordinator_manufacutre_info == ORIVIBO_COORDINATOR)
  {
    return true;
  }
  return false;
}

/**
//函数名：clearCoordinatorManufacturerInfo
//描  述：清除网关产商信息
//参  数：无
//返  回：void
*/
void clearCoordinatorManufacturerInfo(void)
{
  coordinator_manufacutre_info = UNKNOW_COORDINATOR;
  setCoorManufacInfoToToken(coordinator_manufacutre_info);
  command_request_count = 0;
  coordinator_manufacture_identify_step = COOR_IDENTIFY_NONE;
  emberEventControlSetInactive(coordinatorManufaIdentifyStepProEventControl);
}
/**
//函数名：checkOrviboCoordinatorManufactureStr
//描  述：判断网关的产商字串是否为ORB
//参  数：*buf (uint8_t* [输入] 获取到属性值指针，字串类型是长度+字串)
//返  回：void
*/
static void coordinatorManufactureStrCheck(uint8_t *buf)
{
  uint8_t flag = 0;
  if (buf[0] == strlen("ORVIBO"))
  {
    if (memcmp(&buf[1],"ORVIBO",buf[0]) == 0)
    {
      flag = 1;
    }
    else if (memcmp(&buf[1],"orvibo",buf[0]) == 0)
    {
      flag = 1;
    }
  }
  if (flag)
  {
    coordinator_manufacutre_info = ORIVIBO_COORDINATOR;
  }
  else
  {
    coordinator_manufacutre_info = OTHER_COORDINATOR;
  }
  setCoorManufacInfoToToken(coordinator_manufacutre_info);
  identifyCoorManufactureDebugPrintln("Get Coor Info=%d",coordinator_manufacutre_info);
  if (ORIVIBO_COORDINATOR == coordinator_manufacutre_info)
  {
     orviboCoordinatorIdentifyFinishCallback();
  }
}
/** @brief Read Attributes Response
 *
 * This function is called by the application framework when a Read Attributes
 * Response command is received from an external device.  The application should
 * return true if the message was processed or false if it was not.
 *
 * @param clusterId The cluster identifier of this response.  Ver.: always
 * @param buffer Buffer containing the list of read attribute status records.
 * Ver.: always
 * @param bufLen The length in bytes of the list.  Ver.: always
 */
bool emberAfReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                           uint8_t *buffer,
                                           uint16_t bufLen)
{
  if (clusterId == ZCL_BASIC_CLUSTER_ID)
  {
    if (coordinator_manufacutre_info == UNKNOW_COORDINATOR)
    {
      if (coordinator_manufacture_identify_step == COOR_IDENTIFY_WAIT_MANUFACUTRE_INFO)
      {
         //解析 // attr:2Bytes status:1Byte type:1Byte data:2Bytes
         uint16_t index=0;
         uint16_t attribute_id = 0xFFFF;
         uint8_t  satus = 0x00, attribute_type = ZCL_UNKNOWN_ATTRIBUTE_TYPE;
         while (index < bufLen)
         {
           //获取attribute id 2Bytes
           attribute_id = emberAfGetInt16u(buffer,index,bufLen);
           index += 2;
           satus = emberAfGetInt8u(buffer,index,bufLen);
           index += 1;
           attribute_type = emberAfGetInt8u(buffer,index,bufLen);
           index += 1;
           if (attribute_id == ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID)
           {
              if ((satus == 0x00) && (attribute_type == ZCL_CHAR_STRING_ATTRIBUTE_TYPE))
              {
                //属性值正常回应，检测是否为ORB的网关。
                coordinatorManufactureStrCheck(&buffer[index]);
              }
              else
              {
                //属性值不正常回应，不是ORB的网关。
                coordinator_manufacutre_info = OTHER_COORDINATOR;
                setCoorManufacInfoToToken(coordinator_manufacutre_info);
              }
              //已经获取到属性，关闭获取步骤。
              command_request_count = 0;
              coordinator_manufacture_identify_step = COOR_IDENTIFY_FINISH;
              emberEventControlSetActive(coordinatorManufaIdentifyStepProEventControl);
              break;
           }
         }
      }
    }
  }
  return false;
}
/**
//函数名：autoStartOtaRandomDelayEventHandler
//描  述：入网后随机延时启动ota
//参  数：无
//返  回：无
*/
void autoStartOtaRandomDelayEventHandler(void)
{
  emberEventControlSetInactive(autoStartOtaRandomDelayEventControl);
  emberAfOtaClientStartCallback();
}
/**
//函数名：orviboCoordinatorIdentifyFinishCallback
//描  述：第一次辨识到是ORB的网关产生的回调
//参  数：无
//返  回：void
*/
WEAK(void orviboCoordinatorIdentifyFinishCallback(void){};)
/*************************************** 文 件 结 束 ******************************************/
