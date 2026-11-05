/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/** \file sopc_test_services_queue_busy.h
 *
 * \brief Shared helpers for Services-queue busy refusal validation tests
 *        (local and external BadServerTooBusy shedding).
 *
 * Principle: the busy state of a category is evaluated at each dequeue of a request of that category, with the
 * number of requests of the category still pending (::SOPC_SERVICES_QUEUE_BUSY_HIGH_LOCAL,
 * ::SOPC_SERVICES_QUEUE_BUSY_HIGH_EXTERNAL). To get exact results whatever the relative speed of the threads:
 * - the Services thread is blocked by an event treated hook (::SOPC_TestBusy_SetBlockingHook,
 *   ::SOPC_TestBusy_BlockServices) after the treatment of a trigger local request, the Services thread being idle
 *   beforehand (::SOPC_TestBusy_WaitServicesIdle);
 * - a batch of N requests is posted while it is blocked, then it is released (::SOPC_TestBusy_ReleaseServices);
 * - a batch of HIGH requests is never refused, a batch of N > HIGH requests has its N - LOW - 1 first requests
 *   refused then its LOW + 1 last requests accepted (::SOPC_TestBusy_ExpectedRefusedInBatch).
 * The CurrentTime refresh of the server (local Write) shall be deactivated so that only the batch requests are
 * pending.
 *
 * Use cases:
 * - local requests (toolkit_test_server_local_busy, single process): the test blocks its own Services thread and
 *   posts the batch as asynchronous local service requests.
 * - external requests (toolkit_test_server_client_busy + toolkit_test_server_busy_external): the server is a
 *   separate process, since a client of the same process sends its requests through the same Services thread.
 *   For each batch (::SOPC_TEST_BUSY_EXT_NB_BATCHES), the client and the server
 *   are synchronized with files in their common working directory (::SOPC_TestBusy_CreateFile,
 *   ::SOPC_TestBusy_WaitFile): the client creates the ready file, the server blocks its Services thread and creates
 *   the blocked file, the client sends the batch, the server releases its Services thread once the batch requests
 *   are pending.
 */

#ifndef SOPC_TEST_SERVICES_QUEUE_BUSY_H_
#define SOPC_TEST_SERVICES_QUEUE_BUSY_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "libs2opc_common_monitoring.h"
#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"
#include "sopc_enums.h"
#include "sopc_time_reference.h"
#include "sopc_types.h"

/** Default writable node used by heavy Write flood requests (light / demo address spaces). */
#define SOPC_TEST_BUSY_NODE_ID_STR "ns=1;i=1012"

/** Shared server endpoint / identity paths for busy validation tests. */
#define SOPC_TEST_BUSY_ENDPOINT_URL "opc.tcp://localhost:4841"
#define SOPC_TEST_BUSY_APPLICATION_URI "urn:S2OPC:localhost"
#define SOPC_TEST_BUSY_PRODUCT_URI "urn:S2OPC:localhost:product"
#define SOPC_TEST_BUSY_PKI_PATH "./S2OPC_Demo_PKI"
#define SOPC_TEST_BUSY_SERVER_CERT_PATH "./server_public/server_2k_cert.der"
#define SOPC_TEST_BUSY_SERVER_KEY_PATH "./server_private/encrypted_server_2k_key.pem"

/** Number of WriteValues per flood request (adds treatment cost without huge allocations). */
#ifndef SOPC_TEST_BUSY_WRITE_NB_VALUES
#define SOPC_TEST_BUSY_WRITE_NB_VALUES 32u
#endif

/** Max flood requests: stop earlier if Services queue already reached CAP_FACTOR x HIGH. */
#ifndef SOPC_TEST_BUSY_FLOOD_CAP_FACTOR
#define SOPC_TEST_BUSY_FLOOD_CAP_FACTOR 3u
#endif

/** Probe period while waiting for recovery from busy state. */
#ifndef SOPC_TEST_BUSY_PROBE_PERIOD_MS
#define SOPC_TEST_BUSY_PROBE_PERIOD_MS 1000u
#endif

/** Fail the recovery wait after this many seconds. */
#ifndef SOPC_TEST_BUSY_MAX_WAIT_SECS
#define SOPC_TEST_BUSY_MAX_WAIT_SECS 30u
#endif

/** Max wait (ms) for first busy during the flood scenario. */
#ifndef SOPC_TEST_BUSY_FLOOD_WAIT_MS
#define SOPC_TEST_BUSY_FLOOD_WAIT_MS (SOPC_TEST_BUSY_MAX_WAIT_SECS * 1000u)
#endif

/**
 * \brief Max time (ms) to wait for pending async responses to complete.
 *
 * Used to drain the flood/pad before recovery probes (avoids OOM under residual load) and again
 * before shutdown so the toolkit is idle.
 */
#ifndef SOPC_TEST_BUSY_DRAIN_TIMEOUT_MS
#define SOPC_TEST_BUSY_DRAIN_TIMEOUT_MS 30000u
#endif

/**
 * \brief Counters and timestamps collected during a busy-refusal scenario.
 */
typedef struct SOPC_TestBusy_Stats
{
    SOPC_CommonMonitoring_QueueType queueType; /**< Services queue measured by the tested busy category */
    SOPC_TimeReference tFloodStart;            /**< Start of the flood (0 if not marked) */
    SOPC_TimeReference tFirstBusy;             /**< Time of first BadServerTooBusy (0 if none) */
    SOPC_TimeReference tFirstOk;               /**< Time of first non-busy response after busy (0 if none) */
    uint32_t nbSent;                           /**< Requests successfully submitted (async send or sync probe) */
    uint32_t nbSentAtFirstBusy;                /**< ::nbSent snapshot when first BadServerTooBusy was observed */
    uint32_t queueAtFirstBusy;                 /**< ::queueType size when first BadServerTooBusy was observed */
    uint32_t nbResp;                           /**< Responses received */
    uint32_t nbBusy;                           /**< Responses with BadServerTooBusy */
    uint32_t nbOk;                             /**< Responses without BadServerTooBusy */
    uint32_t queueToDrain;                     /**< ::queueType size when drain starts (after recovery) */
    uint64_t drainMs;                          /**< Drain duration from ::tFirstOk until pending responses complete */
} SOPC_TestBusy_Stats;

/**
 * \brief Clears \p stats.
 *
 * \param[out] stats      Statistics to clear
 * \param      queueType  Services queue measured by the tested busy category (see ::SOPC_TestBusy_GetServicesQueueSize)
 */
void SOPC_TestBusy_Stats_Init(SOPC_TestBusy_Stats* stats, SOPC_CommonMonitoring_QueueType queueType);

/**
 * \brief Marks the flood start time used to compute time-to-first-busy in ::SOPC_TestBusy_PrintStats.
 *
 * \param[in, out] stats Statistics to update
 */
void SOPC_TestBusy_Stats_MarkFloodStart(SOPC_TestBusy_Stats* stats);

/**
 * \brief Returns true if \p status is OpcUa_BadServerTooBusy.
 *
 * \param status OPC UA StatusCode to classify
 */
bool SOPC_TestBusy_IsTooBusyStatus(SOPC_StatusCode status);

/**
 * \brief Returns true if \p response is a ServiceFault or service response with BadServerTooBusy.
 *
 * \param type     Encodeable type of \p response
 * \param response Response object (ServiceFault or WriteResponse, etc.)
 */
bool SOPC_TestBusy_IsTooBusyResponse(const SOPC_EncodeableType* const type, const void* const response);

/**
 * \brief Records one response in \p stats (updates busy/ok counts and timestamps).
 *
 * \param[in, out] stats  Statistics to update
 * \param isBusy          true if the response was BadServerTooBusy
 */
void SOPC_TestBusy_Stats_OnResponse(SOPC_TestBusy_Stats* stats, bool isBusy);

/**
 * \brief Creates a WriteRequest of ::SOPC_TEST_BUSY_WRITE_NB_VALUES writes on ::SOPC_TEST_BUSY_NODE_ID_STR.
 *
 * \return Allocated request, or NULL on failure. Ownership transferred to caller / toolkit on send.
 */
OpcUa_WriteRequest* SOPC_TestBusy_CreateHeavyWriteRequest(void);

/**
 * \brief Configures the busy-test server (anonymous user, light embedded address space, encrypted server key).
 *
 * Requires ::SOPC_ServerConfigHelper_Initialize beforehand. Uses ::SOPC_TestHelper_AskPass_FromEnv for the
 * private key password.
 *
 * \param applicationName Application name set in the ApplicationDescription
 * \param secureEndpoint  false: SecurityPolicy None; true: Basic256Sha256 with SignAndEncrypt
 *
 * \return SOPC_STATUS_OK on success
 */
SOPC_ReturnStatus SOPC_TestBusy_ConfigureServer(const char* const applicationName, bool secureEndpoint);

/**
 * \brief Returns current pending size of a Services looper queue.
 *
 * \param queueType The Services queue measured by the tested busy category
 *                  (::SOPC_CommonMonitoring_QueueType_ServicesLocalRequests for local requests,
 *                  ::SOPC_CommonMonitoring_QueueType_ServicesExternalRequests for external requests).
 */
uint32_t SOPC_TestBusy_GetServicesQueueSize(SOPC_CommonMonitoring_QueueType queueType);

/**
 * \brief Maximum number of flood requests for the scenario (3 x \p highThreshold by default).
 *
 * \param highThreshold The category-specific HIGH watermark to use (e.g.
 *                       ::SOPC_SERVICES_QUEUE_BUSY_HIGH_LOCAL or ::SOPC_SERVICES_QUEUE_BUSY_HIGH_EXTERNAL).
 */
uint32_t SOPC_TestBusy_GetFloodCap(uint32_t highThreshold);

/**
 * \brief Prints busy duration in seconds between first busy and first recovered response.
 *
 * \param stats Statistics filled by the scenario (tFirstBusy / tFirstOk must be set)
 */
void SOPC_TestBusy_PrintBusyDuration(const SOPC_TestBusy_Stats* const stats);

/**
 * \brief Prints scenario statistics on three lines: before busy, global, after busy.
 *
 * \param testTag Short test name prefix (e.g. "Test_Server_Local_Busy")
 * \param stats   Statistics (flood start, first busy, queueToDrain, drainMs, ...)
 */
void SOPC_TestBusy_PrintStats(const char* const testTag, const SOPC_TestBusy_Stats* const stats);

/**
 * \brief Callback used by ::SOPC_TestBusy_WaitUntilNotBusy to send one probe request.
 *
 * \param userCtx     Opaque context from the test
 * \param[out] isBusy Set to true if the probe was refused as too busy
 *
 * \return SOPC_STATUS_OK if a definitive busy/not-busy answer was obtained
 */
typedef SOPC_ReturnStatus (*SOPC_TestBusy_Probe_Fct)(void* userCtx, bool* isBusy);

/**
 * \brief Sends a probe every ::SOPC_TEST_BUSY_PROBE_PERIOD_MS until not busy or timeout.
 *
 * On success, sets \p stats->tFirstOk when the first non-busy probe succeeds.
 * Transient probe failures (e.g. OOM under residual load) are treated as still-busy.
 *
 * \param probeCb   Function that sends one request and reports busy/not-busy
 * \param userCtx   Passed to \p probeCb
 * \param[in, out] stats Statistics (tFirstOk updated on recovery)
 *
 * \return SOPC_STATUS_OK if recovered, SOPC_STATUS_TIMEOUT otherwise
 */
SOPC_ReturnStatus SOPC_TestBusy_WaitUntilNotBusy(SOPC_TestBusy_Probe_Fct probeCb,
                                                 void* userCtx,
                                                 SOPC_TestBusy_Stats* stats);

/**
 * \brief Number of requests of the hysteresis batches above the HIGH watermark: such a batch of HIGH + this number of
 *        requests posted while the Services thread is blocked has its HIGH + this number - LOW - 1 first requests
 *        refused once released (busy state evaluated at each dequeue with the remaining queue size).
 */
#define SOPC_TEST_BUSY_BATCH_ABOVE_HIGH 5u

/**
 * \brief Installs (\p install true) or removes the Services event treated hook used by
 *        ::SOPC_TestBusy_BlockServices to block the Services thread. The hook replaces any other one (e.g. server
 *        diagnostics update).
 */
void SOPC_TestBusy_SetBlockingHook(bool install);

/**
 * \brief Waits until the Services thread treated all the pending events (server started): a synchronous local request
 *        is treated, then all the Services queues shall be empty.
 * \param timeoutMs Maximum waiting time
 * \return SOPC_STATUS_OK once idle, SOPC_STATUS_TIMEOUT otherwise
 */
SOPC_ReturnStatus SOPC_TestBusy_WaitServicesIdle(uint32_t timeoutMs);

/**
 * \brief Blocks the Services thread after the treatment of a trigger local request (asynchronous Write): the following
 *        events stay pending until ::SOPC_TestBusy_ReleaseServices. The blocking hook shall be installed and no other
 *        local request shall be posted meanwhile (see ::SOPC_TestBusy_WaitServicesIdle). If the Services thread is
 *        blocked before the trigger request treatment (still pending), it is released and blocked again.
 * \param timeoutMs Maximum waiting time for the Services thread to be blocked
 * \return SOPC_STATUS_OK once blocked (the trigger request response received), an error status otherwise (released,
 *         the failed step and the blocking state are printed)
 */
SOPC_ReturnStatus SOPC_TestBusy_BlockServices(uint32_t timeoutMs);

/** \brief Releases the Services thread blocked by ::SOPC_TestBusy_BlockServices. */
void SOPC_TestBusy_ReleaseServices(void);

/**
 * \brief External busy test (separate server process): number of request batches, the batch 0 of HIGH requests and
 *        the batch 1 of HIGH + ::SOPC_TEST_BUSY_BATCH_ABOVE_HIGH requests (see ::SOPC_TestBusy_BatchSize). Each batch
 *        is synchronized with files in the common working directory: the client
 *        creates ::SOPC_TEST_BUSY_EXT_READY_FILE_FMT, the server blocks its Services thread and creates
 *        ::SOPC_TEST_BUSY_EXT_BLOCKED_FILE_FMT, the client sends the batch and the server releases its Services thread
 *        once the batch requests are pending in its Services queue.
 */
#define SOPC_TEST_BUSY_EXT_NB_BATCHES 2u
#define SOPC_TEST_BUSY_EXT_READY_FILE_FMT "./toolkit_test_server_client_busy_ready_%" PRIu32
#define SOPC_TEST_BUSY_EXT_BLOCKED_FILE_FMT "./toolkit_test_server_client_busy_blocked_%" PRIu32

/**
 * \brief Number of requests of a hysteresis batch: \p high, or \p high + ::SOPC_TEST_BUSY_BATCH_ABOVE_HIGH if
 *        \p aboveHigh.
 * \param high      HIGH watermark of the tested category (e.g. ::SOPC_SERVICES_QUEUE_BUSY_HIGH_LOCAL)
 * \param aboveHigh true for a batch above the HIGH watermark
 */
uint32_t SOPC_TestBusy_BatchSize(uint32_t high, bool aboveHigh);

/**
 * \brief Expected number of refused requests of a batch of \p nbRequests requests posted while the Services thread is
 *        blocked: 0 if \p nbRequests <= \p high, \p nbRequests - \p low - 1 otherwise.
 */
uint32_t SOPC_TestBusy_ExpectedRefusedInBatch(uint32_t nbRequests, uint32_t high, uint32_t low);

/**
 * \brief Builds the name of a synchronization file of the external busy test for the batch \p batchIdx.
 * \param[out] name          Buffer for the file name
 * \param      size          Size of \p name
 * \param      serverBlocked false: ::SOPC_TEST_BUSY_EXT_READY_FILE_FMT, true: ::SOPC_TEST_BUSY_EXT_BLOCKED_FILE_FMT
 * \param      batchIdx      Index of the batch
 */
void SOPC_TestBusy_SyncFileName(char* name, size_t size, bool serverBlocked, uint32_t batchIdx);

/** \brief Creates the (empty) file \p path. */
SOPC_ReturnStatus SOPC_TestBusy_CreateFile(const char* path);

/**
 * \brief Waits until the file \p path exists or \p *stop is set (if \p stop is not NULL).
 * \return SOPC_STATUS_OK once the file exists, SOPC_STATUS_TIMEOUT otherwise
 */
SOPC_ReturnStatus SOPC_TestBusy_WaitFile(const char* path, uint32_t timeoutMs, int32_t* stop);

#endif /* SOPC_TEST_SERVICES_QUEUE_BUSY_H_ */
