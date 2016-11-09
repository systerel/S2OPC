/* Copyright (c) 1996-2016, OPC Foundation. All rights reserved.

   The source code in this file is covered under a dual-license scenario:
     - RCL: for OPC Foundation members in good-standing
     - GPL V2: everybody else

   RCL license terms accompanied with this source code. See http://opcfoundation.org/License/RCL/1.00/

   GNU General Public License as published by the Free Software Foundation;
   version 2 of the License are accompanied with this source code. See http://opcfoundation.org/License/GPLv2

   This source code is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "opcua_channel.h"

#include <string.h>
#include "pki_stack.h"


#ifdef OPCUA_HAVE_CLIENTAPI

typedef enum
{
    TMP_Invalid_PKI   = 0,
    TMP_NO_PKI        = 1,
    TMP_Override      = 2,
    TMP_DefaultPKI    = 3
} PKI_Types;

typedef struct
{
    PKI_Types       type;
    char*           trustListLocation;
    char*           revocationListLocation;
    char*           untrustedListLocation;
    uint32_t        flags;
    void*           unused;
} PKIConfig;


/*============================================================================
 * OpcUa_Channel_Create
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Channel_Create(  OpcUa_Channel*                  a_phChannel,
                                        OpcUa_Channel_SerializerType    a_eSerializerType)
{
    if(a_eSerializerType != OpcUa_Channel_SerializerType_Binary){
        return STATUS_NOK;
    }
    return SOPC_Channel_Create(a_phChannel,
                               SOPC_ChannelSerializer_Binary);
}

/*============================================================================
 * OpcUa_Channel_Clear
 *===========================================================================*/
OpcUa_Void OpcUa_Channel_Clear(OpcUa_Channel a_hChannel)
{
    SOPC_Channel_Disconnect(a_hChannel);
}

/*============================================================================
 * OpcUa_Channel_Delete
 *===========================================================================*/
OpcUa_Void OpcUa_Channel_Delete(OpcUa_Channel* a_phChannel)
{
    SOPC_Channel_Delete(a_phChannel);
}


/*============================================================================
 * OpcUa_Channel_BeginInvokeService
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Channel_BeginInvokeService(  OpcUa_Channel                     a_hChannel,
                                                    OpcUa_StringA                     a_sName,
                                                    OpcUa_Void*                       a_pRequest,
                                                    OpcUa_EncodeableType*             a_pRequestType,
                                                    OpcUa_Channel_PfnRequestComplete* a_pCallback,
                                                    OpcUa_Void*                       a_pCallbackData)
{
    return SOPC_Channel_BeginInvokeService(a_hChannel,
                                         a_sName,
                                         a_pRequest,
                                         a_pRequestType,
                                         NULL,
                                         a_pCallback,
                                         a_pCallbackData);
}

/*============================================================================
 * OpcUa_Channel_InvokeService
 *===========================================================================*/
/* Main service invoke for synchronous behaviour; this function blocks until */
/* the server sends a response for this request.                             */
OpcUa_StatusCode OpcUa_Channel_InvokeService(   OpcUa_Channel           a_pChannel,
                                                OpcUa_StringA           a_sName,
                                                OpcUa_Void*             a_pRequest,
                                                OpcUa_EncodeableType*   a_pRequestType,
                                                OpcUa_Void**            a_ppResponse,
                                                OpcUa_EncodeableType**  a_ppResponseType)
{
    return SOPC_Channel_InvokeService(a_pChannel,
                                    a_sName,
                                    a_pRequest,
                                    a_pRequestType,
                                    NULL,
                                    a_ppResponse,
                                    a_ppResponseType);
}

/*============================================================================
 * OpcUa_Channel_Disconnect
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Channel_BeginDisconnect(OpcUa_Channel                               a_pChannel,
                                               OpcUa_Channel_PfnConnectionStateChanged*    a_pfCallback,
                                               OpcUa_Void*                                 a_pCallbackData)
{
    (void) a_pChannel;
    (void) a_pfCallback;
    (void) a_pCallbackData;
    return STATUS_NOK;
}

/*============================================================================
 * OpcUa_Channel_Disconnect
 *===========================================================================*/
/* synchronous disconnect from server - blocks because of securechannel messages delay */
OpcUa_StatusCode OpcUa_Channel_Disconnect(OpcUa_Channel a_hChannel)
{
    return SOPC_Channel_Disconnect(a_hChannel);
}

/*============================================================================
 * OpcUa_Channel_InternalBeginConnect
 *===========================================================================*/
/* initiates an asynchronous connect process */
OpcUa_StatusCode OpcUa_Channel_BeginConnect(OpcUa_Channel                               a_pChannel,
                                            OpcUa_StringA                               a_sUrl,
                                            OpcUa_ByteString*                           a_pClientCertificate,
                                            OpcUa_ByteString*                           a_pClientPrivateKey,
                                            OpcUa_ByteString*                           a_pServerCertificate,
                                            OpcUa_Void*                                 a_pPKIConfig,
                                            OpcUa_String*                               a_pRequestedSecurityPolicyUri,
                                            OpcUa_Int32                                 a_nRequestedLifetime,
                                            OpcUa_MessageSecurityMode                   a_messageSecurityMode,
                                            OpcUa_UInt32                                a_nNetworkTimeout,
                                            OpcUa_Channel_PfnConnectionStateChanged*    a_pfCallback,
                                            OpcUa_Void*                                 a_pCallbackData)
{
    SOPC_StatusCode status = STATUS_OK;
    Certificate *cli = NULL, *srv = NULL, *crt_ca = NULL;
    AsymmetricKey *pKeyCli = NULL;
    PKIProvider *pki;
    PKIConfig *pPKIConfig = a_pPKIConfig;
    if(a_pClientCertificate != NULL && a_pClientPrivateKey != NULL &&
       a_pServerCertificate != NULL && a_pPKIConfig != NULL){
        status = KeyManager_Certificate_CreateFromDER(a_pClientCertificate->Data,
                                                      a_pClientCertificate->Length,
                                                      &cli);
        if(STATUS_OK == status){
            pKeyCli = (AsymmetricKey*) a_pClientPrivateKey->Data;
        }
        if(STATUS_OK == status){
            status = KeyManager_Certificate_CreateFromDER(a_pServerCertificate->Data,
                                                          a_pServerCertificate->Length,
                                                          &srv);
        }
        //TODO: CA folder != CA cert: how to deal with that ?
        if(STATUS_OK == status){
            const char* cacertname = "/cacert.der";
            char cacert[strlen(pPKIConfig->trustListLocation) + strlen(cacertname) + 1];
            if(cacert != memcpy(cacert, pPKIConfig->trustListLocation, strlen(pPKIConfig->trustListLocation)))
                status = STATUS_NOK;
            if(&cacert[strlen(pPKIConfig->trustListLocation)] !=
                memcpy(&cacert[strlen(pPKIConfig->trustListLocation)], cacertname, strlen(cacertname) + 1))
                status = STATUS_NOK;
            if(STATUS_OK == status){
                status = KeyManager_Certificate_CreateFromFile(cacert, &crt_ca);
            }
        }
        if(STATUS_OK == status){
            status = PKIProviderStack_Create(crt_ca, NULL, &pki);
        }
    }

    if(STATUS_OK == status){
        status = SOPC_Channel_BeginConnect(a_pChannel,
                                         a_sUrl,
                                         cli,
                                         pKeyCli,
                                         srv,
                                         pki,
                                         String_GetRawCString(a_pRequestedSecurityPolicyUri),
                                         a_nRequestedLifetime,
                                         a_messageSecurityMode,
                                         a_nNetworkTimeout,
                                         a_pfCallback,
                                         a_pCallbackData);
    }
    return status;
}

/*============================================================================
 * OpcUa_Channel_Connect
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Channel_Connect( OpcUa_Channel                               a_hChannel,
                                        OpcUa_StringA                               a_sUrl,
                                        OpcUa_Channel_PfnConnectionStateChanged*    a_pfCallback,
                                        OpcUa_Void*                                 a_pvCallbackData,
                                        OpcUa_ByteString*                           a_pClientCertificate,
                                        OpcUa_ByteString*                           a_pClientPrivateKey,
                                        OpcUa_ByteString*                           a_pServerCertificate,
                                        OpcUa_Void*                                 a_pPKIConfig,
                                        OpcUa_String*                               a_pRequestedSecurityPolicyUri,
                                        OpcUa_Int32                                 a_nRequestedLifetime,
                                        OpcUa_MessageSecurityMode                   a_messageSecurityMode,
                                        OpcUa_UInt32                                a_nNetworkTimeout)
{
    (void) a_hChannel;
    (void) a_sUrl;
    (void) a_pfCallback;
    (void) a_pvCallbackData;
    (void) a_pClientCertificate;
    (void) a_pClientPrivateKey;
    (void) a_pServerCertificate;
    (void) a_pPKIConfig;
    (void) a_pRequestedSecurityPolicyUri;
    (void) a_nRequestedLifetime;
    (void) a_messageSecurityMode;
    (void) a_nNetworkTimeout;

    return STATUS_NOK;
}

#endif /* OPCUA_HAVE_CLIENTAPI */
