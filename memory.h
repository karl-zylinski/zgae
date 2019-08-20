#pragma once

void memory_init();

void* mema(size_t s);
void* mema_zero(size_t s);
void* memra(void* p, size_t s);
void* memra_zero(void* p, size_t s);
void memf(void* p);
void memory_check_leaks();
void memzero(void* p, size_t s);

void* mema_copy(void* data, size_t s);