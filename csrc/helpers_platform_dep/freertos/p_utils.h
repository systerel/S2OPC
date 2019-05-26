#ifndef P_UTILS_H
#define P_UTILS_H

#define MAX_SIGNAL (128)

typedef struct T_TASK_LIST_ELT
{
    TaskHandle_t value;
    void* pContext;
    unsigned int infos;
    unsigned short int nxId;
    unsigned short int prId;
} tUtilsListElt;

typedef struct T_TASK_LIST
{
    tUtilsListElt* list;
    unsigned short int first;
    unsigned short int wMaxWaitingTasks;
    unsigned short int wNbRegisteredTasks;
    QueueHandle_t lockHandle;
} tUtilsList;

typedef enum E_UTILS_LIST_RESULT
{
    E_UTILS_LIST_RESULT_OK,
    E_UTILS_LIST_RESULT_ERROR_NOK,
    E_UTILS_LIST_RESULT_ERROR_MAX_ELTS,
    E_UTILS_LIST_RESULT_ERROR_ALREADY_INIT,
} eUtilsListResult;

unsigned short int P_UTILS_LIST_GetEltIndex(tUtilsList* ptr, TaskHandle_t taskNotified, unsigned int infos);

eUtilsListResult P_UTILS_LIST_AddElt(tUtilsList* ptr, TaskHandle_t handleTask, void* pContext, unsigned int infos);

unsigned short P_UTILS_LIST_RemoveElt(tUtilsList* pv, TaskHandle_t taskNotified, unsigned int infos);

void P_UTILS_LIST_DeInit(tUtilsList* ptr);

eUtilsListResult P_UTILS_LIST_Init(tUtilsList* ptr, unsigned short int wMaxRDV);

unsigned short int P_UTILS_LIST_GetEltIndexMT(tUtilsList* ptr, TaskHandle_t taskNotified, unsigned int infos);

void* P_UTILS_LIST_GetContextFromHandleMT(tUtilsList* ptr, TaskHandle_t taskNotified, unsigned int infos);

eUtilsListResult P_UTILS_LIST_AddEltMT(tUtilsList* ptr, TaskHandle_t handleTask, void* pContext, unsigned int infos);

unsigned short P_UTILS_LIST_RemoveEltMT(tUtilsList* pv, TaskHandle_t taskNotified, unsigned int infos);

void P_UTILS_LIST_DeInitMT(tUtilsList* ptr);

eUtilsListResult P_UTILS_LIST_InitMT(tUtilsList* ptr, unsigned short int wMaxRDV);

#endif
