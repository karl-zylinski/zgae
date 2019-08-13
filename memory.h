#pragma once

void* mema(uint64 size);
void* mema_zero(uint64 size);
void* memra(void* cur, uint64 size);
void* memra_zero(void* cur, uint64 size);
void memf(void* p);
void memory_check_leaks();