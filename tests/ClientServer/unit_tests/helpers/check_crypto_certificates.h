/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef SOPC_CHECK_CRYPTO_CERTIFICATES_H_
#define SOPC_CHECK_CRYPTO_CERTIFICATES_H_

#include <stddef.h>

#include "sopc_key_manager.h"

// server_2k_cert.der
extern const char* SRV_CRT;
extern const char* SRV_CRT_THUMB;

// cacert.der
extern const char* CA_CRT;
extern const size_t CA_CRT_LEN;

// cacrl.der
extern const char* CA_CRL;
extern const size_t CA_CRL_LEN;

SOPC_CertificateList* SOPC_UnhexlifyCertificate(const char* hex_data);
SOPC_CRLList* SOPC_UnhexlifyCRL(const char* hex_data);

#endif /* SOPC_CHECK_CRYPTO_CERTIFICATES_H_ */
