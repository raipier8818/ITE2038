#ifndef __TRX_H__
#define __TRX_H__

#include <unordered_map>
#include <pthread.h>
#include "lock_table.h"
#include "debug.h"
#include <map>

extern pthread_mutex_t trx_table_latch;

int trx_init();

int trx_begin();

int trx_commit(int trx_id);

lock_t* trx_add(int trx_id, lock_t* lock_ptr);

bool is_deadlock(lock_t* lock_ptr);

void trx_abort(int trx_id);

#endif // __TRX_H__