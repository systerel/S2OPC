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

#include "p_channel.h"
#include "sopc_macros.h"

// Deinitialize channel
void P_CHANNEL_DeInit(tChannel* p)
{
    if (NULL != p)
    {
        if (NULL != p->isNotEmpty)
        {
            vQueueDelete(p->isNotEmpty);
            p->isNotEmpty = NULL;
            DEBUG_decrementCpt();
        }

        if (NULL != p->lock)
        {
            vQueueDelete(p->lock);
            p->lock = NULL;
            DEBUG_decrementCpt();
        }

        if (NULL != p->channelData)
        {
            memset(p->channelData, 0, p->maxSizeTotalData);
            vPortFree(p->channelData);
            DEBUG_decrementCpt();
        }

        if (NULL != p->channelRecord)
        {
            memset(p->channelRecord, 0, p->maxSizeTotalElts);
            vPortFree(p->channelRecord);
            DEBUG_decrementCpt();
        }

        memset(p, 0, sizeof(tChannel));
    }
}

// Initialize channel
eChannelResult P_CHANNEL_Init(tChannel* p,          // Channel workspace
                              size_t totalDataSize, // Total data size
                              size_t maxEltSize,    // Max size per atomic element
                              size_t nbElts)        // Max nb element.
{
    eChannelResult result = E_CHANNEL_RESULT_NOK;
    if (NULL != p)
    {
        memset(p, 0, sizeof(tChannel));
        p->channelData = pvPortMalloc(totalDataSize);
        p->channelRecord = pvPortMalloc(sizeof(uint16_t) * nbElts);

        p->isNotEmpty = xSemaphoreCreateBinary();
        p->lock = xSemaphoreCreateMutex();

        if ((NULL == p->isNotEmpty) || (NULL == p->lock) || (NULL == p->channelData) || (NULL == p->channelRecord))
        {
            P_CHANNEL_DeInit(p);
        }
        else
        {
            DEBUG_incrementCpt();
            DEBUG_incrementCpt();
            DEBUG_incrementCpt();
            DEBUG_incrementCpt();

            SOPC_UNUSED_RESULT(memset(p->channelData, 0, totalDataSize));
            SOPC_UNUSED_RESULT(memset(p->channelRecord, 0, nbElts * sizeof(uint16_t)));
            p->maxSizeTotalElts = nbElts;
            p->maxSizeTotalData = totalDataSize;
            p->maxSizeDataPerElt = maxEltSize;
            xSemaphoreTake(p->isNotEmpty, 0);
            result = E_CHANNEL_RESULT_OK;
        }
    }
    return result;
}

// Send data on a channel. Zero size buffer can't be sent.
// If atomic buffer (all data with the specified size) and overwrite
// is specified, oldest atomic data buffers are removed until
// space is enough to the buffer to send.
eChannelResult P_CHANNEL_Send(tChannel* p,               // Channel workspace
                              const uint8_t* pBuffer,    // Data to send
                              uint16_t size,             // Size of data to send
                              uint16_t* pbNbWritedBytes, // Size atomically writed
                              eChannelWriteMode mode)    // Mode, overwrite or normal.
{
    eChannelResult result = E_CHANNEL_RESULT_NOK;
    uint16_t dataToDeleteSize = 0;
    uint16_t dataToWriteSize = 0;

    // Check parameters
    if ((p != NULL) && (size > 0) && (pBuffer != NULL))
    {
        // Critical section
        xSemaphoreTake(p->lock, portMAX_DELAY);
        {
            // Define max size to read
            dataToWriteSize = size > p->maxSizeDataPerElt ? p->maxSizeDataPerElt : size;

            // Mode overwrite and elt size ok, prepare enough memory space for new elt
            if (E_CHANNEL_WR_MODE_OVERWRITE == mode)
            {
                // Pop to free memory space to overwrite
                while (((p->currentNbElts + 1) > p->maxSizeTotalElts)                    // Not At least one elt
                       || ((dataToWriteSize + p->currentNbDatas) > p->maxSizeTotalData)) // Not Enough space
                {
                    // Data size of next elt to remove
                    dataToDeleteSize = p->channelRecord[p->iRd];
                    p->channelRecord[p->iRd] = 0;
                    p->iRd++;
                    if (p->iRd >= p->maxSizeTotalElts)
                    {
                        p->iRd = 0;
                    }
                    // If data size exist, remove data of this elt
                    if (dataToDeleteSize > 0)
                    {
                        if ((p->iRdData + dataToDeleteSize) < p->maxSizeTotalData)
                        {
                            memset(&p->channelData[p->iRdData], 0, dataToDeleteSize);
                            p->iRdData = p->iRdData + dataToDeleteSize;
                        }
                        else
                        {
                            memset(&p->channelData[p->iRdData],       //
                                   0,                                 //
                                   p->maxSizeTotalData - p->iRdData); //

                            memset(&p->channelData[0],                                     //
                                   0,                                                      //
                                   dataToDeleteSize - (p->maxSizeTotalData - p->iRdData)); //

                            p->iRdData = dataToDeleteSize - (p->maxSizeTotalData - p->iRdData);
                        }
                    }
                }
            }

            // If enough memory space, write elt
            if (((p->currentNbElts + 1) <= p->maxSizeTotalElts) &&
                ((dataToWriteSize + p->currentNbDatas) <= p->maxSizeTotalData))
            {
                p->channelRecord[p->iWr] = dataToWriteSize;
                p->iWr++;
                if (p->iWr >= p->maxSizeTotalElts)
                {
                    p->iWr = 0;
                }
                p->currentNbElts++;

                if (dataToWriteSize > 0)
                {
                    // Data between upper bound of buffer and current data wr index
                    if ((p->iWrData + dataToWriteSize) < p->maxSizeTotalData)
                    {
                        memcpy(&p->channelData[p->iWrData], pBuffer, dataToWriteSize);

                        p->iWrData = p->iWrData + dataToWriteSize;
                    }
                    else
                    {
                        // Data between wr index and upper bound of data buffer then restart at index 0.
                        memcpy(&p->channelData[p->iWrData],       //
                               &pBuffer[0],                       //
                               p->maxSizeTotalData - p->iWrData); //

                        memcpy(&p->channelData[0],                                    //
                               &pBuffer[p->maxSizeTotalData - p->iWrData],            //
                               dataToWriteSize - (p->maxSizeTotalData - p->iWrData)); //

                        p->iWrData = dataToWriteSize - (p->maxSizeTotalData - p->iWrData);
                    }

                    // Update global nb data
                    p->currentNbDatas = p->currentNbDatas + dataToWriteSize;
                }

                if (pbNbWritedBytes != NULL)
                {
                    *pbNbWritedBytes = dataToWriteSize;
                }

                // Signal channel not empty
                if (p->currentNbElts > 0)
                {
                    xSemaphoreGive(p->isNotEmpty);
                }
                result = E_CHANNEL_RESULT_OK;
            }
            else
            {
                p->overflowCpt++;
                result = E_CHANNEL_RESULT_ERROR_FULL;
            }
        }
        xSemaphoreGive(p->lock);
    }
    return result;
}

// Flush channel, all task waiting on it are unblocked.
eChannelResult P_CHANNEL_Flush(tChannel* p)
{
    eChannelResult result = E_CHANNEL_RESULT_NOK;
    if (NULL != p)
    {
        xSemaphoreTake(p->lock, portMAX_DELAY);
        {
            // Raz
            memset(p->channelData, 0, p->maxSizeTotalData);
            memset(p->channelRecord, 0, p->maxSizeTotalElts * sizeof(uint16_t));
            p->iRd = 0;
            p->iWr = 0;
            p->iRdData = 0;
            p->iWrData = 0;
            p->currentNbElts = 0;
            p->currentNbDatas = 0;
            // Unblock all task waiting on it
            xSemaphoreGive(p->isNotEmpty);
            result = E_CHANNEL_RESULT_OK;
        }
        xSemaphoreGive(p->lock);
    }
    return result;
}

// Receive an element
// If pBuffer = NULL, pOutEltSize return max size required to pop atomically element
// If pBuffer != NULL and maxBytesToRead is specified, maxBytesToRead will be returned, but element not pop.
// It is pop only when all data has been read.
// If flushing on going
eChannelResult P_CHANNEL_Receive(tChannel* p,             // Channel workspace
                                 uint16_t* pOutEltSize,   // Output element size
                                 uint8_t* pBuffer,        // Buffer.  If null, only element size without pop is read
                                 uint16_t* pNbReadBytes,  // Nb bytes read. If limited by maxBytesToRead
                                 uint16_t maxBytesToRead, // and total read bytes will be updated.
                                 TickType_t xTimeToWait,  // Time to wait in ticks
                                 eChannelReadMode mode) // Mode RD or KEEP_ONLY. KEEP ONLY read without pop older elemt.
{
    eChannelResult result = E_CHANNEL_RESULT_NOK;
    uint16_t dataSizeToRead = 0;
    uint16_t dataSize = 0;

    // Check input parameters
    if (NULL != p)
    {
        // Wait for signal not empty
        if (xSemaphoreTake(p->isNotEmpty, xTimeToWait) == pdPASS)
        {
            // Critical section
            xSemaphoreTake(p->lock, portMAX_DELAY);
            {
                // Check if some elts are present
                if (p->currentNbElts > 0)
                {
                    // Take elt size
                    dataSize = p->channelRecord[p->iRd];

                    // Define max size to read
                    dataSizeToRead = dataSize > maxBytesToRead ? maxBytesToRead : dataSize;

                    // If buffer output exist, read elt data
                    if (pBuffer != NULL)
                    {
                        // If elts have data, read data
                        if (dataSizeToRead > 0)
                        {
                            // Data between upper bound of buffer and current data read index
                            if ((p->iRdData + dataSizeToRead) < p->maxSizeTotalData)
                            {
                                // Copy data to external buffer
                                memcpy(pBuffer, &p->channelData[p->iRdData], dataSizeToRead);
                            }
                            else
                            {
                                // Data between read index and upper bound of data buffer and restart at index 0.
                                memcpy(&pBuffer[0],                       //
                                       &p->channelData[p->iRdData],       //
                                       p->maxSizeTotalData - p->iRdData); //

                                memcpy(&pBuffer[p->maxSizeTotalData - p->iRdData],           //
                                       &p->channelData[0],                                   //
                                       dataSizeToRead - (p->maxSizeTotalData - p->iRdData)); //
                            }
                        }

                        // If mode normal (pop), raz elt size and update read index
                        // and nb elmts, only of buffer <> NULL
                        if (mode == E_CHANNEL_RD_MODE_NORMAL)
                        {
                            // Datas
                            if (dataSizeToRead > 0)
                            {
                                if ((p->iRdData + dataSizeToRead) < p->maxSizeTotalData)
                                {
                                    memset(&p->channelData[p->iRdData], 0, dataSizeToRead);

                                    p->iRdData = p->iRdData + dataSizeToRead;
                                }
                                else
                                {
                                    memset(&p->channelData[p->iRdData],       //
                                           0,                                 //
                                           p->maxSizeTotalData - p->iRdData); //

                                    memset(&p->channelData[0],                                   //
                                           0,                                                    //
                                           dataSizeToRead - (p->maxSizeTotalData - p->iRdData)); //

                                    p->iRdData = dataSizeToRead - (p->maxSizeTotalData - p->iRdData);
                                }

                                p->currentNbDatas = p->currentNbDatas - dataSizeToRead;
                            }
                            // Record
                            p->channelRecord[p->iRd] = dataSize - dataSizeToRead;

                            if (p->channelRecord[p->iRd] == 0)
                            {
                                result = E_CHANNEL_RESULT_OK;
                            }
                            else
                            {
                                result = E_CHANNEL_RESULT_MORE_DATA;
                            }

                            if (p->channelRecord[p->iRd] == 0)
                            {
                                p->iRd++;
                                if (p->iRd >= p->maxSizeTotalElts)
                                {
                                    p->iRd = 0;
                                }
                                p->currentNbElts--;
                            }
                        }
                    }
                    else
                    {
                        result = E_CHANNEL_RESULT_MORE_DATA;
                    }

                    // If some elt present, signal not empty is set
                    if (p->currentNbElts > 0)
                    {
                        xSemaphoreGive(p->isNotEmpty);
                    }
                }
                else
                {
                    result = E_CHANNEL_RESULT_ERROR_EMPTY;
                }
            }
            xSemaphoreGive(p->lock);
        }
        else
        {
            result = E_CHANNEL_RESULT_ERROR_TMO;
        }
    }

    if (pOutEltSize != NULL)
    {
        *pOutEltSize = dataSize;
    }

    if (pNbReadBytes != NULL)
    {
        *pNbReadBytes = dataSizeToRead;
    }

    return result;
}
