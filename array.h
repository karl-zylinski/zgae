#pragma once
#include "memory.h"

// Based on "strechy buffers" by Sean Barrett and Niklas Gray

struct ArrayHeader
{
    size_t num;
    size_t capacity;
};

#define array_header(a)\
    ((ArrayHeader*)((char*)(a) - sizeof(ArrayHeader)))

#define array_num(a) ((a) ? array_header(a)->num : 0)

#define array_full(a) ((a) ? (array_header(a)->num == array_header(a)->capacity) : true)

static void array_destroy(void* a)
{
    if (a == 0)
        return;

    zfree(array_header(a));
}

static void* array_grow(void* old_a, size_t item_size)
{
    Assert(item_size != 0, "Trying to grow array with item_size zero.");
    size_t old_capacity = old_a ? array_header(old_a)->capacity : 0;
    size_t num = array_num(old_a);
    size_t new_capacity = old_capacity == 0 ? 2 : old_capacity*2;
    ArrayHeader* new_h = (ArrayHeader*)zalloc(sizeof(ArrayHeader) + new_capacity*item_size);
    void* new_a = ptr_add(new_h, sizeof(ArrayHeader));
    new_h->num = num;
    new_h->capacity = new_capacity;
    if (old_a)
        memcpy(new_a, old_a, item_size * num);
    array_destroy(old_a);
    return new_a;
}

static void* _array_copy_data(void* a, size_t item_size)
{
    size_t num = array_num(a);
    size_t data_size = num * item_size;
    void* copied_data = zalloc(data_size);
    memcpy(copied_data, a, data_size);
    return copied_data;
}

#define array_copy_data(a) (_array_copy_data((a), sizeof(*(a))))

static void* _array_move_data(void* a, size_t item_size)
{
    void* copied = _array_copy_data(a, item_size);
    array_destroy(a);
    return copied;
}

#define array_move_data(a) (_array_move_data((a), sizeof(*(a))))

static void _array_maybe_grow(void** a, size_t item_size)
{
    if ((*a) != nullptr && !array_full(*a))
        return;

    void* new_a = array_grow(*a, item_size);
    memcpy(a, &new_a, sizeof(void*));
}

#define array_push(a, item) \
    _array_maybe_grow(&(void*)(a), sizeof(*(a))), \
    (a)[array_header(a)->num++] = item

#define array_init(a) (_array_maybe_grow(&(void*)(a), sizeof(*(a))))

static void _array_remove(void* a, size_t idx, size_t item_size)
{
    if (idx + 1 == array_num(a))
    {
        array_header(a)->num--;
        return;
    }

    memmove(((char*)a) + idx*item_size, ((char*)a) + (idx + 1)*item_size, array_num(a) - idx - 1);
}

#define array_remove(a, idx) _array_remove(a, idx, sizeof(*(a)))