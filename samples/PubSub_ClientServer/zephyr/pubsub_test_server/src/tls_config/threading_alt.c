#include "threading_alt.h"

#include <kernel.h>
#include <mbedtls/threading.h>

static void mutex_init(mbedtls_threading_mutex_t* pMutex) {
  int res_lock = 0;
  res_lock = Mutex_Initialization(pMutex);
  //printk("\r\nres init = %d - %d\r\n", res_lock, (uint32_t)*pMutex);
}

static void mutex_free(mbedtls_threading_mutex_t* pMutex) {
  int res_lock = 0;
  res_lock = Mutex_Clear(pMutex);
  //printk("\r\nres clear = %d\r\n", res_lock);
}

static int mutex_lock(mbedtls_threading_mutex_t* pMutex) {
  int res_lock = 0;
  res_lock = Mutex_Lock(pMutex);
  //printk("\r\nres lock = %d - %d\r\n", res_lock, (uint32_t)*pMutex);
  return (int)res_lock;
}

static int mutex_unlock(mbedtls_threading_mutex_t* pMutex) {
  int res_lock = 0;
  res_lock = Mutex_Unlock(pMutex);
  //printk("\r\nres unlock = %d - %d\r\n", res_lock, (uint32_t)*pMutex);
  return (int)res_lock;
}

void tls_threading_initialize(void) {
  printk("\r\n Thread safe mbedtls init\r\n");
  mbedtls_threading_set_alt(mutex_init, mutex_free, mutex_lock, mutex_unlock);
}
