/*
 *  Copyright (C) 2018 Systerel and others.
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

#include "sopc_array.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_ELT(a, idx) ((void*) (a->data + idx * a->element_size))

struct _SOPC_Array
{
    uint8_t* data;
    size_t element_size;
    size_t sz;
    size_t cap;
    SOPC_Array_Free_Func free_func;
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

    void* data = realloc(a->data, cap * a->element_size);

    if (data == NULL)
    {
        return false;
    }

    a->data = data;
    a->cap = cap;

    return true;
}

SOPC_Array* SOPC_Array_Create(size_t element_size, size_t initial_capacity, SOPC_Array_Free_Func free_func)
{
    SOPC_Array* a = calloc(1, sizeof(SOPC_Array));

    if (a == NULL)
    {
        return NULL;
    }

    a->element_size = element_size;
    a->free_func = free_func;

    if (!array_grow(a, initial_capacity))
    {
        free(a);
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

    free(array->data);
    free(array);
}

bool SOPC_Array_Append_Values(SOPC_Array* array, void* data, size_t n_elements)
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

    memcpy(ARRAY_ELT(array, array->sz), data, n_elements * array->element_size);
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

void SOPC_Array_Sort(SOPC_Array* array, SOPC_Array_Compare_Func compare_func)
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
        data = realloc(data, array->sz * array->element_size);
    }

    free(array);

    return data;
}

SOPC_Array_Free_Func SOPC_Array_Get_Free_Func(SOPC_Array* array)
{
    assert(array != NULL);
    return array->free_func;
}

void SOPC_Array_Set_Free_Func(SOPC_Array* array, SOPC_Array_Free_Func func)
{
    assert(array != NULL);
    array->free_func = func;
}
