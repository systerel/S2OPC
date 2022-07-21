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

/*
 * Note: this function may seem overkill, as there are only a few PyS2OPC headers.
 * It was the basis of tests to avoid manual maintenance of the PyS2OPC's headers, required by CFFI.
 * CFFI is based on pycparser, and their combined purposes make this clearly and intentionally unfeasible.
 * pycparser does not do preprocessing work and does not support all tweaks that may reside in the standard library
 * headers. It results that it can parse C with tweaks, such as using fake stdlib headers.
 * However CFFI uses pycparser to obtain real types and generate some C afterwards with them.
 * This makes it clear that we cannot fake types of the stdlib
 * (otherwise, there are be conflicting types definitions at compile time).
 *
 * To conclude, we shall only use CFFI on headers that don't rely on the libc,
 * except for the types it already knows: [u]int(8|16|32|64)_t.
 * (Even bool is not supported and its support in our headers should be platform-dependent)
 *
 * Here we make the manual includes when possible, and the copy-paste when not.
 * We make subfiles for copies to clarify where things come from and ease the maintenance.
 */

#include <stdbool.h>
#include "sopc_stdint_cffi.h"

/* sopc_builtintypes.h */
typedef uint32_t SOPC_StatusCode;

#include "sopc_enums.h"
//#include "sopc_builtintypes.h"  // includes stdio
//#include "sopc_time.h"  // includes <times.h>
//#include "sopc_log_manager.h"  // uses var_args
//#include "sopc_types.h"  // includes stdio
//#include "sopc_mem_alloc.h"  // includes stdio

/* sopc_time.h */
int64_t SOPC_Time_GetCurrentTimeUTC(void);

/* sopc_log_manager.h */
typedef enum
{
    SOPC_LOG_LEVEL_ERROR = 0,
    SOPC_LOG_LEVEL_WARNING = 1,
    SOPC_LOG_LEVEL_INFO = 2,
    SOPC_LOG_LEVEL_DEBUG = 3
} SOPC_Log_Level;

/* sopc_mem_alloc.h */
void* SOPC_Malloc(size_t size);
void SOPC_Free(void* ptr);
void* SOPC_Calloc(size_t nmemb, size_t size);
void* SOPC_Realloc(void* ptr, size_t old_size, size_t new_size);

/* sopc_buffer.h */
#include "sopc_buffer.h"

/* sopc_encodeabletype.h */
typedef void(SOPC_EncodeableObject_PfnInitialize)(void* value);
typedef void(SOPC_EncodeableObject_PfnClear)(void* value);
typedef void(SOPC_EncodeableObject_PfnGetSize)(void);
typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnEncode)(const void* value,
                                                           SOPC_Buffer* msgBuffer,
                                                           uint32_t nestedStructLevel);
typedef SOPC_ReturnStatus(SOPC_EncodeableObject_PfnDecode)(void* value,
                                                           SOPC_Buffer* msgBuffer,
                                                           uint32_t nestedStructLevel);
typedef struct SOPC_EncodeableType_Struct
{
    char* TypeName;
    uint32_t TypeId;
    uint32_t BinaryEncodingTypeId;
    uint32_t XmlEncodingTypeId;
    char* NamespaceUri;
    size_t AllocationSize;
    SOPC_EncodeableObject_PfnInitialize* Initialize;
    SOPC_EncodeableObject_PfnClear* Clear;
    SOPC_EncodeableObject_PfnGetSize* GetSize;
    SOPC_EncodeableObject_PfnEncode* Encode;
    SOPC_EncodeableObject_PfnDecode* Decode;
} SOPC_EncodeableType;

/* sopc_singly_linked_list.h */
typedef struct SOPC_SLinkedList SOPC_SLinkedList;

#include "sopc_builtintypes_cffi.h"
#include "sopc_types_cffi.h"

#define SKIP_S2OPC_DEFINITIONS
#include "libs2opc_client.h"

/* As defines are ignored, we should replace them with constants.
 * (and we must still undef them, otherwise gcc -E replaces them in the following definition).
 */
#undef SOPC_LibSub_AttributeId_NodeId
#undef SOPC_LibSub_AttributeId_NodeClass
#undef SOPC_LibSub_AttributeId_BrowseName
#undef SOPC_LibSub_AttributeId_DisplayName
#undef SOPC_LibSub_AttributeId_Description
#undef SOPC_LibSub_AttributeId_WriteMask
#undef SOPC_LibSub_AttributeId_UserWriteMask
#undef SOPC_LibSub_AttributeId_IsAbstract
#undef SOPC_LibSub_AttributeId_Symmetric
#undef SOPC_LibSub_AttributeId_InverseName
#undef SOPC_LibSub_AttributeId_ContainsNoLoops
#undef SOPC_LibSub_AttributeId_EventNotifier
#undef SOPC_LibSub_AttributeId_Value
#undef SOPC_LibSub_AttributeId_DataType
#undef SOPC_LibSub_AttributeId_ValueRank
#undef SOPC_LibSub_AttributeId_ArrayDimensions
#undef SOPC_LibSub_AttributeId_AccessLevel
#undef SOPC_LibSub_AttributeId_UserAccessLevel
#undef SOPC_LibSub_AttributeId_MinimumSamplingInterval
#undef SOPC_LibSub_AttributeId_Historizing
#undef SOPC_LibSub_AttributeId_Executable
#undef SOPC_LibSub_AttributeId_UserExecutable
typedef enum
{
    SOPC_LibSub_AttributeId_NodeId = 1,
    SOPC_LibSub_AttributeId_NodeClass = 2,
    SOPC_LibSub_AttributeId_BrowseName = 3,
    SOPC_LibSub_AttributeId_DisplayName = 4,
    SOPC_LibSub_AttributeId_Description = 5,
    SOPC_LibSub_AttributeId_WriteMask = 6,
    SOPC_LibSub_AttributeId_UserWriteMask = 7,
    SOPC_LibSub_AttributeId_IsAbstract = 8,
    SOPC_LibSub_AttributeId_Symmetric = 9,
    SOPC_LibSub_AttributeId_InverseName = 10,
    SOPC_LibSub_AttributeId_ContainsNoLoops = 11,
    SOPC_LibSub_AttributeId_EventNotifier = 12,
    SOPC_LibSub_AttributeId_Value = 13,
    SOPC_LibSub_AttributeId_DataType = 14,
    SOPC_LibSub_AttributeId_ValueRank = 15,
    SOPC_LibSub_AttributeId_ArrayDimensions = 16,
    SOPC_LibSub_AttributeId_AccessLevel = 17,
    SOPC_LibSub_AttributeId_UserAccessLevel = 18,
    SOPC_LibSub_AttributeId_MinimumSamplingInterval = 19,
    SOPC_LibSub_AttributeId_Historizing = 20,
    SOPC_LibSub_AttributeId_Executable = 21,
    SOPC_LibSub_AttributeId_UserExecutable = 22
} SOPC_LibSub_AttributeId_e; /* Type SOPC_LibSub_AttributeId already exists */

//#undef SECURITY_POLICY_NONE
//#undef SECURITY_POLICY_BASIC128RSA15
//#undef SECURITY_POLICY_BASIC256
//#undef SECURITY_POLICY_BASIC256SHA256
extern const char* SOPC_SecurityPolicy_None_URI;
extern const char* SOPC_SecurityPolicy_Basic128Rsa15;
extern const char* SOPC_SecurityPolicy_Basic256_URI;
extern const char* SOPC_SecurityPolicy_Basic256Sha256_URI;
extern const char* SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI;
extern const char* SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI;

/* Additions for server */
#include "sopc_secret_buffer.h"

/* sopc_key_manager.h */
typedef SOPC_SecretBuffer SOPC_SerializedAsymmetricKey;
typedef SOPC_Buffer SOPC_SerializedCertificate;
SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile(const char* path,
                                                                         SOPC_SerializedAsymmetricKey** key);
SOPC_ReturnStatus SOPC_KeyManager_SerializedCertificate_CreateFromFile(const char* path,
                                                                       SOPC_SerializedCertificate** cert);

/* sopc_user.h */
typedef struct SOPC_User SOPC_User;
bool SOPC_User_IsAnonymous(const SOPC_User* user);
bool SOPC_User_IsUsername(const SOPC_User* user);
const SOPC_String* SOPC_User_GetUsername(const SOPC_User* user);

/* sopc_address_space.h */
typedef struct _SOPC_AddressSpace SOPC_AddressSpace;
void SOPC_AddressSpace_Delete(SOPC_AddressSpace* space);

/* Altered includes */
#include "sopc_call_method_manager_cffi.h"
#include "sopc_common_cffi.h"
#include "sopc_toolkit_async_api_cffi.h"
#include "sopc_user_manager_cffi.h"

#include "sopc_user_app_itf_cffi.h"

#include "sopc_toolkit_config_cffi.h"

/* Defines converted to constants */
//#undef SOPC_SECURITY_MODE_NONE_MASK 0x01
//#undef SOPC_SECURITY_MODE_SIGN_MASK 0x02
//#undef SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK 0x04
//#undef SOPC_SECURITY_MODE_ANY_MASK 0x07
//#undef SOPC_MAX_SECU_POLICIES_CFG 5
extern const uint8_t SOPC_SecurityMode_None_Mask;
extern const uint8_t SOPC_SecurityMode_Sign_Mask;
extern const uint8_t SOPC_SecurityMode_SignAndEncrypt_Mask;
extern const uint8_t SOPC_SecurityMode_Any_Mask;
extern const uint8_t SOPC_MaxSecuPolicies_CFG;

/* sopc_pki.h */
void SOPC_PKIProvider_Free(SOPC_PKIProvider** ppPKI);

/* sopc_pki_stack.h */
SOPC_ReturnStatus SOPC_PKIProviderStack_Create(SOPC_SerializedCertificate* pCertAuth,
                                               struct SOPC_CRLList* pRevocationList,
                                               SOPC_PKIProvider** ppPKI);
SOPC_ReturnStatus SOPC_PKIProviderStack_CreateFromPaths(char** lPathTrustedIssuerRoots,
                                                        char** lPathTrustedIssuerLinks,
                                                        char** lPathUntrustedIssuerRoots,
                                                        char** lPathUntrustedIssuerLinks,
                                                        char** lPathIssuedCerts,
                                                        char** lPathCRL,
                                                        SOPC_PKIProvider** ppPKI);

/* XML Expat: Config and Address Space */
bool SOPC_Config_Parse(FILE* fd, SOPC_S2OPC_Config* config);
SOPC_AddressSpace* SOPC_UANodeSet_Parse(FILE* fd);
