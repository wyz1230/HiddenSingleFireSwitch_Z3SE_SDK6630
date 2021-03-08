/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName   : production-test.h
*Author     : JimYao
*Version    : 1.0
*Date       : 2019-11-13
*Description: efr32方案厂测代码处理入口
*History:
***************************************************************************************************/

/* 防止重复引用区 ----------------------------------------------------------- */
#ifndef _PRODUCTION_TEST_H_
#define _PRODUCTION_TEST_H_

/* 相关包含头文件区 --------------------------------------------------------- */
#include "app/framework/include/af.h"

/* 宏定义区 ----------------------------------------------------------------- */


/* 自定义类型区 ------------------------------------------------------------- */
typedef void (* CLI_func_t)(void);

//---------------参数类型定义-------------------
//1 - 1字节整型，比如char 、uint8_t、 int8_t
//2 - 2字节整型，比如int16_t 、uint16_t
//4 - 4字节整型，比如int32_t 、uint32_t
//f - 浮点类型数据，比如3.14
//s - 字符串数据类型
//b - buffer HEX数据类型(由于bug，目前这个参数只能作为最后一个参数输入)
//比如   test(uint8_t a, uint16_t b , char *name) 我们应该使用 ： “12s”来代表

//CLI命令表结构
typedef struct
{
	char *name;				//命令名
	CLI_func_t func;		//函数体
	char *argumentsType;	//参数类型
	char *argDesc;			//参数介绍
	char *funcDesc;			//函数简介
}CLI_cmdTable_t;

typedef enum{
   WRITE_ATT_TYPE_NONE = 0x00,  
   WRITE_ATT_TYPE_UART,         //串口写的属性值
   WRITE_ATT_TYPE_GATEWAY		//网关写的属性值
} writeAttriType_enum;
/* 外部变量声明区 ----------------------------------------------------------- */

extern const CLI_cmdTable_t CLI_cmdTable[];

/* 全局函数声明区 ----------------------------------------------------------- */
/**
//函数名：checkToEnterProductionTest
//描  述：检测是否需要进入产测模式
//参  数：无
//返  回：void
*/
void checkToEnterProductionTest(void);

/**
//函数名：inProductionTestMode
//描  述：判断是否在产测模式
//参  数：无
//返  回：uint8_t , 0正常模式; 其它值 产测模式
*/
uint8_t inProductionTestMode(void);

/**
//函数名：authorizationPass
//描  述：判断是否授权通过
//参  数：无
//返  回：bool , true 已经授权; false 非授权
*/
bool authorizationPass(void);

/**
//函数名：getAuthCodeFromToken
//描述  ：从token中获取序列号
//参数  ：*sn (tTokenTypeSerialNumber *[输出]，需要读取的序列号数据存放指针)
//返回  ：void
*/
void getAuthCodeFromToken(tTokenTypeCustomAuthCode *code);

/**
//函数名：saveAuthCodeToToken
//描述  ：保存序列号到token中
//参数  ：*sn (tTokenTypeSerialNumber *[输入]，需要存入的SN数据指针)
//返回  ：void
*/
void saveAuthCodeToToken(tTokenTypeCustomAuthCode *code);

/**
//函数名：customWriteMac
//描  述：尝试写入个人的eui64
//参  数：无
//返  回：bool,true,写入成功，false，无法写入
*/
bool customWriteMac(uint8_t *p_custom_eui64);

/**
 //函数名：isHaveValidCustomMac
 //描述  ：读取芯片是否写过有效的自定义mac
 //参数  ：void
 //返回  ：bool; 
 //      true:有写过有效的自定义mac
 //		 false:没有写过MAC
 */
bool isHaveValidCustomMac(void);

/**
//函数名：getWriteAttributeType
//描  述：获取写属性的类型
//参  数：无
//返  回：uint8_t, 0x01 表示串口数据后更新属性值；其它值表示其它方式更新属性值
*/
uint8_t getWriteAttributeType(void);

/**
//函数名：_putchar
//描述：调用tiny print的输出入口函数，转换为efr32平台的串口输出
//参数：character (char [输入]，需要打印的字符)
//返回：void
*/
void _putchar(char character);

/**
//函数名：manufactureTestProc
//描述：厂测处理入口函数
//参数：无
//返回：bool,
//           false 表示厂测代码没处理，可以由SDK的CLI处理；
//           true  表示厂测代码已经处理，跳过SDK的CLI处理。
*/
bool manufactureTestProc(void);

#endif

/***************************************** 文 件 结 束 ************************************************/
