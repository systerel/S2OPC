/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SOPC_MUTEXES_H_
#define SOPC_MUTEXES_H_

#include "sopc_toolkit_constants.h"

// Import Mutex type from platform dependent code
#include "p_threads.h"

SOPC_ReturnStatus Condition_Init(Condition* cond);
SOPC_ReturnStatus Condition_Clear(Condition* cond);

// Must be called between lock and unlock of Mutex ued to wait on condition
SOPC_ReturnStatus Condition_SignalAll(Condition* cond);

SOPC_ReturnStatus Mutex_Initialization(Mutex* mut);
SOPC_ReturnStatus Mutex_Clear(Mutex* mut);
SOPC_ReturnStatus Mutex_Lock(Mutex* mut);
SOPC_ReturnStatus Mutex_Unlock(Mutex* mut);

// Lock on return
SOPC_ReturnStatus Mutex_UnlockAndWaitCond(Condition* cond, Mutex* mut);
// Lock on return. Return SOPC_STATUS_TIMEOUT in case of timeout before condition signaled
SOPC_ReturnStatus Mutex_UnlockAndTimedWaitCond(Condition* cond, Mutex* mut, uint32_t milliSecs);

#endif /* SOPC_MUTEXES_H_ */
