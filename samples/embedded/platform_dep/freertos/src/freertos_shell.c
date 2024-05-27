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
#define PRINTF SOPC_Shell_Printf
#define SHELL_COMMAND_SIZE 256u
#define SHELL_MAX_PRINT_SIZE 256u

#define KEY_ESC (0x1BU)

#ifdef STM32H723xx
#include <stm32h7xx_hal.h>

void SOPC_ETH_MAC_Filter_Config(ETH_HandleTypeDef* heth)
{
    ETH_MACFilterConfigTypeDef macFilterConfig;

    HAL_ETH_GetMACFilterConfig(heth, &macFilterConfig);
    // macFilterConfig.PromiscuousMode = ENABLE;
    macFilterConfig.PassAllMulticast = ENABLE;
    HAL_ETH_SetMACFilterConfig(heth, &macFilterConfig);
}
#elif defined(STM32H735xx)
#include <stm32h7xx_hal.h>

void SOPC_ETH_MAC_Filter_Config(ETH_HandleTypeDef* heth)
{
    ETH_MACFilterConfigTypeDef macFilterConfig;

    HAL_ETH_GetMACFilterConfig(heth, &macFilterConfig);
    // macFilterConfig.PromiscuousMode = ENABLE;
    macFilterConfig.PassAllMulticast = ENABLE;
    HAL_ETH_SetMACFilterConfig(heth, &macFilterConfig);
}
#else
#error
#endif

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
static SOPC_Mutex printMutex;

static uint8_t SOPC_Shell_getc(void)
{
    uint8_t result = 0xFF;
#ifdef STM32H723xx
    {
        extern UART_HandleTypeDef huart3;
        const uint16_t numberOfDataReceived = 1;
        HAL_UART_Receive(&huart3, &result, numberOfDataReceived, HAL_MAX_DELAY);
    }
#elif defined(STM32H735xx)
    {
        extern UART_HandleTypeDef huart3;
        const uint16_t numberOfDataReceived = 1;
        HAL_UART_Receive(&huart3, &result, numberOfDataReceived, HAL_MAX_DELAY);
    }
#else
#error "Unknown target, can't figure out how to communicate over Serial line"
#endif
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

#ifdef STM32H723xx
extern UART_HandleTypeDef huart3;
static inline void shell_putChar(const char c)
{
    HAL_UART_Transmit(&huart3, (const unsigned char*) &c, 1, HAL_MAX_DELAY);
}

int __io_putchar(int ch)
{
    shell_putChar(ch);

#if IMPLICIT_LF_WITH_CR
    if (ch == '\n')
    {
        shell_putChar('\r');
    }
#endif
    return 1;
}
#elif defined(STM32H735xx)
extern UART_HandleTypeDef huart3;
static inline void shell_putChar(const char c)
{
    const uint16_t numberOfDataTransmit = 1;
    HAL_UART_Transmit(&huart3, (const unsigned char*) &c, numberOfDataTransmit, HAL_MAX_DELAY);
}

int __io_putchar(int ch)
{
    shell_putChar(ch);

#if IMPLICIT_LF_WITH_CR
    if (ch == '\n')
    {
        shell_putChar('\r');
    }
#endif
    return 1;
}
#endif

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
    static bool once = true;
    if (once)
    {
        SOPC_ReturnStatus res = SOPC_Mutex_Initialization(&printMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == res);
        once = false;
    }
    SOPC_Mutex_Lock(&printMutex);
    va_list args;

    va_start(args, msg);

    static char buf[0x100]; // Static is OK because this is mutex-protected
    int nbWritten = vsnprintf(buf, sizeof(buf), msg, args);

    for (const char* ptr = buf; 0 != (*ptr); ptr++)
    {
        shell_putChar(*ptr);
    }
    if (nbWritten >= sizeof(buf))
    {
        // Force a EOL char
        shell_putChar('\n');
    }

    va_end(args);
    SOPC_Mutex_Unlock(&printMutex);
}

char* SOPC_Shell_ReadLine(void)
{
    uint8_t ch = 0;
    char* buffer = SOPC_Calloc(SHELL_COMMAND_SIZE, sizeof(char));
    g_shell_context.l_pos = 0;
    g_shell_context.c_pos = 0;
    PRINTF("SHELL >> ");
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
