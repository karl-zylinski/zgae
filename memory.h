#pragma once

void memory_init();

void* mema(size_t s);
void* mema_zero(size_t s);
void* memra(void* p, size_t s);
void* memra_zero(void* p, size_t s);
void* mema_copy(const void* data, size_t s);
void mema__repl(void** p, size_t s);
void memf(void* p);

#define mema_t(t) (t*)mema(sizeof(t))
#define mema_zero_t(t) (t*)mema_zero(sizeof(t))
#define mema_tn(t, n) (t*)mema(sizeof(t) * n)
#define mema_zero_tn(t, n) (t*)mema_zero(sizeof(t) * n)
#define mema_copy_t(p, t) (t*)mema_copy(p, sizeof(t));
#define mema_repl(p) mema__repl((void**)&p, sizeof(*p))
#define mema_repln(p, n) mema__repl((void**)&p, sizeof(*p)*n)
#define mema_repls(p) mema__repl((void**)&p, strlen(p))

void memory_check_leaks();
void memzero(void* p, size_t s);

/*
void* mema_internal(size_t s);
void memf(void* p);
void memory_check_leaks();
void* mema_copy(const void* data, size_t s);

#define mema(t) (t*)mema_internal(sizeof(t))
#define mema_type(t) mema_zero(sizeof(t))
#define mema_copyt(p) mema_copy(p, sizeof(*p))*/