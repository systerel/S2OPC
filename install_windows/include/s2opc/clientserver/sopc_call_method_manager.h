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

/** \file
 *
 * \brief Contains the types to be used by the method call manager to configure the Call service
 *
 */

#ifndef SOPC_CALL_METHOD_MANAGER_H_
#define SOPC_CALL_METHOD_MANAGER_H_

#include "sopc_builtintypes.h"
#include "sopc_service_call_context.h"

typedef struct SOPC_MethodCallManager SOPC_MethodCallManager;
typedef struct SOPC_MethodCallFunc SOPC_MethodCallFunc;

/**
 * \brief  Type of the function to call associated to a method
 *
 * Method call service verifies the input arguments before calling a SOPC_MethodCallFunc_Ptr function.
 *
 * Note: a ByteString could be provided instead of an expected array of Byte and conversely (See Spec 1.03 part 4 -
 * table 66)
 *
 * \param callContextPtr  context provided by server on connection/session associated to method call
 * \param objectId        a valid pointer to the object on which the method is called or a type if the method is static
 * \param nbInputArgs     number of input argument
 * \param inputArgs       an array of input argument of the method. The size is nbInputArgs.
 * \param nbOutputArgs    a valid pointer in which the number of output argument is written by the function
 * \param outputArgs      a valid pointer to an SOPC_Variant[] in which the output arguments are allocated and written
 *                        by the function
 *
 * \return status code of the function. Should be SOPC_STATUS_OK if succeeded.
 */
typedef SOPC_StatusCode SOPC_MethodCallFunc_Ptr(const SOPC_CallContext* callContextPtr,
                                                const SOPC_NodeId* objectId,
                                                uint32_t nbInputArgs,
                                                const SOPC_Variant* inputArgs,
                                                uint32_t* nbOutputArgs,
                                                SOPC_Variant** outputArgs,
                                                void* param);

/**
 * \brief  Type of the function to free param of SOPC_MethodCallFunc.
 *
 * \param data           a pointer to the object to free. Can be NULL
 */
typedef void SOPC_MethodCallFunc_Free_Func(void* data);

/**
 * \brief  Object to describe of function associated to a method and user parameter
 */
struct SOPC_MethodCallFunc
{
    /**
     * \brief a pointer on a function to clear pParam. Can be NULL
     */
    SOPC_MethodCallFunc_Free_Func* pFnFree;

    /**
     * \brief a valid pointer on the function to call
     */
    SOPC_MethodCallFunc_Ptr* pMethodFunc;

    /**
     * \brief parameter to give when call pMethodFunc. Can be NULL
     */
    void* pParam;
};

/* Type of the function to free SOPC_MethodCallManager internal data */
typedef void SOPC_MethodCallManager_Free_Func(void* data);

/* Type of the function to get a C function associated to a SOPC_NodeId of a Method */
typedef SOPC_MethodCallFunc* SOPC_MethodCallManager_Get_Func(SOPC_MethodCallManager* mcm, SOPC_NodeId* methodId);

/**
 * \brief The SOPC_MethodCallManager object defines the common interface for the method manager.
 *
 * The ownership of the output data of functions moved to S2OPC toolkit
 *
 * User can use the SOPC toolkit basic implementation of this interface by calling
 * SOPC_MethodCallManager_Create() and SOPC_MethodCallManager_AddMethod() functions.
 * User can implement its own SOPC_MethodCallManager_Get_Func and pUserData for specific uses.
 */
struct SOPC_MethodCallManager
{
    /**
     * \brief The free function, called upon generic SOPC_MethodCallManager destruction.
     * \param mcm     a valid pointer to the SOPC_MethodCallManager.
     */
    SOPC_MethodCallManager_Free_Func* const pFnFree;

    /**
     * \brief Function to get a function pointer corresponding to an object Method of the Address Space.
     *
     * \param mcm        a valid pointer to a SOPC_MethodCallManager.
     *
     * \param methodId   a valid pointer to the SOPC_NodeId of a method.
     *
     * \return           a valid function pointer (SOPC_MethodCallManager_Free_Func) or NULL if there is no
     * implementation for the given methodId.
     */
    SOPC_MethodCallManager_Get_Func* const pFnGetMethod;

    /**
     * \brief internal data of the manager.
     */
    void* pUserData;
};

/**
 * \brief Provide a basic implementation of MethodCallManager.
 * This implementation can be used with SOPC_MethodCallManager_AddMethod to add method
 *
 * \return a valid SOPC_MethodCallManager pointer or NULL on memory allocation failure.
 */
SOPC_MethodCallManager* SOPC_MethodCallManager_Create(void);

/**
 * \brief Free MethodCallManager created with SOPC_MethodCallManager_Create.
 *
 */
void SOPC_MethodCallManager_Free(SOPC_MethodCallManager* mcm);

/**
 * \brief Associate a C function to a NodeId of a Method.
 * This function should be used only with the basic implementation of SOPC_MethodCallManager provided by the toolkit.
 *
 * \param mcm   a valid pointer on a SOPC_MethodCallManager returned by SOPC_MethodCallManager_Create().
 * \param methodId        a valid pointer on a SOPC_NodeId of the method
 *                        (\p mcm will manage its deallocation, do not reuse this nodeId after call)
 * \param methodFunc      a valid pointer on a C function to associate with the given methodId.
 * \param param           a pointer on data to give as parameter when call methodFunc. Can be NULL.
 * \param fnFree          a pointer on a C function to free param. Can be NULL.
 *
 * \return SOPC_STATUS_OK when the function succeed, SOPC_STATUS_INVALID_PARAMETERS or SOPC_STATUS_OUT_OF_MEMORY.
 */
SOPC_ReturnStatus SOPC_MethodCallManager_AddMethod(SOPC_MethodCallManager* mcm,
                                                   SOPC_NodeId* methodId,
                                                   SOPC_MethodCallFunc_Ptr* methodFunc,
                                                   void* param,
                                                   SOPC_MethodCallFunc_Free_Func* fnFree);

#endif /* SOPC_CALL_METHOD_MANAGER_H_ */
