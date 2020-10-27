/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName   : private-tokens.h
*Author     : JimYao
*Version    : 1.0
*Date       : 2019-11-04
*Description: 保存产测与设备自定的相关数据
*History:
***************************************************************************************************/
// Identifier tags for tokens
#define CREATOR_MIX_SWITCH_SW_REBOOT_REASON         0x0000
#define CREATOR_MIX_SWITCH_ONOFF_STATUS             0x0001
#define CREATOR_MIX_SWITCH_COOR_MANUFACTOR_INFO     0x0002

#define NVM3KEY_MIX_SWITCH_SW_REBOOT_REASON         0x0000    //设备重启原因，用来区分软件重启的状态
#define NVM3KEY_MIX_SWITCH_ONOFF_STATUS             0x0001    //设备重启前开关状态，用于恢复开关状态
#define NVM3KEY_MIX_SWITCH_COOR_MANUFACTOR_INFO     0x0002    //设备入网后获取的到网关的产商信息

// Types for the tokens
#ifdef DEFINETYPES
  typedef struct {
    uint8_t onoff_status[3];       //用于保存开关的状态值
  } tTokenTypeSwitchOnOffStatus;
#endif // DEFINETYPES

// Actual token definitions
#ifdef DEFINETOKENS
  DEFINE_BASIC_TOKEN(MIX_SWITCH_SW_REBOOT_REASON,     uint8_t,                     0xFF)
  DEFINE_BASIC_TOKEN(MIX_SWITCH_ONOFF_STATUS,         tTokenTypeSwitchOnOffStatus, {0x00,0x00,0x00})
  DEFINE_BASIC_TOKEN(MIX_SWITCH_COOR_MANUFACTOR_INFO, int8_t,                      0xFF)
#endif // DEFINETOKENS


/**
//产测通用部分的token定义,TOKEN ID从0x0030开始。-------------------------------------
*/
// Identifier tags for tokens
#define CREATOR_CUSTOM_AUTHORISATION_CODE            0x0030     //自定授权码存放区
#define CREATOR_CUSTOM_INSTALLCODE                   0x0031     //自定InstallCode存放区
#define CREATOR_CUSTOM_SERIAL_NUMBER                 0x0032     //自定系列号存放区
#define CREATOR_CUSTOM_HFX_CTUNE                     0x0033     //自定高频外部晶振频偏调整参数
#define CREATOR_CUSTOM_NODE_TYPE					 0x0034		//节点类型
#define CREATOR_CUSTOM_SWITCH_TYPE					 0x0035		//开关类型


#define NVM3KEY_CUSTOM_AUTHORISATION_CODE            0x0030     //自定授权码存放区
#define NVM3KEY_CUSTOM_INSTALLCODE                   0x0031     //自定InstallCode存放区
#define NVM3KEY_CUSTOM_SERIAL_NUMBER                 0x0032     //自定系列号存放区
#define NVM3KEY_CUSTOM_HFX_CTUNE                     0x0033     //自定高频外部晶振频偏调整参数
#define NVM3KEY_CUSTOM_NODE_TYPE                     0x0034		//节点类型
#define NVM3KEY_CUSTOM_SWITCH_TYPE                   0x0035		//开关类型

// Types for the tokens
#ifdef DEFINETYPES
  typedef struct {
    uint8_t code[16];             //16Byte授权码
  } tTokenTypeCustomAuthorisation;
  
  typedef struct {
    uint8_t  length;               //安装码长度+2Bytes的CRC，四种长度6/8/12/16 + 2
    uint8_t  code[18];             //InstallCode+CRC,CRC跟随在后面，如8bytes长度时，crc跟在9/10两个byte,小端模式
  } tTokenTypeCustomInstallCode;
  
  typedef struct {
    uint8_t sn[17];                //16Byte的SN号字串，+ 1个结束符
  } tTokenTypeSerialNumber;
#endif // DEFINETYPES

// Actual token definitions
#ifdef DEFINETOKENS
  DEFINE_BASIC_TOKEN(CUSTOM_AUTHORISATION_CODE,  tTokenTypeCustomAuthorisation, \
                    {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF})
  DEFINE_BASIC_TOKEN(CUSTOM_INSTALLCODE,         tTokenTypeCustomInstallCode,   {0x00,})
  DEFINE_BASIC_TOKEN(CUSTOM_SERIAL_NUMBER,       tTokenTypeSerialNumber,        {0x00,})
  DEFINE_BASIC_TOKEN(CUSTOM_HFX_CTUNE,           uint16_t,                      0xFFFF)
  DEFINE_BASIC_TOKEN(CUSTOM_NODE_TYPE, 			 uint8_t, 						0x04)
  DEFINE_BASIC_TOKEN(CUSTOM_SWITCH_TYPE, 		 uint8_t, 						0x00)
#endif // DEFINETOKENS

/*************************************** 文 件 结 束 ******************************************/
