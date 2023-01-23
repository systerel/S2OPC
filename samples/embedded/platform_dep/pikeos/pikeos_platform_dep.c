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
 * \brief Provides the implementation for Pikeos of OS-dependant features required for samples
 */

#include "samples_platform_dep.h"

#include <lwip/inet.h>
#include <lwip/netif.h>
#include <vm.h>

#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_platform_time.h"

#define SHELL_COMMAND_SIZE 256u
#define KEY_ESC (0x1BU)

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
static vm_file_desc_t g_fd_serial;

//  TO BE ADAPTED FOR EACH PROJECT
#define FILE_NAME_SERIAL "ser0:0"

void SOPC_Platform_Setup(void)
{
    int res = _lwip_init();
    SOPC_ASSERT(0 == res);
    vm_cprintf("Lwip library initialized\n");
    P4_e_t result = vm_open(FILE_NAME_SERIAL, VM_O_RD, &g_fd_serial);
    if (P4_E_OK == result)
    {
        vm_cprintf("Serial port %s open and ready to be used\n", FILE_NAME_SERIAL);
    }
    else
    {
        vm_cprintf("Failed to open %s with result : %d\n", FILE_NAME_SERIAL, result);
    }
}


void SOPC_Platform_Shutdown(const bool reboot)
{
    SOPC_UNUSED_ARG(reboot);

    P4_e_t result = vm_close(&g_fd_serial);
    if (P4_E_OK != result)
    {
        vm_cprintf("Failed to close serial port %s status : %d\n", FILE_NAME_SERIAL, result);
    }
    vm_cprintf("End of the program\n");
    vm_cprintf("Bye !\n");
}

const char* SOPC_Platform_Get_Default_Net_Itf(void)
{
    static char netif[6] = {0};
    if (netif[0] == 0)
    {
        sprintf(netif, "%c%c%d", netif_default->name[0], netif_default->name[1], (int) netif_default->num);
    }
    return netif;
}

char* SOPC_Shell_ReadLine(void)
{
    uint8_t ch = 0;
    char* buffer = SOPC_Calloc(SHELL_COMMAND_SIZE, sizeof(char));
    P4_size_t bytesRead;
    P4_e_t res;
    g_shell_context.l_pos = 0;
    g_shell_context.c_pos = 0;
    vm_cprintf("SHELL >> ");
    // While we don't received enter command still reading
    while ('\r' != ch && '\n' != ch)
    {
        res = vm_read(&g_fd_serial, &ch, 1, &bytesRead);
        /* If error occurred when getting a char, continue to receive a new char. */
        if (P4_E_OK != res)
        {
            vm_cprintf("\n[EE] Reading from SHELL failed! with status : %d\n", res);
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
                    vm_cprintf("\b");
                    g_shell_context.c_pos--;
                }
                break;
            case 'C': /* Right key */
                if (g_shell_context.c_pos < g_shell_context.l_pos)
                {
                    vm_cprintf("%c", buffer[g_shell_context.c_pos]);
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
                vm_cprintf("\b%s  \b", &buffer[g_shell_context.c_pos]);

                /* Reset position */
                for (int i = g_shell_context.c_pos; i <= g_shell_context.l_pos; i++)
                {
                    vm_cprintf("\b");
                }
            }
            else /* Normal backspace operation */
            {
                vm_cprintf("\b \b");
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
            vm_cprintf("\nUnexpected char: %02X \nSHELL >> %s", (char) ch, buffer);
            g_shell_context.c_pos = g_shell_context.l_pos;
            continue;
        }
        /* Normal character */
        if (g_shell_context.c_pos < g_shell_context.l_pos)
        {
            memmove(&buffer[g_shell_context.c_pos + 1], &buffer[g_shell_context.c_pos],
                    g_shell_context.l_pos - g_shell_context.c_pos);
            buffer[g_shell_context.c_pos] = ch;
            vm_cprintf("%s", &buffer[g_shell_context.c_pos]);
            /* Move the cursor to new position */
            for (int i = g_shell_context.c_pos; i < g_shell_context.l_pos; i++)
            {
                vm_cprintf("\b");
            }
        }
        else
        {
            buffer[g_shell_context.l_pos] = ch;
            vm_cprintf("%c", ch);
        }

        ch = 0;
        g_shell_context.l_pos++;
        g_shell_context.c_pos++;
    }
    buffer[g_shell_context.l_pos] = 0;
    vm_cprintf("\n");

    return buffer;
}


void SOPC_Platform_Target_Debug(const char* param)
{
    SOPC_UNUSED_ARG(param);
}

extern int main(void)
{
    #ifdef PIKEOSDEBUG
        gdb_breakpoint();
    #endif
    SOPC_Platform_Main();
}
