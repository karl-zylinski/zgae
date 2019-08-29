#pragma once

void memory_init();

void* mema(u64 s);
void* mema_zero(u64 s);
void* memra(void* cur, u64 s);
void* memra_zero_added(void* cur, u64 new_size, u64 old_size);
void* mema_copy(void* data, u64 s);
void mema__repl(void** p, u64 s);
void memf(void* p);

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
void memzero(void* p, u64 s);
