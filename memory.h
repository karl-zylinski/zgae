#pragma once

void* mema(size_t s);
void* mema_zero(size_t s);
void* memra(void* p, size_t s);
void* memra_zero(void* p, size_t s);
void memf(void* p);
void memory_check_leaks();
void memzero(void* p, size_t s);

void memcpy_alloc(void** dest, void* source, size_t size);