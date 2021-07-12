/**
 * OPC Foundation OPC UA Safety Mapper
 *
 * \file
 * \author
 *    Copyright 2021 (c) ifak e.V.Magdeburg
 *    Copyright 2021 (c) Tianzhe Yu
 *    Copyright 2021 (c) Matthias Riedl
 *    Copyright 2021 (c) Elke Hintze
 *
 * \brief Interface of the UAM manager
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines the interface of the OPC UA Mapper manager.
 *
 * Safety-Related: no
 */

#ifndef __UAM_MANAGER_H
#define __UAM_MANAGER_H

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uam_types.h"

/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

#ifdef __cplusplus
extern "C"
{
#endif

    /*-------------*/
    /*  T Y P E S  */
    /*-------------*/

    /**
     * \brief Function type for the initialization of the UA Mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_Init_ftype(void);

    /**
     * \brief Function type for reading the instance list of the UA Mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured or mapper has more instance than input max
     * number.
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_GetInstanceList_ftype(
        const char* pszFileName,   /**< [in] Path and name of a config file         */
        uint16* wNumberOfInstance, /**< [in,out]  (Max) number of mapper instances, return actual size of returned list.
                                      if mapper has more instance than maxnumber, returns an error code and maxnumber */
        UAM_InstanceData_type* pzListOfInstances, /**< [out] List of mapper instances               */
        uint32 dwTimeout                          /**< [in] Timeout in seconds for online browsing */
    );

    /**
     * \brief Function type for resetting of the OPC UA Mapper
     * \return  UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_Reset_ftype(void);

    /**
     * \brief Function type for starting a Mapper instance
     * \return  UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_Start_ftype(
        const UAM_InstanceId_type dwInstanceId /**< [in] Instance ID */
    );

    /**
     * \brief Function type for stopping a Mapper instance
     * \return UAM_ERRORCODE_OK if succeed Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_Stop_ftype(
        const UAM_InstanceId_type dwInstanceId /**< [in] Instance ID */
    );

    /**
     * \brief Function type for querying an error string
     * \return A human readable string representing the error code
     */
    UAM_API_TYPE typedef const char* WINAPI UAM_ErrorCodeToString_ftype(
        const UAM_ErrorCode dwErrorCode /**< [in] Error code */
    );

    /*---------------------*/
    /*  F U N C T I O N S  */
    /*---------------------*/

#ifdef _WIN32
    /**
     * Initialization of the UA Mapper
     */
    UAM_Init_ftype UAM_Init;

    /**
     * Read the instance list of the UA Mapper
     */
    UAM_GetInstanceList_ftype UAM_GetInstanceList;

    /**
     * Reset of the OPC UA Mapper
     */
    UAM_Reset_ftype UAM_Reset;

    /**
     * Start a Mapper instance
     */
    UAM_Start_ftype UAM_Start;

    /**
     * Stop a Mapper instance
     */
    UAM_Start_ftype UAM_Stop;

    /**
     * Query an error string
     */
    UAM_ErrorCodeToString_ftype UAM_ErrorCodeToString;
#endif
#ifdef __GNUC__
    /**
     * \brief Function type for the initialization of the UA Mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API UAM_ErrorCode WINAPI UAM_Init(void);

    /**
     * \brief Function type for reading the instance list of the UA Mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured or mapper has more instance than input max
     * number.
     */
    UAM_API UAM_ErrorCode WINAPI UAM_GetInstanceList(
        const char* pszFileName,   /**< [in] Path and name of a config file         */
        uint16* wNumberOfInstance, /**< [in,out] input (Max) number of mapper instances, return actual size of returned
                                      list. if mapper has more instance than Maxnumber, returns an error code and
                                      Maxnumber of instances */
        UAM_InstanceData_type* pzListOfInstances, /**< [out] List of mapper instances               */
        uint32 dwTimeout                          /**< [in] Timeout in seconds for online browsing */
    );

    /**
     * \brief Function type for resetting of the OPC UA Mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API UAM_ErrorCode WINAPI UAM_Reset(void);

    /**
     * \brief Function type for starting a Mapper instance
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API UAM_ErrorCode WINAPI UAM_Start(const UAM_InstanceId_type dwInstanceId /**< [in] Instance ID */
    );

    /**
     * \brief Function type for stopping a Mapper instance
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API UAM_ErrorCode WINAPI UAM_Stop(const UAM_InstanceId_type dwInstanceId /**< [in] Instance ID */
    );

    /**
     * \brief Function type for querying an error string
     * \return A human readable string representing the error code
     */
    UAM_API const char* WINAPI UAM_ErrorCodeToString(const UAM_ErrorCode dwErrorCode /**< [in] Error code */
    );
#endif
    /*---------------------*/
    /*  V A R I A B L E S  */
    /*---------------------*/

#ifdef __cplusplus
}

#endif /* extern C */

#endif /* __UAM_MANAGER_H */
