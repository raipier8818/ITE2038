#include "trx.h"

pthread_mutex_t trx_table_latch;
struct trx_table_entry{
    int64_t trx_id;
    lock_t *first;
    lock_t *last;
};


std::unordered_map<int64_t, trx_table_entry*> trx_table;

typedef struct trx_table_entry trx_table_entry;

#ifdef DEBUG_TRX_H

void print_trx(int trx_id){
    trx_table_entry* temp_entry = trx_table[trx_id];
    if(temp_entry == nullptr){
        print_temp("Invalid trx id.");
        return;
    }

    printf("< TRX %d >\n", trx_id);
    lock_t* temp_lock = temp_entry->first;
    while(temp_lock != nullptr){
        printf("[ trx_id : %d, record_id : %ld, lock_mode : %d, is_acquired : %d ] \n", temp_lock->trx_id, temp_lock->record_id, temp_lock->lock_mode, temp_lock->is_acquired);
        temp_lock = temp_lock->trx_next;
    }
}

#endif


int trx_init(){
    #ifdef DEBUG_TRX_H
    print_trx_function_start(__func__);
    #endif

    trx_table_latch = PTHREAD_MUTEX_INITIALIZER;
    
    #ifdef DEBUG_TRX_H
    print_trx_function_end(__func__);
    #endif
    
    return 0;
}

uint64_t trx_id = 0;

int trx_begin(){
    pthread_mutex_lock(&trx_table_latch);

    #ifdef DEBUG_TRX_H
    print_trx_function_start(__func__);
    #endif


    trx_table_entry *entry = new trx_table_entry;
    entry->trx_id = ++trx_id;
    entry->first = nullptr;
    entry->last = nullptr;

    trx_table[trx_id] = entry;
    printf(" >> Trx %ld begin.\n", trx_id);
    

    #ifdef DEBUG_TRX_H
    print_trx_function_end(__func__);
    #endif

    pthread_mutex_unlock(&trx_table_latch);
    

    return trx_table[trx_id]->trx_id;
}

int trx_commit(int trx_id){
    pthread_mutex_lock(&trx_table_latch);
    
    #ifdef DEBUG_TRX_H
    printf("trx_table_latch lock.\n");
    print_trx_function_start(__func__);
    print_trx(trx_id);
    #endif

    if(trx_id < 1){
        
        #ifdef DEBUG_TRX_H
        print_trx_function_end(__func__);
        #endif
        // pthread_mutex_unlock(&trx_table_latch); 
        // pthread_mutex_unlock(&lock_table_latch);

        exit(1);
    }

    trx_table_entry *entry = trx_table[trx_id];
    if(entry == nullptr){

        #ifdef DEBUG_TRX_H
        print_trx_function_end(__func__);
        #endif

        // pthread_mutex_unlock(&trx_table_latch); 
        // pthread_mutex_unlock(&lock_table_latch);

        exit(1);
    }

    lock_t *lock_obj = entry->first;
    while(lock_obj != nullptr){
        pthread_mutex_unlock(&trx_table_latch);
        lock_t *next_obj = lock_obj->trx_next;
        lock_release(lock_obj);
        lock_obj = next_obj;
        pthread_mutex_lock(&trx_table_latch);
    }

    trx_table.erase(trx_id);
    trx_table[trx_id] = nullptr;
    delete entry;

    #ifdef DEBUG_TRX_H
    printf("trx_table_latch unlock.\n");
    print_trx_function_end(__func__);
    #endif

    pthread_mutex_unlock(&trx_table_latch);

    return trx_id;
}


lock_t* trx_add(int trx_id, lock_t* lock_ptr){
    // pthread_mutex_lock(&trx_table_latch);

    #ifdef DEBUG_TRX_H
    printf("trx_table_latch lock.\n");
    print_trx_function_start(__func__);
    printf("Add %d\n", lock_ptr->record_id);
    // print_trx(trx_id);
    #endif

    if(trx_id < 1 || lock_ptr == nullptr) {
        #ifdef DEBUG_TRX_H
        print_temp("Invalid trx_id or Invalid lock_ptr");
        #endif

        exit(1);

        pthread_mutex_unlock(&trx_table_latch);
    }

    trx_table_entry* trx = trx_table[trx_id];
    if(trx == nullptr) {
        #ifdef DEBUG_TRX_H
        printf(" >> %d\n", trx_id);
        print_temp("Invalid trx_id.");
        print_trx_function_end(__func__);
        #endif
        pthread_mutex_unlock(&trx_table_latch);

        exit(1);
    }

    if(trx->first == nullptr){
        #ifdef DEBUG_TRX_H
        print_temp("First lock.");
        #endif

        lock_ptr->trx_next = nullptr;
        lock_ptr->trx_prev = nullptr;
        trx->first = lock_ptr;
        trx->last = lock_ptr;
    }
    else{
        #ifdef DEBUG_TRX_H
        print_temp("Not first lock.");
        #endif

        lock_ptr->trx_next = nullptr;
        lock_ptr->trx_prev = trx->last;
        trx->last->trx_next = lock_ptr;
        trx->last = lock_ptr;
    }

    // TODO : Deadlock detection
    // if(is_deadlock(lock_ptr)){
    //     trx_abort(trx_id);
    //     return nullptr;
    // }    

    #ifdef DEBUG_TRX_H
    printf("trx_table_latch unlock.\n");
    print_trx_function_end(__func__);
    #endif

    // pthread_mutex_unlock(&trx_table_latch);

    return lock_ptr;
    
}

bool is_deadlock(lock_t* lock_ptr){
    if(lock_ptr->is_acquired == true){
        return false;
    }


    return false;
}

void trx_abort(int trx_id){
    pthread_mutex_lock(&trx_table_latch);
    if(trx_id < 1){
        pthread_mutex_unlock(&trx_table_latch);
        return;
    }

    trx_table_entry *entry = trx_table[trx_id];
    if(entry == nullptr){
        pthread_mutex_unlock(&trx_table_latch);
        return;
    }
    
    // TODO : rollback.

    

    trx_table.erase(trx_id);
    delete entry;
    pthread_mutex_unlock(&trx_table_latch);
    return;
}