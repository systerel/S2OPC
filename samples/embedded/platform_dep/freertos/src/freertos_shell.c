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

/** \file samples_platform_dep.c
 *
 * \brief Provides the implementation for FreeRTOS OS-dependant features required for samples
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <sys/time.h>

#include "sopc_assert.h"
#include "sopc_common_constants.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"

#include "FreeRTOS.h"
#include "atomic.h"
#include "freertos_shell.h"

/*******************************************************************************
 * Options
 ******************************************************************************/
// Set IMPLICIT_LF_WITH_CR to automatically send LF (\r) with CR (\n). This can be required
// depending on remote terminal capabilities.
#define IMPLICIT_LF_WITH_CR 0

/*******************************************************************************
 * Macros
 ******************************************************************************/
#define SHELL_COMMAND_SIZE 256u
#define SHELL_MAX_PRINT_SIZE 256u

#define KEY_ESC (0x1BU)

// Select a UART for STM32 boards. To support new boards, the following lines may be adapted
// depending on UART number for ST-LINK attached UART.
#if defined SDK_PROVIDER_NXP
#include "fsl_debug_console.h"
#include "task.h"
#elif defined SDK_PROVIDER_STM
#include <stm32h7xx_hal.h>
#define STM32_LINK_UART huart3
#include "task.h"
#else
#error "Unsuported or Undefined SDK provider"
#endif // SDK_PROVIDER

#ifdef STM32_LINK_UART
void SOPC_ETH_MAC_Filter_Config(ETH_HandleTypeDef* heth)
{
    ETH_MACFilterConfigTypeDef macFilterConfig;

    HAL_ETH_GetMACFilterConfig(heth, &macFilterConfig);
    // macFilterConfig.PromiscuousMode = ENABLE;
    macFilterConfig.PassAllMulticast = ENABLE;
    HAL_ETH_SetMACFilterConfig(heth, &macFilterConfig);
}
#endif
// NXP's eth config set mac filtering to false

typedef enum _fun_key_status
{
    kSHELL_Normal = 0U,   /*!< Normal key */
    kSHELL_Special = 1U,  /*!< Special key */
    kSHELL_Function = 2U, /*!< Function key */
} fun_key_status_t;

typedef struct _shell_context_struct
{
    enum _fun_key_status stat; /*!< Special key status */
    uint8_t l_pos;             /*!< Total line position */
    uint8_t c_pos;             /*!< Current line position */
} shell_context_struct;

/*******************************************************************************
 * Variables
 ******************************************************************************/
static shell_context_struct g_shell_context = {.stat = kSHELL_Normal, .l_pos = 0, .c_pos = 0};
static SOPC_Mutex printMutex = NULL;

static uint8_t SOPC_Shell_getc(void)
{
    uint8_t result = 0xFF;
#ifdef STM32_LINK_UART
    {
        extern UART_HandleTypeDef STM32_LINK_UART;
        const uint16_t numberOfDataReceived = 1;
        HAL_UART_Receive(&STM32_LINK_UART, &result, numberOfDataReceived, HAL_MAX_DELAY);
    }
#elif defined SDK_PROVIDER_NXP
    result = (uint8_t) DbgConsole_Getchar(); // This give a raw output
#else
#error "Unknown target, can't figure out how to communicate over Serial line"
#endif // STM32_LINK_UART
    return result;
}

int _gettimeofday(struct timeval* tv, void* tzvp)
{
    uint64_t t = 0;                        // get uptime in nanoseconds
    tv->tv_sec = t / 1000000000;           // convert to seconds
    tv->tv_usec = (t % 1000000000) / 1000; // get remaining microseconds
    return 0;                              // return non-zero for error
} // end _gettimeofday()

/*******************************************************************************
 * Extern Functions
 ******************************************************************************/
static void shell_putChar(const char c);

#ifdef STM32_LINK_UART
extern UART_HandleTypeDef STM32_LINK_UART;
static inline void shell_putChar(const char c)
{
    HAL_UART_Transmit(&STM32_LINK_UART, (const unsigned char*) &c, 1, HAL_MAX_DELAY);
}
#elif defined SDK_PROVIDER_NXP
static inline void shell_putChar(const char c)
{
    DbgConsole_Putchar((int) c);
    // uint8_t uartHandleBuffer[HAL_UART_HANDLE_SIZE];
    // HAL_UartSendBlocking((hal_uart_handle_t) uartHandleBuffer[0], (uint8_t*) (&c), 1);
}
#else
#error "Unsuported or Undefined SDK provider"
#endif // SDK_PROVIDER

int __io_putchar(int ch)
{
    shell_putChar(ch);

#if IMPLICIT_LF_WITH_CR
    if (ch == '\n')
    {
        shell_putChar('\r');
    }
#endif // IMPLICIT_LF_WITH_CR
    return 1;
}

void shell_putString(const char* str)
{
    if (NULL == str)
    {
        return;
    }

    for (; 0 != (*str); str++)
    {
        shell_putChar(*str);
    }
}

void SOPC_Shell_Printf(const char* msg, ...)
{
    static uint32_t state = 0; // 0=not init, 1=initializing, 2=initialized
    if (Atomic_CompareAndSwap_u32(&state, 1U, 0U) == ATOMIC_COMPARE_AND_SWAP_SUCCESS)
    {
        SOPC_ReturnStatus res = SOPC_Mutex_Initialization(&printMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == res);
        Atomic_CompareAndSwap_u32(&state, 2U, 1U);
    }
    else
    {
        while (Atomic_CompareAndSwap_u32(&state, 2U, 2U) == ATOMIC_COMPARE_AND_SWAP_FAILURE)
        {
            taskYIELD();
        }
    }

    SOPC_ReturnStatus lock = SOPC_Mutex_Lock(&printMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == lock);
    va_list args;

    va_start(args, msg);

    static char buf[0x100]; // Static is OK because this is mutex-protected

    int nbWritten = vsnprintf(buf, sizeof(buf), msg, args);

    for (const char* ptr = buf; 0 != (*ptr); ptr++)
    {
        if (*ptr == '\n')
        {
            shell_putChar('\r');
        }

        shell_putChar(*ptr);
    }
    if (nbWritten >= sizeof(buf))
    {
        // Force a EOL char
        shell_putChar('\n');
    }

    va_end(args);
    lock = SOPC_Mutex_Unlock(&printMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == lock);
}

char* SOPC_Shell_ReadLine(void)
{
    uint8_t ch = 0;
    char* buffer = SOPC_Calloc(SHELL_COMMAND_SIZE, sizeof(char));
    g_shell_context.l_pos = 0;
    g_shell_context.c_pos = 0;
    PRINTF("\nSHELL >> ");
    // While we don't received enter command still reading
    while ('\r' != ch && '\n' != ch)
    {
        ch = SOPC_Shell_getc();
        /* If error occurred when getting a char, continue to receive a new char. */
        if ((uint8_t)(-1) == ch)
        {
            PRINTF("\n[EE] Reading from SHELL failed!\n");
            SOPC_Sleep(1000);
            continue;
        }
        /* Special key */
        if (KEY_ESC == ch)
        {
            g_shell_context.stat = kSHELL_Special;
            continue;
        }
        else if (g_shell_context.stat == kSHELL_Special)
        {
            /* Function key */
            if ('[' == ch)
            {
                g_shell_context.stat = kSHELL_Function;
                continue;
            }
            g_shell_context.stat = kSHELL_Normal;
        }
        else if (g_shell_context.stat == kSHELL_Function)
        {
            g_shell_context.stat = kSHELL_Normal;

            switch ((uint8_t) ch)
            {
            /* History operation not handle yet */
            case 'A': /* Up key */
                break;
            case 'B': /* Down key */
                break;
            case 'D': /* Left key */
                if (g_shell_context.c_pos)
                {
                    shell_putChar('\b');
                    g_shell_context.c_pos--;
                }
                break;
            case 'C': /* Right key */
                if (g_shell_context.c_pos < g_shell_context.l_pos)
                {
                    shell_putChar(buffer[g_shell_context.c_pos]);
                    g_shell_context.c_pos++;
                }
                break;
            default:
                break;
            }
            continue;
        }
        /* Handle tab key */
        else if (ch == '\t')
        {
            continue;
        }
        /* Handle backspace key */
        else if (ch == '\b' || ch == 0x7F)
        {
            /* There must be at last one char */
            if (g_shell_context.c_pos == 0)
            {
                continue;
            }

            g_shell_context.l_pos--;
            g_shell_context.c_pos--;

            if (g_shell_context.l_pos > g_shell_context.c_pos)
            {
                memmove(&buffer[g_shell_context.c_pos], &buffer[g_shell_context.c_pos + 1],
                        g_shell_context.l_pos - g_shell_context.c_pos);
                buffer[g_shell_context.l_pos] = 0;
                PRINTF("\b%s  \b", &buffer[g_shell_context.c_pos]);

                /* Reset position */
                for (int i = g_shell_context.c_pos; i <= g_shell_context.l_pos; i++)
                {
                    shell_putChar('\b');
                }
            }
            else /* Normal backspace operation */
            {
                PRINTF("\b \b");
                buffer[g_shell_context.l_pos] = 0;
            }
            continue;
        }
        else
        {
        }

        /* Input too long */
        if (g_shell_context.l_pos >= (SHELL_COMMAND_SIZE - 1))
        {
            g_shell_context.l_pos = 0;
        }

        /* Handle end of line, break */
        if ((ch == '\r') || (ch == '\n'))
        {
            continue;
        }

        if (ch < ' ' || ch > 0x7E)
        {
            PRINTF("\nUnexpected char: %02X \nSHELL >> %s", (char) ch, buffer);
            g_shell_context.c_pos = g_shell_context.l_pos;
            continue;
        }
        /* Normal character */
        if (g_shell_context.c_pos < g_shell_context.l_pos)
        {
            memmove(&buffer[g_shell_context.c_pos + 1], &buffer[g_shell_context.c_pos],
                    g_shell_context.l_pos - g_shell_context.c_pos);
            buffer[g_shell_context.c_pos] = ch;
            PRINTF("%s", &buffer[g_shell_context.c_pos]);
            /* Move the cursor to new position */
            for (int i = g_shell_context.c_pos; i < g_shell_context.l_pos; i++)
            {
                shell_putChar('\b');
            }
        }
        else
        {
            buffer[g_shell_context.l_pos] = ch;
            shell_putChar(ch);
        }

        ch = 0;
        g_shell_context.l_pos++;
        g_shell_context.c_pos++;
    }
    buffer[g_shell_context.l_pos] = 0;
    __io_putchar('\n');
    return buffer;
}
