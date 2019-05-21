#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "p_synchronisation.h"

static eConditionVariableResult PushSignal(tConditionVariable* pv,
                                           TaskHandle_t taskNotifiedValue,
                                           unsigned int uwWaitedSignal)
{
    eConditionVariableResult result = E_COND_VAR_RESULT_OK;
    unsigned short int wCurrentSlotId = 0;
    unsigned short int wNextSlotId = 0;

    if (pv != NULL)
    {
        if (pv->nbWaiters < pv->maxWaiters)
        {
            // Recherche du premier slot signal libre
            while (wCurrentSlotId < pv->maxWaiters)
            {
                if (pv->taskList[wCurrentSlotId].value == 0)
                {
                    break;
                }
                else
                {
                    wCurrentSlotId++;
                }
            }

            // Slot libre trouv. Normalement, un slot doit etre trouvé puisque nbWaiters < MAX_SIGNAL...
            if (wCurrentSlotId < pv->maxWaiters)
            {
                // Si des elements existe avant, le precedent indexe le courant.
                // Le courant indexe le precedent.
                if (wCurrentSlotId > 0)
                {
                    pv->taskList[wCurrentSlotId - 1].nxId = wCurrentSlotId;
                    pv->taskList[wCurrentSlotId].prId = wCurrentSlotId - 1;
                }
                else
                {
                    // Si pas d'lement avant, le courant s'indexe lui meme
                    pv->taskList[wCurrentSlotId].prId = pv->maxWaiters;
                    pv->first = wCurrentSlotId;
                }

                // Recherche premier slot occupé après le slot trouvé
                if ((wCurrentSlotId < (pv->maxWaiters - 1)) && (wCurrentSlotId < pv->nbWaiters))
                {
                    wNextSlotId = wCurrentSlotId + 1;

                    // Recherche du prochain enregistrant un task handle
                    while (wNextSlotId < pv->maxWaiters)
                    {
                        if (pv->taskList[wNextSlotId].value != 0)
                        {
                            break;
                        }
                        else
                        {
                            wNextSlotId++;
                        }
                    }

                    // Si un task handle a été trouve. Normalement, c'est forcément le cas,
                    // puisque wCurrentSlotId < pv->nbWaiters
                    if (wNextSlotId < pv->maxWaiters)
                    {
                        // Il index comme precedent le courant
                        // Le courant index le suivant
                        pv->taskList[wNextSlotId].prId = wCurrentSlotId;
                        pv->taskList[wCurrentSlotId].nxId = wNextSlotId;
                    }
                    else
                    {
                        // Pas d'element indexant un task handle, self reference
                        pv->taskList[wCurrentSlotId].nxId = pv->maxWaiters;
                    }
                }
                else
                {
                    // Il n'existe pas de slot après, le courant s'indexe lui-meme
                    pv->taskList[wCurrentSlotId].nxId = pv->maxWaiters;
                }

                pv->taskList[wCurrentSlotId].value = taskNotifiedValue;
                pv->taskList[wCurrentSlotId].uwWaitedSig = uwWaitedSignal > 0 ? uwWaitedSignal : SIGNAL_VALUE;
                pv->nbWaiters = pv->nbWaiters < pv->maxWaiters ? pv->nbWaiters + 1 : pv->nbWaiters;
            } // if(wCurrentSlotId  < MAX_SIGNAL) pas de slot libre
            else
            {
                result = E_COND_VAR_RESULT_ERROR_MAX_WAITERS;
            }
        } // pv->nbWaiters = pv->nbWaiters < MAX_SIGNAL  pas de slot libre
        else
        {
            result = E_COND_VAR_RESULT_ERROR_MAX_WAITERS;
        }
    }
    return result;
}

static void PopSignal(tConditionVariable* pv, TaskHandle_t taskNotified)
{
    unsigned short wCurrentSlotId = 0;

    if ((pv->nbWaiters > 0) && (taskNotified > 0))
    {
        // Recherche task
        while (wCurrentSlotId < pv->maxWaiters)
        {
            if (pv->taskList[wCurrentSlotId].value == taskNotified)
            {
                break;
            }
            else
            {
                wCurrentSlotId = pv->taskList[wCurrentSlotId].nxId;
            }
        }
        // Si task trouvee, 1 waiters de moins, liberer le slot et update slots occupé adjacents.
        if (wCurrentSlotId < pv->maxWaiters)
        {
            pv->nbWaiters = pv->nbWaiters > 0 ? pv->nbWaiters - 1 : pv->nbWaiters;

            if (wCurrentSlotId == pv->first)
            {
                pv->first = pv->taskList[wCurrentSlotId].nxId;
            }
            if (pv->taskList[wCurrentSlotId].nxId < pv->maxWaiters)
            {
                pv->taskList[pv->taskList[wCurrentSlotId].nxId].prId = pv->taskList[wCurrentSlotId].prId;
            }
            if (pv->taskList[wCurrentSlotId].prId < pv->maxWaiters)
            {
                pv->taskList[pv->taskList[wCurrentSlotId].prId].nxId = pv->taskList[wCurrentSlotId].nxId;
            }

            pv->taskList[wCurrentSlotId].value = 0;
            pv->taskList[wCurrentSlotId].uwWaitedSig = 0;
            pv->taskList[wCurrentSlotId].nxId = pv->maxWaiters;
            pv->taskList[wCurrentSlotId].prId = pv->maxWaiters;
        }
    }
}

void DestroyConditionVariable(tConditionVariable** ppv)
{
    if (ppv != NULL)
    {
        if ((*ppv)->taskList != NULL)
        {
            vPortFree((*ppv)->taskList);
            (*ppv)->taskList = NULL;
        }

        if ((*ppv)->handleLockCounter != NULL)
        {
            vQueueDelete((*ppv)->handleLockCounter);
            (*ppv)->handleLockCounter = NULL;
        }

        vPortFree(ppv);
        ppv = NULL;
    }
}

/*Construction condition variable*/
tConditionVariable* BuildConditionVariable(unsigned short int wMaxWaiters)
{
    unsigned short int iIter = 0;
    tConditionVariable* pv = NULL;

    // Nombre de thread en attente maximum
    if (wMaxWaiters > MAX_SIGNAL)
        goto error;

    // Allocation workspace
    pv = (tConditionVariable*) pvPortMalloc(sizeof(tConditionVariable));
    if (pv == NULL)
        goto error;
    (void) memset(pv, 0, sizeof(tConditionVariable));

    // Allocation liste de threads en attente signal
    pv->maxWaiters = wMaxWaiters;
    pv->taskList = (tEltTaskList*) pvPortMalloc(sizeof(tEltTaskList) * pv->maxWaiters);
    if (pv->taskList == NULL)
        goto error;

    // Index du premier élément. Si = maxWaiters, alors pas de thread.
    pv->first = pv->maxWaiters;
    // Init "chainage"
    for (iIter = 0; iIter < pv->maxWaiters; iIter++)
    {
        pv->taskList[iIter].nxId = pv->maxWaiters;
        pv->taskList[iIter].prId = pv->maxWaiters;
    }
    // Creation critical section
    pv->handleLockCounter = xQueueCreateMutex(queueQUEUE_TYPE_MUTEX);
    if (pv->handleLockCounter == NULL)
        goto error;
    goto success;
error:
    DestroyConditionVariable(&pv);
success:
    return pv;
}

eConditionVariableResult SignalAllConditionVariable(tConditionVariable* pv, unsigned int signalValue)
{
    eConditionVariableResult result = E_COND_VAR_RESULT_ERROR_NO_WAITERS;
    unsigned short int wCurrentSlotId = 0;
    unsigned int signal = signalValue > 0 ? signalValue : SIGNAL_VALUE;

    if ((pv != NULL) && (pv->handleLockCounter != NULL))
    {
        // Critical section
        xQueueSemaphoreTake(pv->handleLockCounter, portMAX_DELAY);
        {
            wCurrentSlotId = pv->first;
            if (pv->nbWaiters > 0)
            {
                // Tant que Nb de tache en attente ET operation de signalement < nombre de tache en attente
                while (wCurrentSlotId < pv->maxWaiters)
                {
                    if ((pv->taskList[wCurrentSlotId].value > 0) &&
                        (pv->taskList[wCurrentSlotId].uwWaitedSig == signal))
                    {
                        xTaskGenericNotify(pv->taskList[wCurrentSlotId].value, signal, eSetBits, NULL);
                        result = E_COND_VAR_RESULT_OK;
                    }
                    wCurrentSlotId = pv->taskList[wCurrentSlotId].nxId;
                }
            }
        }
        xSemaphoreGive(pv->handleLockCounter);
    }
    else
    {
        result = E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS;
    }
    return result;
}

eConditionVariableResult UnlockAndWaitForConditionVariable(tConditionVariable* pv,
                                                           QueueHandle_t handleMutex,
                                                           uint32_t uwSignal,
                                                           uint32_t uwTimeOutMs)
{
    eConditionVariableResult result = E_COND_VAR_RESULT_OK;
    TickType_t xTimeToWait = 0;
    TaskHandle_t handleTask = 0;
    TimeOut_t xTimeOut;
    unsigned int notificationValue = 0;

    if ((pv != NULL) && (pv->handleLockCounter != NULL))
    {
        if (uwTimeOutMs == ULONG_MAX)
        {
            xTimeToWait = portMAX_DELAY;
        }
        else
        {
            xTimeToWait = pdMS_TO_TICKS(uwTimeOutMs);
        }

        // Critical section
        xQueueSemaphoreTake(pv->handleLockCounter, portMAX_DELAY);
        {
            // Récupération du handle de task et sauvegarde en pile
            if (pv->nbWaiters < pv->maxWaiters)
            {
                handleTask = xTaskGetCurrentTaskHandle();
                result = PushSignal(pv, handleTask, uwSignal);
            }
            else
            {
                result = E_COND_VAR_RESULT_ERROR_MAX_WAITERS;
            }
        }
        xSemaphoreGive(pv->handleLockCounter);

        // Libération du mutex passé en paramètre
        if (handleMutex != NULL)
        {
            xSemaphoreGive(handleMutex);
        }

        if (result == E_COND_VAR_RESULT_OK)
        {
            // Attente signalement ou Timeout
            // Dans les 2 cas, on depile le handle
            // (on indique à la tache qui signale un waiter de moins)

            for (;;)
            {
                vTaskSetTimeOutState(&xTimeOut); // RAZ timeout
                // Attente signal
                if (xTaskNotifyWait(0, uwSignal, &notificationValue, xTimeToWait) != pdPASS)
                {
                    result = E_COND_VAR_RESULT_TIMEOUT;
                    // Pas de notification reçue pendant le délai imparti
                    break;
                }
                else
                {
                    // Arrivee notification, check si la notif est celle attendu.
                    if ((notificationValue & uwSignal) != uwSignal)
                    {
                        // Si ce n'est pas le cas, update du temps restant à attendre la notification attendue
                        if (xTaskCheckForTimeOut(&xTimeOut, &xTimeToWait) == pdTRUE)
                        {
                            // Sinon timeout
                            result = E_COND_VAR_RESULT_TIMEOUT;
                            break;
                        }
                    }
                    else
                    {
                        // Notif attendue reçue
                        break;
                    }
                }
            }

            // Critical section
            xQueueSemaphoreTake(pv->handleLockCounter, portMAX_DELAY);
            {
                PopSignal(pv, handleTask);
            }
            xSemaphoreGive(pv->handleLockCounter);
        }

        // Prise du mutex passé en paramètre
        if (handleMutex != NULL)
        {
            xSemaphoreTake(handleMutex, portMAX_DELAY);
        }
    }
    else
    {
        result = E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS;
    }

    return result;
}
