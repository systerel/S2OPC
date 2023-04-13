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
 * \privatesection
 *
 * \brief Internal module to share parsing of common parts
 *        between client and server XML configurations
 */

#ifndef SOPC_CONFIG_LOADER_INTERNAL_H_
#define SOPC_CONFIG_LOADER_INTERNAL_H_

#include <stdbool.h>

#include "sopc_array.h"
#include "sopc_helper_expat.h"
#include "sopc_types.h"

bool SOPC_ConfigLoaderInternal_end_locales(bool isServer,
                                           SOPC_HelperExpatCtx* ctx,
                                           SOPC_Array* ctxLocaleIds,
                                           char*** configLocaleIds);

bool SOPC_ConfigLoaderInternal_start_locale(SOPC_HelperExpatCtx* ctx, SOPC_Array* ctxLocaleIds, const char** attrs);

bool SOPC_ConfigLoaderInternal_end_app_desc(bool isServer,
                                            SOPC_HelperExpatCtx* ctx,
                                            OpcUa_ApplicationDescription* appDesc);

bool SOPC_ConfigLoaderInternal_start_app_uri(bool isServer,
                                             SOPC_HelperExpatCtx* ctx,
                                             OpcUa_ApplicationDescription* appDesc,
                                             const char** attrs);

bool SOPC_ConfigLoaderInternal_start_prod_uri(SOPC_HelperExpatCtx* ctx,
                                              OpcUa_ApplicationDescription* appDesc,
                                              const char** attrs);

bool SOPC_ConfigLoaderInternal_start_app_type(bool isServer,
                                              SOPC_HelperExpatCtx* ctx,
                                              OpcUa_ApplicationDescription* appDesc,
                                              const char** attrs);

bool SOPC_ConfigLoaderInternal_start_app_name(bool isServer,
                                              SOPC_HelperExpatCtx* ctx,
                                              OpcUa_ApplicationDescription* appDesc,
                                              char** configLocaleIds,
                                              const char** attrs);

bool SOPC_ConfigLoaderInternal_start_cert(bool isServer,
                                          SOPC_HelperExpatCtx* ctx,
                                          char** certificate,
                                          const char** attrs);

bool SOPC_ConfigLoaderInternal_start_key(bool isServer,
                                         SOPC_HelperExpatCtx* ctx,
                                         char** key,
                                         bool* encrypted,
                                         const char** attrs);

bool SOPC_ConfigLoaderInternal_end_trusted_issuers(bool isServer,
                                                   SOPC_HelperExpatCtx* ctx,
                                                   SOPC_Array* trustedRootIssuers);

bool SOPC_ConfigLoaderInternal_start_issuer(bool isServer,
                                            SOPC_HelperExpatCtx* ctx,
                                            SOPC_Array* rootIssuers,
                                            SOPC_Array* IntermediateIssuers,
                                            SOPC_Array* crlCertificates,
                                            const char** attrs);

bool SOPC_ConfigLoaderInternal_end_issued_certs(bool isServer,
                                                SOPC_HelperExpatCtx* ctx,
                                                SOPC_Array* issuedCertificates);

bool SOPC_ConfigLoaderInternal_start_issued_cert(SOPC_HelperExpatCtx* ctx,
                                                 SOPC_Array* issuedCertificates,
                                                 const XML_Char** attrs);

bool SOPC_ConfigLoaderInternal_end_untrusted_issuers(bool isServer,
                                                     SOPC_HelperExpatCtx* ctx,
                                                     SOPC_Array* untrustedRootIssuers,
                                                     SOPC_Array* untrustedIntermediateIssuers);

bool SOPC_ConfigLoaderInternal_end_application_certificates(bool isServer,
                                                            SOPC_HelperExpatCtx* ctx,
                                                            SOPC_Array** trustedRootIssuers,
                                                            char*** trustedRootIssuersList,
                                                            SOPC_Array** trustedIntermediateIssuers,
                                                            char*** trustedIntermediateIssuersList,
                                                            SOPC_Array** issuedCertificates,
                                                            char*** issuedCertificatesList,
                                                            SOPC_Array** untrustedRootIssuers,
                                                            char*** untrustedRootIssuersList,
                                                            SOPC_Array** untrustedIntermediateIssuers,
                                                            char*** untrustedIntermediateIssuersList,
                                                            SOPC_Array** crlCertificates,
                                                            char*** crlCertificatesList,
                                                            bool issuedCertificatesSet,
                                                            bool trustedIssuersSet);

#endif /* SOPC_CONFIG_LOADER_INTERNAL_H_ */
