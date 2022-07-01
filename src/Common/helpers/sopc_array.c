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

#include "sopc_array.h"
#include "sopc_mem_alloc.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h> /* qsort */
#include <string.h>

#define ARRAY_ELT(a, idx) ((void*) (a->data + idx * a->element_size))

struct _SOPC_Array
{
    uint8_t* data;
    size_t element_size;
    size_t sz;
    size_t cap;
    SOPC_Array_Free_Func* free_func;
};

static bool array_grow(SOPC_Array* a, size_t min_size)
{
    assert(a != NULL);

    if (a->cap >= min_size)
    {
        return true;
    }

    size_t cap = (a->cap != 0 ? a->cap : 1);

    while (cap < min_size)
    {
        cap *= 2;
    }

    void* data = SOPC_Realloc(a->data, a->cap * a->element_size, cap * a->element_size);

    if (data == NULL)
    {
        return false;
    }

    a->data = data;
    a->cap = cap;

    return true;
}

SOPC_Array* SOPC_Array_Create(size_t element_size, size_t initial_capacity, SOPC_Array_Free_Func* free_func)
{
    SOPC_Array* a = SOPC_Calloc(1, sizeof(SOPC_Array));

    if (a == NULL)
    {
        return NULL;
    }

    a->element_size = element_size;
    a->free_func = free_func;

    if (!array_grow(a, initial_capacity))
    {
        SOPC_Free(a);
        return NULL;
    }

    return a;
}

SOPC_Array* SOPC_Array_Copy(const SOPC_Array* array)
{
    assert(array != NULL);

    SOPC_Array* copy = SOPC_Array_Create(array->element_size, array->sz, array->free_func);

    if (copy == NULL)
    {
        return NULL;
    }

    if (copy->data != NULL)
    {
        memcpy(copy->data, array->data, array->element_size * array->sz);
    }

    copy->sz = array->sz;

    return copy;
}

void SOPC_Array_Delete(SOPC_Array* array)
{
    if (array == NULL)
    {
        return;
    }

    if (array->free_func != NULL)
    {
        for (size_t i = 0; i < array->sz; ++i)
        {
            array->free_func(ARRAY_ELT(array, i));
        }
    }

    SOPC_Free(array->data);
    SOPC_Free(array);
}

bool SOPC_Array_Append_Values(SOPC_Array* array, const void* data, size_t n_elements)
{
    assert(array != NULL);

    if (n_elements == 0)
    {
        return true;
    }

    if (!array_grow(array, array->sz + n_elements))
    {
        return false;
    }

    if (data != NULL)
    {
        memcpy(ARRAY_ELT(array, array->sz), data, n_elements * array->element_size);
    }
    else
    {
        memset(ARRAY_ELT(array, array->sz), 0, n_elements * array->element_size);
    }

    array->sz += n_elements;

    return true;
}

void* SOPC_Array_Get_Ptr(const SOPC_Array* array, size_t index)
{
    assert(array != NULL);
    assert(index < array->sz);

    return ARRAY_ELT(array, index);
}

size_t SOPC_Array_Size(const SOPC_Array* array)
{
    assert(array != NULL);
    return array->sz;
}

void SOPC_Array_Sort(SOPC_Array* array, SOPC_Array_Compare_Func* compare_func)
{
    assert(array != NULL);
    qsort(array->data, array->sz, array->element_size, compare_func);
}

void* SOPC_Array_Into_Raw(SOPC_Array* array)
{
    assert(array != NULL);

    void* data = array->data;

    if (array->sz < array->cap)
    {
        data = SOPC_Realloc(data, array->cap * array->element_size, array->sz * array->element_size);
    }

    SOPC_Free(array);

    return data;
}

SOPC_Array_Free_Func* SOPC_Array_Get_Free_Func(SOPC_Array* array)
{
    assert(array != NULL);
    return array->free_func;
}

void SOPC_Array_Set_Free_Func(SOPC_Array* array, SOPC_Array_Free_Func* func)
{
    assert(array != NULL);
    array->free_func = func;
}
