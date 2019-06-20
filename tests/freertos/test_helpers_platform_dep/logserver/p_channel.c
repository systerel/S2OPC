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

//Deinitialize channel
void P_CHANNEL_DeInit(tChannel*p)
{
    if(p!=NULL)
    {
        if(p->isNotEmpty != NULL)
        {
            vQueueDelete(p->isNotEmpty);
            p->isNotEmpty = NULL;
            DEBUG_decrementCpt();
        }
        if(p->lock != NULL)
        {
            vQueueDelete(p->lock);
            p->lock = NULL;
            DEBUG_decrementCpt();
        }
        if(p->channelData != NULL)
        {
            memset(p->channelData,0,p->maxSizeTotalData);
            vPortFree(p->channelData);
            DEBUG_decrementCpt();
        }
        if(p->channelRecord != NULL)
        {
            memset(p->channelRecord,0,p->maxSizeTotalElts);
            vPortFree(p->channelRecord);
            DEBUG_decrementCpt();
        }
        memset(p,0,sizeof(tChannel));
    }
}

//Initialize channel
eChannelResult P_CHANNEL_Init(  tChannel*p,         //Channel workspace
                                size_t totalDataSize,
                                size_t maxEltSize,  //Max size per atomic element
                                size_t nbElts)      //Max nb element.
{
    eChannelResult result = E_CHANNEL_RESULT_NOK;
    if(p!=NULL)
    {
        memset(p,0,sizeof(tChannel));
        p->channelData = pvPortMalloc(totalDataSize ) ;
        p->channelRecord = pvPortMalloc(sizeof(uint16_t) * nbElts) ;

        p->isNotEmpty = xSemaphoreCreateBinary();
        p->lock = xSemaphoreCreateMutex();

        if((p->isNotEmpty == NULL) || (p->lock == NULL) || ( p->channelData == NULL) || (p->channelRecord == NULL))
        {
            P_CHANNEL_DeInit(p);
        }
        else
        {
            DEBUG_incrementCpt();
            DEBUG_incrementCpt();
            DEBUG_incrementCpt();
            DEBUG_incrementCpt();

            (void)memset(p->channelData,0,totalDataSize);
            (void)memset(p->channelRecord,0,nbElts * sizeof(uint16_t));
            p->maxSizeTotalElts = nbElts;
            p->maxSizeTotalData = totalDataSize;
            p->maxSizeDataPerElt = maxEltSize ;
            xSemaphoreTake(p->isNotEmpty,0);
            result = E_CHANNEL_RESULT_OK;
        }
    }
    return result;
}

//Send data on a channel. Zero size buffer can't be sent.
//If atomic buffer (all data with the specified size) and overwrite
//is specified, oldest atomic data buffers are removed until
//space is enough to the buffer to send.
eChannelResult P_CHANNEL_Send(  tChannel*p,                 //Channel workspace
                                const uint8_t * pBuffer,    //Data to send
                                uint16_t size,              //Size of data to send
                                eChannelWriteMode mode)     //Mode, overwrite or normal.
{
    eChannelResult result = E_CHANNEL_RESULT_NOK;
    uint16_t dataToDeleteSize = 0;

    //Check parameters
    if((p!=NULL)&&(size > 0)&&(pBuffer != NULL))
    {
        //Critical section
        xSemaphoreTake(p->lock,portMAX_DELAY);
        {
            //Mode overwrite and elt size ok, prepare enough memory space for new elt
            if(( mode == E_CHANNEL_WR_MODE_OVERWRITE) && (size <= p->maxSizeDataPerElt))
            {
                //Pop to free memory space to overwrite
                while(      ((p->currentNbElts + 1) > p->maxSizeTotalElts)              //Not At least one elt
                        ||  ((size + p->currentNbDatas) > p->maxSizeTotalData )  )      //Not Enough space
                {
                    //Data size of next elt to remove
                    dataToDeleteSize = p->channelRecord[p->iRd];
                    p->channelRecord[p->iRd] = 0;
                    p->iRd++;
                    if(p->iRd >= p->maxSizeTotalElts)
                    {
                        p->iRd = 0;
                    }
                    //If data size exist, remove data of this elt
                    if(dataToDeleteSize > 0)
                    {
                        if ((p->iRdData + dataToDeleteSize) < p->maxSizeTotalData)
                        {
                            memset (&p->channelData[p->iRdData], 0, dataToDeleteSize);
                            p->iRdData =  p->iRdData + dataToDeleteSize;

                        }
                        else
                        {
                            memset (&p->channelData[p->iRdData], 0, p->maxSizeTotalData - p->iRdData);
                            memset (&p->channelData[0], 0, dataToDeleteSize - (p->maxSizeTotalData - p->iRdData));
                            p->iRdData = dataToDeleteSize - (p->maxSizeTotalData - p->iRdData);
                        }
                    }
                }
            }

            //If enough memory space, write elt
            if(         ((p->currentNbElts + 1) <= p->maxSizeTotalElts)
                    &&  ((size + p->currentNbDatas) <= p->maxSizeTotalData )
                    &&  (size <= p->maxSizeDataPerElt))
            {
                p->channelRecord[p->iWr] = size ;
                p->iWr++;
                if(p->iWr >= p->maxSizeTotalElts)
                {
                    p->iWr = 0;
                }
                p->currentNbElts++;

                if (size > 0)
                {
                    //Data between upper bound of buffer and current data wr index
                    if ((p->iWrData + size) < p->maxSizeTotalData)
                    {
                        memcpy (&p->channelData[p->iWrData], pBuffer, size);

                        p->iWrData = p->iWrData + size;
                    }
                    else
                    {
                        //Data between wr index and upper bound of data buffer then restart at index 0.
                        memcpy (&p->channelData[p->iWrData], &pBuffer[0], p->maxSizeTotalData - p->iWrData);
                        memcpy (&p->channelData[0], &pBuffer[p->maxSizeTotalData - p->iWrData], size - (p->maxSizeTotalData - p->iWrData));

                        p->iWrData = size - (p->maxSizeTotalData - p->iWrData);
                    }

                    //Update global nb data
                    p->currentNbDatas = p->currentNbDatas + size;
                }

                //Signal channel not empty
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


//Flush channel, all task waiting on it are unblocked.
eChannelResult P_CHANNEL_Flush(tChannel*p)
{
    eChannelResult result = E_CHANNEL_RESULT_NOK;
    if (p != NULL)
    {
        xSemaphoreTake(p->lock,portMAX_DELAY);
        {
            //Raz
            memset(p->channelData,0, p->maxSizeTotalData);
            memset(p->channelRecord,0, p->maxSizeTotalElts * sizeof(uint16_t));
            p->iRd = 0;
            p->iWr = 0;
            p->iRdData = 0;
            p->iWrData = 0;
            p->currentNbElts = 0;
            p->currentNbDatas = 0;
            //Unblock all task waiting on it
            xSemaphoreGive(p->isNotEmpty);
            result = E_CHANNEL_RESULT_OK;
        }
        xSemaphoreGive(p->lock);
    }
    return result;
}

//Receive an element
eChannelResult P_CHANNEL_Receive(   tChannel*p,                 //Channel workspace
                                    uint16_t* pOutEltSize,      //Output element size
                                    uint8_t*pBuffer,            //Buffer.  If null, only element size without pop is read
                                    TickType_t xTimeToWait,     //Time to wait in ticks
                                    eChannelReadMode mode)      //Mode RD or KEEP_ONLY. KEEP ONLY read without pop older elemt.
{
    eChannelResult result = E_CHANNEL_RESULT_NOK;
    uint16_t size = 0;


    //Check input parameters
    if ((p != NULL) && (pOutEltSize != NULL))
    {
        *pOutEltSize = 0 ;
        //Wait for signal not empty
        if(xSemaphoreTake(p->isNotEmpty,xTimeToWait)==pdPASS)
        {
            //Critical section
            xSemaphoreTake(p->lock,portMAX_DELAY);
            {
                //Check if some elts are present
                if(p->currentNbElts > 0)
                {
                    //Take elt size
                    size = p->channelRecord[p->iRd] ;
                    *pOutEltSize = size ;

                    //If buffer output exist, read elt data
                    if(pBuffer != NULL)
                    {
                        //If mode normal (pop), raz elt size and update read index
                        //and nb elmts
                        if(mode == E_CHANNEL_RD_MODE_NORMAL)
                        {
                            p->channelRecord[p->iRd] = 0;
                            p->iRd++;
                            if(p->iRd >= p->maxSizeTotalElts)
                            {
                                p->iRd = 0;
                            }
                            p->currentNbElts--;
                        }

                        //If elts have data, read data
                        if(size > 0)
                        {
                            //Data between upper bound of buffer and current data read index
                            if ((p->iRdData + size) < p->maxSizeTotalData)
                            {
                                //Copy data to external buffer
                                memcpy (pBuffer,&p->channelData[p->iRdData] , size);

                                //If mode normal, update data index and raz buffer
                                if( mode == E_CHANNEL_RD_MODE_NORMAL)
                                {
                                    memset (&p->channelData[p->iRdData], 0, size);
                                    p->iRdData =  p->iRdData + size;
                                }
                            }
                            else
                            {
                                //Data between read index and upper bound of data buffer and restart at index 0.
                                memcpy (&pBuffer[0], &p->channelData[p->iRdData], p->maxSizeTotalData - p->iRdData);
                                memcpy (&pBuffer[p->maxSizeTotalData - p->iRdData], &p->channelData[0], size - (p->maxSizeTotalData - p->iRdData));

                                //If mode normal, update data index and raz buffer
                                if( mode == E_CHANNEL_RD_MODE_NORMAL)
                                {
                                    memset (&p->channelData[p->iRdData], 0, p->maxSizeTotalData - p->iRdData);
                                    memset (&p->channelData[0], 0, size - (p->maxSizeTotalData - p->iRdData));

                                    p->iRdData = size - (p->maxSizeTotalData - p->iRdData);
                                }
                            }

                            //If mode normal, update global nb data
                            if( mode == E_CHANNEL_RD_MODE_NORMAL)
                            {
                                p->currentNbDatas = p->currentNbDatas - size;
                            }
                        }
                    }

                    //If some elt present, signal not empty is set
                    if(p->currentNbElts > 0)
                    {
                        xSemaphoreGive(p->isNotEmpty);
                    }
                    result = E_CHANNEL_RESULT_OK;
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
    return result;
}
