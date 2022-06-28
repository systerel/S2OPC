#ifndef __NETWORK_INIT_H__
#define __NETWORK_INIT_H__

#include <stdbool.h>

/** \brief
 * Configure the network for ZEPHYR with LOOPBACK and ETH0
 * \param overrideEthAddr Ip address to set-up eth0 card.
 *  \a CONFIG_SOPC_ETH_ADDRESS is used if NULL.
 */
bool Network_Initialize(const char* overrideEthAddr);

#endif /* __NETWORK_INIT_H__ */
