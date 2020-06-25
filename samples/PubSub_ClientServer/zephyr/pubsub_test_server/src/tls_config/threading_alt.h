#ifndef __THREADING_ALT_H__
#define __THREADING_ALT_H__

#ifndef __INT32_MAX__
#include <toolchain/xcc_missing_defs.h>
#endif

#include "sopc_mutexes.h"
#include "sopc_threads.h"

typedef Mutex mbedtls_threading_mutex_t;

void tls_threading_initialize(void);

#endif /* __THREADING_ALT_H__ */