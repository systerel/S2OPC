/**
 * OPC Foundation OPC UA Safety Stack
 *
 * \file
 * \author
 *    Copyright 2021 (c) ifak e.V.Magdeburg
 *    Copyright 2021 (c) Elke Hintze
 *
 * \brief OPC UA Safety timer definition.
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines the functions of the OPC UA Safety timer module.
 *
 * Safety-Related: yes
 */


/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_def.h"
#include "uas_type.h"
#include "uas_rvar.h"


/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_time.h"


/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

/** This variable is used by the UAS_Timer functions to store invocation errors
  * of the UAS instances. The UAS manager functions check this variable
  * and set the resulting error code. The value for no error is "0".
  */
UAS_UInt16  wUASTIME_ErrorCount = 0u;


/*--------------------------------------------------------------------------*/
/******************************** L O C A L *********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

/**
* Maximum value for the timestamp.
*/
#define UASTIME_MAX  ( 0xFFFFFFFFFFFFFFFFuLL )


/*-------------*/
/*  T Y P E S  */
/*-------------*/


/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

 /**
  * UAS timer common checks
  */
UAS_Bool bUASTIME_PreCheck
(
  const UASTIME_Instance_type * const pzTimerInst,  /**< Pointer to the primary instance data */
  const UASTIME_Instance_type * const r_pzTimerInst /**< Pointer to the redundant instance data */
);

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/


/*--------------------------------------------------------------------------*/
/************** I M P L E M E N T A T I O N   ( G L O B A L S ) *************/
/*--------------------------------------------------------------------------*/

/**
 * UAS timer instance initialization
 * This function initializes an UAS timer instance. It has to be called before
 * using any other function of this module.
 * \param[in/out]  pzInst  - Pointer to the primary instance data
 * \param[in/out]  pzParam - Pointer to the redundant instance data
 * \param[in]      dwCurrentTime - Current time value in microseconds
*/
void vUASTIME_Init
(
  UASTIME_Instance_type * const pzTimerInst,
  UASTIME_Instance_type * const r_pzTimerInst,
  UASTIME_Timestamp_type const dwCurrentTime
)
{
#ifdef DBG_FDTIMER

  UASDEF_LOG_DEBUG( "  vUASTIME_Init: entry. dwCurrentTime = %ul", ulCurrentTime );

#endif /* ifdef DBG_FDTIMER */

  UAS_Bool bValid = bUASTIME_PreCheck ( pzTimerInst, r_pzTimerInst );
  if ( bValid )
  {
    /* Initialize the timer instance */
    UASRVAR_SET_USIGN64( pzTimerInst->dwWatchdogTime,       0u );
    UASRVAR_SET_USIGN64( pzTimerInst->dwCurrentTime,        dwCurrentTime );
    UASRVAR_SET_USIGN64( pzTimerInst->dwExpiringTime,       0u );
    UASRVAR_SET_USIGN8 ( pzTimerInst->byCurrentMillennium,  (UAS_UInt8)0u );
    UASRVAR_SET_USIGN8 ( pzTimerInst->byExpiringMillennium, (UAS_UInt8)0u );
    UASRVAR_SET_BOOL   ( pzTimerInst->bRunning,             0u );
  } /* if */
  else
  {
    /* Faulty timer data */
    UASRVAR_INCR_ERROR_COUNT( wUASTIME_ErrorCount );
  } /* else */

#ifdef DBG_FDTIMER

  UASDEF_LOG_DEBUG( "  vFDTimer_Init: exit. wUASTIME_ErrorCount = 0x%04X", wUASTIME_ErrorCount );

#endif /* ifdef DBG_FDTIMER */

} /* end of function */


/**
 * UAS timer instance start ( SafetyConsumerTimeOut )
 * This function starts an UAS timer instance.
 * \param[in/out]  pzInst  - Pointer to the primary instance data
 * \param[in/out]  pzParam - Pointer to the redundant instance data
 * \param[in]      dwCurrentTime - Current time value in microseconds
 * \param[in]      dwWatchdogTime - SafetyConsumerTimeOut in microseconds
 * \error handling: incrementation of wUASTIME_ErrorCount
*/
void vUASTIME_StartWatchdog(
  UASTIME_Instance_type * const pzTimerInst,
  UASTIME_Instance_type * const r_pzTimerInst,
  UASTIME_Timestamp_type const dwCurrentTime,
  UAS_UInt32 const dwWatchdogTime
)
{
#ifdef DBG_FDTIMER

  UASDEF_LOG_DEBUG( "  vUASTIME_StartWatchdog: entry. dwCurrentTime = %ul, dwWatchdogTime =  %ul", ulCurrentTime, dwWatchdogTime );

#endif /* ifdef DBG_FDTIMER */

  UAS_Bool bValid = bUASTIME_PreCheck ( pzTimerInst, r_pzTimerInst );
  if ( bValid )
  {
    /* Update alternating millennium */
    if ( dwCurrentTime < pzTimerInst->dwCurrentTime )
    {
      UASRVAR_SET_USIGN8( pzTimerInst->byCurrentMillennium, (UAS_UInt8)( pzTimerInst->byCurrentMillennium + 1u ) );
    } /* if */
    /* Update current time */
    UASRVAR_SET_USIGN64( pzTimerInst->dwCurrentTime, dwCurrentTime );

    /* Store the watchdog time */
    UASRVAR_SET_USIGN64( pzTimerInst->dwWatchdogTime, dwWatchdogTime );

    /* Start the timer instance */
    UASRVAR_SET_BOOL( pzTimerInst->bRunning, 1u );

    /* Calculate the time of expiration */
    if ( (UASTIME_Timestamp_type)( UASTIME_MAX - pzTimerInst->dwWatchdogTime ) >= pzTimerInst->dwCurrentTime )
    {
      /* Timeout in current millennium */
      UASRVAR_SET_USIGN8( pzTimerInst->byExpiringMillennium, pzTimerInst->byCurrentMillennium );
      UASRVAR_SET_USIGN64( pzTimerInst->dwExpiringTime, ( pzTimerInst->dwCurrentTime + (UAS_UInt32)pzTimerInst->dwWatchdogTime ) );
    } /* if */
    else
    {
      /* Timeout in next millennium */
      UASRVAR_SET_USIGN8( pzTimerInst->byExpiringMillennium, (UAS_UInt8)( pzTimerInst->byCurrentMillennium + 1u ) );
      UASRVAR_SET_USIGN64 ( pzTimerInst->dwExpiringTime, ( ( (UAS_UInt32)pzTimerInst->dwWatchdogTime - ( UASTIME_MAX - pzTimerInst->dwCurrentTime ) ) - 1u ) );
    } /* else */
  } /* if */
  else
  {
    /* Faulty timer data */
    UASRVAR_INCR_ERROR_COUNT( wUASTIME_ErrorCount );
  } /* else */

#ifdef DBG_FDTIMER

  UASDEF_LOG_DEBUG( "  vUASTIME_StartWatchdog: exit. wUASTIME_ErrorCount = 0x%04X", wUASTIME_ErrorCount );

#endif /* ifdef DBG_FDTIMER */

} /* end of function */


/**
 * UAS timer instance start ( SafetyErrorIntervalLimit )
 * This function starts an UAS timer instance.
 * \param[in/out]  pzInst  - Pointer to the primary instance data
 * \param[in/out]  pzParam - Pointer to the redundant instance data
 * \param[in]      dwCurrentTime - Current time value in microseconds
 * \param[in]      wErrorIntervalLimit - SafetyErrorIntervalLimit in minutes
 * \error handling: incrementation of wUASTIME_ErrorCount
*/
void vUASTIME_StartErrorIntervalLimit(
  UASTIME_Instance_type * const pzTimerInst,
  UASTIME_Instance_type * const r_pzTimerInst,
  UASTIME_Timestamp_type const dwCurrentTime,
  UAS_UInt16 const wErrorIntervalLimit
)
{
#ifdef DBG_FDTIMER

  UASDEF_LOG_DEBUG( "  vUASTIME_StartErrorIntervalLimit: entry. dwCurrentTime = %ul, wErrorIntervalLimit =  %u", ulCurrentTime, dwWatchdogTime );

#endif /* ifdef DBG_FDTIMER */

  UAS_Bool bValid = bUASTIME_PreCheck ( pzTimerInst, r_pzTimerInst );
  if ( bValid )
  {
    /* Update alternating millennium */
    if ( dwCurrentTime < pzTimerInst->dwCurrentTime )
    {
      UASRVAR_SET_USIGN8( pzTimerInst->byCurrentMillennium, (UAS_UInt8)( pzTimerInst->byCurrentMillennium + 1u ) );
    } /* if */
    /* Update current time */
    UASRVAR_SET_USIGN64( pzTimerInst->dwCurrentTime, dwCurrentTime );

    /* Store the watchdog time */
    UASRVAR_SET_USIGN64( pzTimerInst->dwWatchdogTime, (UASTIME_Timestamp_type)wErrorIntervalLimit * 60000000uLL );

    /* Start the timer instance */
    UASRVAR_SET_BOOL( pzTimerInst->bRunning, 1u );

    /* Calculate the time of expiration */
    if ( (UASTIME_Timestamp_type)( UASTIME_MAX - pzTimerInst->dwWatchdogTime ) >= pzTimerInst->dwCurrentTime )
    {
      /* Timeout in current millennium */
      UASRVAR_SET_USIGN8( pzTimerInst->byExpiringMillennium, pzTimerInst->byCurrentMillennium );
      UASRVAR_SET_USIGN64( pzTimerInst->dwExpiringTime, ( pzTimerInst->dwCurrentTime + (UAS_UInt32)pzTimerInst->dwWatchdogTime ) );
    } /* if */
    else
    {
      /* Timeout in next millennium */
      UASRVAR_SET_USIGN8( pzTimerInst->byExpiringMillennium, (UAS_UInt8)( pzTimerInst->byCurrentMillennium + 1u ) );
      UASRVAR_SET_USIGN64 ( pzTimerInst->dwExpiringTime, ( ( (UAS_UInt32)pzTimerInst->dwWatchdogTime - ( UASTIME_MAX - pzTimerInst->dwCurrentTime ) ) - 1u ) );
    } /* else */
  } /* if */
  else
  {
    /* Faulty timer data */
    UASRVAR_INCR_ERROR_COUNT( wUASTIME_ErrorCount );
  } /* else */

#ifdef DBG_FDTIMER

  UASDEF_LOG_DEBUG( "  vUASTIME_StartErrorIntervalLimit: exit. wUASTIME_ErrorCount = 0x%04X", wUASTIME_ErrorCount );

#endif /* ifdef DBG_FDTIMER */

} /* end of function */


/**
 * UAS timer instance stop
 * This function stops an UAS timer instance.
 * \param[in/out]  pzInst  - Pointer to the primary instance data
 * \param[in/out]  pzParam - Pointer to the redundant instance data
 * \param[in]      dwCurrentTime - Current time value in microseconds
 * \error handling: incrementation of wUASTIME_ErrorCount
*/
void vUASTIME_Stop(
  UASTIME_Instance_type * const pzTimerInst,
  UASTIME_Instance_type * const r_pzTimerInst,
  UASTIME_Timestamp_type const dwCurrentTime
)
{
#ifdef DBG_FDTIMER

  UASDEF_LOG_DEBUG( "  vUASTIME_Stop: entry. dwCurrentTime = %ul", ulCurrentTime );

#endif /* ifdef DBG_FDTIMER */

  UAS_Bool bValid = bUASTIME_PreCheck ( pzTimerInst, r_pzTimerInst );
  if ( bValid )
  {
    /* Update alternating millennium */
    if ( dwCurrentTime < pzTimerInst->dwCurrentTime )
    {
      UASRVAR_SET_USIGN8( pzTimerInst->byCurrentMillennium, (UAS_UInt8)( pzTimerInst->byCurrentMillennium + 1u ) );
    } /* if */
    /* Update current time */
    UASRVAR_SET_USIGN64( pzTimerInst->dwCurrentTime, dwCurrentTime );

    /* Stop the timer instance */
    UASRVAR_SET_BOOL   ( pzTimerInst->bRunning, 0u );
    UASRVAR_SET_USIGN64( pzTimerInst->dwExpiringTime,       0uLL );
    UASRVAR_SET_USIGN8 ( pzTimerInst->byExpiringMillennium, 0u );
  } /* if */
  else
  {
    /* Faulty timer data */
    UASRVAR_INCR_ERROR_COUNT( wUASTIME_ErrorCount );
  } /* else */

#ifdef DBG_FDTIMER

  UASDEF_LOG_DEBUG( "  vUASTIME_Stop: exit. wUASTIME_ErrorCount = 0x%04X", wUASTIME_ErrorCount );

#endif /* ifdef DBG_FDTIMER */

} /* end of function */


/**
 * UAS timer instance expired
 * This function checks an UAS timer instance for timeout.
 * \param[in/out]  pzInst  - Pointer to the primary instance data
 * \param[in/out]  pzParam - Pointer to the redundant instance data
 * \param[in]      dwCurrentTime - Current time value in microseconds
 * \return 1 if timeout, 0 if still running
 * \error handling: incrementation of wUASTIME_ErrorCount
*/
UAS_Bool bUASTIME_Expired(
  UASTIME_Instance_type * const pzTimerInst,   /**< Pointer to the primary instance data   */
  UASTIME_Instance_type * const r_pzTimerInst, /**< Pointer to the redundant instance data */
  UASTIME_Timestamp_type const dwCurrentTime   /**< Current time value */
)
{
  UAS_Bool bExpired = 0u;

#ifdef DBG_FDTIMER

  UASDEF_LOG_DEBUG( "  vUASTIME_Stop: entry. dwCurrentTime = %ul", ulCurrentTime );

#endif /* ifdef DBG_FDTIMER */

  UAS_Bool bValid = bUASTIME_PreCheck ( pzTimerInst, r_pzTimerInst );
  if ( bValid )
  {
    /* Update alternating millennium */
    if ( dwCurrentTime < pzTimerInst->dwCurrentTime )
    {
      UASRVAR_SET_USIGN8( pzTimerInst->byCurrentMillennium, (UAS_UInt8)( pzTimerInst->byCurrentMillennium + 1u ) );
    } /* if */
    /* Update current time */
    UASRVAR_SET_USIGN64( pzTimerInst->dwCurrentTime, dwCurrentTime );

    /* Timer is expired if */
    if (pzTimerInst->byExpiringMillennium EQ pzTimerInst->byCurrentMillennium)
    {
      /* (1) millennium matches and current time exceeds expiring time       */
      if ( pzTimerInst->dwExpiringTime <= pzTimerInst->dwCurrentTime )
      {
        bExpired = 1u;
      } /* if */
    } /* if */
    else
    {
      /* (2) millennium changed and current time is less than expiring time, */
      /*     i.e. timer expired in last millennium and current time is       */
      /*     already in next millennium.                                     */
      if (pzTimerInst->dwExpiringTime >  pzTimerInst->dwCurrentTime )
      {
        bExpired = 1u;
      } /* if */
    } /* else */

    if (bExpired)
    {
      /* Stop timer instance if expired */
      UASRVAR_SET_BOOL   ( pzTimerInst->bRunning, 0u );
      UASRVAR_SET_USIGN64( pzTimerInst->dwExpiringTime,       0uLL );
      UASRVAR_SET_USIGN8 ( pzTimerInst->byExpiringMillennium, 0u );
    } /* if */
  } /* if */
  else
  {
    /* Faulty timer data */
    UASRVAR_INCR_ERROR_COUNT( wUASTIME_ErrorCount );
  } /* else */

#ifdef DBG_FDTIMER

  UASDEF_LOG_DEBUG( "  vUASTIME_Stop: exit. bExpired = 0x%02X, wUASTIME_ErrorCount = 0x%04X", bExpired, wUASTIME_ErrorCount );

#endif /* ifdef DBG_FDTIMER */

  return (bExpired);

} /* end of function */


/*--------------------------------------------------------------------------*/
/*************** I M P L E M E N T A T I O N   ( L O C A L S ) **************/
/*--------------------------------------------------------------------------*/

/**
* UAS timer common checks
* This function checks common errors before calling the timer instance,
* e.g. pointer and redundant data.
* \param[in/out]  pzInst  - Pointer to the primary instance data
* \param[in/out]  pzParam - Pointer to the redundant instance data
* \param[in]      dwCurrentTime - Current time value
*/
UAS_Bool bUASTIME_PreCheck
(
  const UASTIME_Instance_type * const pzTimerInst,
  const UASTIME_Instance_type * const r_pzTimerInst
)
{
  /*TODO*/
  return 1u;
}

/* end of file */
