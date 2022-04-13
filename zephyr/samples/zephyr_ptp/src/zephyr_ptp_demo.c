/*
 * Copyright (c) 2018 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(net_gptp_sample, LOG_LEVEL_DBG);

#include <zephyr.h>
#include <errno.h>

#include "sopc_time.h"
#include "sopc_assert.h"
#include "sopc_mem_alloc.h"

#include <net/net_core.h>
#include <net/gptp.h>
#include "ethernet/gptp/gptp_messages.h"
#include "ethernet/gptp/gptp_data_set.h"


static int init_app(void)
{

	return 0;
}

static const char* sourceToString(const SOPC_Time_TimeSource source)
{
    switch (source) {
    case SOPC_TIME_TIMESOURCE_INTERNAL:   return "INTERNAL";
    case SOPC_TIME_TIMESOURCE_PTP_SLAVE:  return "PTP_SLAVE";
    case SOPC_TIME_TIMESOURCE_PTP_MASTER: return "PTP_MASTER";
        default: return "INVALID";
    }
}

static void printTime(void)
{
    char* buf = SOPC_Time_GetStringOfCurrentLocalTime(false);
    //uint64_t kernel_clock_ticks = k_cycle_get_32();
    //printk("T[%09llu] = %s\n", kernel_clock_ticks, buf);

    printk("T[%s] = %s\n",
            sourceToString(SOPC_Time_GetTimeSource()),
            buf);

    SOPC_Free(buf);
}

/***************************************************/
static void gptp_phase_dis_cb(uint8_t *gm_identity,
                              uint16_t *time_base,
                              struct gptp_scaled_ns *last_gm_ph_change,
                              double *last_gm_freq_change)
{
    (void)gm_identity;
    (void)time_base;
    (void)last_gm_ph_change;
    (void)last_gm_freq_change;
    /* Note:
     * Monitoring phase discontinuities is not used in current implementation
     * but may be used to correct/smooth time corrections
     * For example:
     */
//    const uint64_t discontinuity = last_gm_ph_change->high << 32 | last_gm_ph_change->low;
}

void main(void)
{
	init_app();

	static struct gptp_phase_dis_cb phase_dis;
	gptp_register_phase_dis_cb(&phase_dis, gptp_phase_dis_cb);

	for (int i = 0 ; ; i++)
	{
	    printTime();
	    printk("Sleep 1s\n");
	    SOPC_Sleep(1000);
	}

}
