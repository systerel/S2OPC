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

#ifndef SOPC_P_THREADS_H_
#define SOPC_P_THREADS_H_

#ifdef __MINGW32__
#include <winsock2.h>
#endif

#include <windows.h>

typedef SRWLOCK Mutex;

typedef CONDITION_VARIABLE Condition;

typedef void*(SOPCThreadStartFct)(void*);

typedef struct Thread
{
    HANDLE thread;
    SOPCThreadStartFct* startFct;
    void* args;
} Thread;

#endif /* SOPC_P_THREADS_H_ */
