/*
 * p_logsrv.c
 *
 *  Created on: 8 juin 2019
 *      Author: elsin
 */

#include "p_logsrv.h"

static void xTaskSocketClientTx(void*pParameters);
static void xTaskSocketClientRx(void*pParameters);
static void xTaskSocketClientCallbackMon(void*pParameters);
static void xTaskSocketServerCallback(void*pParameters);

typedef void (*ptrFct_AnalyzerCallback)(uint8_t*pBuffer, uint16_t dataSize);
typedef void (*ptrFct_AnalyzerTimeoutCallback)(void);
typedef void (*ptrFct_AnalyzerConnexion)(void);
typedef void (*ptrFct_AnalyzerDeconnexion)(void);


typedef enum E_CHANNEL_RESULT
{
    E_CHANNEL_RESULT_OK,
    E_CHANNEL_RESULT_NOK,
    E_CHANNEL_RESULT_ERROR_FULL,
    E_CHANNEL_RESULT_ERROR_EMPTY
}eChannelResult;

typedef enum E_CHANNEL_READ_MODE
{
    E_CHANNEL_RD_MODE_NORMAL,
    E_CHANNEL_RD_MODE_JUST_KEEPING
}eChannelReadMode;

typedef enum E_CHANNEL_WRITE_MODE
{
    E_CHANNEL_WR_MODE_NORMAL,
    E_CHANNEL_WR_MODE_OVERWRITE
}eChannelWriteMode;

typedef struct T_CHANNEL
{
    uint16_t iWr ;
    uint16_t iRd ;
    uint16_t iWrData;
    uint16_t iRdData;
    uint16_t maxSizeTotalData;
    uint16_t maxSizeTotalElts;
    uint16_t maxSizeDataPerElt;
    uint16_t currentNbElts;
    uint16_t currentNbDatas;
    QueueHandle_t lock ;
    QueueHandle_t isNotEmpty;
    uint8_t *channelData;
    uint16_t *channelRecord;
}tChannel;

typedef struct T_LOG_SERVER_WORKSPACE
{

    int32_t socket;

    uint16_t maxClient;
    uint16_t port;

    tUtilsList clientList;

    eLogServerStatus status ;

    TaskHandle_t handleTask;
    QueueHandle_t quitRequest;
    QueueHandle_t joinServerTask ;

}tLogSrvWks;

typedef struct T_LOG_CLIENT_WORKSPACE
{

    int32_t socket;

    eLogClientStatus status ;

    uint32_t timeoutActivite ;
    uint32_t trigTimeoutActivite;

    uint8_t bActiviteTx ;
    uint8_t bActiviteRx ;

    uint8_t bufferTX[P_LOG_FIFO_TX_DATA_SIZE];
    uint8_t bufferRX[P_LOG_FIFO_RX_DATA_SIZE];
    uint8_t bufferANALYZER[P_LOG_FIFO_RX_DATA_SIZE];



    tLogSrvWks*pServer;

    TaskHandle_t handleTaskMonitor;
    TaskHandle_t handleTaskTx;
    TaskHandle_t handleTaskRx;

    QueueHandle_t joinClientTaskMonitor ;
    QueueHandle_t joinClientTaskTx ;
    QueueHandle_t joinClientTaskRx;

    QueueHandle_t quitRequestTaskMonitor;
    QueueHandle_t quitRequestTaskTx;
    QueueHandle_t quitRequestTaskRx;

    tChannel channelInput;
    tChannel channelOutput;

    ptrFct_AnalyzerCallback cbAnalyzerCallback;
    ptrFct_AnalyzerTimeoutCallback cbAnalyzerTimeoutCallback;
    ptrFct_AnalyzerConnexion cbAnalyzerConnexionCallback;
    ptrFct_AnalyzerDeconnexion cbAnalyzerDeconnexionCallback;


}tLogClientWks;

void channel_deinit(tChannel*p)
{
    if(p!=NULL)
    {
        if(p->isNotEmpty != NULL)vQueueDelete(p->isNotEmpty);
        if(p->lock != NULL)vQueueDelete(p->lock);
        if(p->channelData != NULL)
        {
            memset(p->channelData,0,p->maxSizeTotalData);
            vPortFree(p->channelData);
        }
        if(p->channelRecord != NULL)
        {
            memset(p->channelRecord,0,p->maxSizeTotalElts);
            vPortFree(p->channelRecord);
        }
        memset(p,0,sizeof(tChannel));
    }
}

eChannelResult channel_init(tChannel*p, size_t maxEltSize, size_t nbElts)
{
    eChannelResult result = E_CHANNEL_RESULT_NOK;
    if(p!=NULL)
    {
        memset(p,0,sizeof(tChannel));
        p->channelData = pvPortMalloc(maxEltSize * nbElts) ;
        p->channelRecord = pvPortMalloc(sizeof(uint16_t) * nbElts) ;

        p->isNotEmpty = xSemaphoreCreateBinary();
        p->lock = xSemaphoreCreateMutex();

        if((p->isNotEmpty == NULL) || (p->lock == NULL) || ( p->channelData == NULL) || (p->channelRecord == NULL))
        {
            channel_deinit(p);
        }
        else
        {
            (void)memset(p->channelData,0,maxEltSize * nbElts);
            (void)memset(p->channelRecord,0,nbElts * sizeof(uint16_t));
            p->maxSizeTotalElts = nbElts;
            p->maxSizeTotalData = nbElts * maxEltSize;
            p->maxSizeDataPerElt = maxEltSize ;
            xSemaphoreTake(p->isNotEmpty,0);
            result = E_CHANNEL_RESULT_OK;
        }
    }
    return result;
}



eChannelResult channel_send(tChannel*p, const uint8_t * pBuffer, uint16_t size, eChannelWriteMode mode)
{
    eChannelResult result = E_CHANNEL_RESULT_NOK;
    uint32_t dataSize = 0;


    if((p!=NULL)&&(size > 0)&&(pBuffer != NULL))
    {
        xSemaphoreTake(p->lock,portMAX_DELAY);
        {
            if(( mode == E_CHANNEL_WR_MODE_OVERWRITE) && (size <= p->maxSizeDataPerElt))
            {
                while( ((p->currentNbElts + 1) > p->maxSizeTotalElts) || ((size + p->currentNbDatas) > p->maxSizeTotalData )  )
                {
                    dataSize = p->channelRecord[p->iRd];
                    p->channelRecord[p->iRd] = 0;
                    p->iRd++;
                    if(p->iRd >= p->maxSizeTotalElts)
                    {
                        p->iRd = 0;
                    }
                    if(dataSize > 0)
                    {
                        if ((p->iRdData + size) < p->maxSizeTotalData)
                        {

                            memset (&p->channelData[p->iRdData], 0, size);
                            p->iRdData =  p->iRdData + size;

                        }
                        else
                        {
                            memset (&p->channelData[p->iRdData], 0, p->maxSizeTotalData - p->iRdData);
                            memset (&p->channelData[0], 0, size - (p->maxSizeTotalData - p->iRdData));
                            p->iRdData = size - (p->maxSizeTotalData - p->iRdData);
                        }
                    }
                }
            }

            if(((p->currentNbElts + 1) <= p->maxSizeTotalElts) && ((size + p->currentNbDatas) <= p->maxSizeTotalData ) && (size <= p->maxSizeDataPerElt))
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
                    if ((p->iWrData + size) < p->maxSizeTotalData)
                    {
                        memcpy (&p->channelData[p->iWrData], pBuffer, size);

                        p->iWrData = p->iWrData + size;
                    }
                    else
                    {
                        memcpy (&p->channelData[p->iWrData], &pBuffer[0], p->maxSizeTotalData - p->iWrData);
                        memcpy (&p->channelData[0], &pBuffer[p->maxSizeTotalData - p->iWrData], size - (p->maxSizeTotalData - p->iWrData));

                        p->iWrData = size - (p->maxSizeTotalData - p->iWrData);
                    }

                    p->currentNbDatas = p->currentNbDatas + size;
                }

                if (p->currentNbElts > 0)
                {
                    xSemaphoreGive(p->isNotEmpty);
                }
                result = E_CHANNEL_RESULT_OK;
            }
            else
            {
                result = E_CHANNEL_RESULT_ERROR_FULL;
            }
        }
        xSemaphoreGive(p->lock);
    }
    return result;
}

eChannelResult channel_flush(tChannel*p)
{
    eChannelResult result = E_CHANNEL_RESULT_NOK;
    if (p != NULL)
    {
        xSemaphoreTake(p->lock,portMAX_DELAY);
        {
            (void)memset(p->channelData,0, p->maxSizeTotalData);
            (void)memset(p->channelRecord,0, p->maxSizeTotalElts * sizeof(uint16_t));
            p->iRd = 0;
            p->iWr = 0;
            p->iRdData = 0;
            p->iWrData = 0;
            p->currentNbElts = 0;
            p->currentNbDatas = 0;

            xSemaphoreGive(p->isNotEmpty);
            result = E_CHANNEL_RESULT_OK;
        }
        xSemaphoreGive(p->lock);
    }
    return result;
}

eChannelResult channel_receive(tChannel*p, uint16_t* pOutEltSize, uint8_t*pBuffer, TickType_t xTimeToWait, eChannelReadMode mode)
{
    eChannelResult result = E_CHANNEL_RESULT_NOK;
    uint16_t size = 0;
    if ((p != NULL) && (pOutEltSize != NULL))
    {
        if(xSemaphoreTake(p->isNotEmpty,xTimeToWait)==pdPASS)
        {
            xSemaphoreTake(p->lock,portMAX_DELAY);
            {
                if(p->currentNbElts > 0)
                {
                    size = p->channelRecord[p->iRd] ;
                    *pOutEltSize = size ;

                    if(pBuffer != NULL)
                    {

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

                        if(size > 0)
                        {

                            if ((p->iRdData + size) < p->maxSizeTotalData)
                            {
                                memcpy (pBuffer,&p->channelData[p->iRdData] , size);

                                if( mode == E_CHANNEL_RD_MODE_NORMAL)
                                {
                                    memset (&p->channelData[p->iRdData], 0, size);
                                    p->iRdData =  p->iRdData + size;
                                }
                            }
                            else
                            {
                                memcpy (&pBuffer[0], &p->channelData[p->iRdData], p->maxSizeTotalData - p->iRdData);
                                memcpy (&pBuffer[p->maxSizeTotalData - p->iRdData], &p->channelData[0], size - (p->maxSizeTotalData - p->iRdData));

                                if( mode == E_CHANNEL_RD_MODE_NORMAL)
                                {
                                    memset (&p->channelData[p->iRdData], 0, p->maxSizeTotalData - p->iRdData);
                                    memset (&p->channelData[0], 0, size - (p->maxSizeTotalData - p->iRdData));

                                    p->iRdData = size - (p->maxSizeTotalData - p->iRdData);
                                }
                            }

                            if( mode == E_CHANNEL_RD_MODE_NORMAL)
                            {
                                p->currentNbDatas = p->currentNbDatas - size;
                            }
                        }

                    }

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
            result = E_CHANNEL_RESULT_ERROR_EMPTY;
        }
    }
    return result;
}


//Task de transmission vers logger externe
static void xTaskSocketClientTx(void*pParameters)
{
    tLogClientWks*pClt = (tLogClientWks*)pParameters;
    uint16_t nbBytesToSend  = 0;
    int16_t byteSent = 0;
    int16_t iIter = 0;

    if (pClt != NULL)
    {
        if( xTaskCreate(    xTaskSocketClientRx,
                            "logCltRx",
                            P_LOG_CLT_RX_CALLBACK_STACK,
                            pClt,configMAX_PRIORITIES - 1,
                            &pClt->handleTaskRx ) == pdPASS)
        {

            pClt->bActiviteTx = 0;
            while((pClt->status == E_LOG_CLIENT_CONNECTED) && (xSemaphoreTake(pClt->quitRequestTaskTx,0) == pdFAIL ))
            {

                memset(pClt->bufferTX,0,P_LOG_FIFO_TX_DATA_SIZE);
                if(channel_receive(     &pClt->channelOutput,
                                        &nbBytesToSend,
                                        &pClt->bufferTX[0],
                                        pdMS_TO_TICKS(P_LOG_CLT_TX_POP_WAIT),
                                        E_CHANNEL_RD_MODE_NORMAL) == E_CHANNEL_RESULT_OK)
                {

                    for(iIter = 0 ; iIter < ((int16_t)nbBytesToSend) ; iIter += byteSent)
                    {
                        byteSent = lwip_send(pClt->socket,pClt->bufferTX + iIter, nbBytesToSend - iIter,0);
                        if(byteSent < 0)
                        {
                            pClt->status = E_LOG_CLIENT_DISCONNECTED ;
                        }
                        else
                        {
                            pClt->bActiviteTx = 1;
                        }
                    }
                }
                else
                {
                    pClt->bActiviteTx = 0;
                }
            }

            pClt->bActiviteTx = 0;
            pClt->status = E_LOG_CLIENT_DISCONNECTED;
            channel_flush(&pClt->channelInput);
            xSemaphoreGive(pClt->quitRequestTaskRx);
            xSemaphoreTake(pClt->joinClientTaskRx,portMAX_DELAY);
        }
        else
        {
            pClt->status = E_LOG_CLIENT_DISCONNECTED;
        }


        if (pClt->socket != -1)
        {
            lwip_shutdown(pClt->socket,SHUT_RDWR);
            lwip_close(pClt->socket);
            pClt->socket = -1;
        }

        xSemaphoreGive(pClt->joinClientTaskTx);
    }
    vTaskDelete(NULL);
}

//Task de reception
static void xTaskSocketClientRx(void*pParameters)
{
    tLogClientWks*pClt = (tLogClientWks*)pParameters;
    uint16_t nbBytesReceived = 0;
    eChannelResult resultFifo = E_CHANNEL_RESULT_NOK;

    if (pClt != NULL)
    {

        if(pClt->cbAnalyzerConnexionCallback != NULL)
        {
            pClt->cbAnalyzerConnexionCallback();
        }

        pClt->bActiviteRx = 0;
        while((pClt->status == E_LOG_CLIENT_CONNECTED) && (xSemaphoreTake(pClt->quitRequestTaskRx,0) == pdFAIL ))
        {

            memset(pClt->bufferANALYZER,0,P_LOG_FIFO_RX_DATA_SIZE);

            resultFifo = channel_receive(  &pClt->channelInput,
                                           &nbBytesReceived,
                                           &pClt->bufferANALYZER[0],
                                           pdMS_TO_TICKS(P_LOG_CLT_RX_POP_WAIT),
                                           E_CHANNEL_RD_MODE_NORMAL);

            if(resultFifo == E_CHANNEL_RESULT_OK)
            {
                pClt->bActiviteRx = 1;
                if(pClt->cbAnalyzerCallback != NULL)
                {
                    pClt->cbAnalyzerCallback(pClt->bufferANALYZER,nbBytesReceived);
                }
            }
            else
            {
                pClt->bActiviteRx = 0;
                //Timeout nothing to analyze
                if(pClt->cbAnalyzerTimeoutCallback != NULL)
                {
                    pClt->cbAnalyzerTimeoutCallback();
                }
            }
        }

        pClt->bActiviteRx = 0;

        if(pClt->cbAnalyzerDeconnexionCallback != NULL)
        {
            pClt->cbAnalyzerDeconnexionCallback();
        }

        xSemaphoreGive(pClt->joinClientTaskRx);
    }
    vTaskDelete(NULL);
}

static void xTaskSocketClientCallbackMon(void*pParameters)
{

    tLogClientWks*p = (tLogClientWks*)pParameters;
    struct timeval timeout ;
    fd_set rdfs;
    int32_t nbBytesReceived = 0;

    if(p != NULL)
    {

        //Creation task Tx

        xSemaphoreGive(p->joinClientTaskTx);
        if( xTaskCreate(    xTaskSocketClientTx,
                            "logCltTx",
                            P_LOG_CLT_TX_CALLBACK_STACK,
                            p,
                            configMAX_PRIORITIES - 1,
                            &p->handleTaskTx ) == pdPASS)
        {


            while((p->status == E_LOG_CLIENT_CONNECTED) && (xSemaphoreTake(p->quitRequestTaskMonitor,0) == pdFAIL ))
            {
                FD_ZERO(&rdfs);
                FD_SET(p->socket,&rdfs);
                timeout.tv_sec = 0;
                timeout.tv_usec = 1000 * P_LOG_CLT_TIMEOUT_SELECT ;

                //Monitoring socket

                if( lwip_select(p->socket + 1, &rdfs, NULL, NULL, &timeout) == -1)    //Check deconnexion
                {
                    p->status = E_LOG_CLIENT_DISCONNECTED;
                }
                else
                {
                    //Check if data arrived
                    if( FD_ISSET(p->socket,&rdfs) )
                    {
                        nbBytesReceived = lwip_recv( p->socket,
                                                     p->bufferRX,
                                                     P_LOG_FIFO_RX_DATA_SIZE,
                                                     MSG_DONTWAIT);

                        if( nbBytesReceived <= 0)
                        {
                            p->status = E_LOG_CLIENT_DISCONNECTED;
                        }
                        else
                        {
                            p->timeoutActivite = 0;

                            if(channel_send(&p->channelInput,p->bufferRX,nbBytesReceived,E_CHANNEL_WR_MODE_NORMAL) == E_CHANNEL_RESULT_ERROR_FULL)
                            {
                                channel_flush(&p->channelInput) ;
                            }

                        }
                    }
                    else
                    {
                        p->timeoutActivite++;
                        if ( p->timeoutActivite >= p->trigTimeoutActivite)
                        {
                            p->timeoutActivite = 0;
                            if( p->trigTimeoutActivite > 0)
                            {
                                p->status = E_LOG_CLIENT_DISCONNECTED;
                            }
                        }
                    }
                }
            }

            channel_flush(&p->channelOutput);
            p->status = E_LOG_CLIENT_DISCONNECTED;
            xSemaphoreGive(p->quitRequestTaskTx);
            xSemaphoreTake(p->joinClientTaskTx,portMAX_DELAY);
        }
        else
        {
            p->status = E_LOG_CLIENT_DISCONNECTED;
        }

        if (p->socket != -1)
        {
            lwip_shutdown(p->socket,SHUT_RDWR);
            lwip_close(p->socket);
            p->socket = -1;
        }

        xSemaphoreGive(p->joinClientTaskMonitor);



    }
    vTaskDelete(NULL);
}



static void stop_and_destroy_clt_workspace(tLogClientWks**p)
{
    if((p!=NULL)&&((*p)!=NULL))
    {

        (*p)->status = E_LOG_CLIENT_DISCONNECTED;

        if((*p)->quitRequestTaskMonitor != NULL)
        {
            xSemaphoreGive((*p)->quitRequestTaskMonitor);
        }

        if((*p)->joinClientTaskMonitor != NULL)
        {
            xSemaphoreTake((*p)->joinClientTaskMonitor,portMAX_DELAY);
        }

        if((*p)->joinClientTaskMonitor != NULL)
        {
            vQueueDelete((*p)->joinClientTaskMonitor);
            (*p)->joinClientTaskMonitor = NULL;
        }
        if((*p)->joinClientTaskRx != NULL)
        {
            vQueueDelete((*p)->joinClientTaskRx);
            (*p)->joinClientTaskRx = NULL;
        }
        if((*p)->joinClientTaskTx != NULL)
        {
            vQueueDelete((*p)->joinClientTaskTx);
            (*p)->joinClientTaskTx = NULL;
        }

        if((*p)->quitRequestTaskMonitor != NULL)
        {
           vQueueDelete((*p)->quitRequestTaskMonitor);
           (*p)->quitRequestTaskMonitor = NULL;
        }
        if((*p)->quitRequestTaskRx != NULL)
        {
           vQueueDelete((*p)->quitRequestTaskRx);
           (*p)->quitRequestTaskRx = NULL;
        }

        if((*p)->quitRequestTaskTx != NULL)
        {
           vQueueDelete((*p)->quitRequestTaskTx);
           (*p)->quitRequestTaskTx = NULL;
        }

        if((*p)->socket != -1)
        {
            lwip_shutdown((*p)->socket,SHUT_RDWR);
            lwip_close((*p)->socket);
            (*p)->socket = -1;
        }

        channel_deinit(&(*p)->channelOutput);
        channel_deinit(&(*p)->channelInput);

        (void)memset(*p,0,sizeof(tLogClientWks));
        vPortFree(*p);
        *p = NULL;

    }
}

static tLogClientWks* create_clt_workspace_and_start(int32_t socket, tLogSrvWks*pServ)
{
    tLogClientWks*pClt = NULL;
    eChannelResult resFifo = E_CHANNEL_RESULT_NOK;


    if( pServ == NULL)
    {
        return NULL;
    }

    pClt = pvPortMalloc(sizeof(tLogClientWks)) ;
    if (pClt == NULL)
    {
        return NULL;
    }

    (void)memset(pClt,0,sizeof(tLogClientWks));
    pClt->socket = socket ;
    pClt->joinClientTaskMonitor = xSemaphoreCreateBinary();
    pClt->joinClientTaskTx = xSemaphoreCreateBinary();
    pClt->joinClientTaskRx = xSemaphoreCreateBinary();
    pClt->quitRequestTaskMonitor = xSemaphoreCreateBinary();
    pClt->quitRequestTaskRx = xSemaphoreCreateBinary();
    pClt->quitRequestTaskTx = xSemaphoreCreateBinary();
    pClt->cbAnalyzerCallback = NULL;
    pClt->cbAnalyzerConnexionCallback = NULL;
    pClt->cbAnalyzerDeconnexionCallback = NULL;
    pClt->cbAnalyzerTimeoutCallback = NULL;

    pClt->pServer = pServ;
    pClt->status = E_LOG_CLIENT_CONNECTED;
    resFifo = channel_init(    &pClt->channelInput,
                            P_LOG_FIFO_RX_DATA_SIZE,
                            P_LOG_FIFO_RX_ELT_SIZE);
    if(resFifo == E_CHANNEL_RESULT_OK)
    {
        resFifo = channel_init(    &pClt->channelOutput,
                                P_LOG_FIFO_TX_DATA_SIZE,
                                P_LOG_FIFO_TX_ELT_SIZE);
    }

            //Init fifo TX

    if(         (pClt->joinClientTaskMonitor == NULL)
            ||  (pClt->joinClientTaskTx == NULL)
            ||  (pClt->joinClientTaskRx == NULL)
            ||  (pClt->quitRequestTaskMonitor == NULL)
            ||  (pClt->quitRequestTaskRx == NULL)
            ||  (pClt->quitRequestTaskTx == NULL)
            ||  (resFifo != E_CHANNEL_RESULT_OK))
    {
        stop_and_destroy_clt_workspace(&pClt);
        return NULL;
    }

    xSemaphoreTake(pClt->joinClientTaskMonitor,0);
    xSemaphoreTake(pClt->joinClientTaskTx,0);
    xSemaphoreTake(pClt->joinClientTaskRx,0);
    xSemaphoreTake(pClt->quitRequestTaskMonitor,0);
    xSemaphoreTake(pClt->quitRequestTaskRx,0);
    xSemaphoreTake(pClt->quitRequestTaskTx,0);

    xSemaphoreGive(pClt->joinClientTaskMonitor);
    if( xTaskCreate(    xTaskSocketClientCallbackMon,
                        "logClt",
                        P_LOG_SRV_CALLBACK_STACK,
                        pClt,
                        configMAX_PRIORITIES - 1,
                        &pClt->handleTaskMonitor ) != pdPASS)
    {
        stop_and_destroy_clt_workspace(&pClt);
        return NULL;
    }
    else
    {
        xSemaphoreTake(pClt->joinClientTaskMonitor,0);
    }

    return pClt ;
}



static void xTaskSocketServerCallback(void*pParameters)
{

    struct sockaddr_in sin = {0};
    struct timeval lwipTimeOut ;
    struct fd_set rdfs;
    struct sockaddr_in csin = {0};
    socklen_t sinsize = sizeof(csin);
    int32_t csock = -1;
    int32_t opt = 1;
    uint32_t resLwip = 0;
    uint16_t wCurrentSlotId = 0;
    eUtilsListResult resList ;


    tLogSrvWks*p = (tLogSrvWks*)pParameters;
    tLogClientWks*pClt = NULL;

    if(p!=NULL)
    {
        while(p->status != E_LOG_SERVER_CLOSING)
        {
            if(xSemaphoreTake(p->quitRequest,0) == pdPASS)
            {
                p->status = E_LOG_SERVER_CLOSING;
            }

            switch(p->status)
            {
                case E_LOG_SERVER_CLOSING:
                {
                    wCurrentSlotId = USHRT_MAX;
                    do
                    {
                        pClt = P_UTILS_LIST_ParseValueEltMT(&p->clientList,NULL,NULL,NULL,&wCurrentSlotId);
                        if(pClt != NULL)
                        {
                            P_UTILS_LIST_RemoveEltMT(&p->clientList,pClt,0,0,&wCurrentSlotId);
                            stop_and_destroy_clt_workspace(&pClt);
                        }
                    }while(wCurrentSlotId != USHRT_MAX);
                }
                break;
                case E_LOG_SERVER_BINDING:
                {
                    resLwip = -1 ;

                    if(P_ETHERNET_IF_IsReady() == 0)
                    {
                        p->socket = lwip_socket(AF_INET,SOCK_STREAM,0);
                        if(p->socket >= 0)
                        {
                            resLwip = lwip_setsockopt(p->socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
                            if(resLwip == 0)
                            {
                                (void)memset(&sin,0,sizeof(struct sockaddr_in)) ;
                                sin.sin_addr.s_addr = htonl(INADDR_ANY);
                                sin.sin_family = AF_INET;
                                sin.sin_port = htons(p->port);

                                lwip_bind(p->socket,(struct sockaddr*)&sin,sizeof(struct sockaddr_in));
                                if(resLwip == 0)
                                {
                                    resLwip = lwip_listen(p->socket,p->maxClient + 1);
                                    if(resLwip == 0)
                                    {
                                        csock = -1 ;
                                    }
                                }
                            }
                        }
                    }

                    if(resLwip == 0)
                    {
                        p->status = E_LOG_SERVER_ONLINE;
                    }
                    else
                    {
                        if (p->socket >= 0)
                        {
                            lwip_shutdown(p->socket,SHUT_RDWR);
                            lwip_close(p->socket);
                            p->socket = -1;
                        }
                        p->status = E_LOG_SERVER_BINDING;
                        vTaskDelay(pdMS_TO_TICKS(P_LOG_SRV_TIMEOUT_SELECT));
                    }
                }
                break;
                case E_LOG_SERVER_ONLINE:
                {
                    //Reset fd server
                    FD_ZERO(&rdfs);
                    //Add server to fd
                    FD_SET(p->socket,&rdfs);
                    //Timeout
                    lwipTimeOut.tv_sec = 0 ;
                    lwipTimeOut.tv_usec = 0 * P_LOG_SRV_TIMEOUT_SELECT;
                    //New connection monitoring
                    if(lwip_select(p->socket + 1, &rdfs,NULL,NULL,&lwipTimeOut) == -1)
                    {
                        FD_ZERO(&rdfs);
                        FD_SET(p->socket,&rdfs);
                    }

                    if(FD_ISSET(p->socket,&rdfs))
                    {
                        //Accept connexion
                        (void)memset(&csin,0,sinsize);
                        csock = lwip_accept(p->socket,(struct sockaddr*)&csin,&sinsize);
                        if(csock != -1)
                        {
                            //Create client workspace + thread...

                            if(P_UTILS_LIST_GetNbEltMT(&p->clientList) < p->maxClient)
                            {
                                pClt = create_clt_workspace_and_start(csock,p);
                                if(pClt != NULL)
                                {
                                    resList = P_UTILS_LIST_AddEltMT(&p->clientList,pClt,NULL,0,0);
                                    if(resList != E_UTILS_LIST_RESULT_OK)
                                    {
                                        stop_and_destroy_clt_workspace(&pClt);
                                    }
                                }

                            }
                            else
                            {
                                lwip_shutdown(csock,SHUT_RDWR);
                                lwip_close(csock);
                            }


                            pClt = NULL;
                            csock = -1 ;
                        }
                    }
                    else
                    {
                        csock = -1 ;
                        //Timeout de non connexion
                        //Check client to destroy
                        wCurrentSlotId = USHRT_MAX;
                        do
                        {
                            pClt = P_UTILS_LIST_ParseValueEltMT(&p->clientList,NULL,NULL,NULL,&wCurrentSlotId);
                            if(pClt != NULL)
                            {
                                if(pClt->status != E_LOG_CLIENT_CONNECTED)
                                {
                                    P_UTILS_LIST_RemoveEltMT(&p->clientList,pClt,0,0,&wCurrentSlotId);
                                    stop_and_destroy_clt_workspace(&pClt);
                                }
                            }
                        }while(wCurrentSlotId != USHRT_MAX);

                    }
                }
                break;
                default :
                {
                    //Nothing
                }
                break;

            }
        }

        xSemaphoreGive(p->joinServerTask);

    }


    vTaskDelete(NULL);
}



void P_LOG_SRV_StopAndDestroy(tLogSrvWks**p)
{
    if((p!=NULL)&&((*p)!=NULL))
    {
        (*p)->status = E_LOG_SERVER_CLOSING;


        if((*p)->quitRequest != NULL)
        {
            xSemaphoreGive((*p)->quitRequest);
        }

        if((*p)->joinServerTask != NULL)
        {
            xSemaphoreTake((*p)->joinServerTask,portMAX_DELAY);
        }

        if((*p)->quitRequest != NULL)
        {
            vQueueDelete((*p)->quitRequest);
            (*p)->quitRequest = NULL;
        }

        if((*p)->joinServerTask != NULL)
        {
            vQueueDelete((*p)->joinServerTask);
            (*p)->joinServerTask = NULL ;
        }

       P_UTILS_LIST_DeInitMT(&(*p)->clientList);
       (void)memset(*p,0,sizeof(tLogSrvWks));
       vPortFree(*p);
       *p = NULL;
    }
}

tLogSrvWks* P_LOG_SRV_CreateAndStart(uint16_t port, int16_t maxClient)
{
    tLogSrvWks*p = (tLogSrvWks*)pvPortMalloc(sizeof(tLogSrvWks));

    if ( p == NULL )
    {
        return NULL;
    }

    (void)memset(p,0,sizeof(tLogSrvWks));
    p->socket = -1 ;
    p->port = port;
    p->status = E_LOG_SERVER_BINDING;
    p->maxClient = maxClient;
    p->joinServerTask = xSemaphoreCreateBinary();
    if(p->joinServerTask == NULL)
    {
        P_LOG_SRV_StopAndDestroy(&p);
        return p;
    }

    p->quitRequest = xSemaphoreCreateBinary();
    if(p->quitRequest == NULL)
    {
        P_LOG_SRV_StopAndDestroy(&p);
        return p;
    }

    //Raz quit signal
    xSemaphoreTake(p->quitRequest,0);

    if(P_UTILS_LIST_InitMT(&p->clientList,maxClient) != 0)
    {
        P_LOG_SRV_StopAndDestroy(&p);
        return p ;
    }

    //Give join for delete
    xSemaphoreGive(p->joinServerTask);
    if (xTaskCreate(    xTaskSocketServerCallback,      //
                        "logSrv",                       //
                        P_LOG_SRV_CALLBACK_STACK,       //
                        p,                              //
                        configMAX_PRIORITIES - 1,       //
                        &p->handleTask) != pdPASS)      //
    {
        P_LOG_SRV_StopAndDestroy(&p);
        return p ;
    }
    else
    {
        //Raz join signal
        xSemaphoreTake(p->joinServerTask,0);
    }

    return p;
}
