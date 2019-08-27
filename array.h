#pragma once

template<typename T>
struct Array
{
    T* data;
    size_t num;
    size_t cap;

    const T& operator[](size_t i) const
    {
        return data[i];
    }

    T& operator[](size_t i)
    {
        return data[i];
    }
};

void* arr_mem_realloc(void* d, size_t s);
void* arr_mem_copy(void* d, size_t s);
void arr_mem_move(void* dest, void* source, size_t s);
void arr_mem_free(void* d);

template<typename T>
void array_maybe_grow(Array<T>* a)
{
    if (a->num < a->cap)
        return;

    a->cap = a->cap == 0 ? 1 : a->cap * 2;
    a->data = (T*)arr_mem_realloc(a->data, sizeof(T) * a->cap);
}

template<typename T>
void array_destroy(Array<T>* a)
{
    arr_mem_free(a->data);
}

template<typename T>
void array_push(Array<T>* a, const T& item)
{
    array_maybe_grow(a);
    a->data[a->num++] = item;
}

template<typename T>
void array_insert(Array<T>* a, const T& item, size_t idx)
{
    if (idx == a->num)
    {
        array_push(a, item);
        return;
    }

    array_maybe_grow(a);
    arr_mem_move(((char*)(a->data)) + (idx + 1)*sizeof(T), ((char*)(a->data)) + idx*sizeof(T), (a->num - idx)*sizeof(T));
    a->data[idx] = item;
    ++a->num;
}

template<typename T>
const T& array_last(const Array<T>& a)
{
    return a.data[a.num - 1];
}

template<typename T>
T array_pop(Array<T>* a)
{
    return a->data[--a->num];
}

template<typename T>
T* array_copy_data(const Array<T>& a)
{
    return (T*)arr_mem_copy(a.data, sizeof(T) * a.num);
}

template<typename T>
void array_fill_and_set(Array<T>* a, const T& item, size_t idx)
{
    if (idx < a->num)
    {
        a->data[idx] = item;
        return;
    }

    size_t min_num = idx + 1;
    if (min_num > a->cap)
    {
        a->cap = min_num;
        a->data = (T*)arr_mem_realloc(a->data, sizeof(T) * a->cap);
    }

    if (min_num > a->num)
        a->num = min_num;

    a->data[idx] = item;
}