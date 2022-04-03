#include "log.h"

typedef struct dbms_log dbms_log;
struct dbms_log{
    int64_t lsn;
    int64_t table_id;
    int64_t record_id;
    char* before_value;
    char* after_value;
    int trx_id;
    int type; // 0: find, 1: update, 2: commit, -1: abort
};

std::vector<dbms_log*> logs;
int64_t lsn;

pthread_mutex_t log_mutex;

int init_log_buffer(){
    lsn = -1;
    logs.clear();
    log_mutex = PTHREAD_MUTEX_INITIALIZER;
    return 0;
}

int64_t add_normal_log(int64_t table_id, int64_t record_id, char* before_value, char* after_value, int type, int trx_id){
    pthread_mutex_lock(&log_mutex);
    lsn++;
    dbms_log* log = new dbms_log();
    log->lsn = lsn;
    log->trx_id = trx_id;
    log->type = type;
    log->table_id = table_id;
    log->record_id = record_id;
    log->before_value = before_value;
    log->after_value = after_value;
    logs.push_back(log);
    pthread_mutex_unlock(&log_mutex);
    return lsn;
}

int64_t add_commit_log(int trx_id){
    pthread_mutex_lock(&log_mutex);
    lsn++;
    dbms_log* log = new dbms_log();
    log->lsn = lsn;
    log->trx_id = trx_id;
    log->type = 2;
    log->table_id = -1;
    log->record_id = -1;
    log->before_value = NULL;
    log->after_value = NULL;
    logs.push_back(log);
    pthread_mutex_unlock(&log_mutex);
    return lsn;
}

int64_t add_abort_log(int trx_id){
    pthread_mutex_lock(&log_mutex);
    lsn++;
    dbms_log* log = new dbms_log();
    log->lsn = lsn;
    log->trx_id = trx_id;
    log->type = -1;
    log->table_id = -1;
    log->record_id = -1;
    log->before_value = NULL;
    log->after_value = NULL;
    logs.push_back(log);
    pthread_mutex_unlock(&log_mutex);
    return lsn;
}

