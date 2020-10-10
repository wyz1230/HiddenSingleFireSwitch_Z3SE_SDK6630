/***************************************************************************************************
Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
FileName : tiny_cli.h
Description : 一个精简的命令行接口（command line interface）实现，可以方便在裸机或者RTOS系统中使用。
Version : 1.0
Author : chris.huang
Date : 2019-01-10
***************************************************************************************************/
#ifndef TINY_CLI_H
#define TINY_CLI_H

#include <stdint.h>
#include "../production-test.h"
#include "../tiny_printf/tiny_printf.h"

#define TINY_CLI_EN  //是否启用tiny CLI


#ifdef TINY_CLI_EN

//定义CLI模块打印接口
#define CLI_printf tinyPrintf

//定义命令回复打印buffer大小
#define RSP_PRINT_BUF_SIZE 64

//CLI命令回复
#define CLI_RSP(str,...) \
	CLI_processRSP();\
	CLI_printf(str, ##__VA_ARGS__);\
	CLI_printf("\r\n");

//cli命令回复带CRC
#define CLI_RSP_WITH_CRC(str,...) \
	char printBuf[RSP_PRINT_BUF_SIZE];\
	uint8_t printStrLen = 0;\
	printStrLen = tinySnprintf(printBuf, RSP_PRINT_BUF_SIZE, str, ##__VA_ARGS__);\
	CLI_processRSP_withCRC(printBuf, printStrLen);


/***************************************************************************************************
FuncName : CLI_main
Description : CLI主任务(插入到 main循环一直执行)
Param : sysTickMS - 系统Tick时钟（毫秒）
Return :
***************************************************************************************************/
extern void CLI_main(uint32_t sysTickMS);



/***************************************************************************************************
FuncName : CLI_stringInput
Description : CLI字符串输入接口，可以在主循环中也可以在中断中输入数据
Param : pData - 输入字符串数据指针
        len - 数据长度
	    sysTick_ms - 系统tick时钟（毫秒），实现帧超时功能，如果不需要超时功能可以一直填0
Return : CLI 缓冲区可用空间 ， 为0时表示已满就不要再往里面写数据了，等待缓冲区腾出空间后再写
***************************************************************************************************/
extern uint16_t CLI_stringInput(uint8_t *pData, uint16_t len);

/***************************************************************************************************
FuncName : CLI_writeRingBuffer
Description : 写环形缓冲区，可在中断中写入数据
Param : pData - 输入字符数据指针
        len - 数据长度
Return : 剩余空间数bytes
***************************************************************************************************/
//static inline uint16_t CLI_writeRingBuffer(uint8_t *pData, uint16_t len)
extern uint16_t CLI_writeRingBuffer(uint8_t *pData, uint16_t len);

/***************************************************************************************************
FuncName : CLI_getArguments
Description : 获取CLI输入参数
Param : rgNum - 第几个参数
Return : void *, 使用时需要转成对应的数据类型指针
***************************************************************************************************/
extern void *CLI_getArguments(uint8_t argNum);


/***************************************************************************************************
FuncName : CLI_processRSP
Description : 处理返回response
Param :
Return :
***************************************************************************************************/
extern void CLI_processRSP(void);

/***************************************************************************************************
FuncName : CLI_processRSP_withCRC
Description : 处理返回response带crc16
Param : str - 字符串指针
Return :
***************************************************************************************************/
extern void CLI_processRSP_withCRC(char *str, uint8_t len);


/***************************************************************************************************
FuncName : CLI_transferredMeaning
Description : 比较两串字符串是否相同
Param : pA - 字符串A指针
        pB - 字符串B指针
		len - 字符串长度
Return : 1 - 完全相同，0 - 不完全相同, 0xff - 出错
***************************************************************************************************/
uint8_t CLI_stringCompare(char *pA, char *pB, uint16_t len);

#else

#define CLI_main(sysTickMS)
#define CLI_stringInput(pData, len)

#endif



#endif
