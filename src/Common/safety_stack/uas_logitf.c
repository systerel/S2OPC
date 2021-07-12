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
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines functions for logging.
 *
 * Safety-Related: no
 */

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include "sopc_types.h"
#include "uas_stdtypes.h"

/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_logitf.h"

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

/*--------------------------------------------------------------------------*/
/******************************** L O C A L *********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

/** Flushing the log messages (hint: consumes computing time) */
#define LOG_ENABLE_FLUSHING

/** buffer length for a log message */
#define LOG_BUFFER_SIZE 2000

/*-------------*/
/*  T Y P E S  */
/*-------------*/

/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

/**
 * log level.
 */
static LOG_LEVEL log_nLevel = LOG_DEFAULT;

/**
 * handle of the log file.
 */
static FILE* log_hFile = NULL;

/*--------------------------------------------------------------------------*/
/************** I M P L E M E N T A T I O N   ( G L O B A L S ) *************/
/*--------------------------------------------------------------------------*/

/**
 * Assign the log file.
 * This function checks if log file is already opened. Otherwise it opens the log file.
 * \param[in]  pszFilename - name and path to the log file
 * \return 0 if successfull, otherwise 1.
 */
uint8_t LOG_SetFile(const char* pszFilename)
{
    uint8_t nRetValue = 1;

    if (NULL == log_hFile)
    {
#ifdef WIN32
        // nRetValue = fopen_s( &log_hFile, pszFilename, "w" /*"wc"*/ );
#ifdef LOG_ENABLE_FLUSHING
        log_hFile = _fsopen(pszFilename, "wc", _SH_DENYWR);
#else
        log_hFile = _fsopen(pszFilename, "w", _SH_DENYWR);
#endif
#else
        log_hFile = fopen(pszFilename, "wc");
#endif
        if (NULL != log_hFile)
        {
            nRetValue = 0;
        } /* if */
    }     /* if */

    return nRetValue;
} /* LOG_SetFile */

/**
 * Assign the log level.
 * This function sets the global log level.
 * \param[in]  nLevel - log level
 */
void LOG_SetLevel(const LOG_LEVEL nLevel)
{
    log_nLevel = nLevel;
} /* LOG_SetLevel */

/**
 * Print formatted output to the log file.
 * This function prints formatted output with output log level >= global log level
 * to the log file. The format argument has the same syntax and use that it has in printf.
 * \param[in]  nLevel    - log level of the output
 * \param[in]  pszFormat - format-control string.
 * \param[in]  ...       - optional arguments.
 */
void LOG_Trace(const LOG_LEVEL nLevel, const char* pszFormat, ...)
{
    if (nLevel <= log_nLevel)
    {
        va_list args;
        char pszBuffer[LOG_BUFFER_SIZE + 1];

        va_start(args, pszFormat);
        STDFUNC_vsnprintf(pszBuffer, sizeof(pszBuffer) / sizeof(pszBuffer[0]), pszFormat, args);
        va_end(args);

        if (log_hFile != NULL)
        {
            fprintf(log_hFile, "%s\n", pszBuffer);
#ifdef LOG_ENABLE_FLUSHING
            fflush(log_hFile);
#endif /* LOG_ENABLE_FLUSHING */
            if (LOG_DEFAULT == nLevel)
            {
                printf("%s\n", pszBuffer);
            } /* if */
        }     /* if */
        else
        {
            printf("%s\n", pszBuffer);
        } /* else */
    }     /* if */
} /* LOG_Trace */

/**
 * Print formatted output with LOG_LEVEL=LOG_DEBUG to the log file.
 * This function prints formatted output with LOG_DEBUG >= global log level
 * to the log file. The format argument has the same syntax and use that it has in printf.
 * \param[in]  pszFormat - format-control string.
 * \param[in]  ...       - optional arguments.
 */
/**
 */
void LOG_DebugTrace(const char* pszFormat, ...)
{
    if (LOG_DEBUG <= log_nLevel)
    {
        va_list args;
        char pszBuffer[LOG_BUFFER_SIZE + 1];

        va_start(args, pszFormat);
        STDFUNC_vsnprintf(pszBuffer, sizeof(pszBuffer) / sizeof(pszBuffer[0]), pszFormat, args);
        va_end(args);

        if (log_hFile != NULL)
        {
            fprintf(log_hFile, "%s\n", pszBuffer);
#ifdef LOG_ENABLE_FLUSHING
            fflush(log_hFile);
#endif    /* LOG_ENABLE_FLUSHING */
        } /* if */
        else
        {
            printf("%s\n", pszBuffer);
        } /* else */
    }     /* if */
} /* LOG_DebugTrace */

/**
 * Print data to the log file.
 * This function prints data with output log level >= global log level
 * to the log file. It is printed as a sequence of octets.
 * \param[in]  nLevel      - log level of the data.
 * \param[in]  pszDataName - name of the data.
 * \param[in]  wDataLength - length of the data in octets.
 * \param[in]  pbyData     - pointer to the data buffer.
 */
void LOG_Data(const LOG_LEVEL nLevel, const char* pszDataName, const uint16_t wDataLength, const uint8_t* pbyData)
{
    if (nLevel <= log_nLevel)
    {
        char pszBuffer[LOG_BUFFER_SIZE + 1];
        char abyLogTxt[10 + 1];
        int i;

        if (pszDataName == NULL)
        {
            pszDataName = "<NULL>";
        }
        STDFUNC_snprintf(pszBuffer, LOG_BUFFER_SIZE, "%s =", pszDataName);
        if (NULL == pbyData)
        {
            STDFUNC_strncat(pszBuffer, " NULL !!!", LOG_BUFFER_SIZE - strlen(pszBuffer));
        } /* if */
        else
        {
            for (i = 0; (i < wDataLength) && (STDFUNC_strlen(pszBuffer) < (LOG_BUFFER_SIZE - 10)); i++)
            {
                STDFUNC_snprintf(abyLogTxt, 10, " 0x%02X", pbyData[i]);
                STDFUNC_strncat(pszBuffer, abyLogTxt, LOG_BUFFER_SIZE - strlen(pszBuffer));
            } /* for */
            if (i < wDataLength)
            {
                STDFUNC_strncat(pszBuffer, " ...", LOG_BUFFER_SIZE - strlen(pszBuffer));
            } /* if */
        }     /* else */

        if (NULL != log_hFile)
        {
            fprintf(log_hFile, "%s\n", pszBuffer);
#ifdef LOG_ENABLE_FLUSHING
            fflush(log_hFile);
#endif /* LOG_ENABLE_FLUSHING */
            if (nLevel == LOG_DEFAULT)
            {
                printf("%s\n", pszBuffer);
            } /* if */
        }     /* if */
        else
        {
            printf("%s\n", pszBuffer);
        } /* else */
    }     /* if */
} /* LOG_Data */

/**
 * Print data with LOG_LEVEL=LOG_DEBUG to the log file.
 * This function prints data with LOG_DEBUG >= global log level
 * to the log file. It is printed as a sequence of octets.
 * \param[in]  nLevel      - log level of the data.
 * \param[in]  pszDataName - name of the data.
 * \param[in]  wDataLength - length of the data in octets.
 * \param[in]  pbyData     - pointer to the data buffer.
 */
void LOG_DebugData(const char* pszDataName, const uint16_t wDataLength, const uint8_t* pbyData)
{
    LOG_Data(LOG_DEBUG, pszDataName, wDataLength, pbyData);
} /* LOG_DebugData */

/*--------------------------------------------------------------------------*/
/*************** I M P L E M E N T A T I O N   ( L O C A L S ) **************/
/*--------------------------------------------------------------------------*/
