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
 * \brief Internal API to manage the CertificateGroupType according the Push model.
 */

#ifndef SOPC_CERTIFICATE_GROUP_INTERNAL_
#define SOPC_CERTIFICATE_GROUP_INTERNAL_

#include "sopc_certificate_group_itf.h"

#define SOPC_RsaMinApplicationCertificateTypeId "ns=0;i=12559"
#define SOPC_RsaSha256ApplicationCertificateTypeId "ns=0;i=12560"

typedef struct SOPC_CertificateGroup
{
    SOPC_SerializedAsymmetricKey* pKey; /*!< A valid pointer to the private key belongs the CertificateGroup object. */
    SOPC_SerializedCertificate* pCert;  /*!< A valid pointer to the certificate belongs the CertificateGroup object. */
    SOPC_NodeId*
        pCertificateTypeId; /*!< The nodeId of the certificateType variable belongs the CertificateGroup object. */
    SOPC_NodeId* pCertificateTypeValueId; /*!< The value of the certificateType variable. */
    SOPC_PKIProvider* pPKI;    /*!< A valid pointer to the PKI of the TrustList belongs the CertificateGroup object. */
    SOPC_NodeId* pTrustListId; /*!< The TrustList nodeId belongs the CertificateGroup object */
} SOPC_CertificateGroup;

/* Get the trustList internal object from the nodeId */
SOPC_CertificateGroup* CertificateGroup_DictGet(const SOPC_NodeId* objectId, bool* found);

#endif /* SOPC_CERTIFICATE_GROUP_INTERNAL_ */
