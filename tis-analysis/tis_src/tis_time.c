/**************************************************************************/
/*                                                                        */
/*  This file is part of deliverable T3.3 of project INGOPCS              */
/*                                                                        */
/*    Copyright (C) 2017 TrustInSoft                                      */
/*                                                                        */
/*  All rights reserved.                                                  */
/*                                                                        */
/**************************************************************************/

#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <tis_builtin.h>

/*@ ensures gmt_e_limits: \result == \null
         || \result == &__fc_time_tm
             && __fc_time_tm.tm_year <= INT_MAX - 1900
             && 0 <= __fc_time_tm.tm_mon <= 11
             ; */
struct tm *gmtime(const time_t *timep) {
  if (tis_nondet (1,0))
    return NULL;
  else
    return &__fc_time_tm;
}

// date +%s to have the number of second since ...
// date --date="@1497337833" to see the corresponding date
static time_t tis_cur_time = 1497337833L; // mardi 13 juin 2017, 09:10:33
time_t tis_time(time_t *t) {
#ifdef __TRUSTINSOFT_ANALYZER__
  tis_make_unknown (&tis_cur_time, sizeof (tis_cur_time));
#else
  tis_cur_time++;
  if (t)
    *t = tis_cur_time;
#endif
  return tis_cur_time;
}

int gettimeofday(struct timeval *tv, struct timezone *tz) {
#ifdef __TRUSTINSOFT_ANALYZER__
  tis_make_unknown (&tv->tv_sec, sizeof (tv->tv_sec));
  tv->tv_sec = tis_unsigned_long_interval (0, 10000000000L);
  tv->tv_usec = tis_interval (-1000000, 1000000);
#else
  tv->tv_sec = tis_cur_time++;
  tv->tv_usec = 455745;
#endif
  return 0;
}
int clock_gettime(clockid_t clk_id, struct timespec *tp) {
#ifdef __TRUSTINSOFT_ANALYZER__
  tp->tv_sec = tis_unsigned_long_interval (0, 10000000000L);
  tp->tv_nsec = tis_interval (-1000000, 1000000);
#else
  tp->tv_sec = tis_cur_time++;
  tp->tv_nsec = 455745;
#endif
  return 0;
}


#ifdef __TRUSTINSOFT_ANALYZER__
/*@ assigns __s[0 .. __maxsize-1], \result
      \from __maxsize, __format[..], *__timeptr;
    ensures time_e_init: \initialized (__s + (0 .. __maxsize-1));
  */
#endif
size_t strftime(char * __restrict __s,
                size_t __maxsize,
                const char * __restrict __format,
                const struct tm * __restrict __timeptr) __THROW;
