/*
 * p_synchronisation.h
 *
 *  Created on: 17 mai 2019
 *      Author: nottin
 */

#ifndef PORTAGE_HELPERS_PLATFORM_DEP_FREERTOS_P_SYNCHRONISATION_H_
#define PORTAGE_HELPERS_PLATFORM_DEP_FREERTOS_P_SYNCHRONISATION_H_

#define MAX_SIGNAL (128)
#define SIGNAL_VALUE (0x80000000)

typedef struct T_ELT_TASK_LIST
{
    TaskHandle_t value;
    unsigned int uwWaitedSig;
    unsigned short nxId;
    unsigned short prId;
} tEltTaskList;

typedef struct T_CONDITION_VARIABLE
{
    QueueHandle_t handleLockCounter;
    unsigned short first;
    unsigned short nbWaiters;
    unsigned short maxWaiters;
    tEltTaskList* taskList;
} tConditionVariable;

typedef enum T_CONDITION_VARIABLE_RESULT
{
    E_COND_VAR_RESULT_OK,
    E_COND_VAR_RESULT_TIMEOUT,
    E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS,
    E_COND_VAR_RESULT_ERROR_MAX_WAITERS,
    E_COND_VAR_RESULT_ERROR_NO_WAITERS
} eConditionVariableResult;

/*Construction condition variable*/
tConditionVariable* BuildConditionVariable(unsigned short int wMaxWaiters);
void DestroyConditionVariable(tConditionVariable** ppv);

eConditionVariableResult SignalAllConditionVariable(tConditionVariable* pv, unsigned int signalValue);
eConditionVariableResult UnlockAndWaitForConditionVariable(tConditionVariable* pv,
                                                           QueueHandle_t handleMutex,
                                                           uint32_t uwSignal,
                                                           uint32_t uwTimeOutMs);

#endif /* PORTAGE_HELPERS_PLATFORM_DEP_FREERTOS_P_SYNCHRONISATION_H_ */
