#include "lock_table.h"

pthread_mutex_t lock_table_latch;
lock_table_t lock_table;

#ifdef DEBUG_LOCK_TABLE_H

void print_lock_table(std::pair<int64_t, int64_t> key){
    hash_table_entry* temp = lock_table.table[key];
    if(temp == nullptr){
        print_temp("No lock table entry.");
        return;
    }

    printf("< LOCK TABLE >\n");

    for(auto it = temp->head_map.begin(); it != temp->head_map.end(); it++){
        lock_t* target = it->second;
        printf("<record id : %d>\n", target->record_id);
        while (target != nullptr)
        {
            printf("[ trx_id : %d, record_id : %ld, lock_mode : %d, is_acquired : %d ] \n", target->trx_id, target->record_id, target->lock_mode, target->is_acquired);
            target = target->next;
        }
    }

    
}

#endif

int init_lock_table(){
    #ifdef DEBUG_LOCK_TABLE_H
    print_lock_table_function_start(__func__);
    #endif

    lock_table_latch = PTHREAD_MUTEX_INITIALIZER;

    #ifdef DEBUG_LOCK_TABLE_H
    print_lock_table_function_end(__func__);
    #endif

    return 0;
}


lock_t *lock_acquire(int64_t table_id, pagenum_t page_id, int64_t key, int trx_id, int lock_mode){
    pthread_mutex_lock(&lock_table_latch);
    
    #ifdef DEBUG_LOCK_TABLE_H
    printf("lock_table_latch lock.\n");
    print_lock_table_function_start(__func__);
    print_lock_table(std::make_pair(table_id, page_id));
    printf("Trx %d, Acquire %d, lock_mode %d\n", trx_id, key, lock_mode);
    #endif

    std::pair<int64_t, int64_t> temp_pair = std::make_pair(table_id, page_id);
    hash_table_entry* temp_entry = lock_table.table[temp_pair];

    if(temp_entry == nullptr){
        temp_entry = new hash_table_entry();
        temp_entry->table_id = table_id;
        temp_entry->page_id = page_id;
        temp_entry->head_map.clear();
        temp_entry->tail_map.clear();
        lock_table.table[temp_pair] = temp_entry;
    }

    temp_entry = lock_table.table[temp_pair];

    lock_t* new_lock = new lock_t;
    new_lock->sentinel = temp_entry;
    new_lock->cond = PTHREAD_COND_INITIALIZER;
    new_lock->record_id = key;
    new_lock->lock_mode = lock_mode;
    new_lock->trx_next = nullptr;
    new_lock->trx_prev = nullptr;
    new_lock->next = nullptr;
    new_lock->prev = nullptr;
    new_lock->trx_id = trx_id;
    new_lock->is_acquired = true;

    lock_t* head_lock = temp_entry->head_map[key];
    lock_t* tail_lock = temp_entry->tail_map[key];

    if(head_lock == nullptr && tail_lock == nullptr){
        temp_entry->head_map[key] = new_lock;
        temp_entry->tail_map[key] = new_lock;
    }else{
        lock_t* temp_lock = head_lock;
        lock_t* same_trx_lock = nullptr;
        lock_t* other_trx_lock = nullptr;
        lock_t* x_other_trx_lock = nullptr;

        while(temp_lock != nullptr){
            if(temp_lock->trx_id == trx_id){
                if(lock_mode == 0 || temp_lock->lock_mode == 1){
                    delete new_lock;

                    #ifdef DEBUG_LOCK_TABLE_H
                    printf("lock_table_latch unlock.\n");
                    print_lock_table_function_end(__func__);
                    #endif

                    pthread_mutex_unlock(&lock_table_latch);
                    return temp_lock;
                }else{
                    same_trx_lock = temp_lock;
                }
            }else{
                other_trx_lock = temp_lock;
                if(temp_lock->lock_mode == 1){
                    x_other_trx_lock = temp_lock;
                }
            }
            temp_lock = temp_lock->next;
        }

        if(same_trx_lock != nullptr && other_trx_lock == nullptr){
            same_trx_lock->lock_mode = 1;
            delete new_lock;
            #ifdef DEBUG_LOCK_TABLE_H
            printf("lock_table_latch unlock.\n");
            print_lock_table_function_end(__func__);
            #endif

            pthread_mutex_unlock(&lock_table_latch);

            return same_trx_lock;
        }
        

        if(tail_lock->is_acquired == false){
            new_lock->is_acquired = false;
        }else{
            if(lock_mode == 0){
                if(x_other_trx_lock == nullptr){
                    new_lock->is_acquired = true;
                }else{
                    new_lock->is_acquired = false;
                }
            }else{
                if(other_trx_lock == nullptr){
                    new_lock->is_acquired = true;
                }else{
                    new_lock->is_acquired = false;
                }
            }
        }


        tail_lock->next = new_lock;
        new_lock->prev = tail_lock;
        // temp_entry->tail_map.find(key)->second = new_lock;
        temp_entry->tail_map.erase(key);
        temp_entry->tail_map[key] = new_lock;
    }

    // pthread_mutex_unlock(&lock_table_latch);
    new_lock = trx_add(trx_id, new_lock);
    // pthread_mutex_lock(&lock_table_latch);

    if(new_lock == nullptr){
        // deadlock
        exit(1);
    }

    if(new_lock->is_acquired == false){
        #ifdef DEBUG_LOCK_TABLE_H
        print_temp("Waiting");
        #endif

        pthread_cond_wait(&new_lock->cond, &lock_table_latch);
    
        #ifdef DEBUG_LOCK_TABLE_H
        print_temp("Wake up");
        #endif
    }

    #ifdef DEBUG_LOCK_TABLE_H
    printf("lock_table_latch unlock.\n");
    print_lock_table_function_end(__func__);
    #endif

    pthread_mutex_unlock(&lock_table_latch);

    return new_lock;
}

int lock_release(lock_t* lock_obj){
    pthread_mutex_lock(&lock_table_latch);

    #ifdef DEBUG_LOCK_TABLE_H
    printf("lock_table_latch lock.\n");
    // print_lock_table(std::make_pair(lock_obj->sentinel->table_id, lock_obj->sentinel->page_id));
    print_lock_table_function_start(__func__);
    #endif

    if(lock_obj == nullptr){
        #ifdef DEBUG_LOCK_TABLE_H
        print_lock_table_error(__FILE__, __func__, __LINE__);
        #endif

        // pthread_mutex_unlock(&lock_table_latch);
        exit(1);
    }

    hash_table_entry* temp_hash_table_entry = lock_obj->sentinel;

    if(temp_hash_table_entry == nullptr){
        #ifdef DEBUG_LOCK_TABLE_H
        print_lock_table_function_end(__func__);
        #endif
        
        // pthread_mutex_unlock(&lock_table_latch);
        exit(1);
    }

    if(lock_obj->prev == nullptr && lock_obj->next == nullptr){
        temp_hash_table_entry->head_map.erase(lock_obj->record_id);
        temp_hash_table_entry->tail_map.erase(lock_obj->record_id);

        delete lock_obj;

        #ifdef DEBUG_LOCK_TABLE_H
        printf("lock_table_latch unlock.\n");
        print_lock_table_function_end(__func__);
        #endif

        pthread_mutex_unlock(&lock_table_latch);

        return 0;
    }else if(lock_obj->prev != nullptr && lock_obj->next == nullptr){
        temp_hash_table_entry->tail_map.find(lock_obj->record_id)->second = lock_obj->prev;
        lock_obj->prev->next = nullptr;
        delete lock_obj;

        #ifdef DEBUG_LOCK_TABLE_H
        printf("lock_table_latch unlock.\n");
        print_lock_table_function_end(__func__);
        #endif

        pthread_mutex_unlock(&lock_table_latch);

        return 0;
    }else if(lock_obj->prev == nullptr && lock_obj->next != nullptr){
        temp_hash_table_entry->head_map.find(lock_obj->record_id)->second = lock_obj->next;
        lock_obj->next->prev = nullptr;
    }else{
        lock_obj->prev->next = lock_obj->next;
        lock_obj->next->prev = lock_obj->prev;
    }

    if(lock_obj->next->is_acquired == false && lock_obj->prev == nullptr){
        pthread_cond_signal(&lock_obj->next->cond);

        lock_t* temp_lock = lock_obj->next->next;

        if(lock_obj->next->lock_mode == 0){
            while(temp_lock != nullptr){
                if(temp_lock->lock_mode == 0){
                    pthread_cond_signal(&temp_lock->cond);
                }
                else break;
                temp_lock = temp_lock->next;
            }
        }
    }


    delete lock_obj;

    #ifdef DEBUG_LOCK_TABLE_H
    printf("lock_table_latch unlock.\n");
    print_lock_table_function_end(__func__);
    #endif

    pthread_mutex_unlock(&lock_table_latch);

    return 0;
}