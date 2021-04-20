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

#include <stdio.h>

#include "lwip/opt.h"
#include "lwip/tcpip.h"

#include "board.h"
#include "clock_config.h"
#include "ethernetif.h"
#include "netif/ethernet.h"
#include "peripherals.h"
#include "pin_mux.h"

#include "FreeRTOS.h"
#include "MIMXRT1064.h"
#include "queue.h"
#include "task.h"
#include "timers.h"

#include "fsl_debug_console.h"
#include "fsl_device_registers.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "fsl_qtmr.h"

#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_udp_sockets.h"

#include "FreeRTOSTest.h"

#include "ksdk_mbedtls_config.h"
#include "p_ethernet_if.h"
#include "p_generic_socket_srv.h"
#include "p_time.h"

// Global variables

Condition gHandleConditionVariable;

#if (configUSE_BOGOMIPS == 1)
extern float external_bogomips(void);
float gPerformanceValue = 0.0;
#endif

#if (configUSE_ANALYZER == 1)
#include "p_analyzer.h"
tLogSrvWks* gAnalyzerSrv;
static void cbAnalyzerCreation(void** pAnalyzerContext, tLogClientWks* pClt)
{
    *pAnalyzerContext = CreateAnalyzer(8, 5000 / P_LOG_CLT_RX_PERIOD);
}
static void cbAnalyzerDestruction(void** pAnalyzerContext, tLogClientWks* pClt)
{
    DestroyAnalyzer((tAnalyzerWks**) pAnalyzerContext);
}
static eResultDecoder cbAnalyzerExecution(void* pAnalyzerContext,
                                          tLogClientWks* pClt,
                                          uint8_t* pBufferInOut,
                                          uint16_t* dataSize,
                                          uint16_t maxSizeBufferOut)
{
    // Don't modify dataSize output, echo simulation

    ExecuteAnalyzer(pAnalyzerContext, pBufferInOut, *dataSize);
    *dataSize = 0;
    return E_DECODER_RESULT_OK;
}
#endif

int main(void)
{
    // Configuration of MPU. I and D cache are disabled.
    BOARD_ConfigMPU();
    // Initialization of GPIO
    BOARD_InitPins();
    // Initialization  of clocks
    BOARD_BootClockRUN();
    // Initialization of SDK debug console
    BOARD_InitDebugConsole();

    // Initialization of MCU ENET driver
    // Initialization of MCU Timer 3 used by FreeRTOS
    // to generate 10KHz signal and measure cpu load per thread
    BOARD_InitBootPeripherals();

#if (configUSE_BOGOMIPS == 1)
    gPerformanceValue = external_bogomips();
#endif

    // Network interface initialization (IP @...)
    // P_ETHERNET_IF_IsReady shall be called
    // to verify if interface is ready, else
    // lwip socket api crash.
    P_ETHERNET_IF_Initialize();

    // Init tick to UTC time (build date)
    P_TIME_SetInitialDateToBuildTime();

    // Attach S2OPC Mutexes mechanism to mbedtls.
    mbedtls_threading_initialize();

    // used by FreeRTOS thread validation.
    Condition_Init(&gHandleConditionVariable);

    // FreeRTOS thread validation. This test shall be running ad vitam ethernam.
    // FREE_RTOS_TEST_API_S2OPC_THREAD(&gHandleConditionVariable);

    // Toolkit server application
    // FREE_RTOS_TEST_S2OPC_SERVER(&gHandleConditionVariable);

    // Toolkit client application
    // FREE_RTOS_TEST_S2OPC_CLIENT(&gHandleConditionVariable);

    // Unit test copy from tests sources. CHECK_TIME
    // FREE_RTOS_TEST_S2OPC_TIME(&gHandleConditionVariable);

    // Unit test copy from tests sources. CHECK_THREAD and MUTEX and COND VAR
    // FREE_RTOS_TEST_S2OPC_CHECK_THREAD(&gHandleConditionVariable);

    // FREE_RTOS_TEST_S2OPC_UDP_SOCKET_API(&gHandleConditionVariable);

    //SOPC_Logger_SetConsoleOutput(true);

    FREE_RTOS_TEST_S2OPC_PUBSUB(NULL);

    // FREE_RTOS_TEST_S2OPC_USECASE_PUBSUB_SYNCHRO(NULL);

#if (configUSE_TRACE_ANALYZER == 1)
    vTraceEnable(TRC_INIT);
#endif

#if (configUSE_ANALYZER == 1)

    gAnalyzerSrv = P_LOG_SRV_CreateAndStart(61, 4023, 1, 0, 0,                                                    //
                                            cbAnalyzerCreation, cbAnalyzerDestruction, cbAnalyzerExecution, NULL, //
                                            NULL, NULL, NULL, NULL,                                               //
                                            NULL);                                                                //
#endif
    vTaskStartScheduler();

    for (;;)
        ;
}
