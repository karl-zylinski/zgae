#pragma once

void* mema(size_t size);
void* mema_zero(size_t size);
void* memra(void* cur, size_t size);
void* memra_zero(void* cur, size_t size);
void memf(void* p);
void memory_check_leaks();

void memcpy_alloc(void** dest, void* source, size_t size);