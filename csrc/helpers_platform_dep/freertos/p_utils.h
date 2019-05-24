#ifndef P_UTILS_H
#define P_UTILS_H

#define MAX_SIGNAL (128)

typedef struct T_TASK_LIST_ELT
{
    TaskHandle_t value;
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
} tUtilsList;

typedef enum E_UTILS_LIST_ERROR
{
    E_UTILS_LIST_ERROR_OK,
    E_UTILS_LIST_ERROR_MAX_ELTS,
    E_UTILS_LIST_ERROR_NOK
} eUtilsListError;

unsigned short int P_UTILS_LIST_GetEltIndex(tUtilsList* ptr, TaskHandle_t taskNotified, unsigned int infos);

eUtilsListError P_UTILS_LIST_AddElt(tUtilsList* ptr, TaskHandle_t handleTask, unsigned int infos);

unsigned short P_UTILS_LIST_RemoveElt(tUtilsList* pv, TaskHandle_t taskNotified, unsigned int infos);

void P_UTILS_LIST_DeInit(tUtilsList* ptr);

eUtilsListError P_UTILS_LIST_Init(tUtilsList* ptr, unsigned short int wMaxRDV);

#endif
