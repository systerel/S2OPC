/**
 * OPC Foundation OPC UA Safety Stack (Reference implementation)
 *
 * \file
 * \author
 *    Copyright 2021 (c) ifak e.V.Magdeburg
 *    Copyright 2017-2021 (c) Matthias Riedl
 *    Copyright 2020-2021 (c) Elke Hintze
 *
 * \brief Logging functions
 *
 * \date      2021-06-09
 * \revision  0.3
 * \status    in work
 *
 * Declares functions for logging.
 *
 * Safety-Related: no
 */

#ifndef INC_UAS_LOGITF_H
#define INC_UAS_LOGITF_H

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include <stdint.h>
#include "sopc_types.h"

/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

/*-------------*/
/*  T Y P E S  */
/*-------------*/

/**
 * log levels.
 */
typedef enum
{
    LOG_DEFAULT, /**< Default messages are logged */
    LOG_ERROR,   /**< Error messages are logged   */
    LOG_WARN,    /**< Warn messages are logged    */
    LOG_INFO,    /**< Info messages are logged    */
    LOG_DEBUG,   /**< Debug messages are logged   */
    LOG_ALL,     /**< All messages are logged     */
} LOG_LEVEL;

/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

/**
 * Assign the log file.
 */
uint8_t LOG_SetFile(const char* pszFilename);

/**
 * Assign the log level.
 */
void LOG_SetLevel(const LOG_LEVEL nLevel);

/**
 * Print formatted output to the log file.
 */
void LOG_Trace(const LOG_LEVEL nLevel, const char* pszFormat, ...);

/**
 * Print formatted output with LOG_LEVEL=LOG_DEBUG to the log file.
 */
void LOG_DebugTrace(const char* pszFormat, ...);

/**
 * Print data to the log file.
 */
void LOG_Data(const LOG_LEVEL nLevel, const char* pszDataName, const uint16_t wDataLength, const uint8_t* pbyData);

/**
 * Print data with LOG_LEVEL=LOG_DEBUG to the log file.
 */
void LOG_DebugData(const char* pszDataName, const uint16_t wDataLength, const uint8_t* pbyData);

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

#endif /* ifndef INC_UAS_LOGITF_H */
