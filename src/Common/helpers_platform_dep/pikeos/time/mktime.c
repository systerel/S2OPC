#include "p_time_c99.h"

time_t mktime(struct tm* tm)
{
    struct tm new;
    long long t = __tm_to_secs(tm);

    /* We don't support different GMT neither dayligth save flags */
    new.__tm_gmtoff = 0;
    new.tm_isdst = -1;
    new.__tm_zone = "UTC";

    if (__secs_to_tm(t + new.__tm_gmtoff, &new) < 0)
    {
        return 0;
    }

    *tm = new;
    return t;
}
