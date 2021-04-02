/*
 * EthernetConfig.h
 *
 *  Created on: 9 août 2019
 *      Author: nottin
 */

#ifndef ETHERNETCONFIG_H_
#define ETHERNETCONFIG_H_

#define IP_ADDRESS_0 192
#define IP_ADDRESS_1 168
#define IP_ADDRESS_2 1
#define IP_ADDRESS_3 102

#define IP_MASK_0 255
#define IP_MASK_1 255
#define IP_MASK_2 255
#define IP_MASK_3 0

#define IP_GW_0 192
#define IP_GW_1 168
#define IP_GW_2 1
#define IP_GW_3 100

#define PHY_MAC_ADDRESS                    \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x11 \
    }

#endif /* ETHERNETCONFIG_H_ */
