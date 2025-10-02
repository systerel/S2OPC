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

/** \file
 *
 * \brief Defines the cryptographic abstraction interface for the constant profiles.
 *        A cryptographic implementation must define all the constants declared in this file,
 *        based on their specific implementation of \a SOPC_CryptoProfile
 */

#ifndef SOPC_CRYPTO_PROFILES_LIB_ITF_H_
#define SOPC_CRYPTO_PROFILES_LIB_ITF_H_

#include "sopc_crypto_decl.h"

/**
 * This function should return true if any other profile than "None" is supported
 */
bool SOPC_CryptoProfile_Is_Implemented(void);

#endif /* SOPC_CRYPTO_PROFILES_LIB_ITF_H_ */
