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
 * \brief Invariant and quality tests for SOPC_NodeId_Hash.
 *
 * SOPC_NodeId_Hash feeds the NodeId dictionaries (SOPC_NodeId_Dict_Create), which resolve
 * conflicts by quadratic probing over a power-of-two bucket array. Two consequences drive
 * the tests below:
 *
 * - The only hard requirement is congruence with SOPC_NodeId_Equal: two NodeIds that compare
 *   equal must hash equal, otherwise a dictionary can hold the same key twice.
 * - The dictionary indexes buckets with (hash + f(i)) & (size - 1), so only the low
 *   log2(size) bits of the hash are ever used. Hash quality must therefore be judged on the
 *   distribution of those low bits and on the resulting probe chains.
 *
 * The number of distinct hash values is checked as well. It does not discriminate between two
 * reasonable hash functions (for 50000 keys and an ideal 64-bit hash, the expected number of
 * colliding pairs is about 7e-11, so zero is the expected outcome either way), but a hash whose
 * byte weights are not independent collapses whole families of keys. A plain DJB mixing did
 * exactly that here: its weights are powers of 33, and 1 * 33^3 equals 33 * 33^2, so
 * ns=1;i=<n> and ns=1;i=<n+8447> shared the same hash and 50000 sequential numeric NodeIds only
 * produced 8610 distinct values.
 *
 * No test compares the hash against a hard-coded expected value: the assertions hold for any
 * correct hash function, so this file stays valid if the mixing function is replaced. The
 * measured quality figures are printed to allow a before/after comparison.
 */

#include <check.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "check_helpers.h"

#include "sopc_builtintypes.h"
#include "sopc_mem_alloc.h"

/* Probe formula of sopc_dict.c, kept in sync manually (it is a private macro of that module). */
#define HASH_I(hash, i) ((hash) + ((i) / 2) + (((i) * (i)) / 2))

#define DISTRIB_KEYS 50000u

/* Chi-square is computed on the low 12 bits, which gives about 12 keys per bucket: enough
 * expected count per class for the statistic to be meaningful. */
#define CHI2_BUCKETS 4096u
#define CHI2_BUCKET_MASK (CHI2_BUCKETS - 1u)

/* chi2 / dof has mean 1 and standard deviation sqrt(2 / 4095) = 0.022 for a uniform hash, so
 * this bound is about 7 standard deviations away and cannot trigger by chance. The key sets are
 * deterministic anyway: a failure here always means a real loss of uniformity. */
#define CHI2_MAX_RATIO 1.15

/* The two bounds below are set against a reference cost. Inserting these 50000 keys into the
 * 131072-bucket table that the dictionary allocates for them, which is a load factor of 0.38,
 * takes 1.48 probes on average with a longest chain of 11 when the hash is uniformly random.
 *
 * Each bound is high enough above that reference for the variation between key sets never to
 * reach it, and low enough to reject a clustering hash: the same keys hashed with a plain DJB
 * mixing needed 3.1 to 4.5 probes on average. */
#define MAX_AVG_PROBES 2.0  /* 1.35 times the reference of 1.48 probes */
#define MAX_PROBE_CHAIN 32u /* about 3 times the reference chain of 11 */

#define AVALANCHE_BASES 256u
#define NUMERIC_ID_BITS 32u

/* Number of low hash bits required to depend on every identifier bit. It covers the bucket
 * index of any dictionary up to 65536 buckets. */
#define DICT_INDEX_BITS 16u

typedef enum
{
    KEY_PATTERN_NUMERIC, /* ns=1;i=<n>, the sequential numeric ids of a generated address space */
    KEY_PATTERN_STRING,  /* ns=1;s=Objects.<n>, long common prefix, varying suffix */
} key_pattern_t;

static uint64_t nodeid_hash(const SOPC_NodeId* nodeId)
{
    uint64_t hash = 0;
    SOPC_NodeId_Hash(nodeId, &hash);
    return hash;
}

/** \brief Allocates a NodeId from its OPC UA string form, failing the test if it cannot. */
static SOPC_NodeId* nodeid_new(const char* cString)
{
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(cString);
    ck_assert_ptr_nonnull(nodeId);
    return nodeId;
}

static void nodeid_delete(SOPC_NodeId* nodeId)
{
    SOPC_NodeId_Clear(nodeId);
    SOPC_Free(nodeId);
}

/* Reproducible PRNG, so a failure can always be replayed. */
static uint64_t xorshift64(uint64_t* state)
{
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

/** \brief Smallest power of two the dictionary would use to hold \p n_items (see dict_resize). */
static size_t dict_table_size(size_t n_items)
{
    size_t size = 16;
    while (size < (2 * n_items))
    {
        size *= 2;
    }
    return size;
}

static SOPC_NodeId** build_keys(key_pattern_t pattern, size_t n_keys)
{
    SOPC_NodeId** keys = SOPC_Calloc(n_keys, sizeof(SOPC_NodeId*));
    ck_assert_ptr_nonnull(keys);

    for (size_t i = 0; i < n_keys; ++i)
    {
        char buf[64];
        int n = snprintf(buf, sizeof(buf), KEY_PATTERN_NUMERIC == pattern ? "ns=1;i=%zu" : "ns=1;s=Objects.%zu", i);
        ck_assert_int_gt(n, 0);
        ck_assert_uint_lt((size_t) n, sizeof(buf));
        keys[i] = nodeid_new(buf);
    }

    return keys;
}

static void delete_keys(SOPC_NodeId** keys, size_t n_keys)
{
    for (size_t i = 0; i < n_keys; ++i)
    {
        nodeid_delete(keys[i]);
    }
    SOPC_Free(keys);
}

/**
 * \brief Chi-square statistic of the low CHI2_BUCKETS bits, divided by the degrees of freedom.
 *
 * A value near 1 means the low bits are as uniform as a random mapping, a value well above 1
 * means some bucket ranges are favoured. Values below 1 are better than random and harmless
 * for a hash table.
 */
static double low_bits_chi2_ratio(SOPC_NodeId** keys, size_t n_keys)
{
    uint32_t* counts = SOPC_Calloc(CHI2_BUCKETS, sizeof(uint32_t));
    ck_assert_ptr_nonnull(counts);

    for (size_t i = 0; i < n_keys; ++i)
    {
        counts[nodeid_hash(keys[i]) & CHI2_BUCKET_MASK]++;
    }

    const double expected = (double) n_keys / (double) CHI2_BUCKETS;
    double chi2 = 0.0;
    for (size_t b = 0; b < CHI2_BUCKETS; ++b)
    {
        const double delta = (double) counts[b] - expected;
        chi2 += (delta * delta) / expected;
    }

    SOPC_Free(counts);
    return chi2 / (double) (CHI2_BUCKETS - 1u);
}

static int uint64_compare(const void* left, const void* right)
{
    const uint64_t a = *(const uint64_t*) left;
    const uint64_t b = *(const uint64_t*) right;

    if (a < b)
    {
        return -1;
    }
    if (a > b)
    {
        return 1;
    }
    return 0;
}

/**
 * \brief Counts the keys sharing their hash with a previous one.
 *
 * The key sets are generated distinct, so any duplicate hash value found here is a genuine
 * collision.
 */
static size_t count_hash_collisions(SOPC_NodeId** keys, size_t n_keys)
{
    uint64_t* hashes = SOPC_Calloc(n_keys, sizeof(uint64_t));
    ck_assert_ptr_nonnull(hashes);

    for (size_t i = 0; i < n_keys; ++i)
    {
        hashes[i] = nodeid_hash(keys[i]);
    }

    qsort(hashes, n_keys, sizeof(uint64_t), uint64_compare);

    size_t collisions = 0;
    for (size_t i = 1; i < n_keys; ++i)
    {
        if (hashes[i] == hashes[i - 1])
        {
            collisions++;
        }
    }

    SOPC_Free(hashes);
    return collisions;
}

/**
 * \brief Replays the insertion probing of sopc_dict.c to measure the real cost of the hash.
 *
 * \param keys           The key set to insert.
 * \param n_keys         Number of keys.
 * \param[out] avg_probes Average number of buckets examined per insertion.
 * \param[out] max_chain  Longest probe chain observed.
 */
static void measure_probes(SOPC_NodeId** keys, size_t n_keys, double* avg_probes, size_t* max_chain)
{
    const size_t table_size = dict_table_size(n_keys);
    const size_t sizemask = table_size - 1;
    bool* occupied = SOPC_Calloc(table_size, sizeof(bool));
    ck_assert_ptr_nonnull(occupied);

    double probe_sum = 0.0;
    size_t max_probe = 0;

    for (size_t k = 0; k < n_keys; ++k)
    {
        const uint64_t hash = nodeid_hash(keys[k]);
        bool inserted = false;

        for (size_t i = 0; i < table_size; ++i)
        {
            const size_t idx = (size_t) HASH_I(hash, i) & sizemask;
            if (!occupied[idx])
            {
                occupied[idx] = true;
                probe_sum += (double) (i + 1);
                if ((i + 1) > max_probe)
                {
                    max_probe = i + 1;
                }
                inserted = true;
                break;
            }
        }
        /* Saturation is impossible below full load and would mean the probe sequence does not
         * cover the table for this hash value. */
        ck_assert(inserted);
    }

    SOPC_Free(occupied);
    *avg_probes = probe_sum / (double) n_keys;
    *max_chain = max_probe;
}

/* The dictionary invariant: SOPC_NodeId_Equal(a, b) implies equal hashes. Checked on
 * independently built instances, on deep copies, and on repeated calls (determinism). */
START_TEST(test_nodeid_hash_equal_keys)
{
    static const char* const samples[] = {
        "i=2253",
        "ns=1;i=0",
        "ns=1;i=4294967295",
        "s=Objects",
        "ns=1;s=Objects.42",
        "ns=65535;s=Objects.42",
        "g=C496578A-0DFE-4b8f-870A-745238C6AEAE",
        "ns=1;g=C496578A-0DFE-4b8f-870A-745238C6AEAE",
        "b=M/RbKBsRVkePCePcx24oRA==",
        "ns=1;b=M/RbKBsRVkePCePcx24oRA==",
    };

    for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i)
    {
        SOPC_NodeId* a = nodeid_new(samples[i]);
        SOPC_NodeId* b = nodeid_new(samples[i]);
        ck_assert(SOPC_NodeId_Equal(a, b));

        const uint64_t hash_a = nodeid_hash(a);
        ck_assert_uint_eq(hash_a, nodeid_hash(a)); // determinism
        ck_assert_uint_eq(hash_a, nodeid_hash(b)); // independent instances

        SOPC_NodeId copy;
        SOPC_NodeId_Initialize(&copy);
        ck_assert_int_eq(SOPC_STATUS_OK, SOPC_NodeId_Copy(&copy, a));
        ck_assert(SOPC_NodeId_Equal(a, &copy));
        ck_assert_uint_eq(hash_a, nodeid_hash(&copy)); // deep copy
        SOPC_NodeId_Clear(&copy);

        nodeid_delete(a);
        nodeid_delete(b);
    }

    /* A null String identifier (Length -1) and an empty one (Length 0) compare equal, hence
     * they must hash equal too. */
    SOPC_NodeId nullString;
    SOPC_NodeId_Initialize(&nullString);
    nullString.IdentifierType = SOPC_IdentifierType_String;
    nullString.Namespace = 1;
    SOPC_String_Initialize(&nullString.Data.String); // Length -1

    SOPC_NodeId emptyString;
    SOPC_NodeId_Initialize(&emptyString);
    emptyString.IdentifierType = SOPC_IdentifierType_String;
    emptyString.Namespace = 1;
    emptyString.Data.String.Length = 0;

    ck_assert(SOPC_NodeId_Equal(&nullString, &emptyString));
    ck_assert_uint_eq(nodeid_hash(&nullString), nodeid_hash(&emptyString));

    SOPC_NodeId_Clear(&nullString);
    SOPC_NodeId_Clear(&emptyString);
}
END_TEST

/* Every field of the NodeId must take part in the hash, otherwise whole families of keys
 * collapse onto the same bucket. */
START_TEST(test_nodeid_hash_field_sensitivity)
{
    SOPC_NodeId* ns1 = nodeid_new("ns=1;i=5");
    SOPC_NodeId* ns2 = nodeid_new("ns=2;i=5");
    SOPC_NodeId* other = nodeid_new("ns=1;i=6");

    ck_assert_uint_ne(nodeid_hash(ns1), nodeid_hash(ns2));   // Namespace
    ck_assert_uint_ne(nodeid_hash(ns1), nodeid_hash(other)); // identifier

    /* Same payload bytes, different IdentifierType: not equal, so they must not hash equal
     * either or every String key would share a bucket with its ByteString twin. */
    SOPC_NodeId* asString = nodeid_new("ns=1;s=Objects");
    SOPC_NodeId asByteString;
    SOPC_NodeId_Initialize(&asByteString);
    asByteString.IdentifierType = SOPC_IdentifierType_ByteString;
    asByteString.Namespace = 1;
    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_ByteString_CopyFromBytes(&asByteString.Data.Bstring, (const SOPC_Byte*) "Objects", 7));

    ck_assert(!SOPC_NodeId_Equal(asString, &asByteString));
    ck_assert_uint_ne(nodeid_hash(asString), nodeid_hash(&asByteString));

    SOPC_NodeId_Clear(&asByteString);
    nodeid_delete(asString);
    nodeid_delete(other);
    nodeid_delete(ns2);
    nodeid_delete(ns1);
}
END_TEST

/* Quality gate: uniformity of the bits the dictionary actually uses, on the two key shapes an
 * OPC UA address space is made of. */
START_TEST(test_nodeid_hash_distribution)
{
    static const char* const pattern_names[] = {"ns=1;i=<n>", "ns=1;s=Objects.<n>"};
    static const key_pattern_t patterns[] = {KEY_PATTERN_NUMERIC, KEY_PATTERN_STRING};

    const size_t n_patterns = sizeof(patterns) / sizeof(patterns[0]);
    double chi2_ratio[sizeof(patterns) / sizeof(patterns[0])];
    double avg_probes[sizeof(patterns) / sizeof(patterns[0])];
    size_t max_chain[sizeof(patterns) / sizeof(patterns[0])];
    size_t collisions[sizeof(patterns) / sizeof(patterns[0])];

    printf("SOPC_NodeId_Hash distribution over %u keys (table of %zu buckets)\n", DISTRIB_KEYS,
           dict_table_size(DISTRIB_KEYS));

    /* Every pattern is measured and reported before any assertion, so a failure shows the
     * complete picture instead of stopping at the first offending key shape. */
    for (size_t p = 0; p < n_patterns; ++p)
    {
        SOPC_NodeId** keys = build_keys(patterns[p], DISTRIB_KEYS);

        chi2_ratio[p] = low_bits_chi2_ratio(keys, DISTRIB_KEYS);
        collisions[p] = count_hash_collisions(keys, DISTRIB_KEYS);
        measure_probes(keys, DISTRIB_KEYS, &avg_probes[p], &max_chain[p]);

        printf("  %-20s chi2/dof=%.4f avg probes=%.4f max chain=%zu collisions=%zu\n", pattern_names[p], chi2_ratio[p],
               avg_probes[p], max_chain[p], collisions[p]);

        delete_keys(keys, DISTRIB_KEYS);
    }

    for (size_t p = 0; p < n_patterns; ++p)
    {
        ck_assert_uint_eq(0, collisions[p]);
        ck_assert(chi2_ratio[p] < CHI2_MAX_RATIO);
        ck_assert(avg_probes[p] < MAX_AVG_PROBES);
        ck_assert_uint_le(max_chain[p], MAX_PROBE_CHAIN);
    }
}
END_TEST

/*
 * Diffusion of the identifier bits into the bucket index. Flipping any single bit of a numeric
 * identifier must be able to change every bit the dictionary indexes on, otherwise part of the
 * identifier is invisible to bucket selection.
 *
 * The flip rates over the whole 64-bit output are printed rather than asserted: a hash whose
 * step is affine (both DJB and FNV-1a are) cannot reach 0.5 on the lowest bits, and the high
 * bits are unused here, so a strict avalanche bound would be arbitrary.
 */
START_TEST(test_nodeid_hash_avalanche)
{
    uint64_t rng_state = 88172645463325252ULL;
    uint64_t reachable = 0;
    uint32_t flips[64] = {0};

    for (uint32_t b = 0; b < AVALANCHE_BASES; ++b)
    {
        SOPC_NodeId base;
        SOPC_NodeId_Initialize(&base);
        base.Namespace = 1;
        base.Data.Numeric = (uint32_t) xorshift64(&rng_state);
        const uint64_t base_hash = nodeid_hash(&base);

        for (uint32_t bit = 0; bit < NUMERIC_ID_BITS; ++bit)
        {
            SOPC_NodeId flipped = base;
            flipped.Data.Numeric = base.Data.Numeric ^ (1u << bit);

            const uint64_t diff = base_hash ^ nodeid_hash(&flipped);
            reachable |= diff;
            for (uint32_t out = 0; out < 64; ++out)
            {
                flips[out] += (uint32_t)((diff >> out) & 1u);
            }
        }
    }

    const double samples = (double) AVALANCHE_BASES * (double) NUMERIC_ID_BITS;
    double index_rate = 0.0;
    uint32_t live_bits = 0;
    for (uint32_t out = 0; out < 64; ++out)
    {
        if (out < DICT_INDEX_BITS)
        {
            index_rate += (double) flips[out] / samples;
        }
        if (0 != (flips[out]))
        {
            live_bits++;
        }
    }

    printf("SOPC_NodeId_Hash avalanche on numeric ids: %" PRIu32
           "/64 output bits reachable, "
           "mean flip rate on the low %u bits = %.3f\n",
           live_bits, DICT_INDEX_BITS, index_rate / (double) DICT_INDEX_BITS);

    const uint64_t index_mask = (1ULL << DICT_INDEX_BITS) - 1ULL;
    ck_assert_uint_eq(index_mask, reachable & index_mask);
}
END_TEST

Suite* tests_make_suite_nodeid_hash(void)
{
    Suite* s = suite_create("NodeId hash tests");
    TCase* tc_nodeid_hash = tcase_create("NodeId hash");

    tcase_add_test(tc_nodeid_hash, test_nodeid_hash_equal_keys);
    tcase_add_test(tc_nodeid_hash, test_nodeid_hash_field_sensitivity);
    tcase_add_test(tc_nodeid_hash, test_nodeid_hash_distribution);
    tcase_add_test(tc_nodeid_hash, test_nodeid_hash_avalanche);
    suite_add_tcase(s, tc_nodeid_hash);

    return s;
}
