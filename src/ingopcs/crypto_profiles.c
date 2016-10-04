/*
 * Defines the library specific init and deinit.
 *
 *  Created on: Sep 28, 2016
 *      Author: PAB
 */


#include <string.h>

#include "crypto_profiles.h"


const CryptoProfile *CryptoProfile_Get(const uint8_t *uri)
{
    if(strncmp(uri, SecurityPolicy_Basic256Sha256_URI, strlen(SecurityPolicy_Basic256Sha256_URI)) == 0)
        return &g_cpBasic256Sha256;

    return UA_NULL;
}
