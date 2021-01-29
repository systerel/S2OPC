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

/* INCLUDES */
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "udp_json_gtw.h"
#include "sopc_atomic.h"
#include "sopc_time.h"

#include "config.h"

/* DEFINE */
#define UDP_PORT_IN 5000
#define MAXSIZE 1024


/* GLOBAL VARIABLES */
static char* localhost_ip = "127.0.0.1";

/* Reception global variables */
static int32_t UDP_recepRunning = 0;
static int recepSockfd = 0;
static struct sockaddr_in servaddr;
static pthread_t recepThreadId = 0;

/* Sending global variables */
static int32_t UDP_sendRunning = 0;
static int sendSockfd = 0;
static struct sockaddr_in cltaddr;
static pthread_t sendThreadId = 0;

/* STATIC FUNCTIONS */
static SOPC_ReturnStatus UDP_ReceptionConfigure (void);
static SOPC_ReturnStatus UDP_SendingConfigure (void);
static void* UDP_ManageReception (void* arg);
static void* UDP_ManageSending (void* arg);


static SOPC_ReturnStatus UDP_ReceptionConfigure (void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Create socket file descriptor for reception */
    if ((recepSockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("# Error: reception socket creation failed.\n");
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Configure server information */
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(localhost_ip);
        servaddr.sin_port = htons(UDP_PORT_IN);

        /* Bind reception socket to the server address */
        if (bind(recepSockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
        {
            printf("# Error: reception socket binding failed.\n");
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("# Info: UDP reception socket configured.\n");
    }
    else
    {
        printf("# Error: UDP reception socket configuration failed.\n");
    }

    return status;
}


static SOPC_ReturnStatus UDP_SendingConfigure (void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // TODO

    return status;
}


SOPC_ReturnStatus UDP_Configure (void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Configure Reception */
    status = UDP_ReceptionConfigure();

    if (SOPC_STATUS_OK == status)
    {
        /* Configure Sending */
        status = UDP_SendingConfigure();
    }

    return status;
}


static void* UDP_ManageReception (void* arg)
{
    /*unused parameter */
    (void) arg;

    char buffer[MAXSIZE];
    ssize_t data_len;

    printf("# Info: UDP Reception thread running.\n");

    while (SOPC_Atomic_Int_Get(&UDP_recepRunning))
    {
        /* Wait for data reception */
        data_len = recv(recepSockfd, &buffer, MAXSIZE, MSG_DONTWAIT);

        if (data_len < 0)
        {
            SOPC_Sleep(SLEEP_TIMEOUT);
        }
        else
        {
            /* Set end of line */
            buffer[data_len] = '\0';
            printf("# Info: Received value is %s.\n", buffer);

            /* Decode received data */
            // TODO

            /* Update OPC UA Server nodes */
            // TODO
        }
    }

    /* Exit current thread */
    printf("# Info: Exit UDP reception thread.\n");
    pthread_exit(NULL);
}


static void* UDP_ManageSending (void* arg)
{
    /*unused parameter */
    (void) arg;

    printf("# Info: UDP Sending thread running.\n");

    while (SOPC_Atomic_Int_Get(&UDP_sendRunning))
    {
        // TODO
    }

    /* Exit current thread */
    printf("# Info: Exit UDP sending thread.\n");
    pthread_exit(NULL);
}


bool UDP_Start(uint32_t timeResMicroSecs)
{
    /*unused parameter */
    (void)timeResMicroSecs;

    bool UDP_recepOK = false;
    bool UDP_sendOK = true;

    /* Create and start reception thread */
    SOPC_Atomic_Int_Set(&UDP_recepRunning, 1);
    if (0 == pthread_create(&recepThreadId, NULL, &UDP_ManageReception, NULL))
    {
        UDP_recepOK = true;
        printf("# Info: UDP Reception thread %ld created.\n", recepThreadId);
    }

    /* Create and start sending thread */
    SOPC_Atomic_Int_Set(&UDP_sendRunning, 1);
    if (0 == pthread_create(&sendThreadId, NULL, &UDP_ManageSending, NULL))
    {
        UDP_sendOK = true;
        printf("# Info: UDP Sending thread %ld created.\n", sendThreadId);
    }

    return UDP_recepOK || UDP_sendOK;
}


void UDP_Stop(void)
{
    printf("# Info: Stop UDP Gateway.\n");
    SOPC_Atomic_Int_Set(&UDP_recepRunning, 0);
    SOPC_Atomic_Int_Set(&UDP_sendRunning, 0);

    /* Wait for threads to be reset*/
    pthread_join(recepThreadId, NULL);
    pthread_join(sendThreadId, NULL);
}


void UDP_StopAndClear(void)
{
    UDP_Stop();

    /* Clear */
    // TODO
}
