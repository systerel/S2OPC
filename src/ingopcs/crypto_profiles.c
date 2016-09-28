/*
 * Defines the library specific init and deinit.
 *
 *  Created on: Sep 28, 2016
 *      Author: PAB
 */


#include <stdlib.h>

#include "crypto_profiles.h"


const CryptoProfile *CryptoProfile_Get(const char *uri)
{
    if(strncmp(uri, SecurityPolicy_Basic256Sha256_URI, strlen(SecurityPolicy_Basic256Sha256_URI)) == 0)
        return &g_cpBasic256Sha256;

    return UA_NULL;
}
