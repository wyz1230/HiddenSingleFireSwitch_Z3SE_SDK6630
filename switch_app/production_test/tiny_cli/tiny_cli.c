/***************************************************************************************************
Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
FileName : tiny_cli.c
Description : 一个精简的命令行接口（command line interface）实现，可以方便在裸机或者RTOS系统中使用。
Version : 1.0
Author : chris.huang
Date : 2019-01-10
***************************************************************************************************/
#include "tiny_cli.h"


#ifdef TINY_CLI_EN


//调试打印等级: 0 - 关闭调试打印；1 - 打印上层信息； 2 - 打印上层底层信息
#define CLI_DBG_PRINTF_LEVEL 0

//定义打印函数比如tinyPrintf，printf
//使用CLI_dbgLowLevel()打印底层信息，使用CLI_dbgHighLevel()打印上层信息
#if(CLI_DBG_PRINTF_LEVEL == 0)
	#define CLI_dbgLowLevel(str,...)
	#define CLI_dbgHighLevel(str,...)
#elif(CLI_DBG_PRINTF_LEVEL == 1)
	#define CLI_dbgLowLevel(str,...)   tinyPrintf(str, ##__VA_ARGS__)
	#define CLI_dbgHighLevel(str,...)
#elif(CLI_DBG_PRINTF_LEVEL == 2)
	#define CLI_dbgLowLevel(str,...)   tinyPrintf(str, ##__VA_ARGS__)
	#define CLI_dbgHighLevel(str,...)  tinyPrintf(str, ##__VA_ARGS__)
#endif


//使用LED指示CLI模块的运行情况,亮表示正在运行，灭表示未运行
//1 -开启；0 - 关闭
#define LED_IDENTIFY_EN 0//jim modify 1

#if(LED_IDENTIFY_EN)
	#include "stm32f1xx.h"

	//指示CLI模块运行状态
	#define LED_IDENTIFY_RUN() 	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET)
	//指示CLI模块空闲状态
	#define LED_IDENTIFY_IDEL() HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET)
#else
	//指示CLI模块运行状态
	#define LED_IDENTIFY_RUN()
	//指示CLI模块空闲状态
	#define LED_IDENTIFY_IDEL()
#endif


#define CLI_RING_BUF_SIZE 128       //定义CLI RX循环缓冲区大小

#define CLI_NAME_MAX_LEN 64	 		//命令名最大长度
#define CLI_ARG_MAX_LEN 256 		//命令参数最大长度
#define CLI_OPTION_MAX_LEN 16       //命令选项最大长度

#define CMD_CHAR_LEN       26       //命令字符打印占位长度
#define ARGS_CHAR_LEN      40       //参数字符打印占位长度
#define FUNC_DES_CHAR_LEN  38       //斜扛部分没算上，" //"

#define CLI_REV_TIMEOUT 500         //定义一条CLI命令接收超时时间MS


//CLI RX循环缓冲区状态
typedef enum
{
	EMPTY,	//空
	NORMAL, //正常
	FULL,	//满
}CLI_ringBufStatus_t;

//CLI模块处理状态
typedef enum
{
	IDEL,			//空闲
	RECEVING_NEME,  //接收命令名
	RECEVING_ARG,   //接收命令参数
}CLI_parseStatus_t;

//定义一个环形缓冲区结构体
typedef struct
{
	uint16_t head;                          //指向第一个字节
	uint16_t tail;                          //指向第一个空位
	CLI_ringBufStatus_t Status;             //状态
	uint8_t ringBuf[CLI_RING_BUF_SIZE];     //buffer空间
}CLI_ringBuffer_t;

//定义CLI处理结构体
typedef struct
{
	CLI_parseStatus_t parseStatus;		  //CLI解析状态机
	uint8_t nameLen;                      //命令名长度
	char name[CLI_NAME_MAX_LEN];  		  //命令名
	char arguments[CLI_ARG_MAX_LEN];      //命令参数
	char option[CLI_OPTION_MAX_LEN];	  //命令选项: @A 表示本命令需要ACK ; @C-1234,表示命令开启CRC16_CCITT校验,校验值1234
	uint16_t CRC16;                       //命令CRC16
}CLI_parse_t;



//RX接收环形缓冲区
static CLI_ringBuffer_t CLI_ringBuffer;

//CLI处理结构体
static CLI_parse_t CLI_parse;

//当前处理CLI命令表索引
static uint8_t CLI_currentIndex = 0xff;


//CLI主任务
void CLI_main(uint32_t sysTickMS);
//写环形缓冲区
//uint16_t CLI_writeRingBuffer(uint8_t *pData, uint16_t len);
//读环形缓冲区
static uint16_t CLI_readRingBuffer(uint8_t *pData, uint16_t len);
//处理接收字符串
static void CLI_processInputString(void);
//转义字符获取ascii码
static char CLI_transferredMeaning(char c);
//获取CLI输入参数，返回数据指针，uint8_t argNum第几个参数
void *CLI_getArguments(uint8_t argNum);
//字符串到整型
static int32_t CLI_stringToInt(char *str);
//字符串到浮点类型
static double CLI_stringToFloat(char *str);
//从字符到数值转换
static uint8_t CLI_charToHex(char c);
//打印帮助信息
static void CLI_printHelp(void);
//打印重复字符
static void CLI_charPrint(char c, uint16_t cnt);
//打印指定长度的字串，不足部分不打印。
static void CLI_stringPrint(char *Str, uint16_t len);
//两段字符串比较，1 - 完全相同，0 - 不完全相同, 0xff - 出错
uint8_t CLI_stringCompare(char *pA, char *pB, uint16_t len);
//计算字符串长度
uint16_t CLI_string_len(char *p);
//处理命令选项 ，1 - CRC失败，1 - 正常
static uint8_t CLI_processCliOption(void);
//处理命令ACK
static void CLI_processACK(uint8_t state);
//处理返回response
void CLI_processRSP(void);
//处理返回response带CRC
void CLI_processRSP_withCRC(char *str, uint8_t len);
//CRC16_CCITT CRC一串数据
static uint16_t CLI_CRC16_CCITT(uint16_t preCRC16, uint8_t *data, uint16_t len);
//CLI_CRC16_CCITT_byte CRC单字节追加
static uint16_t CLI_CRC16_CCITT_byte(uint16_t preCRC16, uint8_t data);
//内存赋值
static void CLI_memSet(uint8_t *pBuf, uint8_t val, uint16_t len);


/***************************************************************************************************
FuncName : CLI_main
Description : CLI主任务(可以插入到main循环一直跑,也可以使用一个单独的任务线程来跑,为了节约资源可以有
              输入数据触发再运行)
Param : sysTickMS - 系统Tick时钟（毫秒）实现帧超时功能，如果不需要超时功能可以一直填0
Return :
***************************************************************************************************/
void CLI_main(uint32_t sysTickMS)
{
	static uint16_t i = 0;
	static uint8_t j = 0;
	static uint8_t data;
	static uint8_t recevingStringFlag = 0;     //字符串接收标志
	static uint8_t transferredMeaningFlag = 0; //转义字符标志
	static uint8_t separatorFlag = 0;          //分隔符（空格）标志
	static uint8_t recevingCliOption = 0;      //cli命令选项接收标志

	static uint32_t frameHeadSysTick = 0;      //帧头系统时间



	while(CLI_readRingBuffer(&data, 1))
	{
		LED_IDENTIFY_RUN();

        if(sysTickMS - frameHeadSysTick >= CLI_REV_TIMEOUT)
        {
            i = 0;
            CLI_parse.nameLen = 0;
            recevingStringFlag = 0;
            transferredMeaningFlag = 0;
            CLI_memSet((uint8_t *)CLI_parse.name, 0, CLI_NAME_MAX_LEN);
            CLI_parse.parseStatus = IDEL;
            frameHeadSysTick = sysTickMS;
            CLI_parse.CRC16 = 0;
            CLI_dbgLowLevel("CLI recive timeout\r\n");
        }


		if(CLI_parse.parseStatus == IDEL)
		{
			//命令字符串前的换行和空格都忽略
			if(data == '\n'|| data == '\r'|| data == '\0')
			{
				i = 0;
				j = 0;
				CLI_parse.nameLen = 0;
				recevingStringFlag = 0;
				transferredMeaningFlag = 0;
				separatorFlag = 0;
				CLI_parse.CRC16 = 0;
				recevingCliOption = 0;
				CLI_parse.nameLen = 0;
			}
			else
			{
				CLI_parse.name[i++] = data;
				CLI_parse.nameLen++;

				CLI_parse.CRC16 = CLI_CRC16_CCITT_byte(CLI_parse.CRC16, data);

				CLI_parse.parseStatus = RECEVING_NEME;

				frameHeadSysTick = sysTickMS;

				CLI_dbgLowLevel("CLI reciving name\r\n");
			}
		}
		else if(CLI_parse.parseStatus == RECEVING_NEME) //命令名
		{
			//防止越界
			if(i >= CLI_NAME_MAX_LEN)
			{
				if(data == '\n'|| data == '\r')
				{
					CLI_parse.parseStatus = IDEL;
				}
				CLI_printf("\r\n cmd name is too long !!!");
				continue;
			}

			if(data == '\n'|| data == '\r')
			{
				//一条完整的命令接收结束，去处理
				CLI_processInputString();
				CLI_dbgLowLevel("\r\nreciving_name_over |len %d, %s",CLI_parse.nameLen, CLI_parse.name);

				i = 0;
				CLI_parse.nameLen = 0;
				CLI_parse.CRC16 = 0;
				CLI_memSet((uint8_t *)CLI_parse.name, 0, CLI_NAME_MAX_LEN);
				CLI_memSet((uint8_t *)CLI_parse.option, 0, CLI_OPTION_MAX_LEN);
				CLI_parse.parseStatus = IDEL;
			}
			else if(data == ' ')
			{
				i = 0;
				separatorFlag = 1;
				CLI_parse.parseStatus = RECEVING_ARG;
			}
			else
			{
				if(i < CLI_NAME_MAX_LEN)
				{
					CLI_parse.name[i++] = data;
					CLI_parse.nameLen++;

					CLI_parse.CRC16 = CLI_CRC16_CCITT_byte(CLI_parse.CRC16, data);
				}
			}
		}
		else if(CLI_parse.parseStatus == RECEVING_ARG) //命令参数
		{
			//防止越界
			if(i >= CLI_ARG_MAX_LEN)
			{
				if(data == '\n'|| data == '\r')
				{
					CLI_parse.parseStatus = IDEL;
				}
				CLI_printf("\r\n cmd arguments is too long !!!");
				continue;
			}

			//字符串参数解析
			//开始解析字符串内容
			if(data == '"' && recevingStringFlag == 0 )
			{
				recevingStringFlag = 1;

				if(separatorFlag == 1)
				{
					separatorFlag = 0;
					CLI_parse.CRC16 = CLI_CRC16_CCITT_byte(CLI_parse.CRC16, ' ');
				}
				CLI_parse.CRC16 = CLI_CRC16_CCITT_byte(CLI_parse.CRC16, data);

				continue;
			}
			else if(recevingStringFlag == 1)
			{
				//开始接收字符串
				if(data != '"')
				{
					if(transferredMeaningFlag == 0 && data == '\\') // 接收到转义字符'\'
					{
						transferredMeaningFlag = 1;
					}
					else if(transferredMeaningFlag == 1)
					{
						CLI_parse.arguments[i++] = CLI_transferredMeaning((char)data);
						transferredMeaningFlag = 0;
					}
					else
					{
						CLI_parse.arguments[i++] = data;
					}
				}
				else if(data == '"' && transferredMeaningFlag == 1)
				{
					CLI_parse.arguments[i++] = data;
					transferredMeaningFlag = 0;
				}
				//非转义状态下收到 " 接收字符串结束
				else if(transferredMeaningFlag == 0)
				{
					recevingStringFlag = 0;
				}

				CLI_parse.CRC16 = CLI_CRC16_CCITT_byte(CLI_parse.CRC16, data);
				continue;
			}
			if(data == '\n'|| data == '\r')
			{
				//参数最后添加一个结束字符
				CLI_parse.arguments[i] = '\0';

				//选项最后添加一个结束字符
				CLI_parse.option[j] = '\0';

				//一条完整的命令接收结束，去处理
				CLI_processInputString();
				CLI_dbgLowLevel("\r\nreciving_all_over | %s : %s", CLI_parse.name, CLI_parse.arguments);

				i = 0;
				j = 0;
				CLI_parse.nameLen = 0;
				recevingStringFlag = 0;
				transferredMeaningFlag = 0;
				recevingCliOption = 0;
				CLI_memSet((uint8_t *)CLI_parse.name, 0, CLI_NAME_MAX_LEN);
				CLI_memSet((uint8_t *)CLI_parse.arguments, 0, CLI_ARG_MAX_LEN);
				CLI_memSet((uint8_t *)CLI_parse.option, 0, CLI_OPTION_MAX_LEN);

				CLI_parse.parseStatus = IDEL;
			}
			else if(data == ' ')
			{
				//空格使用结束字符'\0'替代
				CLI_parse.arguments[i++] = '\0';

				separatorFlag = 1;
			}
			else
			{
				if(data == '@' && recevingCliOption == 0)  //开始接收命令选项
				{
					recevingCliOption = 1;
					CLI_parse.option[j++] = data;
					CLI_dbgLowLevel("start recevingCliOption \r\n");
				}
				else if(recevingCliOption == 1)
				{
					CLI_parse.option[j++] = data;
				}
				else
				{
					CLI_parse.arguments[i++] = data;

					if(separatorFlag == 1)
					{
						separatorFlag = 0;
						CLI_parse.CRC16 = CLI_CRC16_CCITT_byte(CLI_parse.CRC16, ' ');
					}
					CLI_parse.CRC16 = CLI_CRC16_CCITT_byte(CLI_parse.CRC16, data);
				}
			}
		}
	}
	LED_IDENTIFY_IDEL();
}


/***************************************************************************************************
FuncName : CLI_stringInput
Description : CLI字符串输入接口，可以在主循环中也可以在中断中输入数据
Param : pData - 输入字符串数据指针
        len - 数据长度
	    sysTick_ms - 系统tick时钟（毫秒），实现帧超时功能，如果不需要超时功能可以一直填0
Return : CLI 缓冲区可用空间 ， 为0时表示已满就不要再往里面写数据了，等待缓冲区腾出空间后再写
***************************************************************************************************/
inline uint16_t CLI_stringInput(uint8_t *pData, uint16_t len)
{
	return CLI_writeRingBuffer(pData, len); //写入循环缓冲区，返回剩余空间
}


/***************************************************************************************************
FuncName : CLI_writeRingBuffer
Description : 写环形缓冲区，可在中断中写入数据
Param : pData - 输入字符数据指针
        len - 数据长度
Return : 剩余空间数bytes
***************************************************************************************************/
//static inline uint16_t CLI_writeRingBuffer(uint8_t *pData, uint16_t len)
uint16_t CLI_writeRingBuffer(uint8_t *pData, uint16_t len)
{
	if(CLI_ringBuffer.Status == FULL)
	{
		return 0;
	}

	for(uint16_t i = 0; i<len; i++)
	{
		CLI_ringBuffer.ringBuf[CLI_ringBuffer.tail] = *pData;
		CLI_ringBuffer.tail++;
		CLI_ringBuffer.tail = CLI_ringBuffer.tail % CLI_RING_BUF_SIZE;
		CLI_ringBuffer.Status = NORMAL;
		pData++;

		//首尾重叠，buffer已满
		if(CLI_ringBuffer.tail == CLI_ringBuffer.head)
		{
			CLI_ringBuffer.Status = FULL;

			//还有未写入buffer的数据，返回错误
			if(i < len - 1)
			{
				CLI_dbgHighLevel("RingBuffer full! \r\n");
				return 0;
			}
			return 0;
		}
	}

	return CLI_ringBuffer.tail >= CLI_ringBuffer.head \
	       ? CLI_RING_BUF_SIZE - (CLI_ringBuffer.tail - CLI_ringBuffer.head) \
		   : CLI_ringBuffer.head - CLI_ringBuffer.tail;
}


/***************************************************************************************************
FuncName : CLI_readRingBuffer
Description : 读环形缓冲区
Param : pData - 读取输出数据指针
        len - 读取长度
Return : 实际读到的字节数bytes
***************************************************************************************************/
static uint16_t CLI_readRingBuffer(uint8_t *pData, uint16_t len)
{
	if(CLI_ringBuffer.Status == EMPTY)
	{
		return 0;
	}

	for(uint16_t i = 0; i<len; i++)
	{
		*pData = CLI_ringBuffer.ringBuf[CLI_ringBuffer.head];
		CLI_ringBuffer.head++;
		CLI_ringBuffer.head = CLI_ringBuffer.head % CLI_RING_BUF_SIZE;
		pData++;

		//首尾重叠，数据区已空
		if(CLI_ringBuffer.tail == CLI_ringBuffer.head)
		{
			CLI_ringBuffer.Status = EMPTY;

			//还有未读完的数据
			if(i < len - 1)
			{
				return i + 1;
			}
		}
	}
	return len;
}

/***************************************************************************************************
FuncName : CLI_string_len
Description : 计算字符串长度
Param : p - 字符串指针
Return : 字符串长度
***************************************************************************************************/
uint16_t CLI_string_len(char *p)
{
	uint16_t i;
	for(i = 0; *(p + i) != '\0'; i++);
	return i;
}

/***************************************************************************************************
FuncName : CLI_processACK
Description : 处理命令ACK
Param : p - 字符串指针
Return : 字符串长度
***************************************************************************************************/
static void CLI_processACK(uint8_t state)
{
	CLI_printf("%s", CLI_parse.name);
	if(state == 0)
	{
		CLI_printf("@ACK\r\n");
	}
	else
	{
		CLI_printf("@NACK 0x%2x\r\n", CLI_parse.CRC16);
	}
}

/***************************************************************************************************
FuncName : CLI_processRSP
Description : 处理返回response
Param :
Return :
***************************************************************************************************/
void CLI_processRSP(void)
{
	CLI_printf("%s@RSP ", CLI_parse.name);
}

/***************************************************************************************************
FuncName : CLI_processRSP_withCRC
Description : 处理返回response带crc16
Param : str - 字符串指针
Return :
***************************************************************************************************/
void CLI_processRSP_withCRC(char *str, uint8_t len)
{
	uint16_t CRC16 = CLI_CRC16_CCITT(0, (uint8_t *)CLI_parse.name, CLI_parse.nameLen);
	CLI_printf("%s", CLI_parse.name);

	CRC16 = CLI_CRC16_CCITT(CRC16,(uint8_t *)"@RSP", 4);
	CLI_printf("@RSP ");

	CRC16 = CLI_CRC16_CCITT(CRC16, (uint8_t *)str, len);
	CLI_printf("%s @C-0x%X\r\n",str, CRC16);
}


/***************************************************************************************************
FuncName : CLI_checkCRC16
Description : 处理crc16校验
Param : ps - 字符串指针
Return : 0 - 成功；1 - 失败
***************************************************************************************************/
static uint8_t CLI_checkCRC16(char *ps)
{
	uint16_t crc16 = (uint16_t)CLI_stringToInt(ps);

	if(crc16 == CLI_parse.CRC16)
	{
		return 0;
	}
	else
	{
        CLI_printf("CRC Failure @C-0x%X\r\n", CLI_parse.CRC16);
		return 1;
	}
}


/***************************************************************************************************
FuncName : CLI_processCliOption
Description : 处理命令选项
Param :
Return : 1 - CRC失败，1 - 正常
***************************************************************************************************/
static uint8_t CLI_processCliOption(void)
{
	uint8_t i = 0;
	uint8_t state = 0;
	uint8_t needACK = 0;


	while(i < CLI_OPTION_MAX_LEN - 1)
	{
		if(CLI_parse.option[i] == '@' && (CLI_parse.option[i + 1] == 'A' || CLI_parse.option[i + 1] == 'a'))  //匹配ACK
		{
			needACK = 1;
			i += 2;
		}
		else if(CLI_parse.option[i] == '@' && (CLI_parse.option[i + 1] == 'C' || CLI_parse.option[i + 1] == 'c') \
			    && CLI_parse.option[i + 2] == '-') //匹配CRC16
		{
			uint8_t intString[7] = {0, 0, 0, 0, 0, 0, 0};
			intString[0] = CLI_parse.option[i + 3] ;
			intString[1] = CLI_parse.option[i + 4] ;
			intString[2] = CLI_parse.option[i + 5] ;
			intString[3] = CLI_parse.option[i + 6] ;
			intString[4] = CLI_parse.option[i + 7] ;
			intString[5] = CLI_parse.option[i + 8] ;
			state = CLI_checkCRC16((char *)intString);
			i += 9;
		}
		else
		{
			break;
		}
	}
	if(needACK == 1) //crc不失败且需要ACK
	{
		CLI_processACK(state);
	}

	return state;
}

/***************************************************************************************************
FuncName : CLI_processInputString
Description : 处理CLI命令字符串
Param :
Return : 1 - CRC失败，1 - 正常
***************************************************************************************************/
static void CLI_processInputString(void)
{
	uint16_t i;

	for(i = 0;;i++)
	{
		if(CLI_parse.nameLen == CLI_string_len(CLI_cmdTable[i].name) &&
			CLI_cmdTable[i].func != NULL &&
		   CLI_stringCompare(CLI_parse.name, CLI_cmdTable[i].name, CLI_parse.nameLen) == 1)
		{
			//处理命令选项
			if(CLI_processCliOption() == 0) //命令如果开启CRC16且校验通过
			{
				//找到对应的命令函数，执行
				CLI_dbgHighLevel("\r\n find func %s ,running",CLI_cmdTable[i].name);
				CLI_currentIndex = i;
				CLI_cmdTable[i].func();
			}
            else
            {
                //CLI_dbgHighLevel("");
            }
			break;
		}
		else if(CLI_parse.nameLen == CLI_string_len("Test-Help")&&
				CLI_stringCompare(CLI_parse.name, "Test-Help", CLI_parse.nameLen) == 1)
		{
			CLI_printHelp();
			break;
		}
		else if(CLI_stringCompare(CLI_parse.name, CLI_cmdTable[i].name, CLI_parse.nameLen) == 0xff)
		{
			CLI_printf("\r\n not find the command : %s ",CLI_parse.name);
			//未找到对应命令
			break;
		}
	}
}

/***************************************************************************************************
FuncName : CLI_getArguments
Description : 获取CLI输入参数
Param : rgNum - 第几个参数
Return : void *, 使用时需要转成对应的数据类型指针
***************************************************************************************************/
void *CLI_getArguments(uint8_t argNum)
{
	static void *p  = "0"; 	    //指向一个字符串，防止访问到空指针导致死机
	uint16_t i = 0;
	uint16_t argIndex = 0;	    //参数起始索引
	uint8_t j = 1;  			//记录第几个参数

	for(i = 0; ; i++)
	{
		if(j == argNum)
		{
			break;
		}

		if(CLI_parse.arguments[i] == '\0')
		{
			j++;
			argIndex = i + 1; //记录参数起始索引
		}
	}

	switch(*(CLI_cmdTable[CLI_currentIndex].argumentsType + j - 1))
	{
		case NULL:
		{

		}
		break;

		case '1':
		{
			static uint8_t val;
			val = CLI_stringToInt(&CLI_parse.arguments[argIndex]);
			return &val;
		}

		case '2':
		{
			static uint16_t val;
			val = CLI_stringToInt(&CLI_parse.arguments[argIndex]);
			return &val;
		}

		case '4':
		{
			static uint32_t val;
			val = CLI_stringToInt(&CLI_parse.arguments[argIndex]);
			return &val;
		}

		case 'f':
		case 'F':
		{
			static float val;
			val = CLI_stringToFloat(&CLI_parse.arguments[argIndex]);
			return &val;
		}

		case 's':
		case 'S':
		{
			p = &CLI_parse.arguments[argIndex];
		}
		break;
		case 'b':
		case 'B':
		{
			uint16_t i = 0;
           #if 0 //jim moved
			for(i = 0; CLI_parse.arguments[argIndex + i] != '\0' && argIndex + i < CLI_ARG_MAX_LEN; i+=2)
			{
				//字符buffer转成真实hex,前面第一个字节记录转换后的buff长度
				CLI_parse.arguments[argIndex + 1 + i/2] = \
				CLI_charToHex(CLI_parse.arguments[argIndex + i]) * 0x10 + \
				CLI_charToHex(CLI_parse.arguments[argIndex + i + 1]);
			}
			CLI_parse.arguments[argIndex] = i/2 ;
			p = &CLI_parse.arguments[argIndex];
           #endif
            //jim add for the error char
            for (i=0; argIndex + i < CLI_ARG_MAX_LEN; i++)
            {
              //找出第一个有效的字符
              if (CLI_parse.arguments[argIndex+i] >= '0' &&
                  CLI_parse.arguments[argIndex+i] <= '9' ||
                  CLI_parse.arguments[argIndex+i] >= 'a' &&
                  CLI_parse.arguments[argIndex+i] <= 'f' ||
                  CLI_parse.arguments[argIndex+i] >= 'A' &&
                  CLI_parse.arguments[argIndex+i] <= 'F')
              {
                break;
              }
            }
            uint16_t j=0;
            for(; CLI_parse.arguments[argIndex + i] != '\0' && argIndex + i < CLI_ARG_MAX_LEN; i++)
			{
              if (CLI_parse.arguments[argIndex+i] >= '0' &&
                  CLI_parse.arguments[argIndex+i] <= '9' ||
                  CLI_parse.arguments[argIndex+i] >= 'a' &&
                  CLI_parse.arguments[argIndex+i] <= 'f' ||
                  CLI_parse.arguments[argIndex+i] >= 'A' &&
                  CLI_parse.arguments[argIndex+i] <= 'F')
              {
                //字符有效
                j++;
                if ((j%2 == 0) && (i > 0))
                {
                  //字符buffer转成真实hex,前面第一个字节记录转换后的buff长度
                  CLI_parse.arguments[argIndex + j/2] = \
				  CLI_charToHex(CLI_parse.arguments[argIndex + i - 1]) * 0x10 + \
				  CLI_charToHex(CLI_parse.arguments[argIndex + i]);
                }
              }
              else
              {
                //字串后的第一个无效字符结束
                break;
              }
			}
            //字符buffer转成真实hex,前面第一个字节记录转换后的buff长度
            CLI_parse.arguments[argIndex] = j/2 ;
			p = &CLI_parse.arguments[argIndex];
            //end jim
		}
		break;
		default:
			break;
	}
	return p;
}

/***************************************************************************************************
FuncName : CLI_charToHex
Description : 从字符到数值转换
Param : c - 字符
Return : 返回数值
***************************************************************************************************/
static uint8_t CLI_charToHex(char c)
{
	uint8_t val;
	if(c >= '0' && c <= '9')
	{
		val = c - '0';
	}
	else if(c >= 'A' && c <= 'F')
	{
		val = c - 'A' + 10;
	}
	else if(c >= 'a' && c<= 'f')
	{
		val = c - 'a' + 10;
	}
	return val;
}


/***************************************************************************************************
FuncName : CLI_stringToInt
Description : 整型字符串到数值整型
Param : str - 整型字符串指针
Return : 整型值
***************************************************************************************************/
static int32_t CLI_stringToInt(char *str)
{
	if(str == NULL)
	{
		return -1;
	}

    int32_t num = 0;		//数值
    uint8_t sign = 0; 	//是否带符号
	uint8_t isHex = 0;	//是否为10进制


    while ('0' == *str ||' ' == *str||'\n' == *str || '-' == *str ||
			'+' == *str || 'x' == *str || 'X' == *str)//如果有空,空格或者换行跳过去
    {
        if (*str=='-')//如果为负数，先保存符号
		{
            sign = 1;
		}
		else if(*str=='x')
		{
			isHex = 1;
		}
        str++;
    }

    while ((*str>='0' && *str<='9') || (*str>='A' && *str<='F') || (*str>='a' && *str<='f'))//ASCll有效部分
    {
		if(isHex)
		{
			if(*str >= '0' && *str <= '9')
			{
				num = num*16 + *str - '0';       // '0'是数字48 ,减去后得到对应的数字类型
			}
			else if(*str >= 'A' && *str <= 'F')
			{
				num = num*16 + *str - 'A' + 10;  // 'A'是数字65 ,减去后得到对应的数字类型
			}
			else if(*str >= 'a' && *str <= 'f')
			{
				num = num*16 + *str - 'a' + 10;  // '0'是数字97 ,减去后得到对应的数字类型
			}
		}
		else
		{
			if(*str>='0' && *str<='9')
			{
				num = num*10 + *str - '0';       // '0'是数字48 ,减去后得到对应的数字类型
			}
        }
		str++;
    }

    if (sign == 1)//为1得到负数
	{
        return -num;
	}
    else
	{
        return num;
	}
}


/***************************************************************************************************
FuncName : CLI_stringToInt
Description : 浮点字符串到浮点数值
Param : str - 浮点字符串
Return : 浮点数值
***************************************************************************************************/
static double CLI_stringToFloat(char *str)
{
	char flag = 0;        //表示正数
	double result = 0.0;  //最终结果
	double d = 10.0;      //10进制
	int e = 0;		      //幂
	while(*str != '\0')
	{
		if( !(*str >= '0' && *str <= '9'))  //找到字符串中的第一个数字
		{
			str++;
			continue;
		}
		if(*(str - 1) == '-')
		{
			flag = 1;  //表示是一个负数
		}

		while(*str >= '0' && *str <= '9')
		{
			result = result * 10.0 + (*str - '0');
			str++;
		}
		if(*str == '.')
		{
			str++;
		}
		while(*str >= '0' && *str <= '9')
		{
			result = result + (*str - '0') / d;
			d = d * 10;
			str++;
		}
		if(*str == 'e' || *str == 'E')
		{
			str++;
			if(*str == '+')
			{
				str++;
				while(*str >= '0' && *str <= '9')
				{
					e = e * 10 + (*str - '0');
					str++;
				}
				while(e > 0)
				{
					result = result * 10;
					e--;
				}
			}
			if(*str == '-')
			{
				str++;
				while(*str >= '0' && *str <= '9')
				{
					e = e * 10 + (*str - '0');
					str++;
				}
				while(e > 0)
				{
					result = result / 10;
					e--;
				}
			}
			if(*str >= '0' && *str <= '9')
			{
				while(*str >= '0' && *str <= '9')
				{
					e = e * 10 + (*str - '0');
					str++;
				}
				while(e > 0)
				{
					result = result * 10;
					e--;
				}
			}
		}
		return result * (flag ? -1 : 1);
	}
	return 0.0;
}

/***************************************************************************************************
FuncName : CLI_printHelp
Description : 帮助打印
Param :
Return :
***************************************************************************************************/
static void CLI_printHelp(void)
{
	//帮助命令，打印命令表
	uint8_t i = 0;
	uint8_t k = 0;
#if 0
	CLI_printf("\r\n********************************************* tiny CLI module *********************************************\r\n");
	CLI_printf("arguments type define: \r\n1->uint8_t/int8_t, 2->unit16_t/int16_t, 4->unit32_t/int32_t,");
	CLI_printf("f->float/double, s->char *, b->uint8_t buf[] \r\n");
	CLI_printf("example :\r\n");
	CLI_printf("if test cmd arguments type is (12sfb)->(int8_t a, uint16_t b, char *s, float f, uint8_t *p) \r\n");
	CLI_printf("so you should input as : test -20 0xAB12 \"hello\" 3.141592 11ff86");
	CLI_printf("\r\n********************************************* by chris.huang **********************************************\r\n");
	CLI_printf("\r\n");
	CLI_printf("seq number > command      |(args type) -> args description            //func description\r\n");
#else
	CLI_printf("\r\n********************************************* Production Test *********************************************\r\n");
	CLI_printf("arguments type define:    ");
    CLI_printf("   1-> uint8_t / int8_t   ");
    CLI_printf("   2-> unit16_t / int16_t ");
    CLI_printf("   4-> unit32_t / int32_t \r\n");
    CLI_printf("                          ");
    CLI_printf("   f-> float / double     ");
	CLI_printf("   s-> char *             ");
    CLI_printf("   b-> uint8_t buf[] \r\n");
	CLI_printf("seq number > command      |(args type) -> args description         //func description\r\n");
#endif
	for(i = 0; CLI_cmdTable[i].name != NULL; i++)
	{
		if(CLI_cmdTable[i].func != NULL)
		{
			uint8_t agrsLen = (CLI_cmdTable[i].argumentsType == NULL ? CLI_string_len("void") : CLI_string_len(CLI_cmdTable[i].argumentsType));
			uint8_t agrsDesLen = (CLI_cmdTable[i].argDesc == NULL ? CLI_string_len("void") : CLI_string_len(CLI_cmdTable[i].argDesc));
            uint8_t commandLen = CLI_string_len(CLI_cmdTable[i].name);
            uint8_t funDesLen =  (CLI_cmdTable[i].funcDesc == NULL ? 0 : CLI_string_len(CLI_cmdTable[i].funcDesc));

            //打印指令名部分
			CLI_printf(" %2d> %s", i+1-k, CLI_cmdTable[i].name);
            if (commandLen > CMD_CHAR_LEN -5)
            {   //指令长度大于指令显示长度，换行显示参数说明
              CLI_printf("\r\n");
              CLI_charPrint(' ', CMD_CHAR_LEN);
            }
            else
            {
			  CLI_charPrint(' ', CMD_CHAR_LEN - 5 - commandLen);
            }

            //打印参数部分  //换行前打印功能说明部分
            uint8_t remain_len = 0,current_index = 0;
            uint8_t fun_des_remain_len = 0,fun_des_current_index = 0;
            fun_des_remain_len = funDesLen;
            if (agrsLen > ARGS_CHAR_LEN - 2)
            {
              //参数内容长度大于参数说明显示区，换行接着显示
              CLI_printf("(");
              remain_len = 1;  //先用来表示已经占用字符
              if (CLI_cmdTable[i].argumentsType != NULL)
              {
                for (current_index=0; current_index < agrsLen; )
                {
                  CLI_stringPrint(CLI_cmdTable[i].argumentsType+current_index, ARGS_CHAR_LEN - 2);
                  current_index += ARGS_CHAR_LEN - 2;
                  CLI_printf(" ");
                  if (current_index < agrsLen)
                  {
                    remain_len = agrsLen - current_index;
                    //处理函数功能说明打印部分
                    if (fun_des_remain_len > 0)
                    {
                      if (fun_des_remain_len < FUNC_DES_CHAR_LEN)
                      {
                        if (CLI_cmdTable[i].funcDesc != NULL)
                        {
                          CLI_printf(" //");
                          if (fun_des_current_index < funDesLen)
                          {
                            CLI_stringPrint(CLI_cmdTable[i].funcDesc+fun_des_current_index, fun_des_remain_len);
                          }
                          fun_des_current_index += fun_des_remain_len;
                        }
                        fun_des_remain_len = 0;
                      }
                      else
                      {
                        if (CLI_cmdTable[i].funcDesc != NULL)
                        {
                          CLI_printf(" //");
                          if (fun_des_current_index < funDesLen)
                          {
                            CLI_stringPrint(CLI_cmdTable[i].funcDesc+fun_des_current_index, FUNC_DES_CHAR_LEN);
                          }
                        }
                        fun_des_current_index += FUNC_DES_CHAR_LEN;
                        fun_des_remain_len -= FUNC_DES_CHAR_LEN;
                      }
                    }
                    //处理函数功能说明结束
                    CLI_printf("\r\n");
                    CLI_charPrint(' ', CMD_CHAR_LEN+1);
                    if (remain_len < ARGS_CHAR_LEN - 2)
                    {
                       //最后一行处理完
                       CLI_stringPrint(CLI_cmdTable[i].argumentsType+current_index, remain_len);
                       break;
                    }
                  }
                }
              }
              CLI_printf(")");
              remain_len +=2;  //加上头尾两个空格
              if (ARGS_CHAR_LEN > remain_len)
              {
                remain_len = ARGS_CHAR_LEN - remain_len;
              }
              else
              { //非正常情况
                remain_len = 0;
              }
            }
            else
            {
			  CLI_printf("(%s)",(CLI_cmdTable[i].argumentsType != NULL) ? CLI_cmdTable[i].argumentsType : "void");
              remain_len = ARGS_CHAR_LEN - agrsLen - 2;
            }

            //打印参数说明部分   //换行前打印功能说明部分
            if (((CLI_cmdTable[i].argDesc == NULL) && (remain_len < 6)) ||
                (remain_len <= 2))
            {
              //处理函数功能说明打印部分
              CLI_charPrint(' ', remain_len);
              if (fun_des_remain_len > 0)
              {
                if (fun_des_remain_len < FUNC_DES_CHAR_LEN)
                {
                  if (CLI_cmdTable[i].funcDesc != NULL)
                  {
                    CLI_printf(" //");
                    if (fun_des_current_index < funDesLen)
                    {
                      CLI_stringPrint(CLI_cmdTable[i].funcDesc+fun_des_current_index, fun_des_remain_len);
                    }
                    fun_des_current_index += fun_des_remain_len;
                  }
                  fun_des_remain_len = 0;
                }
                else
                {
                  if (CLI_cmdTable[i].funcDesc != NULL)
                  {
                    CLI_printf(" //");
                    if (fun_des_current_index < funDesLen)
                    {
                      CLI_stringPrint(CLI_cmdTable[i].funcDesc+fun_des_current_index, FUNC_DES_CHAR_LEN);
                    }
                  }
                  fun_des_current_index += FUNC_DES_CHAR_LEN;
                  fun_des_remain_len -= FUNC_DES_CHAR_LEN;
                }
              }
              //处理函数功能说明结束
              remain_len = ARGS_CHAR_LEN;
              CLI_printf("\r\n");
              CLI_charPrint(' ', CMD_CHAR_LEN);
            }
            if (agrsDesLen > remain_len - 2 )
            {
              //超过剩余空间，需要换行处理
              CLI_printf("->");
              current_index = 0;
              if (CLI_cmdTable[i].argDesc != NULL)
              {
                 //首行处理
                CLI_stringPrint(CLI_cmdTable[i].argDesc+current_index, remain_len - 2);
                current_index += remain_len - 2;
                for ( ;current_index < agrsDesLen;)
                {
                  remain_len = agrsDesLen - current_index;
                  //处理函数功能说明打印部分
                  if (fun_des_remain_len > 0)
                  {
                    if (fun_des_remain_len < FUNC_DES_CHAR_LEN)
                    {
                      if (CLI_cmdTable[i].funcDesc != NULL)
                      {
                        CLI_printf(" //");
                        if (fun_des_current_index < funDesLen)
                        {
                          CLI_stringPrint(CLI_cmdTable[i].funcDesc+fun_des_current_index, fun_des_remain_len);
                        }
                        fun_des_current_index += fun_des_remain_len;
                      }
                      fun_des_remain_len = 0;
                    }
                    else
                    {
                      if (CLI_cmdTable[i].funcDesc != NULL)
                      {
                        CLI_printf(" //");
                        if (fun_des_current_index < funDesLen)
                        {
                          CLI_stringPrint(CLI_cmdTable[i].funcDesc+fun_des_current_index, FUNC_DES_CHAR_LEN);
                        }
                      }
                      fun_des_current_index += FUNC_DES_CHAR_LEN;
                      fun_des_remain_len -= FUNC_DES_CHAR_LEN;
                    }
                  }
                  //处理函数功能说明结束
                  CLI_printf("\r\n");
                  CLI_charPrint(' ', CMD_CHAR_LEN);
                  CLI_printf("->");
                  if (remain_len > ARGS_CHAR_LEN - 2 )
                  {
                    CLI_stringPrint(CLI_cmdTable[i].argDesc+current_index, ARGS_CHAR_LEN - 2 );
                    current_index += ARGS_CHAR_LEN - 2;
                  }
                  else
                  {
                    CLI_stringPrint(CLI_cmdTable[i].argDesc+current_index, remain_len);
                    current_index += remain_len;
                    remain_len = ARGS_CHAR_LEN - 2 - remain_len;  //转换为空剩余数
                    //最后一行处理完
                    break;
                  }
                }
              }
            }
            else
            {
              CLI_printf("->%s",(CLI_cmdTable[i].argDesc != NULL) ? CLI_cmdTable[i].argDesc : "NULL");
              remain_len = remain_len - 2 - agrsDesLen;
            }

            if ((remain_len > 0) &&
                (fun_des_remain_len > 0))            //还有剩的空间，补空格
            {
              CLI_charPrint(' ', remain_len);
            }
            //处理函数功能说明打印剩余部分
            while (fun_des_remain_len > 0)
            {
              if (fun_des_remain_len < FUNC_DES_CHAR_LEN)
              {
                if (CLI_cmdTable[i].funcDesc != NULL)
                {
                  CLI_printf(" //");
                  if (fun_des_current_index < funDesLen)
                  {
                    CLI_stringPrint(CLI_cmdTable[i].funcDesc+fun_des_current_index, fun_des_remain_len);
                  }
                  fun_des_current_index += fun_des_remain_len;
                }
                fun_des_remain_len = 0;
                break;
              }
              else
              {
                if (CLI_cmdTable[i].funcDesc != NULL)
                {
                  CLI_printf(" //");
                  if (fun_des_current_index < funDesLen)
                  {
                    CLI_stringPrint(CLI_cmdTable[i].funcDesc+fun_des_current_index, FUNC_DES_CHAR_LEN);
                  }
                }
                fun_des_current_index += FUNC_DES_CHAR_LEN;
                fun_des_remain_len -= FUNC_DES_CHAR_LEN;
                if (fun_des_remain_len > 0)
                {
                  CLI_printf("\r\n");
                  CLI_charPrint(' ', CMD_CHAR_LEN + ARGS_CHAR_LEN);
                }
              }
            }
            //处理函数功能说明结束
            CLI_printf("\r\n");
		}
		else
		{
			k++;
			uint8_t strLen = CLI_string_len(CLI_cmdTable[i].name);
			CLI_printf("\r\n");
			CLI_charPrint('-', 52 - strLen/2); 							//确保等长打印
			CLI_printf(" %s ",CLI_cmdTable[i].name);
			CLI_charPrint('-', 52 - strLen/2 + (strLen%2 ? 0 : 1)); 	//确保等长打印
			CLI_printf("\r\n");
		}
	}
}



/***************************************************************************************************
FuncName : CLI_transferredMeaning
Description : 通过转义字符获取ascii码
Param : 转义字符
Return : ascii码
***************************************************************************************************/
static char CLI_transferredMeaning(char c)
{
	char ascii = ' ';
	switch(c)
	{
		case 'r':
		{
			ascii = 13;
		}
		break;
		case 'n':
		{
			ascii = 10;
		}
		break;
		case 't':
		{
			ascii = 9;
		}
		break;
		case '\'':
		{
			ascii = 39;
		}
		break;
		case '\\':
		{
			ascii = 92;
		}
		case '"':
		{
			ascii = 34;
		}
		break;
		case '0':
		{
			ascii = 0;
		}
		break;
		default:
			break;
	}

	return ascii;
}

/***************************************************************************************************
FuncName : CLI_charPrint
Description : 打印重复字符
Param : c - 字符
        cnt - 重复次数
Return :
***************************************************************************************************/
static void CLI_charPrint(char c, uint16_t cnt)
{
	for(uint16_t i = 0; i < cnt; i++)
	{
		CLI_printf("%c", c);
	}
}

/***************************************************************************************************
FuncName : CLI_stringPrint
Description : 打印指定长度的字串，不足不打印
Param : c - 字串指针
        len - 字串长度
Return :
***************************************************************************************************/
static void CLI_stringPrint(char *Str, uint16_t len)
{
  for(uint16_t i = 0; i < len; i++)
  {
    if (Str[i] == '\0')
    {
      return;
    }
    CLI_printf("%c", Str[i]);
  }
}

/***************************************************************************************************
FuncName : CLI_transferredMeaning
Description : 比较两串字符串是否相同
Param : pA - 字符串A指针
        pB - 字符串B指针
		len - 字符串长度
Return : 1 - 完全相同，0 - 不完全相同, 0xff - 出错
***************************************************************************************************/
uint8_t CLI_stringCompare(char *pA, char *pB, uint16_t len)
{
	if(pA == NULL || pB == NULL)
	{
		return 0xff;
	}

	for(uint16_t i = 0; i < len; i++)
	{
		if(*pA != *pB)
		{
			return 0;
		}
		pA++;
		pB++;
	}
	return 1;
}

/***************************************************************************************************
FuncName : CLI_CRC16_CCITT
Description : 计算指定数据长度的CRC16(CCITT)
Param : preCRC16 - 先前计算好的CRC值
        data - 数据指针
		len - 数据长度
Return : 新CRC16值
***************************************************************************************************/
static uint16_t CLI_CRC16_CCITT(uint16_t preCRC16, uint8_t *data, uint16_t len)
{
	uint16_t crc16 = preCRC16;

	while( len-- )
	{
		for(uint8_t i=0x80; i!=0; i>>=1)
		{
			if((crc16 & 0x8000) != 0)
			{
				crc16 = crc16 << 1;
				crc16 = crc16 ^ 0x1021;
			}
			else
			{
				crc16 = crc16 << 1;
			}
			if((*data & i) != 0)
			{
				crc16 = crc16 ^ 0x1021;  //crc16 = crc16 ^ (0x10000 ^ 0x11021)
			}
		}
		data++;
	}
	return crc16;
}

/***************************************************************************************************
FuncName : CLI_CRC16_CCITT
Description : 追加计算一个字节数据长度的CRC16(CCITT)
Param : preCRC16 - 先前计算好的CRC值
        data - 一个字节数据
Return : 新CRC16值
***************************************************************************************************/
static uint16_t CLI_CRC16_CCITT_byte(uint16_t preCRC16, uint8_t data)
{
	uint16_t crc16 = preCRC16;
	for(uint8_t i=0x80; i!=0; i>>=1)
	{
		if((crc16 & 0x8000) != 0)
		{
			crc16 = crc16 << 1;
			crc16 = crc16 ^ 0x1021;
		}
		else
		{
			crc16 = crc16 << 1;
		}
		if((data & i) != 0)
		{
			crc16 = crc16 ^ 0x1021;  //crc16 = crc16 ^ (0x10000 ^ 0x11021)
		}
	}
	return crc16;
}


/***************************************************************************************************
FuncName : CLI_CRC16_CCITT
Description : 追加计算一个字节数据长度的CRC16(CCITT)
Param : preCRC16 - 先前计算好的CRC值
        data - 一个字节数据
Return : 新CRC16值
***************************************************************************************************/
static void CLI_memSet(uint8_t *pBuf, uint8_t val, uint16_t len)
{
	if(pBuf == NULL)
	{
		return;
	}

	for(uint16_t i = 0; i < len; i++)
	{
		*(pBuf+i) = val;
	}
}

#endif

