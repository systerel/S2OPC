/*
 *  Copyright (C) 2017 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 *
 * Generating the Address Space content. The provided arrays must be allocated with constants__t_Node_i_max elements.
 */


#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "gen_addspace.h"
#include "util_variant.h"
#include "crypto_provider.h"
#include "crypto_profiles.h"


/* Shall not be called anymore */
SOPC_StatusCode gen_addspace_old(constants__t_NodeId_i     *pnids,
                             constants__t_NodeClass_i  *pncls,
                             constants__t_Variant_i    *pvars,
                             constants__t_StatusCode_i *pscs)
{
    size_t i, j, ncl;
    SOPC_NodeId *pnid;
    SOPC_ByteString buf;
    CryptoProvider *pCrypto = NULL;
    uint8_t seed;

    if(NULL == pnids ||
       NULL == pncls ||
       NULL == pvars ||
       NULL == pscs)
        return STATUS_INVALID_PARAMETERS;

    /* CryptoProvider used in fourth batch */
    pCrypto = CryptoProvider_Create(SecurityPolicy_Basic256Sha256_URI);
    if(NULL == pCrypto)
        exit(1);

    /* First batch of variables are int64 */
    for(i=0; i<(NB_NODES-8)/4; ++i)
    {
        j = i+0*(NB_NODES-8)/4;
        /* NodeClass, still a B-value */
        pncls[j] = constants__e_ncl_Variable;
        /* NodeId, a SOPC_NodeId* */
        pnid = malloc(sizeof(SOPC_NodeId));
        if(NULL == pnid)
            exit(1);
        SOPC_NodeId_Initialize(pnid);
        pnid->IdentifierType = IdentifierType_Numeric;
        pnid->Namespace = 0;
        pnid->Data.Numeric = 10+j;
        pnids[j] = (void *)pnid;
        /* The Value, which is a SOPC_Variant* */
        pvars[j] = (void *)util_variant__new_Variant_from_int64((i%2 ? -1:1)*i);
        /* A StatusCode, a B-value */
        pscs[j] = constants__e_sc_ok;
    }

    /* Second batch of variables are uint32, StatusCode are nok */
    for(i=0; i<(NB_NODES-8)/4; ++i)
    {
        j = i+1*(NB_NODES-8)/4;
        /* NodeClass, still a B-value */
        pncls[j] = constants__e_ncl_Variable;
        /* NodeId, a SOPC_NodeId* */
        pnid = malloc(sizeof(SOPC_NodeId));
        if(NULL == pnid)
            exit(1);
        SOPC_NodeId_Initialize(pnid);
        pnid->IdentifierType = IdentifierType_Numeric;
        pnid->Namespace = 0;
        pnid->Data.Numeric = 10+j;
        pnids[j] = (void *)pnid;
        /* The Value, which is a SOPC_Variant* */
        pvars[j] = (void *)util_variant__new_Variant_from_uint32(0xB00B5 + i);
        /* A StatusCode, a B-value */
        pscs[j] = constants__e_sc_nok;
    }

    /* Third batch of variables are double, StatusCode are nok */
    for(i=0; i<(NB_NODES-8)/4; ++i)
    {
        j = i+2*(NB_NODES-8)/4;
        /* NodeClass, still a B-value */
        pncls[j] = constants__e_ncl_Variable;
        /* NodeId, a SOPC_NodeId* */
        pnid = malloc(sizeof(SOPC_NodeId));
        if(NULL == pnid)
            exit(1);
        SOPC_NodeId_Initialize(pnid);
        pnid->IdentifierType = IdentifierType_Numeric;
        pnid->Namespace = 0;
        pnid->Data.Numeric = 10+j;
        pnids[j] = (void *)pnid;
        /* The Value, which is a SOPC_Variant* */
        pvars[j] = (void *)util_variant__new_Variant_from_double(exp(-1.*i));
        /* A StatusCode, a B-value */
        pscs[j] = constants__e_sc_nok;
    }

    /* Fourth batch of variables are ByteString, StatusCode are ok */
    for(i=0; i<(NB_NODES-8)/4; ++i)
    {
        j = i+3*(NB_NODES-8)/4;
        /* NodeClass, still a B-value */
        pncls[j] = constants__e_ncl_Variable;
        /* NodeId, a SOPC_NodeId* */
        pnid = malloc(sizeof(SOPC_NodeId));
        if(NULL == pnid)
            exit(1);
        SOPC_NodeId_Initialize(pnid);
        pnid->IdentifierType = IdentifierType_Numeric;
        pnid->Namespace = 0;
        pnid->Data.Numeric = 10+j;
        pnids[j] = (void *)pnid;
        /* The Value, which is a SOPC_Variant* */
        /* A new buf.Data must be provided each time */
        /* Its content is "INGOPCS "+PSHA256(i,0)+((unsigned char) i)*i
         *  where + is concatenation and
         *  PSHA is 10 bytes the key derivation mecanism from the security policy Basic256Sha256,
         *  the secret used is the "\0" string, and the seed is the byte (i&0xFF)*/
        buf.Length = 8+10+i;
        buf.Data = (SOPC_Byte *)malloc(buf.Length);
        if(NULL == buf.Data)
            exit(1);
        /* Fills the ByteString */
        memcpy((void *)(buf.Data+0), "INGOCPS ", 8);
        seed = ((uint8_t)(i&0xFF));
        if(STATUS_OK != CryptoProvider_DerivePseudoRandomData(pCrypto, (ExposedBuffer *)"", 1, (ExposedBuffer *)&seed, 1, (ExposedBuffer *)(buf.Data+8), 10))
            exit(1);
        memset((void *)(buf.Data+18), i&0xFF, i);
        pvars[j] = (void *)util_variant__new_Variant_from_ByteString(buf);
        /* A StatusCode, a B-value */
        pscs[j] = constants__e_sc_ok;
    }

    /* Fifth batch of nodes: nodes of all classes, but without data for the variables */
    for(i=0; i<8; ++i)
    {
        j = i+NB_NODES-8;
        /* NodeClass, still a B-value, see constants__e_ncl_* */
        ncl = i%8 + 1;
        pncls[j] = ncl;
        /* NodeId, a SOPC_NodeId* */
        pnid = malloc(sizeof(SOPC_NodeId));
        if(NULL == pnid)
            exit(1);
        SOPC_NodeId_Initialize(pnid);
        pnid->IdentifierType = IdentifierType_Numeric;
        pnid->Namespace = 0;
        pnid->Data.Numeric = 10+j;
        pnids[j] = (void *)pnid;
        /* B model protects from trying to access these, but not the C test, which will try them */
        pvars[j] = NULL;
        pscs[j] = 0;
    }

    CryptoProvider_Free(pCrypto);

    return STATUS_OK;
}


SOPC_StatusCode free_addspace(constants__t_NodeId_i     *pnids,
                              constants__t_NodeClass_i  *pncls,
                              constants__t_Variant_i    *pvars,
                              constants__t_StatusCode_i *pscs)
{
    size_t i, j;

    if(NULL == pnids ||
       NULL == pncls ||
       NULL == pvars ||
       NULL == pscs)
        return STATUS_INVALID_PARAMETERS;

    /* Free all the supplementary mallocs */
    /* -> Fourth batch */
    for(i=0; i<(NB_NODES-8)/4; ++i)
    {
        j = i+3*(NB_NODES-8)/4;
        if(NULL == pvars[j])
            continue;
        if(SOPC_ByteString_Id != ((SOPC_Variant *)pvars[j])->BuiltInTypeId)
        {
            printf("free_addspace error: pvar[%zd] is not a ByteString\n", j);
            continue;
        }
        free(((SOPC_Variant *)pvars[j])->Value.Bstring.Data);
        ((SOPC_Variant *)pvars[j])->Value.Bstring.Data = NULL;
    }

    /* Free all the NodeIds and the Variants */
    for(i=0; i<NB_NODES; ++i)
    {
        if(NULL != pnids[i])
        {
            free(pnids[i]);
            pnids[i] = NULL;
        }
        if(NULL != pvars[i])
        {
            free(pvars[i]);
            pvars[i] = NULL;
        }
    }

    return STATUS_OK;
}
