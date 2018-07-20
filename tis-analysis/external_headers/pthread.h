/**************************************************************************/
/*                                                                        */
/*  This file is part of deliverable T3.3 of project INGOPCS              */
/*                                                                        */
/*    Copyright (C) 2017 TrustInSoft                                      */
/*                                                                        */
/*  All rights reserved.                                                  */
/*                                                                        */
/**************************************************************************/
#ifndef _TIS_PTHREAD_H
#define _TIS_PTHREAD_H


#include <tis-kernel/libc/pthread.h>

//@ assigns *mutex \from *mutex;
int pthread_mutex_destroy(pthread_mutex_t *mutex);

#endif
