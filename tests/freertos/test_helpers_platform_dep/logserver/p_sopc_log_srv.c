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

#include "p_generic_socket_srv.h"

static tLogSrvWks* gLogServer = NULL;
static Mutex gLockLogServer = NULL;
static Condition gSignalOneConnexion;

// Callback used by logserv to customize hello message
static uint16_t cbHelloCallback(uint8_t* pBufferInOut, uint16_t nbBytesToEncode, uint16_t maxSizeBufferOut)
{
    uint16_t size;

    snprintf((void*) pBufferInOut + nbBytesToEncode, maxSizeBufferOut - (2 * nbBytesToEncode + 1), "%s",
             "Hello, S2OPC log server is listening on ");
    size = strlen((void*) pBufferInOut + nbBytesToEncode);
    memmove((void*) pBufferInOut + nbBytesToEncode + size, (void*) pBufferInOut, nbBytesToEncode);
    memmove((void*) pBufferInOut, (void*) pBufferInOut + nbBytesToEncode, nbBytesToEncode + size);
    return nbBytesToEncode + size;
}

// Callback used to unlock SOPC_LogSrv_WaitClient function
static void cbOneConnexion(void** pAnalyzerContext, tLogClientWks* pClt)
{
    Condition_SignalAll(&gSignalOneConnexion);

#if (configUSE_BOGOMIPS == 1)
    char sBuffer[50] = {0};
    extern float gPerformanceValue;
    snprintf(sBuffer, sizeof(sBuffer), "\r\n *** BogoMips=%f *** \r\n", gPerformanceValue);
    P_LOG_CLIENT_SendResponse(pClt, (const uint8_t*) sBuffer, strlen(sBuffer), NULL);
#endif
}

// Callback used by analyzer, so *dataSize not changed, buffer in copy to buffer out to client
static eResultDecoder cbEchoCallback(void* pAnalyzerContext,
                                     tLogClientWks* pClt,
                                     uint8_t* pBufferInOut,
                                     uint16_t* dataSize,
                                     uint16_t maxSizeBufferOut)
{
    // Don't modify dataSize output, echo simulation
    return 0;
}

// Callback used by logserv to customize sent buffer
static eResultEncoder cbEncoderCallback(void* pEncoderContext,      // Encoder context
                                        uint8_t* pBufferInOut,      // Buffer in out
                                        uint16_t* pNbBytesToEncode, // Signicant bytes in / out
                                        uint16_t maxSizeBufferOut)
{
    uint16_t lengthLogMemStatus = 0;
    uint16_t lengthLog = *pNbBytesToEncode;

    memmove((void*) pBufferInOut + maxSizeBufferOut / 2, (void*) pBufferInOut, lengthLog);

    snprintf((void*) pBufferInOut, maxSizeBufferOut / 2, "HF=%u / HL=%u - ", xPortGetFreeHeapSize(),
             xPortGetMinimumEverFreeHeapSize());

    pBufferInOut[maxSizeBufferOut / 2 - 1] = 0;

    lengthLogMemStatus = strlen((void*) pBufferInOut);
    memmove((void*) pBufferInOut + lengthLogMemStatus, (void*) pBufferInOut + maxSizeBufferOut / 2, lengthLog);

    *pNbBytesToEncode = lengthLog + lengthLogMemStatus;

    return E_ENCODER_RESULT_OK;
}

//***********S2OPC api wrapper*************

// Wait a client connexion.
SOPC_ReturnStatus SOPC_LogSrv_WaitClient(uint32_t timeoutMs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == gLockLogServer)
    {
        return SOPC_STATUS_NOK;
    }

    Mutex_Lock(&gLockLogServer);

    if (gLogServer == NULL)
    {
        Mutex_Unlock(&gLockLogServer);
        return SOPC_STATUS_NOK;
    }

    status = Mutex_UnlockAndTimedWaitCond(&gSignalOneConnexion, &gLockLogServer, timeoutMs);

    Mutex_Unlock(&gLockLogServer);

    return status;
}

// Wait a client connexion.
SOPC_ReturnStatus SOPC_LogSrv_Print(const uint8_t* buffer, uint16_t length)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == gLockLogServer)
    {
        return SOPC_STATUS_NOK;
    }

    Mutex_Lock(&gLockLogServer);

    if (gLogServer == NULL)
    {
        Mutex_Unlock(&gLockLogServer);
        return SOPC_STATUS_NOK;
    }

    P_LOG_SRV_SendToAllClient(gLogServer, (uint8_t*) buffer, length, NULL);

    Mutex_Unlock(&gLockLogServer);

    return status;
}

// Stop log server
SOPC_ReturnStatus SOPC_LogSrv_Stop(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == gLockLogServer)
    {
        return SOPC_STATUS_NOK;
    }

    Mutex_Lock(&gLockLogServer);

    if (NULL == gLogServer)
    {
        Mutex_Unlock(&gLockLogServer);
        return SOPC_STATUS_NOK;
    }

    Condition_Clear(&gSignalOneConnexion);

    P_LOG_SRV_StopAndDestroy(&gLogServer);

    Mutex_Unlock(&gLockLogServer);

    return status;
}

// Start log server
SOPC_ReturnStatus SOPC_LogSrv_Start(
    uint16_t portSrvTCP, // Server listen port
    uint16_t portCltUDP) // Destination UDP port where server announce its @IP and listen port
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (gLockLogServer == NULL)
    {
        status = Mutex_Initialization(&gLockLogServer);
    }

    if (SOPC_STATUS_OK != status)
    {
        return SOPC_STATUS_NOK;
    }

    Mutex_Lock(&gLockLogServer);

    if (SOPC_STATUS_OK != status)
    {
        Mutex_Unlock(&gLockLogServer);
        return status;
    }

    if (gLogServer == NULL)
    {
        status = Condition_Init(&gSignalOneConnexion);

        gLogServer = P_LOG_SRV_CreateAndStart(portSrvTCP,        //
                                              portCltUDP,        //
                                              1,                 // Max log client
                                              0,                 // Disconnect log client after 0s
                                              10,                // Hello message each 5seconds
                                              cbOneConnexion,    //
                                              NULL,              //
                                              cbEchoCallback,    // Test echo to verify lwip
                                              NULL,              //
                                              NULL,              //
                                              NULL,              //
                                              cbEncoderCallback, //
                                              NULL,              //
                                              cbHelloCallback);  // Customize message hello

        if (gLogServer == NULL)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    Mutex_Unlock(&gLockLogServer);

    return status;
}

//*********Wrappers for newlib**************

int __attribute__((weak)) _open(const char* Path, int flags, int mode)
{
    return (int) stdout->_file;
}

int __attribute__((weak)) _open_r(struct _reent* reent, const char* Path, int flags, int mode)
{
    return _open(Path, flags, mode);
}

int __attribute__((weak)) _close(int fd)
{
    return 0;
}

int __attribute__((weak)) _close_r(struct _reent* reent, int fd)
{
    return _close(fd);
}

FILE* __attribute__((weak)) fopen(const char* file, const char* mode)
{
    return (FILE*) stdout;
}

FILE* __attribute__((weak)) _fopen_r(struct _reent* reent, const char* file, const char* mode)
{
    return fopen(file, mode);
}

int __attribute__((weak)) fclose(FILE* ptrFile)
{
    return 0;
}

int __attribute__((weak)) _fclose_r(struct _reent* reent, FILE* ptrFile)
{
    return 0;
}

int __attribute__((weak)) _write(int handle, const char* buffer, size_t size)
{
    uint16_t length;

    // buffer exist
    if (buffer == 0)
    {
        return -1;
    }

    // Stdout or stdin only are redirected
    if ((handle != 1) && (handle != 2))
    {
        return -1;
    }
    // Log server exist
    if (NULL == gLogServer || E_LOG_SERVER_ONLINE != P_LOG_SRV_GetStatus(gLogServer))
    {
        length = PRINTF(buffer, size);
    }
    else
    {
        /* Send data. */
        P_LOG_SRV_SendToAllClient(gLogServer, (uint8_t*) buffer, size, &length);
    }
    return length;
}

int __attribute__((weak)) _write_r(struct _reent* reent, int handle, const void* buffer, size_t size)
{
    return _write(handle, buffer, size);
}

// Read is not implemented.
int __attribute__((weak)) _read(int handle, char* buffer, int size)
{
    return -1;
}

int __attribute__((weak)) _read_r(struct _reent* reent, int handle, void* buffer, size_t size)
{
    return _read(handle, buffer, size);
}
