// This file is generated by Simplicity Studio.  Please do not edit manually.
//
//

// This file contains the tokens for attributes stored in flash


// Identifier tags for tokens
// Creator for attribute: generic device type, singleton.
#define CREATOR_GENERIC_DEVICE_TYPE_SINGLETON 0xB000
#define NVM3KEY_GENERIC_DEVICE_TYPE_SINGLETON ( NVM3KEY_DOMAIN_ZIGBEE | 0xB000 )
// Creator for attribute: physical environment, singleton.
#define CREATOR_PHYSICAL_ENVIRONMENT_SINGLETON 0xB001
#define NVM3KEY_PHYSICAL_ENVIRONMENT_SINGLETON ( NVM3KEY_DOMAIN_ZIGBEE | 0xB001 )
// Creator for attribute: on/off, endpoint: 1
#define CREATOR_ON_OFF_1 0xB002
#define NVM3KEY_ON_OFF_1 ( NVM3KEY_DOMAIN_ZIGBEE | 0xB002 )
// Creator for attribute: switch all set info, singleton.
#define CREATOR_SWITCH_All_SET_INFO_SINGLETON 0xB003
#define NVM3KEY_SWITCH_All_SET_INFO_SINGLETON ( NVM3KEY_DOMAIN_ZIGBEE | 0xB003 )
// Creator for attribute: auth code, singleton.
#define CREATOR_AUTH_CODE_SINGLETON 0xB004
#define NVM3KEY_AUTH_CODE_SINGLETON ( NVM3KEY_DOMAIN_ZIGBEE | 0xB004 )
// Creator for attribute: on/off, endpoint: 2
#define CREATOR_ON_OFF_2 0xB005
#define NVM3KEY_ON_OFF_2 ( NVM3KEY_DOMAIN_ZIGBEE | 0xB005 )


// Types for the tokens
#ifdef DEFINETYPES
typedef uint8_t  tokType_generic_device_type;
typedef uint8_t  tokType_physical_environment;
typedef uint8_t  tokType_auth_code[50];
typedef uint8_t  tokType_on_off;
typedef uint16_t  tokType_switch_all_set_info;
#endif // DEFINETYPES


// Actual token definitions
#ifdef DEFINETOKENS
DEFINE_BASIC_TOKEN(GENERIC_DEVICE_TYPE_SINGLETON, tokType_generic_device_type, 0)
DEFINE_BASIC_TOKEN(PHYSICAL_ENVIRONMENT_SINGLETON, tokType_physical_environment, 0x00)
DEFINE_BASIC_TOKEN(ON_OFF_1, tokType_on_off, 0x00)
DEFINE_BASIC_TOKEN(SWITCH_All_SET_INFO_SINGLETON, tokType_switch_all_set_info, 0x0001)
DEFINE_BASIC_TOKEN(AUTH_CODE_SINGLETON, tokType_auth_code, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00})
DEFINE_BASIC_TOKEN(ON_OFF_2, tokType_on_off, 0x00)
#endif // DEFINETOKENS


// Macro snippet that loads all the attributes from tokens
#define GENERATED_TOKEN_LOADER(endpoint) do {\
  uint8_t ptr[50]; \
  uint8_t curNetwork = emberGetCurrentNetwork(); \
  uint8_t epNetwork; \
  halCommonGetToken((tokType_generic_device_type *)ptr, TOKEN_GENERIC_DEVICE_TYPE_SINGLETON); \
  emberAfWriteServerAttribute(1, ZCL_BASIC_CLUSTER_ID, ZCL_GENERIC_DEVICE_TYPE_ATTRIBUTE_ID, (uint8_t*)ptr, ZCL_ENUM8_ATTRIBUTE_TYPE); \
  halCommonGetToken((tokType_physical_environment *)ptr, TOKEN_PHYSICAL_ENVIRONMENT_SINGLETON); \
  emberAfWriteServerAttribute(1, ZCL_BASIC_CLUSTER_ID, ZCL_PHYSICAL_ENVIRONMENT_ATTRIBUTE_ID, (uint8_t*)ptr, ZCL_ENUM8_ATTRIBUTE_TYPE); \
  halCommonGetToken((tokType_switch_all_set_info *)ptr, TOKEN_SWITCH_All_SET_INFO_SINGLETON); \
  emberAfWriteServerAttribute(1, ZCL_ORVIBO_PRIVATE_CLUSTER_ID, ZCL_SWITCH_All_SET_INFO_ATTRIBUTE_ID, (uint8_t*)ptr, ZCL_INT16U_ATTRIBUTE_TYPE); \
  halCommonGetToken((tokType_auth_code *)ptr, TOKEN_AUTH_CODE_SINGLETON); \
  emberAfWriteServerAttribute(1, ZCL_ORVIBO_PRIVATE_CLUSTER_ID, ZCL_AUTH_CODE_ATTRIBUTE_ID, (uint8_t*)ptr, ZCL_CHAR_STRING_ATTRIBUTE_TYPE); \
  epNetwork = emberAfNetworkIndexFromEndpoint(1); \
  if((endpoint) == 1 || ((endpoint) == EMBER_BROADCAST_ENDPOINT && epNetwork == curNetwork)) { \
    halCommonGetToken((tokType_on_off *)ptr, TOKEN_ON_OFF_1); \
    emberAfWriteServerAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, (uint8_t*)ptr, ZCL_BOOLEAN_ATTRIBUTE_TYPE); \
  } \
  epNetwork = emberAfNetworkIndexFromEndpoint(2); \
  if((endpoint) == 2 || ((endpoint) == EMBER_BROADCAST_ENDPOINT && epNetwork == curNetwork)) { \
    halCommonGetToken((tokType_on_off *)ptr, TOKEN_ON_OFF_2); \
    emberAfWriteServerAttribute(2, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, (uint8_t*)ptr, ZCL_BOOLEAN_ATTRIBUTE_TYPE); \
  } \
} while(false)


// Macro snippet that saves the attribute to token
#define GENERATED_TOKEN_SAVER do {\
  uint8_t allZeroData[50]; \
  MEMSET(allZeroData, 0, 50); \
  if ( data == NULL ) data = allZeroData; \
  if ( clusterId == 0x00 ) { \
    if ( metadata->attributeId == 0x0009 && 0x0000 == emberAfGetMfgCode(metadata) &&!emberAfAttributeIsClient(metadata) ) \
      halCommonSetToken(TOKEN_GENERIC_DEVICE_TYPE_SINGLETON, data); \
    if ( metadata->attributeId == 0x0011 && 0x0000 == emberAfGetMfgCode(metadata) &&!emberAfAttributeIsClient(metadata) ) \
      halCommonSetToken(TOKEN_PHYSICAL_ENVIRONMENT_SINGLETON, data); \
  } else if ( clusterId == 0xFF00 ) { \
    if ( metadata->attributeId == 0x0004 && 0x0000 == emberAfGetMfgCode(metadata) &&!emberAfAttributeIsClient(metadata) ) \
      halCommonSetToken(TOKEN_SWITCH_All_SET_INFO_SINGLETON, data); \
    if ( metadata->attributeId == 0xFF00 && 0x0000 == emberAfGetMfgCode(metadata) &&!emberAfAttributeIsClient(metadata) ) \
      halCommonSetToken(TOKEN_AUTH_CODE_SINGLETON, data); \
  }\
  if ( endpoint == 1 ) { \
    if ( clusterId == 0x06 ) { \
      if ( metadata->attributeId == 0x0000 && 0x0000 == emberAfGetMfgCode(metadata) &&!emberAfAttributeIsClient(metadata) ) \
        halCommonSetToken(TOKEN_ON_OFF_1, data); \
    } \
  } else if ( endpoint == 2) { \
    if ( clusterId == 0x06 ) { \
      if ( metadata->attributeId == 0x0000 && 0x0000 == emberAfGetMfgCode(metadata) &&!emberAfAttributeIsClient(metadata) ) \
        halCommonSetToken(TOKEN_ON_OFF_2, data); \
    } \
  } \
} while(false)


