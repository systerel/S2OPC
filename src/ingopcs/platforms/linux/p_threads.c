/*
 * p_threads.c
 *
 *  Created on: Nov 24, 2016
 *      Author: vincent
 */


#include "sopc_mutexes.h"

SOPC_StatusCode Mutex_Inititalization(Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if(mut != NULL){
        retCode = pthread_mutex_init(mut, NULL);
        if(retCode == 0){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode Mutex_Clear(Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if(mut != NULL){
        retCode = pthread_mutex_destroy(mut);
        if(retCode == 0){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode Mutex_Lock(Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if(mut != NULL){
        retCode = pthread_mutex_lock(mut);
        if(retCode == 0){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode Mutex_Unlock(Mutex* mut){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int retCode = 0;
    if(mut != NULL){
        retCode = pthread_mutex_unlock(mut);
        if(retCode == 0){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}
