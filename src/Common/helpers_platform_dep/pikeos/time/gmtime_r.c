#include <limits.h>
#include "p_time_c99.h"

struct tm* gmtime_r(const time_t* restrict t, struct tm* restrict tm)
{
    /* Reject time_t values whose year would overflow int because
     * __secs_to_zone cannot safely handle them. */
    if (*t < INT_MIN * 31622400LL || *t > INT_MAX * 31622400LL)
    {
        return 0;
    }

    if (__secs_to_tm((long long) *t, tm) < 0)
    {
        return 0;
    }

    /* We don't handle GMT zone neither dayligth save flag */
    tm->__tm_zone = "UTC";
    tm->__tm_gmtoff = 0;
    tm->tm_isdst = -1;
    return tm;
}
