/*
 *  Copyright (C) 2018 Systerel and others.
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

/**
 *  \file
 *
 *  \brief A platform independent API to use threads
 */

#ifndef SOPC_THREADS_H_
#define SOPC_THREADS_H_

#include "sopc_toolkit_constants.h"

// Import Mutex type from platform dependent code
#include "p_threads.h"

SOPC_ReturnStatus SOPC_Thread_Create(Thread* thread, void* (*startFct)(void*), void* startArgs);
SOPC_ReturnStatus SOPC_Thread_Join(Thread thread);

#endif /* SOPC_THREADS_H_ */
