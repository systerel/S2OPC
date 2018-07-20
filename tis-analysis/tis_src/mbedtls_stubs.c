//**************************************************************************/
//*                                                                        */
//*  This file is part of deliverable T3.3 of project INGOPCS              */
//*                                                                        */
//*    Copyright (C) 2017 TrustInSoft                                      */
//*                                                                        */
//*  All rights reserved.                                                  */
//*                                                                        */
//**************************************************************************/

// ack "^mbedtls" results/addspace_functions.csv | cut -d, -f1
/*
mbedtls_aes_crypt_cbc
mbedtls_aes_free
mbedtls_aes_init
mbedtls_aes_setkey_dec
mbedtls_aes_setkey_enc
mbedtls_ctr_drbg_free
mbedtls_ctr_drbg_init
mbedtls_ctr_drbg_random
mbedtls_ctr_drbg_seed
mbedtls_entropy_free
mbedtls_entropy_func
mbedtls_entropy_init
mbedtls_md
mbedtls_md_free
mbedtls_md_get_size
mbedtls_md_hmac
mbedtls_md_hmac_finish
mbedtls_md_hmac_reset
mbedtls_md_hmac_starts
mbedtls_md_hmac_update
mbedtls_md_init
mbedtls_md_setup
mbedtls_pk_free
mbedtls_pk_get_bitlen
mbedtls_pk_get_len
mbedtls_pk_get_type
mbedtls_pk_init
mbedtls_pk_parse_key
mbedtls_pk_parse_keyfile
mbedtls_pk_rsa
mbedtls_pk_write_key_der
mbedtls_rsa_rsaes_oaep_decrypt
mbedtls_rsa_rsaes_oaep_encrypt
mbedtls_rsa_rsassa_pkcs1_v15_sign
mbedtls_rsa_rsassa_pkcs1_v15_verify
mbedtls_rsa_rsassa_pss_sign
mbedtls_rsa_rsassa_pss_verify
mbedtls_rsa_set_padding
mbedtls_x509_crt_free
mbedtls_x509_crt_init
mbedtls_x509_crt_parse
mbedtls_x509_crt_parse_file
mbedtls_x509_crt_verify_with_profil
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tis_builtin.h>


//----------------------------------------------------------------------------
#include "mbedtls/pk.h"

// TODO see tis-analysis/tis_src/frama_c_stubs.c in frama-c branch
/*@ assigns \result, *ctx \from key[0..keylen-1], pwd[0..pwdlen-1],
                                indirect:*ctx ; */
int mbedtls_pk_parse_key( mbedtls_pk_context *ctx,
                  const unsigned char *key, size_t keylen,
                  const unsigned char *pwd, size_t pwdlen );


/*@
  assigns \result, buf[0..size-1] \from *ctx;
  ensures mpwkd_e_for_test_only: \result == 1192 || \result == 610;
  ensures mpwkd_e_init: \initialized (buf + (size - \result..size - 1));
*/
int mbedtls_pk_write_key_der( mbedtls_pk_context *ctx, unsigned char *buf, size_t size );

/*@ assigns \result, *ctx \from path[..], pwd[..], indirect:*ctx ; */
int mbedtls_pk_parse_keyfile( mbedtls_pk_context *ctx,
                      const char *path, const char *pwd );

//============================================================================
#if defined(WITHOUT_MBEDTLS_SRCS)
//----------------------------------------------------------------------------
// For some functions, it is easier to have a body then to have a spec.
//----------------------------------------------------------------------------
#include "mbedtls/md.h"

extern const mbedtls_md_info_t tis_mbedtls_sha1_info;
extern const mbedtls_md_info_t tis_mbedtls_sha256_info;

const mbedtls_md_info_t *mbedtls_md_info_from_type( mbedtls_md_type_t md_type  )
{
  switch( md_type ) {
    case MBEDTLS_MD_SHA1:
      return &tis_mbedtls_sha1_info;
    case MBEDTLS_MD_SHA256:
      return &tis_mbedtls_sha256_info;
    default: //@ assert \false;
      return NULL;
  }
}
const mbedtls_md_info_t *mbedtls_md_info_from_string( const char *md_name ) {
  if( !strcmp( "SHA256", md_name ) )
    return mbedtls_md_info_from_type (MBEDTLS_MD_SHA1);
  if( !strcmp( "SHA1", md_name ) )
    return mbedtls_md_info_from_type (MBEDTLS_MD_SHA256);
  //@ assert \false;
  return NULL;
}
//----------------------------------------------------------------------------
#include "mbedtls/x509_crt.h"

unsigned char tis_x509_crt_array[10000];

int mbedtls_x509_crt_parse_file( mbedtls_x509_crt *chain, const char *path ) {
  size_t sz = tis_interval (1000, 2000); // reduce later on with tis_force
  chain->raw.p = tis_x509_crt_array;
  chain->raw.len = sz;
  tis_make_unknown (tis_x509_crt_array, 10000);
  tis_x509_crt_array[sz] = 0;
  return tis_nondet (0, -1);
}

void mbedtls_x509_crt_free( mbedtls_x509_crt *crt ) {
}

//============================================================================
#elif defined(WITH_MBEDTLS_SRCS)

#include "mbedtls/bignum.h"

#define ciL    (sizeof(mbedtls_mpi_uint))

int mbedtls_mpi_grow( mbedtls_mpi *X, size_t nblimbs ) {
  mbedtls_mpi_uint *p;

  if( nblimbs > MBEDTLS_MPI_MAX_LIMBS )
    return( MBEDTLS_ERR_MPI_ALLOC_FAILED );

  if( X->n < nblimbs ) {
    if ( X->p == NULL) {
      int nb_alloc = MBEDTLS_MPI_MAX_LIMBS + 1;

      if ( ( p = (mbedtls_mpi_uint *) calloc( nb_alloc, ciL ) ) == NULL )
        return ( MBEDTLS_ERR_MPI_ALLOC_FAILED );

      X->p = p;
    }
    X->n = nblimbs;
  }
  return( 0 );
}

int mbedtls_mpi_shrink( mbedtls_mpi *X, size_t nblimbs ) {
  //@ assert \false;
  return 0;
}

int tis_make_mpi(mbedtls_mpi * n, unsigned int s) {
  int ret = 0;

  mbedtls_mpi_init (n);
  MBEDTLS_MPI_CHK ( mbedtls_mpi_grow (n, s) );

  n->s = tis_nondet (-1, 1);

  tis_make_unknown(n->p, s * sizeof(mbedtls_mpi_uint));

cleanup:
  return ret;
}

#include "mbedtls/rsa.h"
/*
int rsa_prepare_blinding( mbedtls_rsa_context *ctx,
    int (*f_rng)(void *, unsigned char *, size_t), void *p_rng )
{
  int ret = 0;
  MBEDTLS_MPI_CHK (tis_make_mpi (&ctx->Vi, MBEDTLS_MPI_MAX_LIMBS));
  MBEDTLS_MPI_CHK (tis_make_mpi (&ctx->Vf, MBEDTLS_MPI_MAX_LIMBS));
cleanup:
  return ret;
}
*/
int mbedtls_rsa_private( mbedtls_rsa_context *ctx,
                   int (*f_rng)(void *, unsigned char *, size_t),
                   void *p_rng,
                   const unsigned char *input,
                   unsigned char *output ) {
  int ret = 0;
  mbedtls_mpi T;
  for (int i = 0; i < ctx->len; i++) {
    int x = input[i];
    }
  MBEDTLS_MPI_CHK (tis_make_mpi (&ctx->Vi, MBEDTLS_MPI_MAX_LIMBS));
  MBEDTLS_MPI_CHK (tis_make_mpi (&ctx->Vf, MBEDTLS_MPI_MAX_LIMBS));
  tis_make_unknown (output, ctx->len);
cleanup:
  return ret;
}

int mbedtls_rsa_public(mbedtls_rsa_context *ctx, unsigned char const *input,
    unsigned char *output)
{
  int ret = 0;
  mbedtls_mpi T;
  for (int i = 0; i < ctx->len; i++) {
    int x = input[i];
  }
  tis_make_unknown (output, ctx->len);
cleanup:
  return ret;
}
//============================================================================
#else
#error "Either WITH_MBEDTLS_SRCS or WITHOUT_MBEDTLS_SRCS should be defined."
#endif
//============================================================================
