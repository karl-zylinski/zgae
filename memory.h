#pragma once

void memory_init();

void* mema(size_t s);
void* mema_zero(size_t s);
void* memra(mut void* cur, size_t s);
void* memra_zero_added(mut void* cur, size_t new_size, size_t old_size);
void* mema_copy(void* data, size_t s);
void mema__repl(mut void** p, size_t s);
void memf(mut void* p);

#define mema_t(t) (t*)mema(sizeof(t))
#define mema_zero_t(t) (t*)mema_zero(sizeof(t))
#define mema_tn(t, n) (t*)mema(sizeof(t) * n)
#define mema_zero_tn(t, n) (t*)mema_zero(sizeof(t) * n)
#define mema_copy_t(p, t) (t*)mema_copy(p, sizeof(t));

#define memra_t(p, t) (t*)memra(p, sizeof(t))
#define memra_tn(p, t, n) (t*)memra(p, sizeof(t) * n)

#define mema_repl(p) mema__repl((void**)&p, sizeof(*p))
#define mema_repln(p, n) mema__repl((void**)&p, sizeof(*p)*n)
#define mema_repls(p) mema__repl((void**)&p, strlen(p))

void memory_check_leaks();
void memzero(mut void* p, size_t s);
