#include "p_time_c99.h"

struct tm* gmtime(const time_t* t)
{
    struct tm tt = {};
    return gmtime_r(t, &tt);
}
