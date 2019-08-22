#pragma once

void memory_init();

void* mema(sizet s);
void* mema_zero(sizet s);
void* memra(void* p, sizet s);
void* memra_zero(void* p, sizet s);
void memf(void* p);
void memory_check_leaks();
void memzero(void* p, sizet s);

void* mema_copy(const void* data, sizet s);