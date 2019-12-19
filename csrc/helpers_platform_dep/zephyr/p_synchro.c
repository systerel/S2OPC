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

#include <errno.h>
#include <inttypes.h>
#include <kernel.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "sopc_enums.h" /* s2opc includes */

#include "p_synchro.h"

#define P_SYNCHRO_CONDITION_DEBUG (1)
#define P_SYNCHRO_MUTEX_DEBUG (1)

#ifndef NULL
#define NULL ((void*) 0)
#endif
#ifndef K_FOREVER
#define K_FOREVER (-1)
#endif
#ifndef K_NO_WAIT
#define K_NO_WAIT 0
#endif

#define MASK_SET_QUIT_FLAG (0x80000000)

// *** Private enumeration definition ***

// Mutex or condition variable status
typedef enum E_SYNCHRO_STATUS
{
    E_SYNC_STATUS_NOT_INITIALIZED, // Condition / Mutex variable not initialized
    E_SYNC_STATUS_INITIALIZING,    // Condition / Mutex variable initializing
    E_SYNC_STATUS_DEINITIALIZING,  // Condition / Mutex variable de initializing
    E_SYNC_STATUS_INITIALIZED,     // Condition / Mutex variable initialized
    E_SYNC_STATUS_SIZE = INT32_MAX
} eSyncStatus;

// *** Private object definition ***

// Signal status used by condition variable
typedef enum E_SIG_STATUS
{
    E_SIG_STATUS_NOT_RESERVED,    // Signal free to use
    E_SIG_STATUS_RESERVING,       // Signal reserving
    E_SIG_STATUS_WAIT_FOR_SIGNAL, // Signal waiting
    E_SIG_STATUS_SIZE = INT32_MAX
} eSigStatus;

// Signal object definition
typedef struct tSignal
{
    volatile eSigStatus sigStatus; // Signal status
    struct k_sem signal;           // Signal (semaphore object)
} tSignal;

// Condition variable object definition
typedef struct tCondVar
{
    volatile eSyncStatus status;              // Condition variable status
    tSignal tabSignals[MAX_COND_VAR_WAITERS]; // Table of signals
} tCondVar;

// Mutex variable object definition
typedef struct tMutVar
{
    volatile eSyncStatus status; // Mutex status
    tCondVarHandle condVarHdl;   // Handle of condition variable
    uint32_t lockCounter;        // Lock counter
    k_tid_t ownerThread;         // Mutex ownwer thread
    struct k_mutex lock;         // Kernel mutex
} tMutVar;

// *** Private workspace definition, preallocated condition variables and mutex ***

// Condition variables workspace, handle table of condition variables
static struct tCondVarWks
{
    struct tCondVar tabWks[MAX_COND_VAR];
} gCondVarWks;

// Mutex variables workspace, handle table of mutex variables
static struct tMutVarWks
{
    struct tMutVar tabWks[MAX_MUT_VAR];
} gMutVarWks;

// *** Private functions declarations ***

// Modify atomically status of condition variable : set flag quit,
// increment / decrement status from initialized status
static inline eSyncStatus P_SYNCHRO_Condition_Set_Quit_Flag(tCondVar* p);   // Initiate clear operation
static inline eSyncStatus P_SYNCHRO_Condition_Add_Api_User(tCondVar* p);    // Increment in use counter
static inline eSyncStatus P_SYNCHRO_Condition_Remove_Api_User(tCondVar* p); // Decrement in use counter

static inline eSyncStatus P_SYNCHRO_Mutex_Remove_Api_User(tMutVar* p); // Decrement in use counter
static inline eSyncStatus P_SYNCHRO_Mutex_Add_Api_User(tMutVar* p);    // Increment in use counter
static inline eSyncStatus P_SYNCHRO_Mutex_Set_Quit_Flag(tMutVar* p);   // Initiate clear operation

// *** Internal functions definitions for P_SYNCHRO_CONDITION API ***

// Initialize condition variable
// Returns condition variable handle. If not successful, UINT32_MAX returned.
tCondVarHandle P_SYNCHRO_CONDITION_Initialize(void)
{
    eSynchroResult result = E_SYNCHRO_RESULT_OK;

    uint32_t slotId = UINT32_MAX;

    // Search for not initialized condition variable
    for (uint32_t i = 0; i < MAX_COND_VAR; i++)
    {
        {
            // Try to mark as initializing from not initialized
            eSyncStatus fromStatus = __sync_val_compare_and_swap(&gCondVarWks.tabWks[i].status, //
                                                                 E_SYNC_STATUS_NOT_INITIALIZED, //
                                                                 E_SYNC_STATUS_INITIALIZING);   //

            if (fromStatus == E_SYNC_STATUS_NOT_INITIALIZED)
            {
#if P_SYNCHRO_CONDITION_DEBUG == 1
                printk("\r\nP_SYNCHRO: slot %d reserved\r\n", //
                       i);                                    //
#endif
                slotId = i;
                break;
            }
        }
    }

    // If slot found, slotId < MAX_COND_VAR
    if (slotId >= MAX_COND_VAR)
    {
#if P_SYNCHRO_CONDITION_DEBUG == 1
        printk("\r\nP_SYNCHRO: init error, no slot found\r\n");
#endif
        result = E_SYNCHRO_RESULT_NOK;
    }

    // If slot found, get cond var object and init signal
    // then mark it as initialized
    if (E_SYNCHRO_RESULT_OK == result)
    {
        tCondVar* p = &gCondVarWks.tabWks[slotId];
        for (uint32_t i = 0; i < MAX_COND_VAR_WAITERS; i++)
        {
#if P_SYNCHRO_CONDITION_DEBUG == 1
            printk("\r\nP_SYNCHRO: slot %d - init signal %d\r\n", //
                   slotId,                                        //
                   i);                                            //
#endif
            k_sem_init(&p->tabSignals[i].signal, 0, 1);
        }

        p->status = E_SYNC_STATUS_INITIALIZED;
    }
    else
    {
#if P_SYNCHRO_CONDITION_DEBUG == 1
        printk("\r\nP_SYNCHRO: slot %d - init failure, invalid state\r\n", //
               slotId);                                                    //
#endif
    }

    return slotId;
}

// Signal all thread waiting on this condition varibale
// Returns : ok if at least one thread is waiting on. Else no_waiters. Nok if invalid state.
eSynchroResult P_SYNCHRO_CONDITION_SignalAll(tCondVarHandle slotId) // handle returned by initialize
{
    eSynchroResult result = E_SYNCHRO_RESULT_NO_WAITERS;

    if (slotId >= MAX_COND_VAR)
    {
        return E_SYNCHRO_RESULT_INVALID_PARAMETERS;
    }

    tCondVar* p = &gCondVarWks.tabWks[slotId];

    // Test de-initializing flag. if set, avoid new API call. Return NOK if set.
    if ((p->status & MASK_SET_QUIT_FLAG) != 0)
    {
#if P_SYNCHRO_CONDITION_DEBUG == 1
        printk("\r\nP_SYNCHRO: slot %d - sig all - can't be performed, quit signal set\r\n", //
               slotId);                                                                      //
#endif
        result = E_SYNCHRO_RESULT_NOK;
    }

    // If result not modified, verify if some thread are waiting for signal.
    if (E_SYNCHRO_RESULT_NO_WAITERS == result)
    {
        eSyncStatus newStatus = P_SYNCHRO_Condition_Add_Api_User(p);
        if ((newStatus & ~MASK_SET_QUIT_FLAG) > E_SYNC_STATUS_INITIALIZED)
        {
            for (uint32_t i = 0; i < MAX_COND_VAR_WAITERS; i++)
            {
                // If signal is waiting state, send signal
                if (E_SIG_STATUS_WAIT_FOR_SIGNAL == p->tabSignals[i].sigStatus)
                {
#if P_SYNCHRO_CONDITION_DEBUG == 1
                    printk("\r\nP_SYNCHRO: slot %d - sig all - sig %d\r\n", //
                           slotId,                                          //
                           i);                                              //
#endif
                    k_sem_give(&p->tabSignals[i].signal);
                    result = E_SYNCHRO_RESULT_OK;
                }
            }

            P_SYNCHRO_Condition_Remove_Api_User(p);
        }
        else
        {
            result = E_SYNCHRO_RESULT_NOK;
        }
    }

    return result;
}

// Unlock a generic object and wait for a condition variable
eSynchroResult P_SYNCHRO_CONDITION_UnlockAndWait(tCondVarHandle slotId, // Handle of condition variable object
                                                 void* pMutex,          // Generic mutex
                                                 pLockCb cbLock,        // Generic lock
                                                 pUnlockCb cbUnlock,    // Generic unlock
                                                 uint32_t timeoutMs)    // Timeout
{
    eSynchroResult result = E_SYNCHRO_RESULT_OK;

    if (slotId >= MAX_COND_VAR)
    {
        return E_SYNCHRO_RESULT_INVALID_PARAMETERS;
    }

    tCondVar* p = &gCondVarWks.tabWks[slotId];

    // Check if quit flag is set. If set, return NOK.
    if ((p->status & MASK_SET_QUIT_FLAG) != 0)
    {
#if P_SYNCHRO_CONDITION_DEBUG == 1
        printk("\r\nP_SYNCHRO: slot %d - unlock and wait - can't be performed, quit signal set\r\n", //
               slotId);                                                                              //
#endif
        result = E_SYNCHRO_RESULT_NOK;
    }

    // If result not modified, search free pre allocated signal and set it as reserving.
    if (E_SYNCHRO_RESULT_OK == result)
    {
        eSyncStatus newStatus = P_SYNCHRO_Condition_Add_Api_User(p);
        if (((newStatus & ~MASK_SET_QUIT_FLAG) > E_SYNC_STATUS_INITIALIZED))
        {
            bool bTransition = false;
            uint32_t signalId = 0;
            for (uint32_t i = 0; i < MAX_COND_VAR_WAITERS && bTransition == false; i++)
            {
                bTransition = __sync_bool_compare_and_swap(&p->tabSignals[i].sigStatus, //
                                                           E_SIG_STATUS_NOT_RESERVED,   //
                                                           E_SIG_STATUS_RESERVING);     //

                signalId = i;
            }

            // Check for transition. If failed, no free signal.
            if (bTransition)
            {
                k_sem_reset(&p->tabSignals[signalId].signal);
#if P_SYNCHRO_CONDITION_DEBUG == 1
                printk("\r\nP_SYNCHRO: slot %d - unlock and wait - wait on %d\r\n", //
                       slotId,                                                      //
                       signalId);                                                   //
#endif

                // Mar signal as ready to be signaled.
                p->tabSignals[signalId].sigStatus = E_SIG_STATUS_WAIT_FOR_SIGNAL;

                // Unlock generic mutex.
                if (cbUnlock != NULL && pMutex != NULL)
                {
                    cbUnlock(pMutex);
                }

                // Wait for signal
#if P_SYNCHRO_CONDITION_DEBUG == 1
                printk("\r\nP_SYNCHRO: slot %d - signal %d - unlock and wait - after cbUnlock \r\n", //
                       slotId,                                                                       //
                       signalId);                                                                    //
#endif
                int sem_take_res = 0; // Used to check timeout.
                sem_take_res = k_sem_take(&p->tabSignals[signalId].signal, K_MSEC(timeoutMs));

                // Mark signal as not reserved
                p->tabSignals[signalId].sigStatus = E_SIG_STATUS_NOT_RESERVED;

#if P_SYNCHRO_CONDITION_DEBUG == 1
                printk("\r\nP_SYNCHRO: slot %d - signal %d - unlock and wait - after k_sem_take\r\n", //
                       slotId,                                                                        //
                       signalId);                                                                     //
#endif

                // Re lock generic mutex
                if (cbLock != NULL && pMutex != NULL)
                {
                    cbLock(pMutex);
                }

#if P_SYNCHRO_CONDITION_DEBUG == 1
                printk("\r\nP_SYNCHRO: slot %d - signal %d - unlock and wait - after cbLock\r\n", //
                       slotId,                                                                    //
                       signalId);                                                                 //
#endif
                // Check if quit flag is setted. If set, result NOK.
                if ((p->status & MASK_SET_QUIT_FLAG) != 0)
                {
                    result = E_SYNCHRO_RESULT_NOK;
#if P_SYNCHRO_CONDITION_DEBUG == 1
                    printk("\r\nP_SYNCHRO: slot %d - unlock and wait - signal after quit on %d\r\n", //
                           slotId,                                                                   //
                           signalId);                                                                //
#endif
                }
                else
                {
                    // Check for received signal
                    if (sem_take_res != 0)
                    {
                        result = E_SYNCHRO_RESULT_TMO;
#if P_SYNCHRO_CONDITION_DEBUG == 1
                        printk("\r\nP_SYNCHRO: slot %d - unlock and wait - tmo on %d\r\n", //
                               slotId,                                                     //
                               signalId);                                                  //
#endif
                    }
#if P_SYNCHRO_CONDITION_DEBUG == 1
                    else
                    {
                        printk("\r\nP_SYNCHRO: slot %d - unlock and wait - signal on %d\r\n", //
                               slotId,                                                        //
                               signalId);                                                     //
                    }
#endif
                }
            }
            else
            {
#if P_SYNCHRO_CONDITION_DEBUG == 1
                printk("\r\nP_SYNCHRO: slot %d - unlock and wait - can't be performed, max signal waitings\r\n", //
                       slotId);                                                                                  //
#endif
                result = E_SYNCHRO_RESULT_NOK;
            }

            P_SYNCHRO_Condition_Remove_Api_User(p);
        }
        else
        {
            result = E_SYNCHRO_RESULT_NOK;
        }
    }

    return result;
}

// Clear condition variable. If some thread are waiting for, these thread are notified.
// Warning, thread waiting for a signal try to relock mutex. If this mutex is never cleared or unlocked
// this function never returns !!!
eSynchroResult P_SYNCHRO_CONDITION_Clear(tCondVarHandle slotId)
{
    eSynchroResult result = E_SYNCHRO_RESULT_OK;

    if (slotId >= MAX_COND_VAR)
    {
        return E_SYNCHRO_RESULT_INVALID_PARAMETERS;
    }

    tCondVar* p = &gCondVarWks.tabWks[slotId];

    do
    {
#if P_SYNCHRO_CONDITION_DEBUG == 1
        printk("\r\nP_SYNCHRO: slot %d - clear - set flag\r\n", //
               slotId);                                         //
#endif

        // Set quit flag
        P_SYNCHRO_Condition_Set_Quit_Flag(p);

        // Signal all thread waiting for this condition variable
        for (uint32_t i = 0; i < MAX_COND_VAR_WAITERS; i++)
        {
            if (E_SIG_STATUS_WAIT_FOR_SIGNAL == p->tabSignals[i].sigStatus)
            {
#if P_SYNCHRO_CONDITION_DEBUG == 1
                printk("\r\nP_SYNCHRO: slot %d - clear - set signal %d\r\n", //
                       slotId,                                               //
                       i);                                                   //
#endif
                k_sem_give(&p->tabSignals[i].signal);
            }
        }

        // Try to go to de initializing state
        eSyncStatus fromStatus = __sync_val_compare_and_swap(&p->status,                                     //
                                                             E_SYNC_STATUS_INITIALIZED | MASK_SET_QUIT_FLAG, //
                                                             E_SYNC_STATUS_DEINITIALIZING);                  //

        // If deinitializing state reach, reset all signals, set result OK to stop retry loop
        // then mark new status as not initialized.
        if (E_SYNC_STATUS_INITIALIZED == (fromStatus & ~MASK_SET_QUIT_FLAG))
        {
            for (uint32_t i = 0; i < MAX_COND_VAR_WAITERS; i++)
            {
                k_sem_reset(&p->tabSignals[i].signal);
            }

            result = E_SYNCHRO_RESULT_OK;
#if P_SYNCHRO_CONDITION_DEBUG == 1
            printk("\r\nP_SYNCHRO: slot %d - clear - successful\r\n", //
                   slotId);                                           //
#endif

            p->status = E_SYNC_STATUS_NOT_INITIALIZED;
        }
        // Others status indicates that operation is on going or API is in use.
        // Set result to invalid state, then yield.
        else if (E_SYNC_STATUS_DEINITIALIZING == (fromStatus & ~MASK_SET_QUIT_FLAG) ||
                 E_SYNC_STATUS_INITIALIZED < (fromStatus & ~MASK_SET_QUIT_FLAG) ||
                 E_SYNC_STATUS_INITIALIZING == (fromStatus & ~MASK_SET_QUIT_FLAG))
        {
#if P_SYNCHRO_CONDITION_DEBUG == 1
            printk("\r\nP_SYNCHRO: slot %d - clear - some waiting on engaged, yield and retry...\r\n", //
                   slotId);                                                                            //
#endif
            result = E_SYNCHRO_RESULT_INVALID_STATE;
            k_yield();
        }
        // If state is not initialized, return NOK.
        else
        {
#if P_SYNCHRO_CONDITION_DEBUG == 1
            printk("\r\nP_SYNCHRO: slot %d - clear - error, already cleared...\r\n", //
                   slotId);                                                          //
#endif
            result = E_SYNCHRO_RESULT_NOK;
        }
    } while (E_SYNCHRO_RESULT_INVALID_STATE == result);

    return result;
}

// *** Internal functions definitions for P_SYNCHRO_MUTEX API ***

static void P_SYNCHRO_MUTEX_CondVar_UnlockAndWait_Lock_Callback(void* pMutex)
{
    struct k_mutex* pk_mut = (struct k_mutex*) pMutex;
    k_mutex_lock(pk_mut, K_FOREVER);
}

static void P_SYNCHRO_MUTEX_CondVar_UnlockAndWait_UnLock_Callback(void* pMutex)
{
    struct k_mutex* pk_mut = (struct k_mutex*) pMutex;
    k_mutex_unlock(pk_mut);
}

// Initialize a mutex object
// Returns mutex handle to use with lock, unlock and clear.
// In case of failure, return UINT32_MAX invalid value.
tMutVarHandle P_SYNCHRO_MUTEX_Initialize(void)
{
    uint32_t slotId = UINT32_MAX;

    // Search for
    for (uint32_t i = 0; i < MAX_MUT_VAR; i++)
    {
        {
            eSyncStatus fromStatus = __sync_val_compare_and_swap(&gMutVarWks.tabWks[i].status,  //
                                                                 E_SYNC_STATUS_NOT_INITIALIZED, //
                                                                 E_SYNC_STATUS_INITIALIZING);   //
            if (fromStatus == E_SYNC_STATUS_NOT_INITIALIZED)
            {
#if P_SYNCHRO_MUTEX_DEBUG == 1
                printk("\r\nP_MUTEX: Slot id %d - Initialized\r\n", i);
#endif
                gMutVarWks.tabWks[i].condVarHdl = P_SYNCHRO_CONDITION_Initialize();
                gMutVarWks.tabWks[i].lockCounter = 0;
                gMutVarWks.tabWks[i].ownerThread = NULL;
                k_mutex_init(&gMutVarWks.tabWks[i].lock);
                slotId = i;
                gMutVarWks.tabWks[i].status = E_SYNC_STATUS_INITIALIZED;
                break;
            }
        }
    }

    return slotId;
}

// Clear a mutex object. All future calls to API will return NOK.
// If an API LOCK is in use, it will be immediatly unlocked.
// API LOCK returns in that case NOK.
eSynchroResult P_SYNCHRO_MUTEX_Clear(tMutVarHandle slotId)
{
    eSynchroResult result = E_SYNCHRO_RESULT_OK;
    if (slotId >= MAX_MUT_VAR)
    {
        return E_SYNCHRO_RESULT_NOK;
    }

    do
    {
        {
#if P_SYNCHRO_MUTEX_DEBUG == 1
            printk("\r\nP_MUTEX: Slot id %d - clear set flag\r\n", //
                   slotId);                                        //
#endif
            P_SYNCHRO_Mutex_Set_Quit_Flag(&gMutVarWks.tabWks[slotId]);

#if P_SYNCHRO_MUTEX_DEBUG == 1
            printk("\r\nP_MUTEX: Slot id %d - clear sig all to unblock waiting thread\r\n", //
                   slotId);                                                                 //
#endif

            P_SYNCHRO_CONDITION_SignalAll(gMutVarWks.tabWks[slotId].condVarHdl);

            eSyncStatus fromStatus = __sync_val_compare_and_swap(&gMutVarWks.tabWks[slotId].status,              //
                                                                 E_SYNC_STATUS_INITIALIZED | MASK_SET_QUIT_FLAG, //
                                                                 E_SYNC_STATUS_DEINITIALIZING);                  //

            if ((fromStatus & ~MASK_SET_QUIT_FLAG) == E_SYNC_STATUS_INITIALIZED)
            {
#if P_SYNCHRO_MUTEX_DEBUG == 1
                printk("\r\nP_MUTEX: Slot id %d - clear performed successfully !!!\r\n", //
                       slotId);                                                          //
#endif
                P_SYNCHRO_CONDITION_Clear(gMutVarWks.tabWks[slotId].condVarHdl);
                gMutVarWks.tabWks[slotId].condVarHdl = UINT32_MAX;
                gMutVarWks.tabWks[slotId].lockCounter = 0;
                gMutVarWks.tabWks[slotId].ownerThread = NULL;
                gMutVarWks.tabWks[slotId].status = E_SYNC_STATUS_NOT_INITIALIZED;
                result = E_SYNCHRO_RESULT_OK;
            }
            else if (((fromStatus & ~MASK_SET_QUIT_FLAG) == E_SYNC_STATUS_DEINITIALIZING) ||
                     ((fromStatus & ~MASK_SET_QUIT_FLAG) == E_SYNC_STATUS_INITIALIZING) ||
                     ((fromStatus & ~MASK_SET_QUIT_FLAG) > E_SYNC_STATUS_INITIALIZED))
            {
#if P_SYNCHRO_MUTEX_DEBUG == 1
                printk("\r\nP_MUTEX: Slot id %d - clear can't be performed, yield and retry...\r\n", //
                       slotId);                                                                      //
#endif
                result = E_SYNCHRO_RESULT_INVALID_STATE;
                k_yield();
            }
            else
            {
#if P_SYNCHRO_MUTEX_DEBUG == 1
                printk("\r\nP_MUTEX: Slot id %d - clear not performed, already cleared\r\n", //
                       slotId);                                                              //
#endif
                result = E_SYNCHRO_RESULT_NOK;
            }
        }
    } while (E_SYNCHRO_RESULT_INVALID_STATE == result);

    return result;
}

eSynchroResult P_SYNCHRO_MUTEX_Lock(tMutVarHandle slotId)
{
    eSynchroResult result = E_SYNCHRO_RESULT_OK;
    if (slotId >= MAX_MUT_VAR)
    {
#if P_SYNCHRO_MUTEX_DEBUG == 1
        printk("\r\nP_MUTEX: Slot id %d - Lock - Failure, invalid parameters\r\n", //
               slotId);                                                            //
#endif
        return E_SYNCHRO_RESULT_NOK;
    }

    if ((gMutVarWks.tabWks[slotId].status & MASK_SET_QUIT_FLAG) != 0)
    {
#if P_SYNCHRO_MUTEX_DEBUG == 1
        printk("\r\nP_MUTEX: Slot id %d - Lock - Failure, quit request set\r\n", //
               slotId);                                                          //
#endif
        return E_SYNCHRO_RESULT_NOK;
    }

    eSyncStatus currentStatus = P_SYNCHRO_Mutex_Add_Api_User(&gMutVarWks.tabWks[slotId]);

    if ((currentStatus & ~MASK_SET_QUIT_FLAG) > E_SYNC_STATUS_INITIALIZED)
    {
        k_mutex_lock(&gMutVarWks.tabWks[slotId].lock, K_FOREVER);
        {
            if (gMutVarWks.tabWks[slotId].lockCounter == 0)
            {
#if P_SYNCHRO_MUTEX_DEBUG == 1
                printk("\r\nP_MUTEX: Slot id %d - Lock - First lock\r\n", //
                       slotId);                                           //
#endif
                gMutVarWks.tabWks[slotId].ownerThread = k_current_get();
            }

            if (gMutVarWks.tabWks[slotId].ownerThread != k_current_get())
            {
                do
                {
#if P_SYNCHRO_MUTEX_DEBUG == 1
                    printk("\r\nP_MUTEX: Slot id %d - Try Lock by thread %08lX owned by %08lX\r\n", //
                           slotId,                                                                  //
                           (unsigned long int) k_current_get(),                                     //
                           (unsigned long int) gMutVarWks.tabWks[slotId].ownerThread);              //
#endif

                    result = P_SYNCHRO_CONDITION_UnlockAndWait(gMutVarWks.tabWks[slotId].condVarHdl,                  //
                                                               (void*) &gMutVarWks.tabWks[slotId].lock,               //
                                                               P_SYNCHRO_MUTEX_CondVar_UnlockAndWait_Lock_Callback,   //
                                                               P_SYNCHRO_MUTEX_CondVar_UnlockAndWait_UnLock_Callback, //
                                                               UINT32_MAX);                                           //

                    if ((gMutVarWks.tabWks[slotId].status & MASK_SET_QUIT_FLAG) != 0)
                    {
#if P_SYNCHRO_MUTEX_DEBUG == 1
                        printk("\r\nP_MUTEX: Slot id %d - Try lock - ignore, quit request set\r\n", //
                               slotId);                                                             //
#endif
                        result = E_SYNCHRO_RESULT_NOK;
                    }
                    else
                    {
                        if (E_SYNCHRO_RESULT_OK == result && gMutVarWks.tabWks[slotId].lockCounter == 0)
                        {
#if P_SYNCHRO_MUTEX_DEBUG == 1
                            printk("\r\nP_MUTEX: Slot id %d - Try Lock successful by %08lX \r\n", //
                                   slotId,                                                        //
                                   (unsigned long int) k_current_get());                          //
#endif
                            gMutVarWks.tabWks[slotId].ownerThread = k_current_get();
                            gMutVarWks.tabWks[slotId].lockCounter = 1;
                        }
                        else
                        {
                            if (E_SYNCHRO_RESULT_OK == result && gMutVarWks.tabWks[slotId].lockCounter > 0)
                            {
                                result = E_SYNCHRO_RESULT_INVALID_STATE;
                            }
                            else
                            {
#if P_SYNCHRO_MUTEX_DEBUG == 1
                                printk("\r\nP_MUTEX: Slot id %d - Try Lock failed by %08lX \r\n", //
                                       slotId,                                                    //
                                       (unsigned long int) k_current_get());                      //
#endif
                                result = E_SYNCHRO_RESULT_NOK;
                            }
                        }
                    }

                } while (E_SYNCHRO_RESULT_INVALID_STATE == result);
            }
            else
            {
#if P_SYNCHRO_MUTEX_DEBUG == 1
                printk("\r\nP_MUTEX: Slot id %d - Lock - Recursive lock\r\n", //
                       slotId);                                               //
#endif
                gMutVarWks.tabWks[slotId].lockCounter++;
            }
        }
        k_mutex_unlock(&gMutVarWks.tabWks[slotId].lock);

        P_SYNCHRO_Mutex_Remove_Api_User(&gMutVarWks.tabWks[slotId]);
    }
    else
    {
#if P_SYNCHRO_MUTEX_DEBUG == 1
        printk("\r\nP_MUTEX: Slot id %d - Lock - Failure, invalid state\r\n", //
               slotId);                                                       //
#endif
        result = E_SYNCHRO_RESULT_NOK;
    }
    return result;
}

eSynchroResult P_SYNCHRO_MUTEX_Unlock(tMutVarHandle slotId)
{
    if (slotId >= MAX_MUT_VAR)
    {
#if P_SYNCHRO_MUTEX_DEBUG == 1
        printk("\r\nP_MUTEX: Slot id %d - UnLock - Failure, invalid parameters\r\n", //
               slotId);                                                              //
#endif
        return E_SYNCHRO_RESULT_NOK;
    }

    if ((gMutVarWks.tabWks[slotId].status & MASK_SET_QUIT_FLAG) != 0)
    {
#if P_SYNCHRO_MUTEX_DEBUG == 1
        printk("\r\nP_MUTEX: Slot id %d - UnLock - Failure, quit request set\r\n", slotId);
#endif
        return E_SYNCHRO_RESULT_NOK;
    }

    eSynchroResult result = E_SYNCHRO_RESULT_OK;

    eSyncStatus currentStatus = P_SYNCHRO_Mutex_Add_Api_User(&gMutVarWks.tabWks[slotId]);

    if ((currentStatus & ~MASK_SET_QUIT_FLAG) > E_SYNC_STATUS_INITIALIZED)
    {
        k_mutex_lock(&gMutVarWks.tabWks[slotId].lock, K_FOREVER);
        {
            if (gMutVarWks.tabWks[slotId].ownerThread == k_current_get())
            {
#if P_SYNCHRO_MUTEX_DEBUG == 1
                printk("\r\nP_MUTEX: Slot id %d - UnLock - valid thread %08lX\r\n", //
                       slotId,                                                      //
                       (unsigned long int) gMutVarWks.tabWks[slotId].ownerThread);  //
#endif
                if (gMutVarWks.tabWks[slotId].lockCounter > 0)
                {
                    gMutVarWks.tabWks[slotId].lockCounter--;
                    if (gMutVarWks.tabWks[slotId].lockCounter == 0)
                    {
#if P_SYNCHRO_MUTEX_DEBUG == 1
                        printk("\r\nP_MUTEX: Slot id %d - UnLock - no owner, signal all from %08lX\r\n", //
                               slotId,                                                                   //
                               (unsigned long int) k_current_get());                                     //
#endif
                        gMutVarWks.tabWks[slotId].ownerThread = NULL;
                        result = P_SYNCHRO_CONDITION_SignalAll(gMutVarWks.tabWks[slotId].condVarHdl);
                    }
                    else
                    {
                        result = E_SYNCHRO_RESULT_NO_WAITERS;
                    }
                }
            }
            else
            {
#if P_SYNCHRO_MUTEX_DEBUG == 1
                printk("\r\nP_MUTEX: Slot id %d - UnLock - Failure, invalid thread\r\n", //
                       slotId);                                                          //
#endif
                result = E_SYNCHRO_RESULT_NOK;
            }
        }
        k_mutex_unlock(&gMutVarWks.tabWks[slotId].lock);
        P_SYNCHRO_Mutex_Remove_Api_User(&gMutVarWks.tabWks[slotId]);

        // This result indicates successful with waiters on this mutex. So, yield !!!
        if (E_SYNCHRO_RESULT_OK == result)

        {
#if P_SYNCHRO_MUTEX_DEBUG == 1
            printk("\r\nP_MUTEX: Slot id %d - UnLock - yield from %08lX\r\n", //
                   slotId,                                                    //
                   (unsigned long int) k_current_get());                      //
#endif
            k_yield();
        }
    }
    else
    {
#if P_SYNCHRO_MUTEX_DEBUG == 1
        printk("\r\nP_MUTEX: Slot id %d - UnLock - Failure, invalid status\r\n", //
               slotId);                                                          //
#endif
        result = E_SYNCHRO_RESULT_NOK;
    }

    return result;
}

// Set quit flag and returns new status with quit flag
static inline eSyncStatus P_SYNCHRO_Condition_Set_Quit_Flag(tCondVar* p)
{
    eSyncStatus currentStatus = p->status;
    eSyncStatus newStatus = 0;
    bool bTransition = false;
    do
    {
        newStatus = currentStatus | 0x80000000;

        bTransition = __sync_bool_compare_and_swap(&p->status, currentStatus, newStatus);

    } while (!bTransition);
    return newStatus;
}

// Returns new status with quit flag
static inline eSyncStatus P_SYNCHRO_Condition_Add_Api_User(tCondVar* p)
{
    eSyncStatus currentStatus = p->status;
    eSyncStatus newStatus = 0;
    bool bTransition = false;
    do
    {
        if ((currentStatus & ~MASK_SET_QUIT_FLAG) >= E_SYNC_STATUS_INITIALIZED)
        {
            newStatus = currentStatus + 1;
        }
        else
        {
            newStatus = currentStatus;
        }
        bTransition = __sync_bool_compare_and_swap(&p->status, currentStatus, newStatus);

    } while (!bTransition);
    return newStatus;
}

// Returns new status with quit flag
static inline eSyncStatus P_SYNCHRO_Condition_Remove_Api_User(tCondVar* p)
{
    eSyncStatus currentStatus = p->status;
    eSyncStatus newStatus = 0;
    bool bTransition = false;
    do
    {
        if ((currentStatus & ~MASK_SET_QUIT_FLAG) > E_SYNC_STATUS_INITIALIZED)
        {
            newStatus = currentStatus - 1;
        }
        else
        {
            newStatus = currentStatus;
        }
        bTransition = __sync_bool_compare_and_swap(&p->status, currentStatus, newStatus);

    } while (!bTransition);

    return newStatus;
}

// Set quit flag and returns new status with quit flag
static inline eSyncStatus P_SYNCHRO_Mutex_Set_Quit_Flag(tMutVar* p)
{
    eSyncStatus currentStatus = p->status;
    eSyncStatus newStatus = 0;
    bool bTransition = false;
    do
    {
        newStatus = currentStatus | 0x80000000;

        bTransition = __sync_bool_compare_and_swap(&p->status, currentStatus, newStatus);

    } while (!bTransition);
    return newStatus;
}

// Returns new status with quit flag
static inline eSyncStatus P_SYNCHRO_Mutex_Add_Api_User(tMutVar* p)
{
    eSyncStatus currentStatus = p->status;
    eSyncStatus newStatus = 0;
    bool bTransition = false;
    do
    {
        if ((currentStatus & ~MASK_SET_QUIT_FLAG) >= E_SYNC_STATUS_INITIALIZED)
        {
            newStatus = currentStatus + 1;
        }
        else
        {
            newStatus = currentStatus;
        }
        bTransition = __sync_bool_compare_and_swap(&p->status, currentStatus, newStatus);

    } while (!bTransition);
    return newStatus;
}

// Returns new status with quit flag
static inline eSyncStatus P_SYNCHRO_Mutex_Remove_Api_User(tMutVar* p)
{
    eSyncStatus currentStatus = p->status;
    eSyncStatus newStatus = 0;
    bool bTransition = false;
    do
    {
        if ((currentStatus & ~MASK_SET_QUIT_FLAG) > E_SYNC_STATUS_INITIALIZED)
        {
            newStatus = currentStatus - 1;
        }
        else
        {
            newStatus = currentStatus;
        }
        bTransition = __sync_bool_compare_and_swap(&p->status, currentStatus, newStatus);

    } while (!bTransition);

    return newStatus;
}
