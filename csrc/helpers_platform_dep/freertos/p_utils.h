#ifndef P_UTILS_H
#define P_UTILS_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#define MAX_P_UTILS_LIST (16)

typedef struct T_TASK_LIST_ELT
{
    TaskHandle_t value;
    void* pContext;
    uint32_t infosField1;
    uint32_t infosField2;
    uint16_t nxId;
    uint16_t prId;
} tUtilsListElt;

typedef struct T_TASK_LIST
{
    tUtilsListElt* list;
    uint16_t first;
    uint16_t wMaxWaitingTasks;
    uint16_t wNbRegisteredTasks;
    QueueHandle_t lockHandle;
} tUtilsList;

typedef enum E_UTILS_LIST_RESULT
{
    E_UTILS_LIST_RESULT_OK,
    E_UTILS_LIST_RESULT_ERROR_NOK,
    E_UTILS_LIST_RESULT_ERROR_MAX_ELTS,
    E_UTILS_LIST_RESULT_ERROR_ALREADY_INIT
} eUtilsListResult;

/* Non Thread safe private list api */

uint16_t P_UTILS_LIST_GetEltIndex(tUtilsList* ptr, TaskHandle_t taskNotified, uint32_t infos1, uint32_t infos2);

eUtilsListResult P_UTILS_LIST_AddElt(tUtilsList* ptr,
                                     TaskHandle_t handleTask,
                                     void* pContext,
                                     uint32_t infos1,
                                     uint32_t infos2);

TaskHandle_t P_UTILS_LIST_ParseValueElt(tUtilsList* ptr,
                                        uint32_t* pOutValue,
                                        uint32_t* pOutValue2,
                                        void** ppOutContext,
                                        uint16_t* pCurrentSlotId);

void* P_UTILS_LIST_ParseContextEltMT(tUtilsList* ptr, uint16_t* pCurrentSlotId);

unsigned short P_UTILS_LIST_RemoveElt(tUtilsList* pv, TaskHandle_t taskNotified, uint32_t infos1, uint32_t infos2);

void P_UTILS_LIST_DeInit(tUtilsList* ptr);

eUtilsListResult P_UTILS_LIST_Init(tUtilsList* ptr, uint16_t wMaxRDV);

/* Thread safe private list api */

uint16_t P_UTILS_LIST_GetEltIndexMT(tUtilsList* ptr, TaskHandle_t taskNotified, uint32_t infos1, uint32_t infos2);

void* P_UTILS_LIST_GetContextFromHandleMT(tUtilsList* ptr, TaskHandle_t taskNotified, uint32_t infos1, uint32_t infos2);

TaskHandle_t P_UTILS_LIST_ParseValueEltMT(tUtilsList* ptr,
                                          uint32_t* pOutValue,
                                          uint32_t* pOutValue2,
                                          void** ppOutContext,
                                          uint16_t* pCurrentSlotId);

eUtilsListResult P_UTILS_LIST_AddEltMT(tUtilsList* ptr,
                                       TaskHandle_t handleTask,
                                       void* pContext,
                                       uint32_t infos,
                                       uint32_t info2);

unsigned short P_UTILS_LIST_RemoveEltMT(tUtilsList* pv, TaskHandle_t taskNotified, uint32_t infos1, uint32_t infos2);

void P_UTILS_LIST_DeInitMT(tUtilsList* ptr);

eUtilsListResult P_UTILS_LIST_InitMT(tUtilsList* ptr, uint16_t wMaxRDV);

void P_UTILS_LIST_DeInitMT(tUtilsList* ptr);

#ifdef FOLLOW_ALLOC
extern uint32_t cptAlloc;
extern uint32_t cptFree;
extern QueueHandle_t cptMutex;
void incrementCpt(void);
void decrementCpt(void);
#endif

#endif
