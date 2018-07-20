/**************************************************************************/
/*                                                                        */
/*  This file is part of deliverable T3.3 of project INGOPCS              */
/*                                                                        */
/*    Copyright (C) 2017 TrustInSoft                                      */
/*                                                                        */
/*  All rights reserved.                                                  */
/*                                                                        */
/**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>

#include <tis_builtin.h>

int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res) {
  static struct addrinfo tis_addrinfo;
  static char addr[] = "127.0.0.1";
  static struct sockaddr_storage ai_addr = {
    sizeof (addr), AF_UNSPEC , { 0 }, "127.0.0.1" };
  tis_addrinfo.ai_flags = hints->ai_flags;
  if (hints->ai_family == AF_UNSPEC)
    tis_addrinfo.ai_family = PF_INET;
  else
    tis_addrinfo.ai_family = hints->ai_family;
  tis_addrinfo.ai_socktype = hints->ai_socktype;
  tis_addrinfo.ai_protocol = hints->ai_protocol;
  tis_addrinfo.ai_addrlen = sizeof (ai_addr);
  tis_addrinfo.ai_addr = (struct sockaddr *) &ai_addr;
  tis_addrinfo.ai_canonname = NULL;
  tis_addrinfo.ai_next = NULL;
  *res = &tis_addrinfo;
  return 0;
}

void freeaddrinfo(struct addrinfo *res) {
}

/*@ requires gso_r_level: level == SOL_SOCKET;
    requires gso_r_optname: optname == SO_ERROR;
*/
int getsockopt(int sockfd, int level, int optname,
                      void *optval, socklen_t *optlen) {
  return 0;
}

int setsockopt(int sockfd, int level, int optname,
                      const void *optval, socklen_t optlen) {
  switch (optname) {
    case TCP_NODELAY:
      //@ assert sso_tcp: level == IPPROTO_TCP;
      // TODO ?
      break;
    case SO_RCVBUF:
      //@ assert sso_so_rcvbuf: level == SOL_SOCKET;
      // TODO ?
      break;
    case SO_SNDBUF:
      //@ assert sso_so_sndbuf: level == SOL_SOCKET;
      // TODO ?
      break;
    case SO_REUSEADDR:
      //@ assert sso_so_reuseaddr: level == SOL_SOCKET;
      // TODO ?
      break;
    default:
      //@ assert sso_opt: \false;
      break;
  }
  return 0;
}

#ifdef TEST_SOCKETS
static int socket_fd = 4;
#define WITH_SOCKETS
#endif
#ifdef TEST_TK_CLIENT
static int socket_fd = 3;
#define WITH_SOCKETS
#endif
#ifdef TEST_SCH_CLIENT
static int socket_fd = 3;
#define WITH_SOCKETS
#endif

#ifdef WITH_SOCKETS
int socket(int domain, int type, int protocol) {
  return socket_fd++;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
  return socket_fd++;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  return 0;
}

int listen(int sockfd, int backlog) {
  return 0;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  return 0;
}

//@ requires fcntl_r_cmd: cmd == 5; // F_SETFL;
int fcntl(int fd, int cmd , ...) {
  return 0;
}

#include <sys/time.h>
#include <unistd.h>

void FD_ZERO (fd_set *set) {
  set->__fc_fd_set = 0;
}

void FD_SET(int fd, fd_set *set) {
  //@ assert fd_set_max: 0 <= fd < 8;
  set->__fc_fd_set |= (1 << fd);
}

int  FD_ISSET(int fd, fd_set *set) {
  //@ assert fd_isset_max: 0 <= fd < 8;
  return !!(set->__fc_fd_set & (1 << fd));
}

void FD_CLR (int fd, fd_set *set) {
  //@ assert fd_clr_max: 0 <= fd < 8;
  set->__fc_fd_set &= ~(1 << fd);
}

int select(int nfds, fd_set *readfds, fd_set *writefds,
    fd_set *exceptfds, struct timeval *timeout) {
  int nbReady;
// #ifdef __TRUSTINSOFT_ANALYZER__
//   nbReady = 0;
//   for (int fd = 0; fd < nfds; fd++) {
//     if (FD_ISSET (fd, readfds)) {
//       if (tis_nondet (0, 1)) nbReady ++; else FD_CLR (fd, readfds);
//     }
//     if (FD_ISSET (fd, writefds)) {
//       if (tis_nondet (0, 1)) nbReady ++; else FD_CLR (fd, writefds);
//     }
//     if (FD_ISSET (fd, exceptfds)) {
//       if (tis_nondet (0, 1)) nbReady ++; else FD_CLR (fd, exceptfds);
//     }
//   }
// #else
  static int nb_call = 1;
  printf ("TIS: Stub: select(%d)", nb_call);
  FD_ZERO (readfds); FD_ZERO (writefds); FD_ZERO (exceptfds);
  switch (nb_call++) {
#ifdef TEST_SOCKETS
    case 1: nbReady=1; FD_SET (4, readfds); break;
    case 2: nbReady=1; FD_SET (5, writefds); break;
    case 3: nbReady=1; FD_SET (6, readfds); break;
    case 4: nbReady=1; FD_SET (5, readfds); break;
    case 5: nbReady=0; break;
    case 6: nbReady=1; FD_SET (6, readfds); break;
#endif
#ifdef TEST_TK_CLIENT
    case 1: nbReady=1; FD_SET (3, writefds); break;
    case 2: nbReady=1; FD_SET (3, readfds); break;
    case 3: nbReady=1; FD_SET (3, readfds); break;
    case 4: nbReady=1; FD_SET (3, readfds); break;
    case 5: nbReady=1; FD_SET (3, readfds); break;
    case 6: nbReady=1; FD_SET (3, readfds); break;
#endif
#ifdef TEST_SCH_CLIENT
    case 1: nbReady=1; FD_SET (3, writefds); break;
    case 2: nbReady=1; FD_SET (3, readfds); break;
    case 3: nbReady=1; FD_SET (3, readfds); break;
    case 4: nbReady=1; FD_SET (3, readfds); break;
    case 5: nbReady=0; break;
    case 6: nbReady=0; break;
            /*
    case 5: nbReady=1; FD_SET (3, readfds); break;
    case 6: nbReady=0; break;
    case 7: nbReady=1; FD_SET (3, writefds); break;
    case 8: nbReady=1; FD_SET (3, readfds); break;
    */
#endif
    default: ; //@ assert missing_select_spec: \false;
  }
// #endif
  return nbReady;
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
  return len;
}
#endif

int close(int fd) {
  return 0;
}

#ifdef __TRUSTINSOFT_ANALYZER__
#else // TIS_INTERPRETER
void flockfile(FILE *filehandle) {}
void funlockfile(FILE *filehandle) {}
int fflush(FILE *stream) { return 0; }
#endif

#ifdef TEST_SOCKETS
ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
  static const size_t msglen = 1000;
  static size_t nb = 1;

  if (nb++ > 2)
    return 0;

  //@ assert recv_size: msglen <= len;
#ifdef __TRUSTINSOFT_ANALYZER__
  tis_make_unknown (buf, msglen);
#else // TIS_INTERPRETER
  char * p = buf;
  for(size_t idx = 0; idx < msglen; idx++){
    p[idx] = (idx % 256);
  }
#endif
  return msglen;
}
#endif

#include <pthread.h>
pthread_mutex_t tis_dummy_mutex;
int pthread_mutex_init(pthread_mutex_t *restrict mutex,
                       const pthread_mutexattr_t *restrict attr) {
  *mutex = tis_dummy_mutex++;
  return 0;
}
int pthread_mutex_lock (pthread_mutex_t *mutex) {
  return 0;
}
int pthread_mutex_unlock (pthread_mutex_t *mutex) {
  return 0;
}

//@ assigns \result \from *cond;
int pthread_cond_broadcast(pthread_cond_t *cond);

#ifdef __TRUSTINSOFT_NO_MTHREAD__
int pthread_mutex_destroy(pthread_mutex_t *mutex) {
  return 0;
}

pthread_cond_t tis_dummy_cond;
int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *cond_attr) {
  *cond = tis_dummy_cond++;
  return 0;
}
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg) {
//   start_routine (arg);
  return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
  return 0;
}

//@ requires retval == NULL;
int pthread_join(pthread_t thread, void **retval) {
  return 0;
}

#else

int tis_cond_lock = 0;

//@ requires cond_init_r_lock: tis_cond_lock == 0;
int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *cond_attr) {
  return 0;
}
//@ requires cond_wait_r_lock: tis_cond_lock == 1;
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
  pthread_mutex_unlock (mutex);
  while (tis_cond_lock != 0) ;
  pthread_mutex_lock (mutex);
  return 0;
}
int pthread_cond_broadcast(pthread_cond_t *cond) {
  tis_cond_lock = 0;
  return 0;
}
//@ requires cond_destroy_r_lock: tis_cond_lock == 0;
int pthread_cond_destroy(pthread_cond_t *cond) {
  return 0;
}
int pthread_mutex_destroy(pthread_mutex_t *mutex) {
  return 0;
}

#endif // __TRUSTINSOFT_NO_MTHREAD__


#include <unistd.h>
int usleep(useconds_t usec) {
  // TODO: may return -1 for __TRUSTINSOFT_ANALYZER__
  return 0;
}

double pow(double x, double y) {
  if (x == 2 && y == 1)
    return 2;
  //@ assert pow_niy: \false;
  return 0;
}

// default function when not forcing values
int tis_force_value (const char * f, const char * id, size_t n, int old) {
  return old;
}

//@ ensures mcpb_e_init: \initialized ((char*)dst + (0..n-1));
void * tis_memcpy_bounded(void * dst, const void * src, size_t n,
                          void * dst_bound, const void * src_bound);
