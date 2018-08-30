#pragma once
#include "memory.h"

// Based on "strechy buffers" by Sean Barrett and Niklas Gray with super-ugly hacks to make it
// work in C++ (no impicit void* cast made everthing pooooop)

struct ArrayHeader
{
    size_t size;
    size_t capacity;
};

#define array_header(a)\
    ((ArrayHeader*)((char*)(a) - sizeof(ArrayHeader)))

#define array_size(a) ((a) ? array_header(a)->size : 0)
#define ArrayDefaultCapacity 2
#define array_capacity(a) ((a) ? array_header(a)->capacity : ArrayDefaultCapacity)

#define array_full(a) ((a) ? (array_header(a)->size == array_header(a)->capacity) : true)

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
    size_t num = array_size(old_a);
    size_t new_capacity = old_capacity == 0 ? ArrayDefaultCapacity : old_capacity*2;
    ArrayHeader* new_h = (ArrayHeader*)zalloc(sizeof(ArrayHeader) + new_capacity*item_size);
    void* new_a = ptr_add(new_h, sizeof(ArrayHeader));
    new_h->size = num;
    new_h->capacity = new_capacity;
    if (old_a)
        memcpy(new_a, old_a, item_size * num);
    array_destroy(old_a);
    return new_a;
}

static void* _array_copy_data(void* a, size_t item_size)
{
    size_t num = array_size(a);
    size_t data_size = num * item_size;
    void* copied_data = zalloc(data_size);
    memcpy(copied_data, a, data_size);
    return copied_data;
}

#define array_copy_data(a) (_array_copy_data((a), sizeof(*(a))))

static void* _array_grab_data(void* a, size_t item_size)
{
    void* copied = _array_copy_data(a, item_size);
    array_destroy(a);
    return copied;
}

#define array_grab_data(a) (_array_grab_data((a), sizeof(*(a))))

static void _array_maybe_grow(void** ap, size_t item_size)
{
    if ((*ap) != nullptr && !array_full(*ap))
        return;

    void* new_a = array_grow(*ap, item_size);
    memcpy(ap, &new_a, sizeof(void*));
}

#define array_push(a, item) \
    _array_maybe_grow(&(void*)(a), sizeof(*(a))), \
    (a)[array_header(a)->size++] = item

#define array_init(a) (_array_maybe_grow(&(void*)(a), sizeof(*(a))))

static void _array_remove(void* a, size_t idx, size_t item_size)
{
    Assert(idx >= 0 && idx < array_size(a), "Trying to remove outside of array.");

    if (idx + 1 == array_size(a))
    {
        array_header(a)->size--;
        return;
    }

    memmove(((char*)a) + idx*item_size, ((char*)a) + (idx + 1)*item_size, (array_size(a) - idx - 1)*item_size);
}

#define array_remove(a, idx) _array_remove(a, idx, sizeof(*(a)))

static size_t _array_maybe_make_insert_room(void** ap, size_t idx, size_t item_size)
{
    Assert(idx <= array_size(*ap), "Trying to make room for index outside of array.");
    _array_maybe_grow(ap, item_size);
    if (idx == array_size(*ap))
    {
        ++(array_header(*ap)->size);
        return idx;
    }
    memmove(((char*)(*ap)) + (idx + 1)*item_size, ((char*)(*ap)) + idx*item_size, (array_size(*ap) - idx)*item_size);
    ++(array_header(*ap)->size);
    return idx;
}


#define array_insert(a, item, idx) (a)[_array_maybe_make_insert_room(&(void*)(a), idx, sizeof(*(a)))] = item

static void array_empty(void* a)
{
    if (a == nullptr)
        return;
    
    array_header(a)->size = 0;
}