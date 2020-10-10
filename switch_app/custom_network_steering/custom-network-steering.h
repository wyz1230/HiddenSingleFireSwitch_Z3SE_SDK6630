/***************************************************************************************************
*Copyright(C),2019, Shenzhen ORVIBO Technology Co., Ltd.
*FileName   : custom-network-steering.h
*Author     : JimYao
*Version    : 1.0
*Date       : 2019-11-04
*Description: 客制zigbee3.0加网
*History:
***************************************************************************************************/

/* 防止重复引用区 ----------------------------------------------------------- */

#ifndef _CUSTOM_NETWORK_STEERING_H_
#define _CUSTOM_NETWORK_STEERING_H_

#if defined(USE_CUSTOM_NETWORK_STEERING_CODE)   //jim add 20191104 for the new networkscan
/* 相关包含头文件区 --------------------------------------------------------- */
#include "app/framework/include/af.h"

/* 宏定义区 ----------------------------------------------------------------- */
#define CUSTOM_NW_STEERING_TRY_CURRENT_NW     0x00
#define CUSTOM_NW_STEERING_TRY_NEXT_NW        0x01
#define CUSTOM_NW_STEERING_DO_NEXT_STEP       0x02

/* 自定义类型区 ------------------------------------------------------------- */
extern const uint8_t emAfNetworkSteeringPluginName[];

enum {
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE                         = 0x00,
  // These next two states are only run if explicitly configured to do so
  // See emAfPluginNetworkSteeringSetConfiguredKey()
//  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CONFIGURED      = 0x01,
//  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_CONFIGURED    = 0x02,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE    = 0x01,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_INSTALL_CODE  = 0x02,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CENTRALIZED     = 0x03,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_CENTRALIZED   = 0x04,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_DISTRIBUTED     = 0x05,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_DISTRIBUTED   = 0x06,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_FINISHED                = 0x07,

  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_UPDATE_TCLK                  = 0x10,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_VERIFY_TCLK                  = 0x20,
};
typedef uint8_t EmberAfPluginNetworkSteeringJoiningState;

enum {
  EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIONS_NONE                       = 0x00,
  EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIONS_NO_TCLK_UPDATE             = 0x01,
};
typedef uint8_t EmberAfPluginNetworkSteeringOptions;

extern EmberAfPluginNetworkSteeringOptions emAfPluginNetworkSteeringOptionsMask;

/** @brief The first set of channels on which to search for joinable networks. */
extern uint32_t emAfPluginNetworkSteeringPrimaryChannelMask;
/** @brief The second set of channels on which to search for joinable networks. */
extern uint32_t emAfPluginNetworkSteeringSecondaryChannelMask;

/* 全局函数声明区 ----------------------------------------------------------- */


/** @brief Initiates a network-steering procedure.
 *
 *
 * If the node is currently on a network, it will perform network steering,
 * in which it opens up the network with a broadcast
 * permit join message.
 *
 * If the node is not on a network, it will scan a series of primary channels
 * (see ::emAfPluginNetworkSteeringPrimaryChannelMask) to find possible
 * networks to join. If it is unable to join any of those networks, it will
 * try scanning on a set of secondary channels
 * (see ::emAfPluginNetworkSteeringSecondaryChannelMask). Upon completion of
 * this process, the plugin will call
 * ::emberAfPluginNetworkSteeringCompleteCallback with information regarding
 * the success or failure of the procedure.
 *
 * This procedure will try to join networks using install codes, the centralized
 * default key, and the distributed default key.
 *
 * @return An ::EmberStatus value that indicates the success or failure of
 * the initiating of the network steering process.
 *
 * @note Do not call this API from a stack status callback, as this plugin acts
 * when its own stack status callback is invoked.
 */
EmberStatus emberAfPluginNetworkSteeringStart(void);

/** @brief Stops the network steering procedure.
 *
 *
 * @return An ::EmberStatus value that indicates the success or failure of
 * the initiating of the network steering process.
 */
EmberStatus emberAfPluginNetworkSteeringStop(void);

/** @brief Overrides the channel mask. */
void emAfPluginNetworkSteeringSetChannelMask(uint32_t mask, bool secondaryMask);

/** @brief Sets extended PAN ID to search for. */
void emAfPluginNetworkSteeringSetExtendedPanIdFilter(uint8_t* extendedPanId,
                                                     bool turnFilterOn);

/** @brief Set a different key to use when joining. */
void emAfPluginNetworkSteeringSetConfiguredKey(uint8_t *key,
                                               bool useConfiguredKey);

/** @brief Cleans up the network steering process which took place */
void emAfPluginNetworkSteeringCleanup(EmberStatus status);

EmberStatus  customNetworkSteeringRestart(uint8_t NextChannelFlag);//jim add
void customNetworkSteeringReset(void); //jim add
uint8_t customNetworkSteeringHasNetworkToTry(void);//jim add

#endif
#endif

/***************************************** 文 件 结 束 ************************************************/
