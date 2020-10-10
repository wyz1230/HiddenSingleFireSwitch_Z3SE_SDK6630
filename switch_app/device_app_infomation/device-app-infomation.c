/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName   : device-app-infomation.c
*Author     : JimYao
*Version    : 1.0
*Date       : 2019-10-12
*Description: 重定义设备的相关基本信息
*History    :
***************************************************************************************************/
/* 相关包含头文件区 ------------------------------------------------------------ */
//#include "device-app-infomation.h"
#include "common-app.h"

/* 宏定义区 -------------------------------------------------------------------- */
//debug开关设定
//#define DEVICE_INFO_APP_DEBUG_ENABLE
#ifdef DEVICE_INFO_APP_DEBUG_ENABLE
  #define DEBUG_STRING                        "DevAppInfo-DB:"
  #define devAppInfoDebugPrint(...)           emberAfPrint(0xFFFF, DEBUG_STRING __VA_ARGS__)
  #define devAppInfoDebugPrintln(...)         emberAfPrintln(0xFFFF, DEBUG_STRING __VA_ARGS__)
#else
  #define devAppInfoDebugPrint(...)
  #define devAppInfoDebugPrintln(...)
#endif
#define devAppInfoPrintln(...)                emberAfPrintln(0xFFFF, __VA_ARGS__)

#define DEVICE_INFO_BASIC_CLUSTER_ENDPOINT     (1U)  //此宏定义为设备信息存留的对应endpoint号，一般是1

/* 自定义类型区 ---------------------------------------------------------------- */

/* 全局变量区 ------------------------------------------------------------------ */

/* 本地变量区 ------------------------------------------------------------------ */

/* 全局函数声明区 -------------------------------------------------------------- */

/* 本地函数声明区 -------------------------------------------------------------- */

/* 函数原型 -------------------------------------------------------------------- */

/**
//函数名：deviceInfoAppBasicAttributeInit
//描述：初始化设备信息，按宏定义的版本与日期信息更新basic里的相应属性值。
//参数：无
//返回：void
*/
void deviceInfoAppBasicAttributeInit(void)
{
  EmberAfStatus status;
  uint8_t data_temp[33];

  if (emberAfReadAttribute(DEVICE_INFO_BASIC_CLUSTER_ENDPOINT,
                                ZCL_BASIC_CLUSTER_ID,
                                ZCL_APPLICATION_VERSION_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                data_temp,
                                1,
                                NULL) == EMBER_ZCL_STATUS_SUCCESS)
  {
    if (ORB_APP_VERSION_UINT8 != data_temp[0]) //当属性值中的版本与自定的宏定义版本不相同时，以宏义为准，更新属性值。
    {
      data_temp[0] = ORB_APP_VERSION_UINT8;
      status = emberAfWriteAttribute(DEVICE_INFO_BASIC_CLUSTER_ENDPOINT,
                                   ZCL_BASIC_CLUSTER_ID,
                                   ZCL_APPLICATION_VERSION_ATTRIBUTE_ID,
                                   CLUSTER_MASK_SERVER,
                                   data_temp,
                                   ZCL_INT8U_ATTRIBUTE_TYPE);
      if (status == EMBER_ZCL_STATUS_SUCCESS)
      {
        devAppInfoDebugPrintln("update basic appversion attribute OK");
      }
      else
      {
        devAppInfoDebugPrintln("update basic appversion attribute Error");
      }
    }
  }
  else
  {
    devAppInfoDebugPrintln("get basic appversion attribute error!");
  }

//最长32个Bytes
  if (emberAfReadAttribute(DEVICE_INFO_BASIC_CLUSTER_ENDPOINT,
                                ZCL_BASIC_CLUSTER_ID,
                                ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                data_temp,
                                33,
                                NULL) == EMBER_ZCL_STATUS_SUCCESS)
  {
    if (data_temp[0] < strlen(ORB_MANUFACTURE_NAME_STRING))
    {
      data_temp[0] = strlen(ORB_MANUFACTURE_NAME_STRING); //避免有出现字串2包含字串1的时候，出现不更新字串的问题。
    }
    if (memcmp(&data_temp[1],ORB_MANUFACTURE_NAME_STRING,data_temp[0]))
    { //当属性值中的manufacture name与自定的宏定义manufacture name不相同时，以宏义为准，更新属性值。
      data_temp[0] = strlen(ORB_MANUFACTURE_NAME_STRING);
      memcpy(&data_temp[1],ORB_MANUFACTURE_NAME_STRING,data_temp[0]);
      status = emberAfWriteAttribute(DEVICE_INFO_BASIC_CLUSTER_ENDPOINT,
                                   ZCL_BASIC_CLUSTER_ID,
                                   ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID,
                                   CLUSTER_MASK_SERVER,
                                   data_temp,
                                   ZCL_CHAR_STRING_ATTRIBUTE_TYPE);

      if (status == EMBER_ZCL_STATUS_SUCCESS)
      {
        devAppInfoDebugPrintln("update basic manufacture name attribute OK");
      }
      else
      {
        devAppInfoDebugPrintln("update basic manufacture name attribute Error");
      }
    }
  }
  else
  {
    devAppInfoDebugPrintln("get basic manufacture name attribute error!");
  }

//最长32个Bytes
  if (emberAfReadAttribute(DEVICE_INFO_BASIC_CLUSTER_ENDPOINT,
                                ZCL_BASIC_CLUSTER_ID,
                                ZCL_DATE_CODE_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                data_temp,
                                33,
                                NULL) == EMBER_ZCL_STATUS_SUCCESS)
  {
    if (data_temp[0] < strlen(ORB_DATE_CODE_STRING))
    {
      data_temp[0] = strlen(ORB_DATE_CODE_STRING); //避免有出现字串2包含字串1的时候，出现不更新字串的问题。
    }
    if (memcmp(&data_temp[1],ORB_DATE_CODE_STRING,data_temp[0]))
    { //当属性值中的date code与自定的宏定义date code不相同时，以宏义为准，更新属性值。
      data_temp[0] = strlen(ORB_DATE_CODE_STRING);
      memcpy(&data_temp[1],ORB_DATE_CODE_STRING,data_temp[0]);
      status = emberAfWriteAttribute(DEVICE_INFO_BASIC_CLUSTER_ENDPOINT,
                                   ZCL_BASIC_CLUSTER_ID,
                                   ZCL_DATE_CODE_ATTRIBUTE_ID,
                                   CLUSTER_MASK_SERVER,
                                   data_temp,
                                   ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
      if (status == EMBER_ZCL_STATUS_SUCCESS)
      {
        devAppInfoDebugPrintln("update basic date code attribute OK");
      }
      else
      {
        devAppInfoDebugPrintln("update basic date code attribute Error");
      }
    }
  }
  else
  {
    devAppInfoDebugPrintln("get basic date code attribute error!");
  }

//最长16个Bytes
  if (emberAfReadAttribute(DEVICE_INFO_BASIC_CLUSTER_ENDPOINT,
                                ZCL_BASIC_CLUSTER_ID,
                                ZCL_SW_BUILD_ID_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                data_temp,
                                17,
                                NULL) == EMBER_ZCL_STATUS_SUCCESS)
  {
    if (data_temp[0] < strlen(ORB_SW_BUILD_ID_STRING))
    {
      data_temp[0] = strlen(ORB_SW_BUILD_ID_STRING); //避免有出现字串2包含字串1的时候，出现不更新字串的问题。
    }
    if (memcmp(&data_temp[1],ORB_SW_BUILD_ID_STRING,data_temp[0]))
    { //当属性值中的sw build id与自定的宏定义sw build id不相同时，以宏义为准，更新属性值。
      data_temp[0] = strlen(ORB_SW_BUILD_ID_STRING);
      memcpy(&data_temp[1],ORB_SW_BUILD_ID_STRING,data_temp[0]);
      status = emberAfWriteAttribute(DEVICE_INFO_BASIC_CLUSTER_ENDPOINT,
                                   ZCL_BASIC_CLUSTER_ID,
                                   ZCL_SW_BUILD_ID_ATTRIBUTE_ID,
                                   CLUSTER_MASK_SERVER,
                                   data_temp,
                                   ZCL_CHAR_STRING_ATTRIBUTE_TYPE);

      if (status == EMBER_ZCL_STATUS_SUCCESS)
      {
        devAppInfoDebugPrintln("update basic sw build id attribute OK");
      }
      else
      {
        devAppInfoDebugPrintln("update basic sw build id attribute Error");
      }
    }
  }
  else
  {
    devAppInfoDebugPrintln("get basic sw build id attribute error!");
  }

}
/*************************************** 文 件 结 束 ******************************************/
