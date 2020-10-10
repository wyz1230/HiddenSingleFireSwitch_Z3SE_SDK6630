/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName   : custom-network-steering.c
*Author     : JimYao
*Version    : 1.0
*Date       : 2019-11-04
*Description: 客制zigbee3.0加网
*History:
***************************************************************************************************/
/* 相关包含头文件区 ------------------------------------------------------------ */
#if defined(USE_CUSTOM_NETWORK_STEERING_CODE)   //jim add 20191104 for the new networkscan

#include "app/framework/include/af.h"
#include "app/framework/security/af-security.h" // emAfClearLinkKeyTable()
#include "app/framework/plugin/network-steering/network-steering.h"
#include "app/framework/plugin/network-steering/network-steering-internal.h"

#if defined(EMBER_AF_API_SCAN_DISPATCH)
  #include EMBER_AF_API_SCAN_DISPATCH
#elif defined(EMBER_TEST)
  #include "../scan-dispatch/scan-dispatch.h"
#endif

#if defined(EMBER_AF_API_UPDATE_TC_LINK_KEY)
  #include EMBER_AF_API_UPDATE_TC_LINK_KEY
#elif defined(EMBER_TEST)
  #include "../update-tc-link-key/update-tc-link-key.h"
#endif


//#include "custom-network-steering.h"
#include "common-app.h"

/* 宏定义区 -------------------------------------------------------------------- */
//debug开关设定
//#define CUSTOM_NETWORK_STEERING_DEBUG_ENABLE
#ifdef CUSTOM_NETWORK_STEERING_DEBUG_ENABLE
  #define DEBUG_STRING                          "CustomNWSteering-DB:"
  #define customNwSteeringDebugPrint(...)       emberAfPrint(0xFFFF, DEBUG_STRING __VA_ARGS__)
  #define customNwSteeringDebugPrintln(...)     emberAfPrintln(0xFFFF, DEBUG_STRING __VA_ARGS__)
#else
  #define customNwSteeringDebugPrint(...)
  #define customNwSteeringDebugPrintln(...)
#endif

#ifdef EMBER_TEST
  #define HIDDEN
  #define EMBER_AF_PLUGIN_NETWORK_STEERING_RADIO_TX_POWER 3
#else
  #define HIDDEN static
#endif

#if !defined(EMBER_AF_PLUGIN_NETWORK_STEERING_CHANNEL_MASK)
  #define EMBER_AF_PLUGIN_NETWORK_STEERING_CHANNEL_MASK \
  (BIT32(11) | BIT32(14))
#endif

#if !defined(EMBER_AF_PLUGIN_NETWORK_STEERING_SCAN_DURATION)
  #define EMBER_AF_PLUGIN_NETWORK_STEERING_SCAN_DURATION 5
#endif

#if !defined(EMBER_AF_PLUGIN_NETWORK_STEERING_COMMISSIONING_TIME_S)
  #define EMBER_AF_PLUGIN_NETWORK_STEERING_COMMISSIONING_TIME_S (180)
#endif

#ifdef  EMBER_AF_PLUGIN_NETWORK_STEERING_RADIO_TX_CALLBACK
  #define GET_RADIO_TX_POWER(channel) emberAfPluginNetworkSteeringGetPowerForRadioChannelCallback(channel)
#else
  #define GET_RADIO_TX_POWER(channel) EMBER_AF_PLUGIN_NETWORK_STEERING_RADIO_TX_POWER
#endif

#define LAST_JOINING_STATE \
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_DISTRIBUTED

#define SECONDARY_CHANNEL_MASK EMBER_ALL_802_15_4_CHANNELS_MASK

#define REQUIRED_STACK_PROFILE 2

/* 自定义类型区 ---------------------------------------------------------------- */

/* 全局变量区 ------------------------------------------------------------------ */

/* 本地变量区 ------------------------------------------------------------------ */
const char * emAfPluginNetworkSteeringStateNames[] = {
  "None",
  // These next two states are only run if explicitly configured to do so
  // See emAfPluginNetworkSteeringSetConfiguredKey()
//  "Scan Primary Channels and use Configured Key",	//jim move
//  "Scan Secondary Channels and use Configured Key", //jim move
  "Scan Primary Channels and use Install Code",
  "Scan Secondary Channels and use Install Code",
  "Scan Primary Channels and Use Centralized Key",
  "Scan Secondary Channels and Use Centralized Key",
  "Scan Primary Channels and Use Distributed Key",
  "Scan Secondary Channels and Use Distributed Key",
};

EmberAfPluginNetworkSteeringJoiningState emAfPluginNetworkSteeringState
  = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE;

const uint8_t emAfNetworkSteeringPluginName[] = "NWK Steering";
#define PLUGIN_NAME emAfNetworkSteeringPluginName

static const EmberKeyData defaultLinkKey = {
  { 0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C,
    0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 }
};
static const EmberKeyData distributedTestKey = {
  { 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF }
};

// These parameters allow for filtering which networks to find or which specific
// key to use
static bool gFilterByExtendedPanId = false;
static uint8_t gExtendedPanIdToFilterOn[8];
static bool gUseConfiguredKey = false;
static EmberKeyData gConfiguredKey = {
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

static bool printedMaxPanIdsWarning = false;
uint8_t emAfPluginNetworkSteeringPanIdIndex = 0;
uint8_t emAfPluginNetworkSteeringCurrentChannel;

// We make these into variables so that they can be changed at run time.
// This is very useful for unit and interop tests.
uint32_t emAfPluginNetworkSteeringPrimaryChannelMask
  = EMBER_AF_PLUGIN_NETWORK_STEERING_CHANNEL_MASK;
uint32_t emAfPluginNetworkSteeringSecondaryChannelMask
  = SECONDARY_CHANNEL_MASK;

uint8_t emAfPluginNetworkSteeringTotalBeacons = 0;
uint8_t emAfPluginNetworkSteeringJoinAttempts = 0;
EmberKeyData emberPluginNetworkSteeringDistributedKey;

static uint32_t currentChannelMask = 0;

EmberEventControl emberAfPluginNetworkSteeringFinishSteeringEventControl;
#define finishSteeringEvent (emberAfPluginNetworkSteeringFinishSteeringEventControl)


// TODO: good value for this?
// Let's try jittering our TCLK update and permit join broadcast to cut down
// on commission-time traffic.
#define FINISH_STEERING_JITTER_MIN_MS (MILLISECOND_TICKS_PER_SECOND << 1)
#define FINISH_STEERING_JITTER_MAX_MS (MILLISECOND_TICKS_PER_SECOND << 2)
#define randomJitterMS()                                               \
  ((emberGetPseudoRandomNumber()                                       \
    % (FINISH_STEERING_JITTER_MAX_MS - FINISH_STEERING_JITTER_MIN_MS)) \
   + FINISH_STEERING_JITTER_MIN_MS)
#define UPDATE_TC_LINK_KEY_JITTER_MIN_MS (MILLISECOND_TICKS_PER_SECOND * 10)
#define UPDATE_TC_LINK_KEY_JITTER_MAX_MS (MILLISECOND_TICKS_PER_SECOND * 40)

// This is an attribute specified in the BDB.
//#define VERIFY_KEY_TIMEOUT_MS (5 * MILLISECOND_TICKS_PER_SECOND)
#define VERIFY_KEY_TIMEOUT_MS (15 * MILLISECOND_TICKS_PER_SECOND)  //jim change to 15 seconds

EmberAfPluginNetworkSteeringOptions emAfPluginNetworkSteeringOptionsMask
  = EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIONS_NONE;

//jim add
#define ONE_CHANNEL_TRY_JOIN_TIME  2  //每个信道偿试次数
#define MAX_NETWORK_ORDER_STORE    20  //排序保存最多网络数
#define COMPARE_EXPAN_ID_BYTES     4  //比较expand id的字节数
typedef struct
{
   uint8_t channel;
   uint16_t  panId;
   int8_t rssi;
   uint8_t extendedPanId[8];
}tChannelAndRssi;
static tChannelAndRssi networkValidChannelSortTable[MAX_NETWORK_ORDER_STORE];
static uint8_t CurrentScanChannel=0;

const static uint8_t custom_primary_ExPanId[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

static uint8_t FoundTotalChannel = 0;
static uint8_t CurrentOrderNeworkIndex = 0;
static uint8_t TryJoiningNetworkFlag = 0;
static uint8_t ScanningNetworkFlag = 0;
//end jim

/* 本地函数声明区 -------------------------------------------------------------- */

//jim add
static EmberStatus startScanNextChannelNetwork(void);
static void orderChannelList(void);
static void networkFoundAndStoreChannelCallback(EmberZigbeeNetwork *networkFound,
                                 uint8_t lqi,
                                 int8_t rssi);
static void tryToJoinNetwork(void);
EmberStatus  customNetworkSteeringRestart(uint8_t NextChannelFlag);
//end jim

static void cleanupAndStop(EmberStatus status);
static EmberStatus stateMachineRun(void);
//static uint8_t getNextChannel(void); //jim moved
static EmberStatus tryNextMethod(void);
static EmberStatus setupSecurity(void);
static uint32_t jitterTimeDelayMs();
HIDDEN void scanResultsHandler(EmberAfPluginScanDispatchScanResults *results);
bool emIsWellKnownKey(EmberKeyData key);

// Callback declarations for the sake of the compiler during unit tests.
int8_t emberAfPluginNetworkSteeringGetPowerForRadioChannelCallback(uint8_t channel);
bool emberAfPluginNetworkSteeringGetDistributedKeyCallback(EmberKeyData * key);
EmberNodeType emberAfPluginNetworkSteeringGetNodeTypeCallback(EmberAfPluginNetworkSteeringJoiningState state);

/* 全局函数声明区 -------------------------------------------------------------- */


static bool addPanIdCandidate(uint16_t panId)
{
  uint16_t* panIdPointer = emAfPluginNetworkSteeringGetStoredPanIdPointer(0);
  if (panIdPointer == NULL) {
    customNwSteeringDebugPrintln("Error: %p could not get memory pointer for stored PAN IDs",
                       PLUGIN_NAME);
    return false;
  }
  uint8_t maxNetworks = emAfPluginNetworkSteeringGetMaxPossiblePanIds();
  uint8_t i;
  for (i = 0; i < maxNetworks; i++) {
    if (panId == *panIdPointer) {
      // We already know about this PAN, no point in recording it twice.
      customNwSteeringDebugPrintln("Ignoring duplicate PAN ID 0x%2X", panId);
      return true;
    } else if (*panIdPointer == EMBER_AF_INVALID_PAN_ID) {
      *panIdPointer = panId;
      customNwSteeringDebugPrintln("Stored PAN ID 0x%2X", *panIdPointer);
      return true;
    }
    panIdPointer++;
  }

  if (!printedMaxPanIdsWarning) {
    customNwSteeringDebugPrintln("Warning: %p Max PANs reached (%d)",
                       PLUGIN_NAME,
                       maxNetworks);
    printedMaxPanIdsWarning = true;
  }
  return true;
}

static void clearPanIdCandidates(void)
{
  printedMaxPanIdsWarning = false;
  emAfPluginNetworkSteeringClearStoredPanIds();
  emAfPluginNetworkSteeringPanIdIndex = 0;
}

static uint16_t getNextCandidate(void)
{
  customNwSteeringDebugPrintln("Getting candidate at index %d", emAfPluginNetworkSteeringPanIdIndex);
  uint16_t* pointer =
    emAfPluginNetworkSteeringGetStoredPanIdPointer(emAfPluginNetworkSteeringPanIdIndex);
  if (pointer == NULL) {
    customNwSteeringDebugPrintln("Error: %p could not get pointer to stored PANs", PLUGIN_NAME);
    return EMBER_AF_INVALID_PAN_ID;
  }
  emAfPluginNetworkSteeringPanIdIndex++;
  return *pointer;
}
//jim add
static EmberStatus startScanNextChannelNetwork(void)
{
  EmberAfPluginScanDispatchScanData scanData;
  EmberStatus status;
 if((emAfPluginNetworkSteeringState ==
	EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE) &&
    (ScanningNetworkFlag == 1)) //jim add for network jonig bug 20171124
 {
  if(CurrentScanChannel < EMBER_MIN_802_15_4_CHANNEL_NUMBER)
  {
     CurrentScanChannel = EMBER_MIN_802_15_4_CHANNEL_NUMBER;
  }
  else
  {
     CurrentScanChannel++;
  }

  if(CurrentScanChannel > EMBER_MAX_802_15_4_CHANNEL_NUMBER)
  {
     CurrentScanChannel = EMBER_MAX_802_15_4_CHANNEL_NUMBER;
     // TODO: jim need to order the network and change the statue to try join network
     emberAfPluginScanDispatchClear(); //jim add for network jonig bug 20171124
     orderChannelList();
     ScanningNetworkFlag = 0; //jim add for network jonig bug 20171124
     emberAfPluginScanDispatchClear(); //jim add for network jonig bug 20171124
     return stateMachineRun();
  }
  scanData.scanType = EMBER_ACTIVE_SCAN;
  scanData.channelMask = BIT32(CurrentScanChannel);
  scanData.duration = EMBER_AF_PLUGIN_NETWORK_STEERING_SCAN_DURATION;
  scanData.handler = scanResultsHandler;
  status = emberAfPluginScanDispatchScheduleScan(&scanData);
  customNwSteeringDebugPrintln("Trige scanning"); //jim
 }
 else
 {
  status = EMBER_BAD_ARGUMENT;
 }
  if (EMBER_SUCCESS != status)
  {
     // TODO: jim need add error process  may stop scan
    customNwSteeringDebugPrintln("Error: %p start scan channel %d failed: 0x%X",
		               PLUGIN_NAME,CurrentScanChannel, status);
	 customNetworkSteeringReset();
  }
  else
  {
    customNwSteeringDebugPrintln("Starting scan on channel %d",
                       CurrentScanChannel);
  }
  return status;
}
//end jim
//jim add
static void orderChannelList(void)
{
  int8_t rssitemp;
  uint16_t PanIdTemp;
  uint8_t channelnumbertemp;
  uint8_t extendedPanIdTemp[8];

  if (FoundTotalChannel > 1)
  {
  	//order reference to rssi
	for (uint8_t i = 0; i < FoundTotalChannel-1; i++)
	{
      for (uint8_t j = i+1; j < FoundTotalChannel; j++)
	  {
        if (networkValidChannelSortTable[i].rssi < networkValidChannelSortTable[j].rssi)
		{
          rssitemp = networkValidChannelSortTable[i].rssi;
		  PanIdTemp = networkValidChannelSortTable[i].panId;
		  channelnumbertemp = networkValidChannelSortTable[i].channel;
		  MEMCOPY(extendedPanIdTemp,networkValidChannelSortTable[i].extendedPanId,EUI64_SIZE);

		  networkValidChannelSortTable[i].rssi = networkValidChannelSortTable[j].rssi;
		  networkValidChannelSortTable[i].panId = networkValidChannelSortTable[j].panId;
		  networkValidChannelSortTable[i].channel = networkValidChannelSortTable[j].channel;
		  MEMCOPY(networkValidChannelSortTable[i].extendedPanId,networkValidChannelSortTable[j].extendedPanId,EUI64_SIZE);

		  networkValidChannelSortTable[j].rssi = rssitemp;
		  networkValidChannelSortTable[j].panId = PanIdTemp;
		  networkValidChannelSortTable[j].channel = channelnumbertemp;
		  MEMCOPY(networkValidChannelSortTable[j].extendedPanId,extendedPanIdTemp,EUI64_SIZE);
		}
	  }
    }
	//order reference to ExPenID
	uint8_t NextIndex = 0;
	for (uint8_t i = 0; i < FoundTotalChannel; i++)
	{
      if (MEMCOMPARE(networkValidChannelSortTable[i].extendedPanId, custom_primary_ExPanId, COMPARE_EXPAN_ID_BYTES) == 0)
	  {
          if (NextIndex != i)
		  {
			  rssitemp = networkValidChannelSortTable[i].rssi;
			  PanIdTemp = networkValidChannelSortTable[i].panId;
			  channelnumbertemp = networkValidChannelSortTable[i].channel;
			  MEMCOPY(extendedPanIdTemp,networkValidChannelSortTable[i].extendedPanId,EUI64_SIZE);

		  	 for (uint8_t j = i; j > NextIndex; j--)
			 {
		        networkValidChannelSortTable[j].rssi = networkValidChannelSortTable[j-1].rssi;
				networkValidChannelSortTable[j].panId = networkValidChannelSortTable[j-1].panId;
		        networkValidChannelSortTable[j].channel = networkValidChannelSortTable[j-1].channel;
		        MEMCOPY(networkValidChannelSortTable[j].extendedPanId,networkValidChannelSortTable[j-1].extendedPanId,EUI64_SIZE);
			 }
			 networkValidChannelSortTable[NextIndex].rssi = rssitemp;
			 networkValidChannelSortTable[NextIndex].panId = PanIdTemp;
			 networkValidChannelSortTable[NextIndex].channel = channelnumbertemp;
			 MEMCOPY(networkValidChannelSortTable[NextIndex].extendedPanId,extendedPanIdTemp,EUI64_SIZE);
          }
		  NextIndex++;
	  }
	}
  }
  customNwSteeringDebugPrintln("Found channels: %d ", FoundTotalChannel);
  for (uint8_t i = 0; i < FoundTotalChannel; i++)
  {
  	 customNwSteeringDebugPrintln(" %d . channel: %d  panID 0x%2X  rssi: %d ",i+1,
	 	networkValidChannelSortTable[i].channel,
	 	networkValidChannelSortTable[i].panId,
	 	networkValidChannelSortTable[i].rssi);
  }
}
//end jim

static void tryToJoinNetwork(void)
{
  EmberNetworkParameters networkParams;
  EmberStatus status;
  EmberNodeType nodeType;

  TryJoiningNetworkFlag = 1;
	while(1)
	{
	  if (emAfPluginNetworkSteeringState ==
		  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE)
	  {
		 customNwSteeringDebugPrintln("%p nwk try joining stop!",PLUGIN_NAME);
		 customNetworkSteeringReset();
		 return;
	  }
	  emberAfPluginUpdateTcLinkKeyStop();
	  //setupSecurity();
	  if (emAfPluginNetworkSteeringJoinAttempts >= ONE_CHANNEL_TRY_JOIN_TIME)
	  {
		 if (tryNextMethod() != EMBER_SUCCESS)
		 {
			CurrentOrderNeworkIndex++;
			emAfPluginNetworkSteeringState = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE;
		 }
		 emAfPluginNetworkSteeringJoinAttempts = 0;
	  }

	  if (CurrentOrderNeworkIndex >= FoundTotalChannel)
	  {
		 customNwSteeringDebugPrintln("No more channels");
		 customNetworkSteeringReset();
		 return;
	  }
	  emAfPluginNetworkSteeringJoinAttempts++;
	  customNwSteeringDebugPrintln("%p State: %p",
						 PLUGIN_NAME,
						 emAfPluginNetworkSteeringStateNames[emAfPluginNetworkSteeringState]);
	  if (setupSecurity() == EMBER_SUCCESS)
	  {
		MEMSET(&networkParams, 0, sizeof(EmberNetworkParameters));
		clearPanIdCandidates();

		 //addPanIdCandidate(networkValidChannelSortTable[CurrentOrderNeworkIndex].panId); //jim modify 20171122
		   //networkParams.panId = getNextCandidate(); //jim modify 20171122
		   networkParams.panId = networkValidChannelSortTable[CurrentOrderNeworkIndex].panId; //jim modify 20171122
		 emAfPluginNetworkSteeringCurrentChannel=networkValidChannelSortTable[CurrentOrderNeworkIndex].channel;
		   customNwSteeringDebugPrintln("%p channel %d joining 0x%2x", PLUGIN_NAME, emAfPluginNetworkSteeringCurrentChannel ,networkParams.panId);
			 networkParams.radioChannel = emAfPluginNetworkSteeringCurrentChannel;
			 networkParams.radioTxPower = emberAfPluginNetworkSteeringGetPowerForRadioChannelCallback(emAfPluginNetworkSteeringCurrentChannel);
			 nodeType = emberAfPluginNetworkSteeringGetNodeTypeCallback(emAfPluginNetworkSteeringState);
			 status = emberJoinNetwork(nodeType, &networkParams);

		if (EMBER_SUCCESS != status)
		{
		  customNwSteeringDebugPrintln("Error: %p could not attempt join: 0x%X",
							 PLUGIN_NAME,
							 status);
		  // TODO: jim need add error process  may try next.
		  //emberNetworkInit();  //jim modify 20171124
		}
		else
			break;
	  }
	}
  TryJoiningNetworkFlag=0;
}
//Description: Generates a random number between 10000-40000.
static uint32_t jitterTimeDelayMs()
{
  uint16_t seed;
  halStackSeedRandom((uint32_t)&seed);
  uint32_t jitterDelayMs = (emberGetPseudoRandomNumber() % (UPDATE_TC_LINK_KEY_JITTER_MAX_MS - UPDATE_TC_LINK_KEY_JITTER_MIN_MS + 1)) + UPDATE_TC_LINK_KEY_JITTER_MIN_MS;
  return jitterDelayMs;
}

void emberAfPluginNetworkSteeringStackStatusCallback(EmberStatus status)
{
  if (emAfPluginNetworkSteeringState
      == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE) {
    EmberKeyStruct entry;
    EmberStatus keystatus = emberGetKey(EMBER_TRUST_CENTER_LINK_KEY, &entry);
    if (keystatus == EMBER_SUCCESS && emIsWellKnownKey(entry.key) && status == EMBER_NETWORK_UP) {
      emberAfPluginUpdateTcLinkKeySetDelay(jitterTimeDelayMs());
    } else if (status == EMBER_NETWORK_DOWN) {
      emberAfPluginUpdateTcLinkKeySetInactive();
    }
    return;
  } else if (status == EMBER_NETWORK_UP) {
    customNwSteeringDebugPrintln("%p network joined.", PLUGIN_NAME);
    if (!emAfPluginNetworkSteeringStateUsesDistributedKey()
        && !(emAfPluginNetworkSteeringOptionsMask
             & EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIONS_NO_TCLK_UPDATE)) {
      emAfPluginNetworkSteeringStateSetUpdateTclk();
    }
    emberEventControlSetDelayMS(finishSteeringEvent, randomJitterMS());
    return;
  }

//  tryToJoinNetwork(); //jim modify
 #if 1
  else
  {
    if (emberAfNetworkState() == EMBER_NO_NETWORK)
    {
       customNetworkSteeringRestart(CUSTOM_NW_STEERING_DO_NEXT_STEP); //try next
    }
  }
 #endif
}

// Returns true if the key value is equal to defaultLinkKey
bool emIsWellKnownKey(EmberKeyData key)
{
  for (uint8_t i = 0; i < EMBER_ENCRYPTION_KEY_SIZE; i++) {
    if (key.contents[i] != defaultLinkKey.contents[i]) {
      return false;
    }
  }
  return true;
}

static void scanCompleteCallback(uint8_t channel, EmberStatus status)
{
  if (status != EMBER_SUCCESS) {
    customNwSteeringDebugPrintln("Error: Scan complete handler returned 0x%X", status);
    cleanupAndStop(status);
    return;
  }

  // EMAPPFWKV2-1462 - make sure we didn't cleanupAndStop() above.
  if (emAfPluginNetworkSteeringState
      != EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE) {
    tryToJoinNetwork();
  }
}

static void networkFoundAndStoreChannelCallback(EmberZigbeeNetwork *networkFound,
                                 uint8_t lqi,
                                 int8_t rssi)
{
uint8_t i=0;
  if (!(networkFound->allowingJoin
        && networkFound->stackProfile == REQUIRED_STACK_PROFILE)) {
    return;
  }
  if(emAfPluginNetworkSteeringState==
	  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE)
  {
     customNwSteeringDebugPrintln("%p nwk found stop!",PLUGIN_NAME);
	 customNetworkSteeringReset();
	 return;
  }

  customNwSteeringDebugPrint("%p nwk found ch: %d, panID 0x%2X, xpan: ",
             PLUGIN_NAME,
             networkFound->channel,
             networkFound->panId);
  customNwSteeringDebugPrint(
               "(%c)%X%X%X%X%X%X%X%X",
               '>',
               networkFound->extendedPanId[7],
               networkFound->extendedPanId[6],
               networkFound->extendedPanId[5],
               networkFound->extendedPanId[4],
               networkFound->extendedPanId[3],
               networkFound->extendedPanId[2],
               networkFound->extendedPanId[1],
               networkFound->extendedPanId[0]);
  customNwSteeringDebugPrintln("");
  if (FoundTotalChannel == 0)
  {
    networkValidChannelSortTable[FoundTotalChannel].channel=networkFound->channel;
	networkValidChannelSortTable[FoundTotalChannel].panId=networkFound->panId;
    networkValidChannelSortTable[FoundTotalChannel].rssi=rssi;
	MEMCOPY(networkValidChannelSortTable[FoundTotalChannel].extendedPanId,networkFound->extendedPanId,EUI64_SIZE);
	FoundTotalChannel++;
  }
  else
  {
//store the different expanId and bypass the same
    for (i=0; i<FoundTotalChannel; i++)
  	{
  	  if (networkValidChannelSortTable[i].channel == networkFound->channel)
  	  {
  	     //if (networkFound->panId == networkValidChannelSortTable[i].panId) //jim modify 20180716 the expanid is the uniqueness
         if (MEMCOMPARE(networkFound->extendedPanId,networkValidChannelSortTable[i].extendedPanId, 8) == 0)
  	     {
            if (rssi > networkValidChannelSortTable[i].rssi) //jim add 20180716 for update the better rssi on the same expid
            {
               networkValidChannelSortTable[i].rssi = rssi;
            }
  	        break;
  	     }
  	  }
    }
    if (i == FoundTotalChannel)
    {
		if (FoundTotalChannel < MAX_NETWORK_ORDER_STORE)
		{
		  networkValidChannelSortTable[i].channel = networkFound->channel;
		  networkValidChannelSortTable[i].panId = networkFound->panId;
		  networkValidChannelSortTable[i].rssi = rssi;
		  MEMCOPY(networkValidChannelSortTable[i].extendedPanId,networkFound->extendedPanId,EUI64_SIZE);
		  FoundTotalChannel++;
		}
		else //jim add for the full buffer process 20171122
		{
		  uint8_t temp_worst_rssi_index,primary_ExPanId_worst_rssi_index;
		  int8_t temp_worst_rssi,primary_ExPanId_worst_rssi;

		   primary_ExPanId_worst_rssi_index = 0xFF;
		   temp_worst_rssi_index = 0xFF;
		   for (i = 0; i < MAX_NETWORK_ORDER_STORE; i++) //found the worst rssi index
		   {
		      if (i == 0)
    		  {
 			  	primary_ExPanId_worst_rssi = networkValidChannelSortTable[i].rssi;
				temp_worst_rssi = networkValidChannelSortTable[i].rssi;
    		  }
    		  if (MEMCOMPARE(networkValidChannelSortTable[i].extendedPanId, custom_primary_ExPanId, COMPARE_EXPAN_ID_BYTES) == 0)
    		  {
    		    if (primary_ExPanId_worst_rssi >= networkValidChannelSortTable[i].rssi)
    		    {
                   primary_ExPanId_worst_rssi = networkValidChannelSortTable[i].rssi;
 				   primary_ExPanId_worst_rssi_index = i;
    		    }
    		  }
 			  else
 			  {
 			    if (temp_worst_rssi >= networkValidChannelSortTable[i].rssi)
    		    {
                   temp_worst_rssi = networkValidChannelSortTable[i].rssi;
 				   temp_worst_rssi_index = i;
    		    }
 			  }
		   }
		   if (MEMCOMPARE(networkFound->extendedPanId,custom_primary_ExPanId, COMPARE_EXPAN_ID_BYTES) == 0)
		   {
		     if ((temp_worst_rssi_index == 0xFF) && (primary_ExPanId_worst_rssi_index != 0xFF))
		     {
		       temp_worst_rssi_index = primary_ExPanId_worst_rssi_index;
		     }
		   }

		   if ((temp_worst_rssi_index != 0xFF) && (rssi > networkValidChannelSortTable[temp_worst_rssi_index].rssi))
		   {
    		   networkValidChannelSortTable[temp_worst_rssi_index].channel = networkFound->channel;
    		   networkValidChannelSortTable[temp_worst_rssi_index].panId = networkFound->panId;
    		   networkValidChannelSortTable[temp_worst_rssi_index].rssi = rssi;
    		   MEMCOPY(networkValidChannelSortTable[temp_worst_rssi_index].extendedPanId,networkFound->extendedPanId,EUI64_SIZE);
		   }
		}
	}
  }
}

HIDDEN void scanResultsHandler(EmberAfPluginScanDispatchScanResults *results)
{
  if (emberAfPluginScanDispatchScanResultsAreComplete(results)
      || emberAfPluginScanDispatchScanResultsAreFailure(results))
  {
	  startScanNextChannelNetwork();
  }
  else
  {
     networkFoundAndStoreChannelCallback(results->network,
					 results->lqi,
					 results->rssi);
  }
}

static EmberStatus tryNextMethod(void)
{
  emAfPluginNetworkSteeringState++;

  if ((emAfPluginNetworkSteeringState == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_INSTALL_CODE) ||
      (emAfPluginNetworkSteeringState == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_CENTRALIZED) ||
      (emAfPluginNetworkSteeringState == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_DISTRIBUTED))
     emAfPluginNetworkSteeringState++;

  if (emAfPluginNetworkSteeringState > LAST_JOINING_STATE)
  	return EMBER_JOIN_FAILED;
  else
  	return EMBER_SUCCESS;
}

static void cleanupAndStop(EmberStatus status)
{
  customNwSteeringDebugPrintln("%p Stop.  Cleaning up.", PLUGIN_NAME);
  emberAfPluginNetworkSteeringCompleteCallback(status,
                                               emAfPluginNetworkSteeringTotalBeacons,
                                               emAfPluginNetworkSteeringJoinAttempts,
                                               emAfPluginNetworkSteeringState);
  clearPanIdCandidates();
}
static uint8_t getNextChannel(void)
{
  if (emAfPluginNetworkSteeringCurrentChannel == 0) {
    emAfPluginNetworkSteeringCurrentChannel = (emberGetPseudoRandomNumber() & 0x0F)
                                              + EMBER_MIN_802_15_4_CHANNEL_NUMBER;
    customNwSteeringDebugPrintln("Randomly choosing a starting channel %d.", emAfPluginNetworkSteeringCurrentChannel);
  } else {
    emAfPluginNetworkSteeringCurrentChannel++;
  }
  while (currentChannelMask != 0) {
    if (BIT32(emAfPluginNetworkSteeringCurrentChannel) & currentChannelMask) {
      currentChannelMask &= ~(BIT32(emAfPluginNetworkSteeringCurrentChannel));
      return emAfPluginNetworkSteeringCurrentChannel;
    }
    emAfPluginNetworkSteeringCurrentChannel++;
    if (emAfPluginNetworkSteeringCurrentChannel > EMBER_MAX_802_15_4_CHANNEL_NUMBER) {
      emAfPluginNetworkSteeringCurrentChannel = EMBER_MIN_802_15_4_CHANNEL_NUMBER;
    }
  }
  return 0;
}

static EmberStatus stateMachineRun(void)
{
  EmberStatus status = EMBER_SUCCESS;
  if (FoundTotalChannel == 0)
  {
	 customNetworkSteeringReset();
	 customNwSteeringDebugPrintln("%p no network found!", PLUGIN_NAME);
	 return status;
  }

  CurrentOrderNeworkIndex = 0;
  emAfPluginNetworkSteeringJoinAttempts = 0;
  tryToJoinNetwork();
  return status;
}
#ifdef USE_CUSTOM_INSTALLCODE_CODE   //引入调用的函数声明
/**
//函数名：checkCustomInstallCodeAndChangeToKey
//描述  ：检验token中的InstallCode并转换为key
//参数  ：*key (EmberKeyData *[输出]，转换后的key值存放指针)
//返回  ：EmberStatus EMBER_SUCCESS：存在；其它值：不存在，或者不正确
*/
EmberStatus checkCustomInstallCodeAndChangeToKey(EmberKeyData *key);
WEAK(EmberStatus checkCustomInstallCodeAndChangeToKey(EmberKeyData *key)
{
  return EMBER_NOT_FOUND;
})
static EmberStatus setupSecurity(void)
{
  EmberStatus status;
  EmberInitialSecurityState state;
  EmberExtendedSecurityBitmask extended;

  state.bitmask = (EMBER_TRUST_CENTER_GLOBAL_LINK_KEY
                   | EMBER_HAVE_PRECONFIGURED_KEY
                   | EMBER_REQUIRE_ENCRYPTED_KEY
                   | EMBER_NO_FRAME_COUNTER_RESET
                   | (emAfPluginNetworkSteeringStateUsesDistributedKey()
                      ? EMBER_DISTRIBUTED_TRUST_CENTER_MODE
                      : 0)
                   );

  if (!emberAfPluginNetworkSteeringGetDistributedKeyCallback(&emberPluginNetworkSteeringDistributedKey)) {
    MEMMOVE(emberKeyContents(&emberPluginNetworkSteeringDistributedKey),
            emberKeyContents(&distributedTestKey),
            EMBER_ENCRYPTION_KEY_SIZE);
  }

  if (emAfPluginNetworkSteeringStateUsesInstallCodes())
  {
    //在install code模式下
    EmberKeyData install_to_key;
    status = checkCustomInstallCodeAndChangeToKey(&install_to_key);
    if (status != EMBER_SUCCESS)
    {
      goto done;
    }
    else
    {
      MEMMOVE(emberKeyContents(&(state.preconfiguredKey)),
              emberKeyContents(&install_to_key),
              EMBER_ENCRYPTION_KEY_SIZE);
    }
  }
  else
  {
    MEMMOVE(emberKeyContents(&(state.preconfiguredKey)),
            gUseConfiguredKey ? emberKeyContents(&(gConfiguredKey))
            : (emAfPluginNetworkSteeringStateUsesDistributedKey()
               ? emberKeyContents(&emberPluginNetworkSteeringDistributedKey)
               : emberKeyContents(&defaultLinkKey)),
            EMBER_ENCRYPTION_KEY_SIZE);
  }

  if ((status = emberSetInitialSecurityState(&state))
      != EMBER_SUCCESS) {
    goto done;
  }

  extended = (EMBER_JOINER_GLOBAL_LINK_KEY
              | EMBER_EXT_NO_FRAME_COUNTER_RESET);

  if ((status = emberSetExtendedSecurityBitmask(extended)) != EMBER_SUCCESS) {
    goto done;
  }

  emAfClearLinkKeyTable();

  done:
  return status;
}
#else
static EmberStatus setupSecurity(void)
{
  EmberStatus status;
  EmberInitialSecurityState state;
  EmberExtendedSecurityBitmask extended;

  state.bitmask = (EMBER_TRUST_CENTER_GLOBAL_LINK_KEY
                   | EMBER_HAVE_PRECONFIGURED_KEY
                   | EMBER_REQUIRE_ENCRYPTED_KEY
                   | EMBER_NO_FRAME_COUNTER_RESET
                   | (emAfPluginNetworkSteeringStateUsesInstallCodes()
                      ? EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE
                      : 0)
                   | (emAfPluginNetworkSteeringStateUsesDistributedKey()
                      ? EMBER_DISTRIBUTED_TRUST_CENTER_MODE
                      : 0)
                   );

  if (!emberAfPluginNetworkSteeringGetDistributedKeyCallback(&emberPluginNetworkSteeringDistributedKey)) {
    MEMMOVE(emberKeyContents(&emberPluginNetworkSteeringDistributedKey),
            emberKeyContents(&distributedTestKey),
            EMBER_ENCRYPTION_KEY_SIZE);
  }
  MEMMOVE(emberKeyContents(&(state.preconfiguredKey)),
          gUseConfiguredKey ? emberKeyContents(&(gConfiguredKey))
          : (emAfPluginNetworkSteeringStateUsesDistributedKey()
             ? emberKeyContents(&emberPluginNetworkSteeringDistributedKey)
             : emberKeyContents(&defaultLinkKey)),
          EMBER_ENCRYPTION_KEY_SIZE);

  if ((status = emberSetInitialSecurityState(&state))
      != EMBER_SUCCESS) {
    goto done;
  }

  extended = (EMBER_JOINER_GLOBAL_LINK_KEY
              | EMBER_EXT_NO_FRAME_COUNTER_RESET);

  if ((status = emberSetExtendedSecurityBitmask(extended)) != EMBER_SUCCESS) {
    goto done;
  }

  emAfClearLinkKeyTable();

  done:
  return status;
}
#endif

EmberStatus emberAfPluginNetworkSteeringStart(void)
{
  EmberStatus status = EMBER_INVALID_CALL;

  if (emAfProIsCurrentNetwork()
      && (emAfPluginNetworkSteeringState
          == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE))
   {
     if (emberAfNetworkState() == EMBER_NO_NETWORK)
	 {
       emAfPluginNetworkSteeringState
         = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE;

      // Stop any previous trust center link key update.
      emberAfPluginUpdateTcLinkKeyStop();

      clearPanIdCandidates();

	  FoundTotalChannel = 0;
	  CurrentScanChannel = 0;
	  TryJoiningNetworkFlag = 0;
      ScanningNetworkFlag = 1 ; //start scanning network 20171124 network joining bug
	  status=startScanNextChannelNetwork();
    }
  }

  customNwSteeringDebugPrintln("%p: %p: 0x%X",
                     emAfNetworkSteeringPluginName,
                     "Start",
                     status);

  EmberNetworkStatus StateTemp= emberAfNetworkState();
  customNwSteeringDebugPrintln("Network Status %d", StateTemp);
  customNwSteeringDebugPrintln("Steering Status %d", emAfPluginNetworkSteeringState);
  if (emAfProIsCurrentNetwork()==false)
    customNwSteeringDebugPrintln("Current Networ None!!!!");

  return status;
}
EmberStatus emberAfPluginNetworkSteeringStop(void)
{
  if (emAfPluginNetworkSteeringState
      == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE) {
    return EMBER_INVALID_CALL;
  }
  cleanupAndStop(EMBER_JOIN_FAILED);
  customNetworkSteeringReset();//jim add
  return EMBER_SUCCESS;
}
//jim add
uint8_t customNetworkSteeringHasNetworkToTry(void)
{
      return FoundTotalChannel;
}
EmberStatus  customNetworkSteeringRestart(uint8_t NextChannelFlag)
{
EmberStatus status = EMBER_INVALID_CALL;
customNwSteeringDebugPrintln("TRY NEXT RESTART");//jim 20170630
   if (emAfProIsCurrentNetwork())
   {
	  if ((emberAfNetworkState() == EMBER_NO_NETWORK) &&
	  	 (emAfPluginNetworkSteeringState != EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE) &&
	  	 (TryJoiningNetworkFlag == 0))
	  {
             if (CurrentOrderNeworkIndex < FoundTotalChannel)
             {
                if (NextChannelFlag == CUSTOM_NW_STEERING_TRY_CURRENT_NW) //current channel
                {
                  if (emAfPluginNetworkSteeringJoinAttempts > 0 )
                  {
                     emAfPluginNetworkSteeringJoinAttempts--;
                  }
                }
				else if (NextChannelFlag == CUSTOM_NW_STEERING_TRY_NEXT_NW)//next channel
				{
				  CurrentOrderNeworkIndex++;
		          emAfPluginNetworkSteeringState = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE;
	              emAfPluginNetworkSteeringJoinAttempts = 0;
				}
             }
			tryToJoinNetwork();
			status = EMBER_SUCCESS;
      }
   }
   return status;
}
void customNetworkSteeringReset(void)
{
      clearPanIdCandidates();
	  FoundTotalChannel = 0;
	  emAfPluginNetworkSteeringCurrentChannel = 0;
	  emAfPluginNetworkSteeringPanIdIndex = 0;
	  emAfPluginNetworkSteeringTotalBeacons = 0;
	  emAfPluginNetworkSteeringState = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE;
	  CurrentScanChannel = 0;
	  TryJoiningNetworkFlag = 0;
}
//end jim
// =============================================================================
// Finish Steering

// At the end of the network steering process, we need to update the
// trust center link key (if we are in a centralized network) and broadcast
// a permit join to extend the network. This process needs to happen after
// we send our device announce and possibly our network timeout request if we
// are an end device.
void emberAfPluginNetworkSteeringFinishSteeringEventHandler(void)
{
  EmberStatus status;

  emberEventControlSetInactive(finishSteeringEvent);

  if (emAfPluginNetworkSteeringStateVerifyTclk()) {
    // If we get here, then we have failed to verify the TCLK. Therefore,
    // we leave the network.
    emberAfPluginUpdateTcLinkKeyStop();
    emberLeaveNetwork();
    customNwSteeringDebugPrintln("%p: %p",
                       PLUGIN_NAME,
                       "Key verification failed. Leaving network");
    cleanupAndStop(EMBER_ERR_FATAL);
  } else if (emAfPluginNetworkSteeringStateUpdateTclk()) {
    // Start the process to update the TC link key. We will set another event
    // for the broadcast permit join.
    // Attempt a TC link key update now.
    emberAfPluginUpdateTcLinkKeySetDelay(0);
  } else {
    // Broadcast permit join to extend the network.
    // We are done!
    status = emberAfPermitJoin(EMBER_AF_PLUGIN_NETWORK_STEERING_COMMISSIONING_TIME_S,
                               true); // Broadcast permit join?
    customNwSteeringDebugPrintln("%p: %p: 0x%X",
                       PLUGIN_NAME,
                       "Broadcasting permit join",
                       status);
    cleanupAndStop(status);
    customNetworkSteeringReset();//jim add
  }
}

void emberAfPluginUpdateTcLinkKeyStatusCallback(EmberKeyStatus keyStatus)
{
  if (emAfPluginNetworkSteeringStateUpdateTclk()) {
    customNwSteeringDebugPrintln("%p: %p: 0x%X",
                       PLUGIN_NAME,
                       "Trust center link key update status",
                       keyStatus);
    switch (keyStatus) {
      case EMBER_TRUST_CENTER_LINK_KEY_ESTABLISHED:
        // Success! But we should still wait to make sure we verify the key.
        emAfPluginNetworkSteeringStateSetVerifyTclk();
        emberEventControlSetDelayMS(finishSteeringEvent, VERIFY_KEY_TIMEOUT_MS);
        return;
      case EMBER_TRUST_CENTER_IS_PRE_R21:
      case EMBER_VERIFY_LINK_KEY_SUCCESS:
        // If the trust center is pre-r21, then we don't update the link key.
        // If the key status is that the link key has been verified, then we
        // have successfully updated our trust center link key and we are done!
        emAfPluginNetworkSteeringStateClearVerifyTclk();
        emberEventControlSetDelayMS(finishSteeringEvent, randomJitterMS());
        break;
      default:
        // Failure!
        emberLeaveNetwork();
        cleanupAndStop(EMBER_NO_LINK_KEY_RECEIVED);
    }
    emAfPluginNetworkSteeringStateClearUpdateTclk();
  }

  return;
}

void emAfPluginNetworkSteeringSetChannelMask(uint32_t mask, bool secondaryMask)
{
  if (secondaryMask) {
    emAfPluginNetworkSteeringSecondaryChannelMask = mask;
  } else {
    emAfPluginNetworkSteeringPrimaryChannelMask = mask;
  }
}

void emAfPluginNetworkSteeringSetExtendedPanIdFilter(uint8_t* extendedPanId,
                                                     bool turnFilterOn)
{
  if (!extendedPanId) {
    return;
  }
  MEMCOPY(gExtendedPanIdToFilterOn,
          extendedPanId,
          COUNTOF(gExtendedPanIdToFilterOn));
  gFilterByExtendedPanId = turnFilterOn;
}

void emAfPluginNetworkSteeringSetConfiguredKey(uint8_t *key,
                                               bool useConfiguredKey)
{
  if (!key) {
    return;
  }
  MEMCOPY(gConfiguredKey.contents, key, EMBER_ENCRYPTION_KEY_SIZE);
  gUseConfiguredKey = useConfiguredKey;
}

void emAfPluginNetworkSteeringCleanup(EmberStatus status)
{
  cleanupAndStop(status);
}
#ifdef ORB_PRODUCTION_TEST_CODE  //产测指令用到的启动加入特定网络指令
/**
//函数名：startJoinSpecificNetwork
//描  述：开始加入指定网络
//参  数：channel (uint8_t  [输入]，需要加入网络的信道)
//        pand_id (uint16_t [输入]，需要加入网络的pand id)
//返  回：EmberStatus EMBER_SUCCESS：正常启动；其它值：启动异常
*/
EmberStatus startJoinSpecificNetwork(uint8_t channel, uint16_t pand_id)
{
  EmberStatus status = EMBER_INVALID_CALL;
  if (emberAfNetworkState() == EMBER_NO_NETWORK)
  {
    if ((channel >= EMBER_MIN_802_15_4_CHANNEL_NUMBER) &&
        (channel <= EMBER_MAX_802_15_4_CHANNEL_NUMBER))
    {
      emberAfPluginNetworkSteeringStop(); //停止之前的加网操作
      status = setupSecurity();
      if (status == EMBER_SUCCESS)
      {
        EmberNetworkParameters network_params;
        EmberNodeType node_type;
        MEMSET(&network_params, 0, sizeof(EmberNetworkParameters));
  	  network_params.panId = pand_id;
  	  network_params.radioChannel = channel;
  	  network_params.radioTxPower = emberAfPluginNetworkSteeringGetPowerForRadioChannelCallback(channel);
  	  node_type = (emAfCurrentZigbeeProNetwork->nodeType == EMBER_COORDINATOR) \
                    ? EMBER_ROUTER : emAfCurrentZigbeeProNetwork->nodeType;
  	  status = emberJoinNetwork(node_type, &network_params);
      }
    }
  }
  return status;
}
#endif
#endif
/*************************************** 文 件 结 束 ******************************************/
