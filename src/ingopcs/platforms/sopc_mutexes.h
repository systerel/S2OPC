/*
 * sopc_threads.h
 *
 *  Created on: Nov 24, 2016
 *      Author: vincent
 */

#ifndef SOPC_MUTEXES_H_
#define SOPC_MUTEXES_H_

#include "sopc_base_types.h"

// Import Mutex type from platform dependent code
#include "p_threads.h"

SOPC_StatusCode Mutex_Inititalization(Mutex* mut);
SOPC_StatusCode Mutex_Clear(Mutex* mut);
SOPC_StatusCode Mutex_Lock(Mutex* mut);
SOPC_StatusCode Mutex_Unlock(Mutex* mut);

#endif /* SOPC_MUTEXES_H_ */
