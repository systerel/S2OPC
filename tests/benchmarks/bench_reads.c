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

#include <assert.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sopc_crypto_profiles.h>
#include <sopc_mutexes.h>
#include <sopc_time.h>
#include <sopc_toolkit_async_api.h>
#include <sopc_toolkit_config.h>
#include <sopc_user_app_itf.h>

static const char* SERVER_URL = "opc.tcp://localhost:4841";

// Minimum number of measurements to consider a statistic representative.
#define MIN_N_MEASUREMENTS 1000

// Maximum number of measurements after which we give up doing more measurements.
#define MAX_N_MEASUREMENTS 10000

// Relative margin of error threshold below which we consider our sample set is
// representative.
#define REASONABLE_RMOE 10.

typedef enum {
    BENCH_RUNNING,
    BENCH_FINISHED_OK,
    BENCH_FINISHED_ERROR,
} bench_status_t;

// See https://www.itl.nist.gov/div898/handbook/eda/section3/eda3672.htm
static const double T_TABLE[] = {0,     12.706, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262, 2.228,
                                 2.201, 2.179,  2.16,  2.145, 2.131, 2.12,  2.11,  2.101, 2.093, 2.086, 2.08,
                                 2.074, 2.069,  2.064, 2.06,  2.056, 2.052, 2.048, 2.045, 2.042, 1.96};
#define T_TABLE_LAST_IDX 30
#define T_TABLE_INF_IDX (T_TABLE_LAST_IDX + 1)

struct app_ctx_t
{
    Mutex run_mutex;
    Condition run_cond;
    int32_t address_space_size;
    int32_t read_request_size;
    int32_t address_space_offset;
    uint32_t session_id;
    uint64_t n_total_requests;
    uint64_t n_sent_requests;
    uint64_t n_measurements;
    uint64_t measurements[MAX_N_MEASUREMENTS];
    bench_status_t status;
    SOPC_DateTime cycle_start_ts;
    double mean;
    double rmoe;
};

static void bench_cycle_start(struct app_ctx_t* ctx)
{
    OpcUa_ReadRequest* req = calloc(1, sizeof(OpcUa_ReadRequest));
    assert(req != NULL);

    OpcUa_ReadValueId* req_contents = calloc((size_t) ctx->read_request_size, sizeof(OpcUa_ReadValueId));
    assert(req_contents != NULL);

    OpcUa_ReadRequest_Initialize(req);

    req->TimestampsToReturn = OpcUa_TimestampsToReturn_Neither;
    req->NoOfNodesToRead = ctx->read_request_size;
    req->NodesToRead = req_contents;

    char buf[1024];

    for (int32_t i = 0; i < ctx->read_request_size; ++i)
    {
        OpcUa_ReadValueId* r = &req_contents[i];
        OpcUa_ReadValueId_Initialize(r);

        r->AttributeId = 1; // NodeId

        snprintf(buf, sizeof(buf) / sizeof(char), "ns=42;s=Objects.%d",
                 (i + ctx->address_space_offset) % ctx->address_space_size);
        SOPC_NodeId* id = SOPC_NodeId_FromCString(buf, (int32_t) strlen(buf));
        assert(id != NULL);

        SOPC_StatusCode status = SOPC_NodeId_Copy(&r->NodeId, id);
        assert(status == SOPC_STATUS_OK);

        SOPC_NodeId_Clear(id);
        free(id);
    }

    ctx->address_space_offset = (ctx->address_space_offset + ctx->read_request_size) % ctx->address_space_size;
    ctx->cycle_start_ts = SOPC_Time_GetCurrentTimeUTC();

    SOPC_ToolkitClient_AsyncSendRequestOnSession(ctx->session_id, req, (uintptr_t) ctx);
}

static double mean(uint64_t* data, uint64_t n)
{
    if (n == 0)
    {
        return 0;
    }

    double acc = 0;

    for (uint64_t i = 0; i < n; ++i)
    {
        assert(data[i] <= DBL_MAX);
        double x = acc + ((double) data[i]);
        assert(x >= acc); // In case we overflow
        acc = x;
    }

    assert(n <= DBL_MAX);
    acc /= ((double) n);

    return acc;
}

static double stddev(uint64_t* data, uint64_t n, double mean)
{
    if (n < 2)
    {
        return 0;
    }

    double var = 0;

    for (uint64_t i = 0; i < n; ++i)
    {
        assert(data[i] <= DBL_MAX);
        double d = (double) data[i];
        double x = var + (d - mean) * (d - mean);
        assert(x >= var);
        var = x;
    }

    assert((n - 1) <= DBL_MAX);
    var /= ((double) (n - 1));

    return sqrt(var);
}

static bool bench_cycle_end(struct app_ctx_t* ctx)
{
    if (ctx->n_measurements % 10 == 0)
    {
        printf(".");
        fflush(stdout);
    }

    if (ctx->n_measurements == MAX_N_MEASUREMENTS)
    {
        printf("\n");
        fflush(stdout);
        fprintf(stderr, "Could not reach a stable measure in %d measurements.\n", MAX_N_MEASUREMENTS);
        ctx->status = BENCH_FINISHED_ERROR;
        return false;
    }

    SOPC_DateTime now = SOPC_Time_GetCurrentTimeUTC();
    assert(now >= ctx->cycle_start_ts);

    // OPC UA DateTime is 100 nanoseconds
    ctx->measurements[ctx->n_measurements] = 100 * ((uint64_t)(now - ctx->cycle_start_ts));
    ctx->n_measurements++;

    // Standard deviation
    double m = mean(ctx->measurements, ctx->n_measurements);
    double sd = stddev(ctx->measurements, ctx->n_measurements, m);

    // Standard error of the mean
    assert(ctx->n_measurements <= DBL_MAX);
    double sem = sd / sqrt((double) ctx->n_measurements);

    double critical_value;

    if (ctx->n_measurements < 2)
    {
        critical_value = T_TABLE[1];
    }
    else if (ctx->n_measurements <= T_TABLE_INF_IDX)
    {
        critical_value = T_TABLE[ctx->n_measurements - 1];
    }
    else
    {
        critical_value = T_TABLE[T_TABLE_INF_IDX];
    }

    // In percents
    double relative_margin_of_error = (sem * critical_value) / m * 100;

    ctx->mean = m;
    ctx->rmoe = relative_margin_of_error;

    if ((ctx->n_measurements >= MIN_N_MEASUREMENTS) && (ctx->rmoe <= REASONABLE_RMOE))
    {
        printf("\n");
        fflush(stdout);
        ctx->status = BENCH_FINISHED_OK;
        return false;
    }
    else
    {
        return true;
    }
}

static void event_handler(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx)
{
    (void) pParam;

    struct app_ctx_t* ctx = (struct app_ctx_t*) smCtx;
    bool shutdown = false;

    switch (event)
    {
    case SE_ACTIVATED_SESSION:
        fprintf(stderr, "Connected, session activated\n");
        ctx->session_id = arg;
        bench_cycle_start(ctx);
        break;
    case SE_RCV_SESSION_RESPONSE:
        if (arg != ctx->session_id)
        {
            fprintf(stderr, "Received a session response for an unknown session ID %d\n", arg);
            break;
        }

        bool more_needed = bench_cycle_end(ctx);

        if (more_needed)
        {
            bench_cycle_start(ctx);
        }
        else
        {
            assert(ctx->status != BENCH_RUNNING);
            shutdown = true;
        }

        break;
    case SE_SESSION_ACTIVATION_FAILURE:
        fprintf(stderr, "Could not activate session\n");
        ctx->status = BENCH_FINISHED_ERROR;
        shutdown = true;
        break;
    case SE_CLOSED_SESSION:
        fprintf(stderr, "Session closed\n");
        ctx->status = BENCH_FINISHED_ERROR;
        shutdown = true;
        break;
    case SE_SND_REQUEST_FAILED:
        fprintf(stderr, "Request failed\n");
        ctx->status = BENCH_FINISHED_ERROR;
        shutdown = true;
        break;
    default:
        return;
    }

    if (shutdown)
    {
        assert(ctx->status != BENCH_RUNNING);
        assert(Condition_SignalAll(&ctx->run_cond) == SOPC_STATUS_OK);
    }
}

static void usage(char** argv)
{
    printf(
        "Usage: %s AS_SIZE READ_SIZE\n\n"
        "Benchmarks reads against an OPC-UA server.\n\n"
        "Arguments:\n"
        "AS_SIZE    Number of benchmark nodes in the server address space.\n"
        "READ_SIZE  Size of the read operations to do.\n\n"
        "The address space is supposed to hold AS_SIZE variables with a string\n"
        "NodeId following the syntax \"ns=42;s=Objects.IDX\" with IDX varying\n"
        "from 0 to AS_SIZE-1.\n",
        argv[0]);
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        usage(argv);
        return 1;
    }

    char* endptr;
    errno = 0;
    uint64_t as_size = strtoul(argv[1], &endptr, 10);

    if (*argv[1] == '\0' || *endptr != '\0' || errno != 0 || as_size > INT32_MAX)
    {
        fprintf(stderr, "AS_SIZE is not a valid integer or is too large.\n");
        return 1;
    }

    uint64_t read_size = strtoul(argv[2], &endptr, 10);

    if (*argv[2] == '\0' || *endptr != '\0' || errno != 0 || read_size > INT32_MAX)
    {
        fprintf(stderr, "READ_SIZE is not a valid integer or is too large.\n");
        return 1;
    }

    struct app_ctx_t ctx;
    memset(&ctx, 0, sizeof(struct app_ctx_t));

    assert(Mutex_Initialization(&ctx.run_mutex) == SOPC_STATUS_OK);
    assert(Mutex_Lock(&ctx.run_mutex) == SOPC_STATUS_OK);
    assert(Condition_Init(&ctx.run_cond) == SOPC_STATUS_OK);

    ctx.address_space_size = (int32_t) as_size;
    ctx.read_request_size = (int32_t) read_size;
    ctx.status = BENCH_RUNNING;

    SOPC_StatusCode status = SOPC_Toolkit_Initialize(event_handler);
    assert(status == SOPC_STATUS_OK);

    status = SOPC_Toolkit_Configured();
    assert(status == SOPC_STATUS_OK);

    SOPC_SecureChannel_Config scConfig;
    memset(&scConfig, 0, sizeof(SOPC_SecureChannel_Config));

    scConfig.isClientSc = true;
    scConfig.url = SERVER_URL;
    scConfig.reqSecuPolicyUri = SOPC_SecurityPolicy_None_URI;
    scConfig.msgSecurityMode = OpcUa_MessageSecurityMode_None;
    scConfig.requestedLifetime = 60000;

    uint32_t configIdx = SOPC_ToolkitClient_AddSecureChannelConfig(&scConfig);
    assert(configIdx != 0);

    printf("Connecting to the server...\n");
    SOPC_ToolkitClient_AsyncActivateSession(configIdx, (uintptr_t) &ctx);

    assert(Mutex_UnlockAndWaitCond(&ctx.run_cond, &ctx.run_mutex) == SOPC_STATUS_OK);

    SOPC_Toolkit_Clear();

    printf("Finished benchmarking.\n");
    printf("Address space size: %lu\n", as_size);
    printf("Read request size: %lu\n", read_size);
    printf("Number of measurements: %lu\n", ctx.n_measurements);
    printf("Average response time: %.2f us\n", ctx.mean / 1000);
    printf("Relative margin of error: %.2f%%\n", ctx.rmoe);

    return (ctx.status == BENCH_FINISHED_OK) ? 0 : 1;
}
