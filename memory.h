#pragma once

void* mema(uint64_t size);
void* mema_zero(uint64_t size);
void* memra(void* cur, uint64_t size);
void* memra_zero(void* cur, uint64_t size);
void memf(void* p);
void memory_check_leaks();