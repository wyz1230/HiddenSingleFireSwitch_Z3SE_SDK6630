// This file is generated by Simplicity Studio.  Please do not edit manually.
//
//

// Enclosing macro to prevent multiple inclusion
#ifndef SILABS_AF_ENDPOINT_CONFIG
#define SILABS_AF_ENDPOINT_CONFIG


// Fixed number of defined endpoints
#define FIXED_ENDPOINT_COUNT (2)


// Generated defaults
#if BIGENDIAN_CPU
#define GENERATED_DEFAULTS { \
6,'O','R','V','I','B','O',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 /* 0,Default value: Basic,manufacturer name */, \
32,'2','8','1','0','c','2','4','0','3','b','9','c','4','a','5','d','b','6','2','c','c','6','2','d','1','0','3','0','d','9','5','e' /* 33,Default value: Basic,model identifier */, \
8,'2','0','2','0','0','6','0','8',0,0,0,0,0,0,0,0 /* 66,Default value: Basic,date code */, \
5,'1','.','0','.','1',0,0,0,0,0,0,0,0,0,0,0 /* 83,Default value: Basic,sw build id */, \
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF /* 100,Default value: Over the Air Bootloading,OTA Upgrade Server ID */, \
0xFF, 0xFF, 0xFF, 0xFF /* 108,Default value: Over the Air Bootloading,Offset (address) into the file */, \
  }
#else // ! BIGENDIAN_CPU
#define GENERATED_DEFAULTS { \
6,'O','R','V','I','B','O',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 /* 0,Default value: Basic,manufacturer name */, \
32,'2','8','1','0','c','2','4','0','3','b','9','c','4','a','5','d','b','6','2','c','c','6','2','d','1','0','3','0','d','9','5','e' /* 33,Default value: Basic,model identifier */, \
8,'2','0','2','0','0','6','0','8',0,0,0,0,0,0,0,0 /* 66,Default value: Basic,date code */, \
5,'1','.','0','.','1',0,0,0,0,0,0,0,0,0,0,0 /* 83,Default value: Basic,sw build id */, \
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF /* 100,Default value: Over the Air Bootloading,OTA Upgrade Server ID */, \
0xFF, 0xFF, 0xFF, 0xFF /* 108,Default value: Over the Air Bootloading,Offset (address) into the file */, \
  }
#endif // BIGENDIAN_CPU




// Generated attributes
#define GENERATED_ATTRIBUTES { \
    { 0xFFFD, ZCL_INT16U_ATTRIBUTE_TYPE, 2, (ATTRIBUTE_MASK_CLIENT|ATTRIBUTE_MASK_SINGLETON), { (uint8_t*)0x0001 } }, /* 0 / Basic / cluster revision*/\
    { 0x0000, ZCL_INT8U_ATTRIBUTE_TYPE, 1, (ATTRIBUTE_MASK_SINGLETON), { (uint8_t*)0x03 } }, /* 1 / Basic / ZCL version*/\
    { 0x0001, ZCL_INT8U_ATTRIBUTE_TYPE, 1, (ATTRIBUTE_MASK_SINGLETON), { (uint8_t*)0x02 } }, /* 2 / Basic / application version*/\
    { 0x0004, ZCL_CHAR_STRING_ATTRIBUTE_TYPE, 33, (ATTRIBUTE_MASK_SINGLETON), { (uint8_t*)&(generatedDefaults[0]) } }, /* 3 / Basic / manufacturer name*/\
    { 0x0005, ZCL_CHAR_STRING_ATTRIBUTE_TYPE, 33, (ATTRIBUTE_MASK_SINGLETON), { (uint8_t*)&(generatedDefaults[33]) } }, /* 4 / Basic / model identifier*/\
    { 0x0006, ZCL_CHAR_STRING_ATTRIBUTE_TYPE, 17, (ATTRIBUTE_MASK_SINGLETON), { (uint8_t*)&(generatedDefaults[66]) } }, /* 5 / Basic / date code*/\
    { 0x0007, ZCL_ENUM8_ATTRIBUTE_TYPE, 1, (ATTRIBUTE_MASK_SINGLETON), { (uint8_t*)0x01 } }, /* 6 / Basic / power source*/\
    { 0x0009, ZCL_ENUM8_ATTRIBUTE_TYPE, 1, (ATTRIBUTE_MASK_TOKENIZE|ATTRIBUTE_MASK_SINGLETON), { (uint8_t*)0 } }, /* 7 / Basic / generic device type*/\
    { 0x0011, ZCL_ENUM8_ATTRIBUTE_TYPE, 1, (ATTRIBUTE_MASK_WRITABLE|ATTRIBUTE_MASK_TOKENIZE|ATTRIBUTE_MASK_SINGLETON), { (uint8_t*)0x00 } }, /* 8 / Basic / physical environment*/\
    { 0x4000, ZCL_CHAR_STRING_ATTRIBUTE_TYPE, 17, (ATTRIBUTE_MASK_SINGLETON), { (uint8_t*)&(generatedDefaults[83]) } }, /* 9 / Basic / sw build id*/\
    { 0xFFFD, ZCL_INT16U_ATTRIBUTE_TYPE, 2, (ATTRIBUTE_MASK_SINGLETON), { (uint8_t*)0x0001 } }, /* 10 / Basic / cluster revision*/\
    { 0x0000, ZCL_INT16U_ATTRIBUTE_TYPE, 2, (ATTRIBUTE_MASK_WRITABLE), { (uint8_t*)0x0000 } }, /* 11 / Identify / identify time*/\
    { 0xFFFD, ZCL_INT16U_ATTRIBUTE_TYPE, 2, (0x00), { (uint8_t*)0x0001 } }, /* 12 / Identify / cluster revision*/\
    { 0x0000, ZCL_BITMAP8_ATTRIBUTE_TYPE, 1, (0x00), { (uint8_t*)0x00 } }, /* 13 / Groups / name support*/\
    { 0xFFFD, ZCL_INT16U_ATTRIBUTE_TYPE, 2, (0x00), { (uint8_t*)0x0001 } }, /* 14 / Groups / cluster revision*/\
    { 0x0000, ZCL_INT8U_ATTRIBUTE_TYPE, 1, (0x00), { (uint8_t*)0x00 } }, /* 15 / Scenes / scene count*/\
    { 0x0001, ZCL_INT8U_ATTRIBUTE_TYPE, 1, (0x00), { (uint8_t*)0x00 } }, /* 16 / Scenes / current scene*/\
    { 0x0002, ZCL_INT16U_ATTRIBUTE_TYPE, 2, (0x00), { (uint8_t*)0x0000 } }, /* 17 / Scenes / current group*/\
    { 0x0003, ZCL_BOOLEAN_ATTRIBUTE_TYPE, 1, (0x00), { (uint8_t*)0x00 } }, /* 18 / Scenes / scene valid*/\
    { 0x0004, ZCL_BITMAP8_ATTRIBUTE_TYPE, 1, (0x00), { (uint8_t*)0x00 } }, /* 19 / Scenes / name support*/\
    { 0xFFFD, ZCL_INT16U_ATTRIBUTE_TYPE, 2, (0x00), { (uint8_t*)0x0001 } }, /* 20 / Scenes / cluster revision*/\
    { 0x0000, ZCL_BOOLEAN_ATTRIBUTE_TYPE, 1, (0x00), { (uint8_t*)0x00 } }, /* 21 / On/off / on/off*/\
    { 0xFFFD, ZCL_INT16U_ATTRIBUTE_TYPE, 2, (0x00), { (uint8_t*)0x0001 } }, /* 22 / On/off / cluster revision*/\
    { 0x0000, ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE, 8, (ATTRIBUTE_MASK_CLIENT|ATTRIBUTE_MASK_SINGLETON), { (uint8_t*)&(generatedDefaults[100]) } }, /* 23 / Over the Air Bootloading / OTA Upgrade Server ID*/\
    { 0x0001, ZCL_INT32U_ATTRIBUTE_TYPE, 4, (ATTRIBUTE_MASK_CLIENT|ATTRIBUTE_MASK_SINGLETON), { (uint8_t*)&(generatedDefaults[108]) } }, /* 24 / Over the Air Bootloading / Offset (address) into the file*/\
    { 0x0006, ZCL_ENUM8_ATTRIBUTE_TYPE, 1, (ATTRIBUTE_MASK_CLIENT|ATTRIBUTE_MASK_SINGLETON), { (uint8_t*)0x00 } }, /* 25 / Over the Air Bootloading / OTA Upgrade Status*/\
    { 0xFFFD, ZCL_INT16U_ATTRIBUTE_TYPE, 2, (ATTRIBUTE_MASK_CLIENT), { (uint8_t*)0x0001 } }, /* 26 / Over the Air Bootloading / cluster revision*/\
    { 0x0001, ZCL_INT8U_ATTRIBUTE_TYPE, 1, (ATTRIBUTE_MASK_WRITABLE|ATTRIBUTE_MASK_TOKENIZE|ATTRIBUTE_MASK_SINGLETON), { (uint8_t*)0x01 } }, /* 27 / OrviboPrivate / power on status*/\
    { 0xFF00, ZCL_CHAR_STRING_ATTRIBUTE_TYPE, 50, (ATTRIBUTE_MASK_WRITABLE|ATTRIBUTE_MASK_TOKENIZE|ATTRIBUTE_MASK_SINGLETON), { NULL } }, /* 28 / OrviboPrivate / auth code*/\
    { 0xFFFD, ZCL_INT16U_ATTRIBUTE_TYPE, 2, (0x00), { (uint8_t*)0x0001 } }, /* 29 / OrviboPrivate / cluster revision*/\
  }


// Cluster function static arrays
#define GENERATED_FUNCTION_ARRAYS \
const EmberAfGenericClusterFunction emberAfFuncArrayBasicClusterServer[] = { (EmberAfGenericClusterFunction)emberAfBasicClusterServerAttributeChangedCallback}; \
const EmberAfGenericClusterFunction emberAfFuncArrayIdentifyClusterServer[] = { (EmberAfGenericClusterFunction)emberAfIdentifyClusterServerInitCallback,(EmberAfGenericClusterFunction)emberAfIdentifyClusterServerAttributeChangedCallback}; \
const EmberAfGenericClusterFunction emberAfFuncArrayGroupsClusterServer[] = { (EmberAfGenericClusterFunction)emberAfGroupsClusterServerInitCallback}; \
const EmberAfGenericClusterFunction emberAfFuncArrayScenesClusterServer[] = { (EmberAfGenericClusterFunction)emberAfScenesClusterServerInitCallback}; \
const EmberAfGenericClusterFunction emberAfFuncArrayOnOffClusterServer[] = { (EmberAfGenericClusterFunction)emberAfOnOffClusterServerInitCallback,(EmberAfGenericClusterFunction)emberAfOnOffClusterServerAttributeChangedCallback}; \
const EmberAfGenericClusterFunction emberAfFuncArrayOtaBootloadClusterClient[] = { (EmberAfGenericClusterFunction)emberAfOtaBootloadClusterClientInitCallback,(EmberAfGenericClusterFunction)emberAfOtaBootloadClusterClientDefaultResponseCallback}; \


// Clusters definitions
#define GENERATED_CLUSTERS { \
    { 0x0000, (EmberAfAttributeMetadata*)&(generatedAttributes[0]), 1, 0, (CLUSTER_MASK_CLIENT), NULL,  },    \
    { 0x0000, (EmberAfAttributeMetadata*)&(generatedAttributes[1]), 10, 0, (CLUSTER_MASK_SERVER| CLUSTER_MASK_ATTRIBUTE_CHANGED_FUNCTION), emberAfFuncArrayBasicClusterServer, },    \
    { 0x0003, (EmberAfAttributeMetadata*)&(generatedAttributes[11]), 2, 4, (CLUSTER_MASK_SERVER| CLUSTER_MASK_INIT_FUNCTION| CLUSTER_MASK_ATTRIBUTE_CHANGED_FUNCTION), emberAfFuncArrayIdentifyClusterServer, },    \
    { 0x0004, (EmberAfAttributeMetadata*)&(generatedAttributes[13]), 2, 3, (CLUSTER_MASK_SERVER| CLUSTER_MASK_INIT_FUNCTION), emberAfFuncArrayGroupsClusterServer, },    \
    { 0x0005, (EmberAfAttributeMetadata*)&(generatedAttributes[15]), 6, 8, (CLUSTER_MASK_SERVER| CLUSTER_MASK_INIT_FUNCTION), emberAfFuncArrayScenesClusterServer, },    \
    { 0x0006, (EmberAfAttributeMetadata*)&(generatedAttributes[21]), 2, 3, (CLUSTER_MASK_SERVER| CLUSTER_MASK_INIT_FUNCTION| CLUSTER_MASK_ATTRIBUTE_CHANGED_FUNCTION), emberAfFuncArrayOnOffClusterServer, },    \
    { 0x0019, (EmberAfAttributeMetadata*)&(generatedAttributes[23]), 4, 2, (CLUSTER_MASK_CLIENT| CLUSTER_MASK_INIT_FUNCTION| CLUSTER_MASK_DEFAULT_RESPONSE_FUNCTION), emberAfFuncArrayOtaBootloadClusterClient, },    \
    { 0xFF00, (EmberAfAttributeMetadata*)&(generatedAttributes[27]), 3, 2, (CLUSTER_MASK_SERVER), NULL,  },    \
    { 0x0000, (EmberAfAttributeMetadata*)&(generatedAttributes[1]), 10, 0, (CLUSTER_MASK_SERVER| CLUSTER_MASK_ATTRIBUTE_CHANGED_FUNCTION), emberAfFuncArrayBasicClusterServer, },    \
    { 0x0003, (EmberAfAttributeMetadata*)&(generatedAttributes[11]), 2, 4, (CLUSTER_MASK_SERVER| CLUSTER_MASK_INIT_FUNCTION| CLUSTER_MASK_ATTRIBUTE_CHANGED_FUNCTION), emberAfFuncArrayIdentifyClusterServer, },    \
    { 0x0004, (EmberAfAttributeMetadata*)&(generatedAttributes[13]), 2, 3, (CLUSTER_MASK_SERVER| CLUSTER_MASK_INIT_FUNCTION), emberAfFuncArrayGroupsClusterServer, },    \
    { 0x0005, (EmberAfAttributeMetadata*)&(generatedAttributes[15]), 6, 8, (CLUSTER_MASK_SERVER| CLUSTER_MASK_INIT_FUNCTION), emberAfFuncArrayScenesClusterServer, },    \
    { 0x0006, (EmberAfAttributeMetadata*)&(generatedAttributes[21]), 2, 3, (CLUSTER_MASK_SERVER| CLUSTER_MASK_INIT_FUNCTION| CLUSTER_MASK_ATTRIBUTE_CHANGED_FUNCTION), emberAfFuncArrayOnOffClusterServer, },    \
  }


// Endpoint types
#define GENERATED_ENDPOINT_TYPES {        \
    { (EmberAfCluster*)&(generatedClusters[0]), 8, 22 }, \
    { (EmberAfCluster*)&(generatedClusters[8]), 5, 18 }, \
  }


// Cluster manufacturer codes
#define GENERATED_CLUSTER_MANUFACTURER_CODES {      \
{0x00, 0x00} \
  }
#define GENERATED_CLUSTER_MANUFACTURER_CODE_COUNT (0)

// Attribute manufacturer codes
#define GENERATED_ATTRIBUTE_MANUFACTURER_CODES {      \
{0x00, 0x00} \
  }
#define GENERATED_ATTRIBUTE_MANUFACTURER_CODE_COUNT (0)


// Largest attribute size is needed for various buffers
#define ATTRIBUTE_LARGEST (50)
// Total size of singleton attributes
#define ATTRIBUTE_SINGLETONS_SIZE (173)

// Total size of attribute storage
#define ATTRIBUTE_MAX_SIZE 40

// Array of endpoints that are supported
#define FIXED_ENDPOINT_ARRAY { 1, 2 }

// Array of profile ids
#define FIXED_PROFILE_IDS { 260, 260 }

// Array of device ids
#define FIXED_DEVICE_IDS { 256, 256 }

// Array of device versions
#define FIXED_DEVICE_VERSIONS { 1, 1 }

// Array of endpoint types supported on each endpoint
#define FIXED_ENDPOINT_TYPES { 0, 1 }

// Array of networks supported on each endpoint
#define FIXED_NETWORKS { 0, 0 }


#define EMBER_AF_GENERATED_PLUGIN_INIT_FUNCTION_DECLARATIONS \
  void emberAfPluginEepromInitCallback(void); \
  void emberAfPluginReportingInitCallback(void); \
  void emberAfPluginIdleSleepInitCallback(void); \
  void emberAfPluginCountersInitCallback(void); \


#define EMBER_AF_GENERATED_PLUGIN_INIT_FUNCTION_CALLS \
  emberAfPluginEepromInitCallback(); \
  emberAfPluginReportingInitCallback(); \
  emberAfPluginIdleSleepInitCallback(); \
  emberAfPluginCountersInitCallback(); \


#define EMBER_AF_GENERATED_PLUGIN_STACK_STATUS_FUNCTION_DECLARATIONS \
  void emberAfPluginEndDeviceSupportStackStatusCallback(EmberStatus status); \
  void emberAfPluginReportingStackStatusCallback(EmberStatus status); \
  void emberAfPluginOtaClientStackStatusCallback(EmberStatus status); \
  void emberAfPluginNetworkSteeringStackStatusCallback(EmberStatus status); \


#define EMBER_AF_GENERATED_PLUGIN_STACK_STATUS_FUNCTION_CALLS \
  emberAfPluginEndDeviceSupportStackStatusCallback(status); \
  emberAfPluginReportingStackStatusCallback(status); \
  emberAfPluginOtaClientStackStatusCallback(status); \
  emberAfPluginNetworkSteeringStackStatusCallback(status); \


// Generated data for the command discovery
#define GENERATED_COMMANDS { \
    { 0x0000, 0x00, COMMAND_MASK_INCOMING_SERVER }, /* Basic / ResetToFactoryDefaults */ \
    { 0x0003, 0x00, COMMAND_MASK_OUTGOING_SERVER }, /* Identify / IdentifyQueryResponse */ \
    { 0x0003, 0x00, COMMAND_MASK_INCOMING_SERVER }, /* Identify / Identify */ \
    { 0x0003, 0x01, COMMAND_MASK_INCOMING_SERVER }, /* Identify / IdentifyQuery */ \
    { 0x0004, 0x00, COMMAND_MASK_OUTGOING_SERVER }, /* Groups / AddGroupResponse */ \
    { 0x0004, 0x00, COMMAND_MASK_INCOMING_SERVER }, /* Groups / AddGroup */ \
    { 0x0004, 0x01, COMMAND_MASK_OUTGOING_SERVER }, /* Groups / ViewGroupResponse */ \
    { 0x0004, 0x01, COMMAND_MASK_INCOMING_SERVER }, /* Groups / ViewGroup */ \
    { 0x0004, 0x02, COMMAND_MASK_OUTGOING_SERVER }, /* Groups / GetGroupMembershipResponse */ \
    { 0x0004, 0x02, COMMAND_MASK_INCOMING_SERVER }, /* Groups / GetGroupMembership */ \
    { 0x0004, 0x03, COMMAND_MASK_OUTGOING_SERVER }, /* Groups / RemoveGroupResponse */ \
    { 0x0004, 0x03, COMMAND_MASK_INCOMING_SERVER }, /* Groups / RemoveGroup */ \
    { 0x0004, 0x04, COMMAND_MASK_INCOMING_SERVER }, /* Groups / RemoveAllGroups */ \
    { 0x0004, 0x05, COMMAND_MASK_INCOMING_SERVER }, /* Groups / AddGroupIfIdentifying */ \
    { 0x0005, 0x00, COMMAND_MASK_OUTGOING_SERVER }, /* Scenes / AddSceneResponse */ \
    { 0x0005, 0x00, COMMAND_MASK_INCOMING_SERVER }, /* Scenes / AddScene */ \
    { 0x0005, 0x01, COMMAND_MASK_OUTGOING_SERVER }, /* Scenes / ViewSceneResponse */ \
    { 0x0005, 0x01, COMMAND_MASK_INCOMING_SERVER }, /* Scenes / ViewScene */ \
    { 0x0005, 0x02, COMMAND_MASK_OUTGOING_SERVER }, /* Scenes / RemoveSceneResponse */ \
    { 0x0005, 0x02, COMMAND_MASK_INCOMING_SERVER }, /* Scenes / RemoveScene */ \
    { 0x0005, 0x03, COMMAND_MASK_OUTGOING_SERVER }, /* Scenes / RemoveAllScenesResponse */ \
    { 0x0005, 0x03, COMMAND_MASK_INCOMING_SERVER }, /* Scenes / RemoveAllScenes */ \
    { 0x0005, 0x04, COMMAND_MASK_OUTGOING_SERVER }, /* Scenes / StoreSceneResponse */ \
    { 0x0005, 0x04, COMMAND_MASK_INCOMING_SERVER }, /* Scenes / StoreScene */ \
    { 0x0005, 0x05, COMMAND_MASK_INCOMING_SERVER }, /* Scenes / RecallScene */ \
    { 0x0005, 0x06, COMMAND_MASK_OUTGOING_SERVER }, /* Scenes / GetSceneMembershipResponse */ \
    { 0x0005, 0x06, COMMAND_MASK_INCOMING_SERVER }, /* Scenes / GetSceneMembership */ \
    { 0x0005, 0x40, COMMAND_MASK_OUTGOING_SERVER }, /* Scenes / EnhancedAddSceneResponse */ \
    { 0x0005, 0x40, COMMAND_MASK_INCOMING_SERVER }, /* Scenes / EnhancedAddScene */ \
    { 0x0005, 0x41, COMMAND_MASK_OUTGOING_SERVER }, /* Scenes / EnhancedViewSceneResponse */ \
    { 0x0005, 0x41, COMMAND_MASK_INCOMING_SERVER }, /* Scenes / EnhancedViewScene */ \
    { 0x0006, 0x00, COMMAND_MASK_INCOMING_SERVER }, /* On/off / Off */ \
    { 0x0006, 0x01, COMMAND_MASK_INCOMING_SERVER }, /* On/off / On */ \
    { 0x0006, 0x02, COMMAND_MASK_INCOMING_SERVER }, /* On/off / Toggle */ \
    { 0x0019, 0x00, COMMAND_MASK_INCOMING_CLIENT }, /* Over the Air Bootloading / ImageNotify */ \
    { 0x0019, 0x01, COMMAND_MASK_OUTGOING_CLIENT }, /* Over the Air Bootloading / QueryNextImageRequest */ \
    { 0x0019, 0x03, COMMAND_MASK_OUTGOING_CLIENT }, /* Over the Air Bootloading / ImageBlockRequest */ \
    { 0x0019, 0x06, COMMAND_MASK_OUTGOING_CLIENT }, /* Over the Air Bootloading / UpgradeEndRequest */ \
    { 0xFF00, 0xFE, COMMAND_MASK_OUTGOING_SERVER }, /* OrviboPrivate / ResetDeviceResponse */ \
    { 0xFF00, 0xFF, COMMAND_MASK_OUTGOING_SERVER }, /* OrviboPrivate / OrbReStartDeviceResponse */ \
  }
#define EMBER_AF_GENERATED_COMMAND_COUNT (40)

// Command manufacturer codes
#define GENERATED_COMMAND_MANUFACTURER_CODES {      \
{0x00, 0x00} \
  }
#define GENERATED_COMMAND_MANUFACTURER_CODE_COUNT (0)


// Generated reporting configuration defaults
#define EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS {\
  { EMBER_ZCL_REPORTING_DIRECTION_REPORTED, 1, 0x0006, 0x0000, CLUSTER_MASK_SERVER, 0x0000, 1, 1200, 0 }, \
  { EMBER_ZCL_REPORTING_DIRECTION_REPORTED, 2, 0x0006, 0x0000, CLUSTER_MASK_SERVER, 0x0000, 1, 1200, 0 }, \
}
#define EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE (2)
#endif // SILABS_AF_ENDPOINT_CONFIG
