#ifndef __LOCK_TABLE_H__
#define __LOCK_TABLE_H__

#include <stdint.h>
#include <pthread.h>
#include <unordered_map>
#include "file.h"
#include "debug.h"
#include <map>


/* APIs for lock table */

extern pthread_mutex_t lock_table_latch;

typedef struct lock_t lock_t;
typedef struct lock_table lock_table_t;
struct hash_table_entry{
    int64_t table_id;
    int64_t page_id;
    std::map<int64_t, lock_t*> head_map;
    std::map<int64_t, lock_t*> tail_map;
};

struct lock_t{
    lock_t* prev;
    lock_t* next;
    hash_table_entry* sentinel;
    pthread_cond_t cond;
    int64_t record_id;
    int lock_mode;
    lock_t* trx_prev;
    lock_t* trx_next;
    int trx_id;

    bool is_acquired;
    bool in_trx;

    char* value;
};

struct hash{
    std::size_t operator()(const std::pair<int64_t, int64_t>& k) const{
        return (k.first % 10) + (k.second % 1000) * 100;
    }
};


struct lock_table{
    std::unordered_map<std::pair<int64_t, int64_t>, hash_table_entry*, hash> table;
};


int init_lock_table();
lock_t *lock_acquire(int64_t table_id, pagenum_t page_id, int64_t key, int trx_id, int lock_mode);
int lock_release(lock_t* lock_obj);

#include "trx.h"
#endif /* __LOCK_TABLE_H__ */
