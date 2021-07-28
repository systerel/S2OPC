/**
 * OPC Foundation OPC UA Safety Stack
 *
 * Copyright (c) 2021 OPC Foundation. All rights reserved.
 * This Software is licensed under OPC Foundation's proprietary Enhanced
 * Commercial Software License Agreement [LINK], and may only be used by
 * authorized Licensees in accordance with the terms of such license.
 * THE SOFTWARE IS LICENSED "AS-IS" WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.
 * This notice must be included in all copies or substantial portions of the Software.
 *
 * \file
 *
 * \brief OPC UA Safety instance manager.
 *
 * \date      2021-07-08
 * \revision  0.4
 * \status    in work
 *
 * Defines the functions of the OPC UA Safety instance manager.
 *
 * Safety-Related: yes
 */

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_cons.h"
#include "uas_def.h"
#include "uas_prov.h"
#include "uas_rvar.h"
#include "uas_time.h"
#include "uas_type.h"

/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas.h"

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

/**
 * Array of SafetyProvider data sets
 * Each data set contains the interfaces of one SafetyProvider
 */
UAS_SafetyProvider_type azUAS_SafetyProviders[UASDEF_MAX_SAFETYPROVIDERS] = {{0u}};

/**
 * Array of SafetyConsumer data sets
 * Each data set contains the interfaces of one SafetyConsumer
 */
UAS_SafetyConsumer_type azUAS_SafetyConsumers[UASDEF_MAX_SAFETYCONSUMERS] = {{0u}};

/*--------------------------------------------------------------------------*/
/******************************** L O C A L *********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

/*-------------*/
/*  T Y P E S  */
/*-------------*/

/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

/**
 * De-initialization of all UAS instances
 */
static void vUAS_ResetAllInstances(void);

/**
 * Check of unique identifiers
 */
static UAS_UInt8 byUAS_UniqueProvIdentifiers(
    /** IN: Index (Number) of the SafetyProvider instance */
    UAS_UInt16 uInstanceIndex);

/**
 * Check of unique identifiers
 */
static UAS_UInt8 byUAS_UniqueConsIdentifiers(
    /** IN: Index (Number) of the SafetyConsumer instance */
    UAS_UInt16 uInstanceIndex);

/**
 * Common checks before calling a UAS instance
 */
static UAS_UInt8 byUAS_PreCheck(
    /** IN: Index (Number) of the SafetyConsumer instance */
    UAS_UInt16 uInstanceIndex);

/**
 * Common checks after calling a UAS instance
 */
static UAS_UInt8 byUAS_PostCheck(
    /** IN: Return value with error code of the calling function */
    UAS_UInt8 byRetVal);

/**
 * Check of basic data type definitions
 */
static void vUAS_CheckDataTypes(void);

/*--------------------------------------------------------------------------*/
/******************************** L O C A L *********************************/
/*--------------------------------------------------------------------------*/

/* Array of instance data sets for the SafetyProvider state machines */
static UASPROV_StateMachine_type azUAS_ProvStateMachines[UASDEF_MAX_SAFETYPROVIDERS] = {{0u}};

/* Redundant (inverse) Array of instance data sets for for the SafetyProvider state machines */
/* All redundant variables are defined im module uas_rvar                                    */
extern UASPROV_StateMachine_type r_azUAS_ProvStateMachines[UASDEF_MAX_SAFETYPROVIDERS];

/* Array of instance data sets for the SafetyConsumer state machines */
static UASCONS_StateMachine_type azUAS_ConsStateMachines[UASDEF_MAX_SAFETYCONSUMERS] = {{0u}};

/* Redundant (inverse) Array of instance data sets for for the SafetyConsumer state machines */
/* All redundant variables are defined im module uas_rvar                                    */
extern UASCONS_StateMachine_type r_azUAS_ConsStateMachines[UASDEF_MAX_SAFETYCONSUMERS];

/* This variable is used by the UAS interface functions to store an fatal   */
/* error. If any error code is stored the UAS instances are not executed.   */
/* The value for no error is UAS_OK                                         */
static UAS_UInt8 byUAS_Error = UAS_OK;

/* Redundant (inverse) error code of the UAS module                         */
/* All redundant variables are defined im module uas_rvar                   */
extern UAS_UInt8 r_byUAS_Error;

/* Redundant variables shall be initialized only once */
static UAS_Bool bUAS_FirstInit = 1u;

/*--------------------------------------------------------------------------*/
/************** I M P L E M E N T A T I O N   ( G L O B A L S ) *************/
/*--------------------------------------------------------------------------*/

/*-------------------------------*/
/* S A F E T Y   P R O V I D E R */
/*-------------------------------*/

/**
 * Initialization of a SafetyProvider instance
 * This function initializes an SafetyProvider instance. After checking the plausibility of
 * instance number and the unambiguousness of the instance handle and IDs it calls the
 * initialization function of the uas_prov module with the corresponding instance data reference.
 * Error Handling:
 *  - Check of byUASError: The function only initializes the UAS instance if the value
 *    of byUASError is UAS_OK.
 *  - Check of function parameters, return values of function calls, wUASTIME_ErrorCount:
 *    If any error is detected the value of byUASError will be set to an error code (not FDL_OK).
 *    A fatal error exists if byUASError is not equal UAS_OK. In this case the function returns
 *    immediatly with this error code and does not initialize the UAS instance.
 *    Restart of UAS can only be done by Power OFF/ON.
 * Warnings:
 *  - The output of failsafe values to the application as well as the generation of safety
 *    messages with failsafe values is not assured if the return value is not equal UAS_OK.
 *    Therefore the UAS user has to enter a safe state.
 * \param[in/out]  pzInstanceData - Pointer to the data structure of the SafetyProvider instance
 * \param[in]      uInstanceIndex - Index (Number) of the SafetyProvider instance
 * \param[out]     pnResult       - Pointer to an output variable for the result of the parameter check
 * \return UAS_OK if ok, error code if error occured
 */
UAS_UInt8 byUAS_InitSafetyProvider(UAS_SafetyProvider_type* pzInstanceData,
                                   UAS_UInt16 uInstanceIndex,
                                   UAS_ParameterError_type* pnResult)
{
    UAS_UInt8 byRetVal;

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_InitSafetyProvider: Entry. pzInstanceData = %p, uInstanceIndex = %04Xh", pzInstanceData,
                     uInstanceIndex);

#endif /* UASDEF_DBG */

    /* Initialize the redundant variables */
    if (1u EQ bUAS_FirstInit)
    {
        vUASRVAR_Init();
        vUAS_CheckDataTypes();
        bUAS_FirstInit = 0u;
    } /* if */

    byRetVal = byUAS_PreCheck(uInstanceIndex);

    if (UAS_OK EQ byRetVal)
    {
        /* Function parameter 1 invalid ? */
        if (pzInstanceData NOT_EQ & (azUAS_SafetyProviders[uInstanceIndex]))
        {
            if (NULL EQ pzInstanceData)
            {
                byRetVal = UAS_POINTER_ERR;
            } /* if */
            else
            {
                byRetVal = UAS_FCT_PARAM_ERR;
            } /* else */
        }     /* if */

        /* Instance already initialized ? */
        else if ((0uL NOT_EQ azUAS_ProvStateMachines[uInstanceIndex].dwParamFcs) OR(
                     NULL NOT_EQ azUAS_ProvStateMachines[uInstanceIndex].pzInstanceData))
        {
            byRetVal = UAS_STATE_ERR;
        } /* else if */

        else
        {
            /* Check that the combination of Identifiers is not used by an other UAS rinstance */
            byRetVal = byUAS_UniqueProvIdentifiers(uInstanceIndex);

            /* Initiate the SafetyProvider instance */
            if (UAS_OK EQ byRetVal)
            {
                byRetVal = byUASPROV_Init(&(azUAS_ProvStateMachines[uInstanceIndex]),
                                          &(r_azUAS_ProvStateMachines[uInstanceIndex]), pzInstanceData, pnResult);
            } /* if */
        }     /* else */
    }         /* if */

    byRetVal = byUAS_PostCheck(byRetVal);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_InitSafetyProvider: Exit. byRetVal = %02Xh, *pnResult = %04Xh", byRetVal, *pnResult);

#endif /* UASDEF_DBG */

    return byRetVal;

} /* end of function */

/**
 * Change parameters of a SafetyProvider instance
 * This function re-parameterizes an SafetyProvider instance. After checking the plausibility of
 * instance number and initialization state of the instance it calls the change parameters function
 * of the uas_prov module with the corresponding instance data reference.
 * The input parameters contain the SPI parameters to be set. The output parameter contains the result
 * of the parameter check. If the result is UAS_PARAM_OK, the SPI parameters has been accepted and
 * the re-parameterization of the SafetyProvider instance succeeded.
 * If the result is different from UAS_PARAM_OK, the SPI parameters has been rejected and the
 * parameterization of the SafetyProviderinstance failed. In this case the old parameters remain valid.
 * Notes:
 * - An instance must first be successfully initialized with byUAS_InitSafetyProvider
 *   before it can be re-parameterized with byUAS_ChangeSafetyProviderSPI.
 * Error Handling:
 *  - Check of byUASError: The function only initializes the UAS instance if the value
 *    of byUASError is UAS_OK.
 *  - Check of function parameters, return values of function calls, wUASTIME_ErrorCount:
 *    If any error is detected the value of byUASError will be set to an error code (not FDL_OK).
 *    A fatal error exists if byUASError is not equal UAS_OK. In this case the function returns
 *    immediatly with this error code and does not initialize the UAS instance.
 *    Restart of UAS can only be done by Power OFF/ON.
 * Warnings:
 *  - The output of failsafe values to the application as well as the generation of safety
 *    messages with failsafe values is not assured if the return value is not equal UAS_OK.
 *    Therefore the UAS user has to enter a safe state.
 * \param[in]  uInstanceIndex - Index (Number) of the SafetyProvider instance
 * \param[in]  dwHandle       - Handle of of the SafetyProvider instance
 * \param[in]  pzSPI          - Pointer to new SPI parameters of the SafetyProvider instance
 * \param[out] pnResult       - Pointer to an output variable for the result of the parameter check
 * \return UAS_OK if ok, error code if error occured
 */
UAS_UInt8 byUAS_ChangeSafetyProviderSPI(UAS_UInt16 uInstanceIndex,
                                        UAS_UInt32 dwHandle,
                                        UAS_SafetyProviderSPI_type* pzSPI,
                                        UAS_ParameterError_type* pnResult)
{
    UAS_UInt8 byRetVal = byUAS_PreCheck(uInstanceIndex);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_ChangeSafetyProviderSPI: Entry. uInstanceIndex = %04Xh, dwHandle = %08Xh", uInstanceIndex,
                     dwHandle);
#else
    (void)dwHandle;
#endif /* UASDEF_DBG */

    if (UAS_OK EQ byRetVal)
    {
        if ((NULL EQ pzSPI) OR(NULL EQ pnResult))
        {
            byRetVal = UAS_POINTER_ERR;
        } /* if */

        else
        {
            UASPROV_StateMachine_type* pzStateMachine = &(azUAS_ProvStateMachines[uInstanceIndex]);
            UASPROV_StateMachine_type* r_pzStateMachine = &(r_azUAS_ProvStateMachines[uInstanceIndex]);
            UAS_SafetyProvider_type* pzInstanceData = &(azUAS_SafetyProviders[uInstanceIndex]);

            /* Instance not initialized ? */
            if ((0uL EQ pzStateMachine->dwParamFcs) OR(pzInstanceData NOT_EQ pzStateMachine->pzInstanceData))
            {
                byRetVal = UAS_STATE_ERR;
            } /* if */

            else
            {
                /* Re-Parameterize the SafetyProvider instance */
                byRetVal = byUASPROV_ChangeSPI(pzStateMachine, r_pzStateMachine, pzSPI, pnResult);
#ifdef UASDEF_DBG

                UASDEF_LOG_DEBUG("  byUAS_ChangeSafetyProviderSPI: *pnResult = %04Xh", *pnResult);

#endif        /* UASDEF_DBG */
            } /* else */

        } /* else */
    }     /* if */

    byRetVal = byUAS_PostCheck(byRetVal);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_ChangeSafetyProviderSPI: Exit. byRetVal = %02Xh", byRetVal);

#endif /* UASDEF_DBG */

    return byRetVal;
}

/**
 * Start of a SafetyProvider instance
 * This function starts a SafetyProvider instance. After checking the plausibility of
 * instance number and initialization state of the instance it calls the start function
 * of the uas_prov module with the corresponding instance data reference to start the
 * SafetyProvider state machine. Results of the start transitions will be written to
 * the SAPI output parameters of the instance.
 * Notes:
 * - An instance must first be successfully initialized with byUAS_InitSafetyProvider
 *   before it can be started with byUAS_StartSafetyProvider.
 * Error Handling:
 *  - Check of byUASError: The function only initializes the UAS instance if the value
 *    of byUASError is UAS_OK.
 *  - Check of function parameters, return values of function calls, wUASTIME_ErrorCount:
 *    If any error is detected the value of byUASError will be set to an error code (not FDL_OK).
 *    A fatal error exists if byUASError is not equal UAS_OK. In this case the function returns
 *    immediatly with this error code and does not initialize the UAS instance.
 *    Restart of UAS can only be done by Power OFF/ON.
 * Warnings:
 *  - The output of failsafe values to the application as well as the generation of safety
 *    messages with failsafe values is not assured if the return value is not equal UAS_OK.
 *    Therefore the UAS user has to enter a safe state.
 * \param[in]  uInstanceIndex - Index (Number) of the SafetyProvider instance
 * \param[in]  dwHandle       - Handle of of the SafetyProvider instance
 * \return UAS_OK if ok, error code if error occured
 */
UAS_UInt8 byUAS_StartSafetyProvider(UAS_UInt16 uInstanceIndex, UAS_UInt32 dwHandle)
{
    UAS_UInt8 byRetVal = byUAS_PreCheck(uInstanceIndex);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_StartSafetyProvider: Entry. uInstanceIndex = %04Xh, dwHandle = %08Xh", uInstanceIndex,
                     dwHandle);

#else
    (void)dwHandle;
#endif /* UASDEF_DBG */

    if (UAS_OK EQ byRetVal)
    {
        UASPROV_StateMachine_type* pzStateMachine = &(azUAS_ProvStateMachines[uInstanceIndex]);
        UASPROV_StateMachine_type* r_pzStateMachine = &(r_azUAS_ProvStateMachines[uInstanceIndex]);
        UAS_SafetyProvider_type* pzInstanceData = &(azUAS_SafetyProviders[uInstanceIndex]);

        /* Instance not initialized ? */
        if ((0uL EQ pzStateMachine->dwParamFcs) OR(pzInstanceData NOT_EQ pzStateMachine->pzInstanceData))
        {
            byRetVal = UAS_STATE_ERR;
        } /* if */

        else
        {
            /* Start the SafetyProvider instance ? */
            byRetVal = byUASPROV_Start(pzStateMachine, r_pzStateMachine);
        } /* else */
    }     /* if */

    byRetVal = byUAS_PostCheck(byRetVal);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_StartSafetyProvider: Exit. byRetVal = %02Xh", byRetVal);

#endif /* UASDEF_DBG */

    return byRetVal;

} /* end of function */

/**
 * Execution of a SafetyProvider instance
 * This function executes a SafetyProvider instance. It must be cyclically called!
 * After checking the plausibility of instance number, initialization state of the instance
 * and function invocation sequence it calls the execute function of the uas_prov module
 * with the corresponding instance data reference to execute the SafetyProvider state machine.
 * Results of the executed transitions will be written to the SAPI output parameters of the instance.
 * In case of a communication error, a diagnostic code will be written to the DI output parameters
 * of the instance. The UAS application shall use this code to present a diagnostics message.
 * Notes:
 * - An instance must first be successfully initialized with byUAS_InitSafetyProvider
 *   before it can be executed with byUAS_ExecuteSafetyProvider.
 * Error Handling:
 *  - Check of byUASError: The function only initializes the UAS instance if the value
 *    of byUASError is UAS_OK.
 *  - Check of function parameters, return values of function calls, wUASTIME_ErrorCount:
 *    If any error is detected the value of byUASError will be set to an error code (not FDL_OK).
 *    A fatal error exists if byUASError is not equal UAS_OK. In this case the function returns
 *    immediatly with this error code and does not initialize the UAS instance.
 *    Restart of UAS can only be done by Power OFF/ON.
 * Warnings:
 *  - The output of failsafe values to the application as well as the generation of safety
 *    messages with failsafe values is not assured if the return value is not equal UAS_OK.
 *    Therefore the UAS user has to enter a safe state.
 * \param[in]  uInstanceIndex - Index (Number) of the SafetyProvider instance
 * \param[in]  dwHandle       - Handle of of the SafetyProvider instance
 * \return UAS_OK if ok, error code if error occured
 */
UAS_UInt8 byUAS_ExecuteSafetyProvider(UAS_UInt16 uInstanceIndex, UAS_UInt32 dwHandle)
{
    UAS_UInt8 byRetVal = byUAS_PreCheck(uInstanceIndex);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_ExecuteSafetyProvider: Entry. uInstanceIndex = %04Xh, dwHandle = %08Xh", uInstanceIndex,
                     dwHandle);

#else
    (void)dwHandle;
#endif /* UASDEF_DBG */

    if (UAS_OK EQ byRetVal)
    {
        UASPROV_StateMachine_type* pzStateMachine = &(azUAS_ProvStateMachines[uInstanceIndex]);
        UASPROV_StateMachine_type* r_pzStateMachine = &(r_azUAS_ProvStateMachines[uInstanceIndex]);
        UAS_SafetyProvider_type* pzInstanceData = &(azUAS_SafetyProviders[uInstanceIndex]);

        /* Instance not initialized ? */
        if ((0uL EQ pzStateMachine->dwParamFcs) OR(pzInstanceData NOT_EQ pzStateMachine->pzInstanceData))
        {
            byRetVal = UAS_STATE_ERR;
        } /* if */

        else
        {
            /* Check the function invocation sequence of the UAS user */
            if ((1u EQ pzInstanceData->bAppDone) AND(1u EQ pzInstanceData->bCommDone))
            {
                /* Reset Communication-Done flag and Application-Done flag */
                pzInstanceData->bCommDone = 0u;
                pzInstanceData->bAppDone = 0u;
                byRetVal = UAS_OK;
            } /* if */
            else
            {
#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_ExecuteSafetyProvider: Set error to UAS_STATE_ERR : bCommDone = %d, bAppDone=%d", pzInstanceData->bCommDone,
            pzInstanceData->bAppDone);

#endif /* UASDEF_DBG */

                byRetVal = UAS_STATE_ERR;
            } /* else */
        }     /* else */

        if (UAS_OK EQ byRetVal)
        {
            /* Execute the SafetyProvider instance ? */
            byRetVal = byUASPROV_Execute(pzStateMachine, r_pzStateMachine);
        } /* if */
    }     /* if */

    byRetVal = byUAS_PostCheck(byRetVal);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_ExecuteSafetyProvider: Exit. byRetVal = %02Xh", byRetVal);

#endif /* UASDEF_DBG */

    return byRetVal;

} /* end of function */

/**
 * Stop of a SafetyProvider instance
 * This function stops a SafetyProvider instance. After checking the plausibility of
 * instance number and initialization state of the instance it calls the stop function
 * of the uas_prov module with the corresponding instance data reference to stop the
 * SafetyProvider state machine. The SAPI output parameters of the instance will be resetted.
 * Notes:
 * - An instance must first be successfully initialized with byUAS_InitSafetyProvider
 *   before it can be stopped with byUAS_StopSafetyProvider.
 * Error Handling:
 *  - Check of byUASError: The function only initializes the UAS instance if the value
 *    of byUASError is UAS_OK.
 *  - Check of function parameters, return values of function calls, wUASTIME_ErrorCount:
 *    If any error is detected the value of byUASError will be set to an error code (not FDL_OK).
 *    A fatal error exists if byUASError is not equal UAS_OK. In this case the function returns
 *    immediatly with this error code and does not initialize the UAS instance.
 *    Restart of UAS can only be done by Power OFF/ON.
 * Warnings:
 *  - The output of failsafe values to the application as well as the generation of safety
 *    messages with failsafe values is not assured if the return value is not equal UAS_OK.
 *    Therefore the UAS user has to enter a safe state.
 * \param[in]  uInstanceIndex - Index (Number) of the SafetyProvider instance
 * \param[in]  dwHandle       - Handle of of the SafetyProvider instance
 * \return UAS_OK if ok, error code if error occured
 */
UAS_UInt8 byUAS_StopSafetyProvider(UAS_UInt16 uInstanceIndex, UAS_UInt32 dwHandle)
{
    UAS_UInt8 byRetVal = byUAS_PreCheck(uInstanceIndex);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_StopSafetyProvider: Entry. uInstanceIndex = %04Xh, dwHandle = %08Xh", uInstanceIndex,
                     dwHandle);

#else
    (void)dwHandle;
#endif /* UASDEF_DBG */

    if (UAS_OK EQ byRetVal)
    {
        UASPROV_StateMachine_type* pzStateMachine = &(azUAS_ProvStateMachines[uInstanceIndex]);
        UASPROV_StateMachine_type* r_pzStateMachine = &(r_azUAS_ProvStateMachines[uInstanceIndex]);
        UAS_SafetyProvider_type* pzInstanceData = &(azUAS_SafetyProviders[uInstanceIndex]);

        /* Instance not initialized ? */
        if ((0uL EQ pzStateMachine->dwParamFcs) OR(pzInstanceData NOT_EQ pzStateMachine->pzInstanceData))
        {
            byRetVal = UAS_STATE_ERR;
        } /* if */

        else
        {
            /* Start the SafetyProvider instance ? */
            byRetVal = byUASPROV_Stop(pzStateMachine, r_pzStateMachine);
        } /* else */
    }     /* if */

    byRetVal = byUAS_PostCheck(byRetVal);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_StopSafetyProvider: Exit. byRetVal = %02Xh", byRetVal);

#else

    (void)dwHandle;

#endif /* UASDEF_DBG */

    return byRetVal;

} /* end of function */

/**
 * De-initialization of a SafetyProvider instance
 * This function de-initializes a SafetyProvider instance. After checking the plausibility of
 * instance number of the instance it calls the reset function of the uas_prov module with the
 * corresponding instance data reference to reset the SafetyProvider state machine.
 * Error Handling:
 *  - Check of byUASError: The function only initializes the UAS instance if the value
 *    of byUASError is UAS_OK.
 *  - Check of function parameters, return values of function calls, wUASTIME_ErrorCount:
 *    If any error is detected the value of byUASError will be set to an error code (not FDL_OK).
 *    A fatal error exists if byUASError is not equal UAS_OK. In this case the function returns
 *    immediatly with this error code and does not initialize the UAS instance.
 *    Restart of UAS can only be done by Power OFF/ON.
 * Warnings:
 *  - The output of failsafe values to the application as well as the generation of safety
 *    messages with failsafe values is not assured if the return value is not equal UAS_OK.
 *    Therefore the UAS user has to enter a safe state.
 * \param[in]  uInstanceIndex - Index (Number) of the SafetyProvider instance
 * \param[in]  dwHandle       - Handle of of the SafetyProvider instance
 * \return UAS_OK if ok, error code if error occured
 */
UAS_UInt8 byUAS_ResetSafetyProvider(UAS_UInt16 uInstanceIndex, UAS_UInt32 dwHandle)
{
    UAS_UInt8 byRetVal = byUAS_PreCheck(uInstanceIndex);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_ResetSafetyProvider: Entry. uInstanceIndex = %04Xh, dwHandle = %08Xh", uInstanceIndex,
                     dwHandle);

#else

    (void)dwHandle;

#endif /* UASDEF_DBG */

    if (UAS_OK EQ byRetVal)
    {
        UASPROV_StateMachine_type* pzStateMachine = &(azUAS_ProvStateMachines[uInstanceIndex]);
        UASPROV_StateMachine_type* r_pzStateMachine = &(r_azUAS_ProvStateMachines[uInstanceIndex]);

        /* Reset the SafetyProvider instance ? */
        byRetVal = byUASPROV_Reset(pzStateMachine, r_pzStateMachine);
    } /* if */

    byRetVal = byUAS_PostCheck(byRetVal);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_ResetSafetyProvider: Exit. byRetVal = %02Xh", byRetVal);

#else

    (void)dwHandle;

#endif /* UASDEF_DBG */

    return byRetVal;

} /* end of function */

/*-------------------------------*/
/* S A F E T Y   C O N S U M E R */
/*-------------------------------*/

/**
 * Initialization of a SafetyConsumer instance
 * This function initializes an SafetyConsumer instance. After checking the plausibility of
 * instance number and the unambiguousness of the instance handle and IDs it calls the
 * initialization function of the uas_cons module with the corresponding instance data reference.
 * Error Handling:
 *  - Check of byUASError: The function only initializes the UAS instance if the value
 *    of byUASError is UAS_OK.
 *  - Check of function parameters, return values of function calls, wUASTIME_ErrorCount:
 *    If any error is detected the value of byUASError will be set to an error code (not FDL_OK).
 *    A fatal error exists if byUASError is not equal UAS_OK. In this case the function returns
 *    immediatly with this error code and does not initialize the UAS instance.
 *    Restart of UAS can only be done by Power OFF/ON.
 * Warnings:
 *  - The output of failsafe values to the application as well as the generation of safety
 *    messages with failsafe values is not assured if the return value is not equal UAS_OK.
 *    Therefore the UAS user has to enter a safe state.
 * \param[in/out]  pzInstanceData - Pointer to the data structure of the SafetyConsumer instance
 * \param[in]      uInstanceIndex - Index (Number) of the SafetyConsumer instance
 * \param[out]     pnResult       - Pointer to an output variable for the result of the parameter check
 * \return UAS_OK if ok, error code if error occured
 */
UAS_UInt8 byUAS_InitSafetyConsumer(UAS_SafetyConsumer_type* pzInstanceData,
                                   UAS_UInt16 uInstanceIndex,
                                   UAS_ParameterError_type* pnResult)
{
    UAS_UInt8 byRetVal;

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_InitSafetyConsumer: Entry. pzInstanceData = %p, uInstanceIndex = %04Xh", pzInstanceData,
                     uInstanceIndex);

#endif /* UASDEF_DBG */

    /* Initialize the redundant variables */
    if (1u EQ bUAS_FirstInit)
    {
        vUASRVAR_Init();
        vUAS_CheckDataTypes();
        bUAS_FirstInit = 0u;
    } /* if */

    byRetVal = byUAS_PreCheck(uInstanceIndex);

    if (UAS_OK EQ byRetVal)
    {
        /* Function parameter 1 invalid ? */
        if (pzInstanceData NOT_EQ & (azUAS_SafetyConsumers[uInstanceIndex]))
        {
            if (NULL EQ pzInstanceData)
            {
                byRetVal = UAS_POINTER_ERR;
            } /* if */
            else
            {
                byRetVal = UAS_FCT_PARAM_ERR;
            } /* else */
        }     /* if */

        /* Instance already initialized ? */
        else if ((0uL NOT_EQ azUAS_ConsStateMachines[uInstanceIndex].dwParamFcs) OR(
                     NULL NOT_EQ azUAS_ConsStateMachines[uInstanceIndex].pzInstanceData))
        {
            byRetVal = UAS_STATE_ERR;
        } /* else if */

        else
        {
            /* Check that the combination of Identifiers is not used by an other UAS rinstance */
            byRetVal = byUAS_UniqueConsIdentifiers(uInstanceIndex);

            /* Initiate the SafetyConsumer instance */
            if (UAS_OK EQ byRetVal)
            {
                byRetVal = byUASCONS_Init(&(azUAS_ConsStateMachines[uInstanceIndex]),
                                          &(r_azUAS_ConsStateMachines[uInstanceIndex]), pzInstanceData, pnResult);
            } /* if */
        }     /* else */
    }         /* if */

    byRetVal = byUAS_PostCheck(byRetVal);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_InitSafetyConsumer: Exit. byRetVal = %02Xh, *pnResult = %04Xh", byRetVal, *pnResult);

#endif /* UASDEF_DBG */

    return byRetVal;

} /* end of function */

/**
 * Change parameters of a SafetyConsumer instance
 * This function re-parameterizes an SafetyConsumer instance. After checking the plausibility of
 * instance number and initialization state of the instance it calls the change parameters function
 * of the uas_cons module with the corresponding instance data reference.
 * The input parameters contain the SPI parameters to be set. The output parameter contains the result
 * of the parameter check. If the result is UAS_PARAM_OK, the SPI parameters has been accepted and
 * the re-parameterization of the SafetyConsumer instance succeeded.
 * If the result is different from UAS_PARAM_OK, the SPI parameters has been rejected and the
 * parameterization of the SafetyConsumer failed. In this case the old parameters remain valid.
 * Notes:
 * - An instance must first be successfully initialized with byUAS_InitSafetyConsumer
 *   before it can be re-parameterized with byUAS_ChangeSafetyConsumerSPI.
 * Error Handling:
 *  - Check of byUASError: The function only initializes the UAS instance if the value
 *    of byUASError is UAS_OK.
 *  - Check of function parameters, return values of function calls, wUASTIME_ErrorCount:
 *    If any error is detected the value of byUASError will be set to an error code (not FDL_OK).
 *    A fatal error exists if byUASError is not equal UAS_OK. In this case the function returns
 *    immediatly with this error code and does not initialize the UAS instance.
 *    Restart of UAS can only be done by Power OFF/ON.
 * Warnings:
 *  - The output of failsafe values to the application as well as the generation of safety
 *    messages with failsafe values is not assured if the return value is not equal UAS_OK.
 *    Therefore the UAS user has to enter a safe state.
 * \param[in]  uInstanceIndex - Index (Number) of the SafetyConsumer instance
 * \param[in]  dwHandle       - Handle of of the SafetyConsumer instance
 * \param[in]  pzSPI          - Pointer to new SPI parameters of the SafetyConsumer instance
 * \param[out] pnResult       - Pointer to an output variable for the result of the parameter check
 * \return UAS_OK if ok, error code if error occured
 */
UAS_UInt8 byUAS_ChangeSafetyConsumerSPI(UAS_UInt16 uInstanceIndex,
                                        UAS_UInt32 dwHandle,
                                        UAS_SafetyConsumerSPI_type* pzSPI,
                                        UAS_ParameterError_type* pnResult)
{
    UAS_UInt8 byRetVal = byUAS_PreCheck(uInstanceIndex);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_ChangeSafetyConsumerSPI: Entry. uInstanceIndex = %04Xh, dwHandle = %08Xh", uInstanceIndex,
                     dwHandle);

#else

    (void)dwHandle;

#endif /* UASDEF_DBG */

    if (UAS_OK EQ byRetVal)
    {
        if ((NULL EQ pzSPI) OR(NULL EQ pnResult))
        {
            byRetVal = UAS_POINTER_ERR;
        } /* if */

        else
        {
            UASCONS_StateMachine_type* pzStateMachine = &(azUAS_ConsStateMachines[uInstanceIndex]);
            UASCONS_StateMachine_type* r_pzStateMachine = &(r_azUAS_ConsStateMachines[uInstanceIndex]);
            UAS_SafetyConsumer_type* pzInstanceData = &(azUAS_SafetyConsumers[uInstanceIndex]);

            /* Instance not initialized ? */
            if ((0uL EQ pzStateMachine->dwParamFcs) OR(pzInstanceData NOT_EQ pzStateMachine->pzInstanceData))
            {
                byRetVal = UAS_STATE_ERR;
            } /* if */

            else
            {
                /* Re-Parameterize the SafetyConsumer instance */
                byRetVal = byUASCONS_ChangeSPI(pzStateMachine, r_pzStateMachine, pzSPI, pnResult);
#ifdef UASDEF_DBG

                UASDEF_LOG_DEBUG("  byUAS_ChangeSafetyConsumerSPI: *pnResult = %04Xh", *pnResult);

#endif        /* UASDEF_DBG */
            } /* else */
        }     /* else */
    }         /* if */

    byRetVal = byUAS_PostCheck(byRetVal);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_ChangeSafetyConsumerSPI: Exit. byRetVal = %02Xh", byRetVal);

#endif /* UASDEF_DBG */

    return byRetVal;
}

/**
 * Change watchdog of a SafetyConsumer instance
 * This function re-parameterizes the SafetyConsumerTimeout of a SafetyConsumer instance. After checking
 * the plausibility of instance number and initialization state of the instance it calls the change watchdog
 * function of the uas_cons module with the corresponding instance data reference.
 * The input parameters contain the SafetyConsumerTimeout to be set. The output parameter contains the result
 * of the parameter check. If the result is UAS_PARAM_OK, the new watchdog value has been accepted and
 * the re-parameterization of the SafetyConsumer instance succeeded.
 * If the result is different from UAS_PARAM_OK, the new watchdog value has been rejected and the
 * parameterization of the SafetyConsumer failed. In this case the old value remain valid.
 * Notes:
 * - An instance must first be successfully initialized with byUAS_InitSafetyConsumer
 *   before it can be re-parameterized with byUAS_ChangeSafetyConsumerSPI.
 * Error Handling:
 *  - Check of byUASError: The function only initializes the UAS instance if the value
 *    of byUASError is UAS_OK.
 *  - Check of function parameters, return values of function calls, wUASTIME_ErrorCount:
 *    If any error is detected the value of byUASError will be set to an error code (not FDL_OK).
 *    A fatal error exists if byUASError is not equal UAS_OK. In this case the function returns
 *    immediatly with this error code and does not initialize the UAS instance.
 *    Restart of UAS can only be done by Power OFF/ON.
 * Warnings:
 *  - The output of failsafe values to the application as well as the generation of safety
 *    messages with failsafe values is not assured if the return value is not equal UAS_OK.
 *    Therefore the UAS user has to enter a safe state.
 * \param[in]  uInstanceIndex          - Index (Number) of the SafetyConsumer instance
 * \param[in]  dwHandle                - Handle of of the SafetyConsumer instance
 * \param[in]  dwSafetyConsumerTimeout - New timeout value of the SafetyConsumer instance
 * \param[out] pnResult                - Pointer to an output variable for the result of the parameter check
 * \return UAS_OK if ok, error code if error occured
 */
UAS_UInt8 byUAS_ChangeSafetyConsumerTimeout(UAS_UInt16 uInstanceIndex,
                                            UAS_UInt32 dwHandle,
                                            UAS_UInt32 dwSafetyConsumerTimeout,
                                            UAS_ParameterError_type* pnResult)
{
    UAS_UInt8 byRetVal = byUAS_PreCheck(uInstanceIndex);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_ChangeSafetyConsumerTimeout: Entry. uInstanceIndex = %04Xh, dwHandle = %08Xh",
                     uInstanceIndex, dwHandle);

#else

    (void)dwHandle;

#endif /* UASDEF_DBG */

    if (UAS_OK EQ byRetVal)
    {
        if (NULL EQ pnResult)
        {
            byRetVal = UAS_POINTER_ERR;
        } /* if */

        else
        {
            UASCONS_StateMachine_type* pzStateMachine = &(azUAS_ConsStateMachines[uInstanceIndex]);
            UASCONS_StateMachine_type* r_pzStateMachine = &(r_azUAS_ConsStateMachines[uInstanceIndex]);
            UAS_SafetyConsumer_type* pzInstanceData = &(azUAS_SafetyConsumers[uInstanceIndex]);

            /* Instance not initialized ? */
            if ((0uL EQ pzStateMachine->dwParamFcs) OR(pzInstanceData NOT_EQ pzStateMachine->pzInstanceData))
            {
                byRetVal = UAS_STATE_ERR;
            } /* if */

            else
            {
                /* Re-Parameterize the SafetyConsumer instance */
                byRetVal = byUASCONS_ChangeTimeout(pzStateMachine, r_pzStateMachine, dwSafetyConsumerTimeout, pnResult);
#ifdef UASDEF_DBG

                UASDEF_LOG_DEBUG("  byUAS_ChangeSafetyConsumerTimeout: *pnResult = %04Xh", *pnResult);

#endif        /* UASDEF_DBG */
            } /* else */
        }     /* else */
    }         /* if */

    byRetVal = byUAS_PostCheck(byRetVal);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_ChangeSafetyConsumerTimeout: Exit. byRetVal = %02Xh", byRetVal);

#endif /* UASDEF_DBG */

    return byRetVal;
}

/**
 * Start of a SafetyConsumer instance
 * This function starts a SafetyConsumer instance. After checking the plausibility of
 * instance number and initialization state of the instance it calls the start function
 * of the uas_cons module with the corresponding instance data reference to start the
 * SafetyConsumer state machine. Results of the start transitions will be written to
 * the SAPI output parameters of the instance.
 * Notes:
 * - An instance must first be successfully initialized with byUAS_InitSafetyConsumer
 *   before it can be started with byUAS_StartSafetyConsumer.
 * Error Handling:
 *  - Check of byUASError: The function only initializes the UAS instance if the value
 *    of byUASError is UAS_OK.
 *  - Check of function parameters, return values of function calls, wUASTIME_ErrorCount:
 *    If any error is detected the value of byUASError will be set to an error code (not FDL_OK).
 *    A fatal error exists if byUASError is not equal UAS_OK. In this case the function returns
 *    immediatly with this error code and does not initialize the UAS instance.
 *    Restart of UAS can only be done by Power OFF/ON.
 * Warnings:
 *  - The output of failsafe values to the application as well as the generation of safety
 *    messages with failsafe values is not assured if the return value is not equal UAS_OK.
 *    Therefore the UAS user has to enter a safe state.
 * \param[in]  uInstanceIndex - Index (Number) of the SafetyConsumer instance
 * \param[in]  dwHandle       - Handle of of the SafetyConsumer instance
 * \return UAS_OK if ok, error code if error occured
 */
UAS_UInt8 byUAS_StartSafetyConsumer(UAS_UInt16 uInstanceIndex, UAS_UInt32 dwHandle)
{
    UAS_UInt8 byRetVal = byUAS_PreCheck(uInstanceIndex);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_StartSafetyConsumer: Entry. uInstanceIndex = %04Xh, dwHandle = %08Xh", uInstanceIndex,
                     dwHandle);

#else

    (void)dwHandle;

#endif /* UASDEF_DBG */

    if (UAS_OK EQ byRetVal)
    {
        UASCONS_StateMachine_type* pzStateMachine = &(azUAS_ConsStateMachines[uInstanceIndex]);
        UASCONS_StateMachine_type* r_pzStateMachine = &(r_azUAS_ConsStateMachines[uInstanceIndex]);
        UAS_SafetyConsumer_type* pzInstanceData = &(azUAS_SafetyConsumers[uInstanceIndex]);

        /* Instance not initialized ? */
        if ((0uL EQ pzStateMachine->dwParamFcs) OR(pzInstanceData NOT_EQ pzStateMachine->pzInstanceData))
        {
            byRetVal = UAS_STATE_ERR;
        } /* if */

        else
        {
            /* Start the SafetyConsumer instance ? */
            byRetVal = byUASCONS_Start(pzStateMachine, r_pzStateMachine);
        } /* else */
    }     /* if */

    byRetVal = byUAS_PostCheck(byRetVal);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_StartSafetyConsumer: Exit. byRetVal = %02Xh", byRetVal);

#endif /* UASDEF_DBG */

    return byRetVal;

} /* end of function */

/**
 * Execution of a SafetyConsumer instance
 * This function executes a SafetyConsumer instance. It must be cyclically called!
 * After checking the plausibility of instance number, initialization state of the instance
 * and function invocation sequence it calls the execute function of the uas_cons module
 * with the corresponding instance data reference to execute the SafetyConsumer state machine.
 * Results of the executed transitions will be written to the SAPI output parameters of the instance.
 * In case of a communication error, a diagnostic code will be written to the DI output parameters
 * of the instance. The UAS application shall use this code to present a diagnostics message.
 * Notes:
 * - An instance must first be successfully initialized with byUAS_InitSafetyConsumer
 *   before it can be executed with byUAS_ExecuteSafetyConsumer.
 * Error Handling:
 *  - Check of byUASError: The function only initializes the UAS instance if the value
 *    of byUASError is UAS_OK.
 *  - Check of function parameters, return values of function calls, wUASTIME_ErrorCount:
 *    If any error is detected the value of byUASError will be set to an error code (not FDL_OK).
 *    A fatal error exists if byUASError is not equal UAS_OK. In this case the function returns
 *    immediatly with this error code and does not initialize the UAS instance.
 *    Restart of UAS can only be done by Power OFF/ON.
 * Warnings:
 *  - The output of failsafe values to the application as well as the generation of safety
 *    messages with failsafe values is not assured if the return value is not equal UAS_OK.
 *    Therefore the UAS user has to enter a safe state.
 * \param[in]  uInstanceIndex - Index (Number) of the SafetyConsumer instance
 * \param[in]  dwHandle       - Handle of of the SafetyConsumer instance
 * \return UAS_OK if ok, error code if error occured
 */
UAS_UInt8 byUAS_ExecuteSafetyConsumer(UAS_UInt16 uInstanceIndex, UAS_UInt32 dwHandle)
{
    UAS_UInt8 byRetVal = byUAS_PreCheck(uInstanceIndex);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_ExecuteSafetyConsumer: Entry. uInstanceIndex = %04Xh, dwHandle = %08Xh", uInstanceIndex,
                     dwHandle);

#else

    (void)dwHandle;

#endif /* UASDEF_DBG */

    if (UAS_OK EQ byRetVal)
    {
        UASCONS_StateMachine_type* pzStateMachine = &(azUAS_ConsStateMachines[uInstanceIndex]);
        UASCONS_StateMachine_type* r_pzStateMachine = &(r_azUAS_ConsStateMachines[uInstanceIndex]);
        UAS_SafetyConsumer_type* pzInstanceData = &(azUAS_SafetyConsumers[uInstanceIndex]);

        /* Instance not initialized ? */
        if ((0uL EQ pzStateMachine->dwParamFcs) OR(pzInstanceData NOT_EQ pzStateMachine->pzInstanceData))
        {
            byRetVal = UAS_STATE_ERR;
        } /* if */

        else
        {
            /* Check the function invocation sequence of the UAS user */
            if ((1u EQ pzInstanceData->bAppDone) AND(1u EQ pzInstanceData->bCommDone))
            {
                /* Reset Communication-Done flag and Application-Done flag */
                pzInstanceData->bCommDone = 0u;
                pzInstanceData->bAppDone = 0u;
                byRetVal = UAS_OK;
            } /* if */
            else
            {
                byRetVal = UAS_STATE_ERR;
            } /* else */
        }     /* else */

        if (UAS_OK EQ byRetVal)
        {
            /* Execute the SafetyConsumer instance ? */
            byRetVal = byUASCONS_Execute(pzStateMachine, r_pzStateMachine);
        } /* if */
    }     /* if */

    byRetVal = byUAS_PostCheck(byRetVal);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_ExecuteSafetyConsumer: Exit. byRetVal = %02Xh", byRetVal);

#endif /* UASDEF_DBG */

    return byRetVal;

} /* end of function */

/**
 * Stop of a SafetyConsumer instance
 * This function stops a SafetyConsumer instance. After checking the plausibility of
 * instance number and initialization state of the instance it calls the stop function
 * of the uas_cons module with the corresponding instance data reference to stop the
 * SafetyConsumer state machine. The SAPI output parameters of the instance will be resetted.
 * Notes:
 * - An instance must first be successfully initialized with byUAS_InitSafetyConsumer
 *   before it can be stopped with byUAS_StopSafetyConsumer.
 * Error Handling:
 *  - Check of byUASError: The function only initializes the UAS instance if the value
 *    of byUASError is UAS_OK.
 *  - Check of function parameters, return values of function calls, wUASTIME_ErrorCount:
 *    If any error is detected the value of byUASError will be set to an error code (not FDL_OK).
 *    A fatal error exists if byUASError is not equal UAS_OK. In this case the function returns
 *    immediatly with this error code and does not initialize the UAS instance.
 *    Restart of UAS can only be done by Power OFF/ON.
 * Warnings:
 *  - The output of failsafe values to the application as well as the generation of safety
 *    messages with failsafe values is not assured if the return value is not equal UAS_OK.
 *    Therefore the UAS user has to enter a safe state.
 * \param[in]  uInstanceIndex - Index (Number) of the SafetyConsumer instance
 * \param[in]  dwHandle       - Handle of of the SafetyConsumer instance
 * \return UAS_OK if ok, error code if error occured
 */
UAS_UInt8 byUAS_StopSafetyConsumer(UAS_UInt16 uInstanceIndex, UAS_UInt32 dwHandle)
{
    UAS_UInt8 byRetVal = byUAS_PreCheck(uInstanceIndex);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_StopSafetyConsumer: Entry. uInstanceIndex = %04Xh, dwHandle = %08Xh", uInstanceIndex,
                     dwHandle);

#else

    (void)dwHandle;

#endif /* UASDEF_DBG */

    if (UAS_OK EQ byRetVal)
    {
        UASCONS_StateMachine_type* pzStateMachine = &(azUAS_ConsStateMachines[uInstanceIndex]);
        UASCONS_StateMachine_type* r_pzStateMachine = &(r_azUAS_ConsStateMachines[uInstanceIndex]);
        UAS_SafetyConsumer_type* pzInstanceData = &(azUAS_SafetyConsumers[uInstanceIndex]);

        /* Instance not initialized ? */
        if ((0uL EQ pzStateMachine->dwParamFcs) OR(pzInstanceData NOT_EQ pzStateMachine->pzInstanceData))
        {
            byRetVal = UAS_STATE_ERR;
        } /* if */

        else
        {
            /* Start the SafetyConsumer instance ? */
            byRetVal = byUASCONS_Stop(pzStateMachine, r_pzStateMachine);
        } /* else */
    }     /* if */

    byRetVal = byUAS_PostCheck(byRetVal);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_StopSafetyConsumer: Exit. byRetVal = %02Xh", byRetVal);

#endif /* UASDEF_DBG */

    return byRetVal;

} /* end of function */

/**
 * De-initialization of a SafetyConsumer instance
 * This function de-initializes a SafetyConsumer instance. After checking the plausibility of
 * instance number of the instance it calls the reset function of the uas_cons module with the
 * corresponding instance data reference to reset the SafetyConsumer state machine.
 * Error Handling:
 *  - Check of byUASError: The function only initializes the UAS instance if the value
 *    of byUASError is UAS_OK.
 *  - Check of function parameters, return values of function calls, wUASTIME_ErrorCount:
 *    If any error is detected the value of byUASError will be set to an error code (not FDL_OK).
 *    A fatal error exists if byUASError is not equal UAS_OK. In this case the function returns
 *    immediatly with this error code and does not initialize the UAS instance.
 *    Restart of UAS can only be done by Power OFF/ON.
 * Warnings:
 *  - The output of failsafe values to the application as well as the generation of safety
 *    messages with failsafe values is not assured if the return value is not equal UAS_OK.
 *    Therefore the UAS user has to enter a safe state.
 * \param[in]  uInstanceIndex - Index (Number) of the SafetyConsumer instance
 * \param[in]  dwHandle       - Handle of of the SafetyConsumer instance
 * \return UAS_OK if ok, error code if error occured
 */
UAS_UInt8 byUAS_ResetSafetyConsumer(UAS_UInt16 uInstanceIndex, UAS_UInt32 dwHandle)
{
    UAS_UInt8 byRetVal = byUAS_PreCheck(uInstanceIndex);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_ResetSafetyConsumer: Entry. uInstanceIndex = %04Xh, dwHandle = %08Xh", uInstanceIndex,
                     dwHandle);

#else

    (void)dwHandle;

#endif /* UASDEF_DBG */

    if (UAS_OK EQ byRetVal)
    {
        UASCONS_StateMachine_type* pzStateMachine = &(azUAS_ConsStateMachines[uInstanceIndex]);
        UASCONS_StateMachine_type* r_pzStateMachine = &(r_azUAS_ConsStateMachines[uInstanceIndex]);

        /* Reset the SafetyConsider instance ? */
        byRetVal = byUASCONS_Reset(pzStateMachine, r_pzStateMachine);
    } /* if */

    byRetVal = byUAS_PostCheck(byRetVal);

#ifdef UASDEF_DBG

    UASDEF_LOG_DEBUG("  byUAS_ResetSafetyConsumer: Exit. byRetVal = %02Xh", byRetVal);

#endif /* UASDEF_DBG */

    return byRetVal;

} /* end of function */

/*--------------------------------------------------------------------------*/
/*************** I M P L E M E N T A T I O N   ( L O C A L S ) **************/
/*--------------------------------------------------------------------------*/

/**
 * De-initialization of all UAS instances
 * This function de-initializes all UAS instances.
 */
static void vUAS_ResetAllInstances(void)
{
    UAS_UInt32 dwInstIndex;
    UAS_UInt16 wDataIndex;

    /* Reset all SafetyProviders */
    for (dwInstIndex = 0uL; dwInstIndex < UASDEF_MAX_SAFETYPROVIDERS; dwInstIndex++)
    {
        UASPROV_StateMachine_type* pzStateMachine = &(azUAS_ProvStateMachines[dwInstIndex]);
        UASPROV_StateMachine_type* r_pzStateMachine = &(r_azUAS_ProvStateMachines[dwInstIndex]);
        UAS_SafetyProviderSAPIO_type* pzOutputSAPI = &(azUAS_SafetyProviders[dwInstIndex].zOutputSAPI);

        (void) byUASPROV_Reset(pzStateMachine, r_pzStateMachine);
        pzOutputSAPI->bOperatorAckRequested = 0u;
        pzOutputSAPI->dwSafetyConsumerId = 0uL;
        pzOutputSAPI->dwMonitoringNumber = 0uL;
    } /* for */

    /* Reset all SafetyConsumers */
    for (dwInstIndex = 0uL; dwInstIndex < UASDEF_MAX_SAFETYCONSUMERS; dwInstIndex++)
    {
        UASCONS_StateMachine_type* pzStateMachine = &(azUAS_ConsStateMachines[dwInstIndex]);
        UASCONS_StateMachine_type* r_pzStateMachine = &(r_azUAS_ConsStateMachines[dwInstIndex]);
        UAS_SafetyConsumerSAPIO_type* pzOutputSAPI = &(azUAS_SafetyConsumers[dwInstIndex].zOutputSAPI);

        (void) byUASCONS_Reset(pzStateMachine, r_pzStateMachine);
        for (wDataIndex = 0u; wDataIndex < azUAS_SafetyConsumers[dwInstIndex].wSafetyDataLength; wDataIndex++)
        {
            pzOutputSAPI->pbySerializedSafetyData[wDataIndex] = UAS_FSV;
        } /* for */
        for (wDataIndex = 0u; wDataIndex < azUAS_SafetyConsumers[dwInstIndex].wNonSafetyDataLength; wDataIndex++)
        {
            pzOutputSAPI->pbySerializedNonSafetyData[wDataIndex] = UAS_FSV;
        } /* for */
        pzOutputSAPI->bFsvActivated = 1u;
        pzOutputSAPI->bOperatorAckRequested = 0u;
        pzOutputSAPI->bOperatorAckProvider = 0u;
        pzOutputSAPI->bTestModeActivated = 0u;
    } /* for */
} /* end of function */

/**
 * Check of unique identifiers
 * This function checks that the instance handle and the combination of SafetyProviderID, SafetyBaseID
 * and SafetyConsumerID in the parameter set is unique, i.e. not used in any other parameter set.
 * \param[in]  uInstanceIndex - Index (Number) of the SafetyProvider instance
 * \return UAS_OK if ok, error code if not (already used)
 */
static UAS_UInt8 byUAS_UniqueProvIdentifiers(
    /** IN: Index (Number) of the SafetyProvider instance */
    UAS_UInt16 uInstanceIndex)
{
    /* Return value of the function */
    UAS_UInt8 byRetVal = UAS_OK;

    /* Index for the loop over UAS instance data sets */
    UAS_UInt32 dwIndex;

    /* Pointer to the new SafetyProvider instance data sets */
    UAS_SafetyProvider_type* pzNewSafetyProvider = &(azUAS_SafetyProviders[uInstanceIndex]);

    /* Check all SafetyConsumer instance data sets */
    for (dwIndex = 0uL; (UAS_OK EQ byRetVal) AND(dwIndex < UASDEF_MAX_SAFETYCONSUMERS); dwIndex++)
    {
        /* Pointer to the SafetyConsumer instance data sets */
        UASCONS_StateMachine_type* pzStateMachine = &(azUAS_ConsStateMachines[dwIndex]);
        UASCONS_StateMachine_type* r_pzStateMachine = &(r_azUAS_ConsStateMachines[dwIndex]);

        if (UASRVAR_VALID_POINTER(pzStateMachine->pzInstanceData))
        {
            /* Check if SafetyConsumer instance data set is used */
            if (NULL NOT_EQ pzStateMachine->pzInstanceData)
            {
                /* Compare the handles */
                if (pzNewSafetyProvider->dwHandle EQ pzStateMachine->pzInstanceData->dwHandle)
                {
                    /* Handle always used! */
                    byRetVal = UAS_FCT_PARAM_ERR;
                } /* if */
            }     /* if */
        }         /* if */
        else
        {
            /* Faulty instance data! */
            byRetVal = UAS_SOFT_ERR;
        } /* else */
    }     /* for */

    /* Check all SafetyProvider instance data sets */
    for (dwIndex = 0uL; (UAS_OK EQ byRetVal) AND(dwIndex < UASDEF_MAX_SAFETYPROVIDERS); dwIndex++)
    {
        /* Pointer to the SafetyProvider instance data sets */
        UASPROV_StateMachine_type* pzStateMachine = &(azUAS_ProvStateMachines[dwIndex]);
        UASPROV_StateMachine_type* r_pzStateMachine = &(r_azUAS_ProvStateMachines[dwIndex]);

        if (UASRVAR_VALID_POINTER(pzStateMachine->pzInstanceData))
        {
            /* Check if SafetyProvider instance data set is used */
            if (NULL NOT_EQ pzStateMachine->pzInstanceData)
            {
                /* Compare the handles */
                if (pzNewSafetyProvider->dwHandle EQ pzStateMachine->pzInstanceData->dwHandle)
                {
                    /* Handle always used! */
                    byRetVal = UAS_FCT_PARAM_ERR;
                } /* if */
                /* Compare the safety identifiers */
                else if ((pzNewSafetyProvider->zSPI.dwSafetyProviderId EQ pzStateMachine->pzInstanceData->zSPI
                              .dwSafetyProviderId) AND(pzNewSafetyProvider->zSPI.zSafetyBaseId
                                                           .dwData1 EQ pzStateMachine->pzInstanceData->zSPI
                                                           .zSafetyBaseId.dwData1)
                             AND(pzNewSafetyProvider->zSPI.zSafetyBaseId.wData2 EQ
                                     pzStateMachine->pzInstanceData->zSPI.zSafetyBaseId
                                         .wData2) AND(pzNewSafetyProvider->zSPI.zSafetyBaseId.wData3 EQ
                                                          pzStateMachine->pzInstanceData->zSPI.zSafetyBaseId.wData3)
                                 AND(pzNewSafetyProvider->zSPI.zSafetyBaseId.abyData4[0] EQ pzStateMachine
                                         ->pzInstanceData->zSPI.zSafetyBaseId.abyData4[0])
                                     AND(pzNewSafetyProvider->zSPI.zSafetyBaseId.abyData4[1] EQ pzStateMachine
                                             ->pzInstanceData->zSPI.zSafetyBaseId.abyData4[2])
                                         AND(pzNewSafetyProvider->zSPI.zSafetyBaseId.abyData4[2] EQ pzStateMachine
                                                 ->pzInstanceData->zSPI.zSafetyBaseId.abyData4[3])
                                             AND(pzNewSafetyProvider->zSPI.zSafetyBaseId.abyData4[3] EQ pzStateMachine
                                                     ->pzInstanceData->zSPI.zSafetyBaseId.abyData4[4])
                                                 AND(pzNewSafetyProvider->zSPI.zSafetyBaseId
                                                         .abyData4[4] EQ pzStateMachine->pzInstanceData->zSPI
                                                         .zSafetyBaseId.abyData4[5])
                                                     AND(pzNewSafetyProvider->zSPI.zSafetyBaseId
                                                             .abyData4[5] EQ pzStateMachine->pzInstanceData->zSPI
                                                             .zSafetyBaseId.abyData4[6])
                                                         AND(pzNewSafetyProvider->zSPI.zSafetyBaseId
                                                                 .abyData4[7] EQ pzStateMachine->pzInstanceData->zSPI
                                                                 .zSafetyBaseId.abyData4[7]))
                {
                    /* IDs always used! */
                    byRetVal = UAS_FCT_PARAM_ERR;
                } /* else if */
                else
                {
                    /* Other IDs used, do nothing */
                } /* else */
            }     /* if */
        }         /* if */
        else
        {
            /* Faulty instance data! */
            byRetVal = UAS_SOFT_ERR;
        } /* else */
    }     /* for */

    /* Return the result */
    return byRetVal;

} /* end of function */

/**
 * Check of unique identifiers
 * This function checks that the instance handle and the combination of SafetyProviderID, SafetyBaseID
 * and SafetyConsumerID in the parameter set is unique, i.e. not used in any other parameter set.
 * \param[in]  uInstanceIndex - Index (Number) of the SafetyProvider instance
 * \return UAS_OK if ok, error code if not (already used)
 */
static UAS_UInt8 byUAS_UniqueConsIdentifiers(
    /** IN: Index (Number) of the SafetyConsumer instance */
    UAS_UInt16 uInstanceIndex)
{
    /* Return value of the function */
    UAS_UInt8 byRetVal = UAS_OK;

    /* Index for the loop over UAS instance data sets */
    UAS_UInt32 dwIndex;

    /* Pointer to the new SafetyConsumer instance data sets */
    UAS_SafetyConsumer_type* pzNewSafetyConsumer = &(azUAS_SafetyConsumers[uInstanceIndex]);

    /* Check all SafetyProvider instance data sets */
    for (dwIndex = 0uL; (UAS_OK EQ byRetVal) AND(dwIndex < UASDEF_MAX_SAFETYPROVIDERS); dwIndex++)
    {
        /* Pointer to the SafetyProvider instance data sets */
        UASPROV_StateMachine_type* pzStateMachine = &(azUAS_ProvStateMachines[dwIndex]);
        UASPROV_StateMachine_type* r_pzStateMachine = &(r_azUAS_ProvStateMachines[dwIndex]);

        if (UASRVAR_VALID_POINTER(pzStateMachine->pzInstanceData))
        {
            /* Check if SafetyProvider instance data set is used */
            if (NULL NOT_EQ pzStateMachine->pzInstanceData)
            {
                /* Compare the handles */
                if (pzNewSafetyConsumer->dwHandle EQ pzStateMachine->pzInstanceData->dwHandle)
                {
                    /* Handle always used! */
                    byRetVal = UAS_FCT_PARAM_ERR;
                } /* if */
            }     /* if */
        }         /* if */
        else
        {
            /* Faulty instance data! */
            byRetVal = UAS_SOFT_ERR;
        } /* else */
    }     /* for */

    /* Check all SafetyConsumer instance data sets */
    for (dwIndex = 0uL; (UAS_OK EQ byRetVal) AND(dwIndex < UASDEF_MAX_SAFETYCONSUMERS); dwIndex++)
    {
        /* Pointer to the SafetyConsumer instance data sets */
        UASCONS_StateMachine_type* pzStateMachine = &(azUAS_ConsStateMachines[dwIndex]);
        UASCONS_StateMachine_type* r_pzStateMachine = &(r_azUAS_ConsStateMachines[dwIndex]);

        if (UASRVAR_VALID_POINTER(pzStateMachine->pzInstanceData))
        {
            /* Check if SafetyConsumer instance data set is used */
            if (NULL NOT_EQ pzStateMachine->pzInstanceData)
            {
                /* Compare the handles */
                if (pzNewSafetyConsumer->dwHandle EQ pzStateMachine->pzInstanceData->dwHandle)
                {
                    /* Handle always used! */
                    byRetVal = UAS_FCT_PARAM_ERR;
                } /* if */
                  /* Compare the safety identifiers */
                else if ((pzNewSafetyConsumer->zSPI.dwSafetyProviderId EQ pzStateMachine->pzInstanceData->zSPI
                              .dwSafetyProviderId) AND(pzNewSafetyConsumer->zSPI.dwSafetyConsumerId EQ
                                                           pzStateMachine->pzInstanceData->zSPI.dwSafetyConsumerId)
                             AND(pzNewSafetyConsumer->zSPI.zSafetyBaseId.dwData1 EQ
                                     pzStateMachine->pzInstanceData->zSPI.zSafetyBaseId
                                         .dwData1) AND(pzNewSafetyConsumer->zSPI.zSafetyBaseId.wData2 EQ
                                                           pzStateMachine->pzInstanceData->zSPI.zSafetyBaseId.wData2)
                                 AND(pzNewSafetyConsumer->zSPI.zSafetyBaseId.wData3 EQ
                                         pzStateMachine->pzInstanceData->zSPI.zSafetyBaseId.wData3)
                                     AND(pzNewSafetyConsumer->zSPI.zSafetyBaseId.abyData4[0] EQ pzStateMachine
                                             ->pzInstanceData->zSPI.zSafetyBaseId.abyData4[0])
                                         AND(pzNewSafetyConsumer->zSPI.zSafetyBaseId.abyData4[1] EQ pzStateMachine
                                                 ->pzInstanceData->zSPI.zSafetyBaseId.abyData4[1])
                                             AND(pzNewSafetyConsumer->zSPI.zSafetyBaseId.abyData4[2] EQ pzStateMachine
                                                     ->pzInstanceData->zSPI.zSafetyBaseId.abyData4[2])
                                                 AND(pzNewSafetyConsumer->zSPI.zSafetyBaseId
                                                         .abyData4[3] EQ pzStateMachine->pzInstanceData->zSPI
                                                         .zSafetyBaseId.abyData4[3])
                                                     AND(pzNewSafetyConsumer->zSPI.zSafetyBaseId
                                                             .abyData4[4] EQ pzStateMachine->pzInstanceData->zSPI
                                                             .zSafetyBaseId.abyData4[4])
                                                         AND(pzNewSafetyConsumer->zSPI.zSafetyBaseId
                                                                 .abyData4[5] EQ pzStateMachine->pzInstanceData->zSPI
                                                                 .zSafetyBaseId.abyData4[5])
                                                             AND(pzNewSafetyConsumer->zSPI.zSafetyBaseId
                                                                     .abyData4[6] EQ pzStateMachine->pzInstanceData
                                                                     ->zSPI.zSafetyBaseId.abyData4[6])
                                                                 AND(pzNewSafetyConsumer->zSPI.zSafetyBaseId
                                                                         .abyData4[7] EQ pzStateMachine->pzInstanceData
                                                                         ->zSPI.zSafetyBaseId.abyData4[7]))
                {
                    /* Safety identifiers always used! */
                    byRetVal = UAS_FCT_PARAM_ERR;
                } /* else if */
                else
                {
                    /* Other IDs used, do nothing */
                } /* else */
            }     /* if */
        }         /* if */
        else
        {
            /* Faulty instance data! */
            byRetVal = UAS_SOFT_ERR;
        } /* else */
    }     /* for */

    /* Return the result */
    return byRetVal;

} /* end of function */

/**
 * Common checks before calling a UAS instance
 * This function checks common errors before calling the UAS state machine,
 * e.g. stored error code.
 * \param[in]  uInstanceIndex - Index (Number) of the SafetyProvider instance
 * \return UAS_OK if ok, error code if error occured
 */
static UAS_UInt8 byUAS_PreCheck(UAS_UInt16 uInstanceIndex)
{
    UAS_UInt8 byRetVal = UAS_DEFAULT_ERR;

    // Note SYSTEREL: The following line is correct but produced an unavoidable GCC warning...
    // if ( UASRVAR_INVALID_USIGN8( byUAS_Error ) )
    const unsigned char tmp8 = (unsigned char) (~(r_byUAS_Error));
    if (byUAS_Error != tmp8)
    {
        /* Inconsistent error code --> return soft error! */
        byRetVal = UAS_SOFT_ERR;
        UASRVAR_SET_USIGN8(byUAS_Error, byRetVal);
    } /* if */

    else if (UAS_OK NOT_EQ byUAS_Error)
    {
        /* Function invocation with stored error code --> return the stored error code! */
        byRetVal = byUAS_Error;
    } /* else if */

    else if (UASDEF_MAX_SAFETYPROVIDERS <= uInstanceIndex)
    {
        /* Function parameter invalid --> return error code! */
        byRetVal = UAS_FCT_PARAM_ERR;
    } /* else if */

    else
    {
        byRetVal = UAS_OK;
    } /* else */

    return byRetVal;

} /* end of function */

/**
 * Common checks after calling a UAS instance
 * This function checks common errors after calling the UAS state machine,
 * e.g. timer errors.
 * \param[in]  uInstanceIndex - Index (Number) of the SafetyProvider instance
 * \return UAS_OK if ok, error code if error occured
 */
static UAS_UInt8 byUAS_PostCheck(
    /** IN: Return value with error code of the calling function */
    UAS_UInt8 byRetVal)
{
    /* Timer error ocurred ? */
    if ((UAS_OK EQ byRetVal) AND(UASRVAR_INVALID_ERROR_COUNT(wUASTIME_ErrorCount)))
    {
        byRetVal = UAS_TIMER_ERR;
    } /* if */

    /* Any error detected ? */
    if (UAS_OK NOT_EQ byRetVal)
    {
        /* Store the error code */
        UASRVAR_SET_USIGN8(byUAS_Error, byRetVal);
        /* Reset all UAS instances */
        vUAS_ResetAllInstances();
    } /* if */

    return byRetVal;

} /* end of function */

/**
 * Check of basic data type definitions
 * This function checks the basic data type definitions.
 */
static void vUAS_CheckDataTypes(void)
{
    if ((1 NOT_EQ sizeof(UAS_Bool)) OR(1 NOT_EQ sizeof(UAS_Char)) OR(1 NOT_EQ sizeof(UAS_Int8))
            OR(2 NOT_EQ sizeof(UAS_Int16)) OR(4 NOT_EQ sizeof(UAS_Int32)) OR(8 NOT_EQ sizeof(UAS_Int64))
                OR(1 NOT_EQ sizeof(UAS_UInt8)) OR(2 NOT_EQ sizeof(UAS_UInt16)) OR(4 NOT_EQ sizeof(UAS_UInt32))
                    OR(8 NOT_EQ sizeof(UAS_UInt64)) OR(sizeof(UAS_Int8*) NOT_EQ sizeof(UAS_INVERSE_PTR)))
    {
        UASRVAR_SET_USIGN8(byUAS_Error, UAS_MEMORY_ERR);
    } /* if */
}

/* end of file */
