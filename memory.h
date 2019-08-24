#pragma once

void memory_init();

void* mema(size_t s);
void* mema_zero(size_t s);
void* memra(void* p, size_t s);
void* memra_zero(void* p, size_t s);
void memf(void* p);
void memory_check_leaks();
void memzero(void* p, size_t s);
void* mema_copy(const void* data, size_t s);

#define mema_repl(p) p = mema_copy(p, sizeof(*p))
#define mema_repln(p, n) p = mema_copy(p, sizeof(*p)*n)
#define mema_repls(p) p = mema_copy(p, strlen(p))