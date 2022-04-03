
#ifndef __NEW_BPT_H__
#define __NEW_BTP_H__

#include "lock_table.h"
#include "trx.h"
#include "file.h"
#include "buffer.h"
#include "page.h"
#include "debug.h"

#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <algorithm>


#define MIN_SLOT_NUMBER 32
#define MAX_SLOT_NUMBER 64

#define MIN_VALUE_SIZE 50
#define MAX_VALUE_SIZE 112

#define MAX_FREE_SPACE 3968U

#define MAX_PAGE_NUMBER 249
#define MAX_KEY_NUMBER 248

void print_page(int64_t table_id, pagenum_t page_number);

pagenum_t db_insert_into_leaf_page_after_splitting(int64_t table_id, pagenum_t root_page_number, pagenum_t leaf_page_number, int64_t key, char* value, uint16_t val_size);

pagenum_t db_insert_into_parent(int64_t table_id, pagenum_t root_page_number, pagenum_t left_page_number, int64_t key, pagenum_t right_page_number);

pagenum_t insert_into_node_after_splitting(int64_t table_id, pagenum_t root_page_number, pagenum_t parent_page_number, int left_index, int64_t key, pagenum_t right_page_number);

pagenum_t db_insert_into_internal(int64_t table_id, pagenum_t root_page_number, pagenum_t parent_page_number, int left_index, int64_t key, pagenum_t right_page_number);

int db_insert_into_leaf(int64_t table_id, pagenum_t leaf_page_number, int64_t key, char* value, uint16_t val_size);

int get_left_index(int64_t table_id, pagenum_t parent_page_number, pagenum_t left_page_number);

pagenum_t db_insert_into_new_root(int64_t table_id, pagenum_t left, int64_t key, pagenum_t right);

pagenum_t db_find_leaf_page_number(int64_t table_id, pagenum_t root_page_number, int key);

pagenum_t db_delete_entry(int64_t table_id, pagenum_t root_page_number, pagenum_t n_page_number, int64_t key, pagenum_t pointer);

pagenum_t coalesce_nodes(int64_t table_id, pagenum_t root_page_number, pagenum_t n_page_number, pagenum_t neighbor_page_number, int neighbor_index, int64_t k_prime);

pagenum_t redistribute_nodes(int64_t table_id, pagenum_t root_page_number, pagenum_t n_page_number, pagenum_t neighbor_page_number, int neighbor_index, int k_prime_index, int64_t k_prime);

int get_neighbor_index(int64_t table_id, pagenum_t n_page_number);

pagenum_t adjust_root(int64_t table_id, pagenum_t root_page_number);

pagenum_t db_remove_entry_from_node(int64_t table_id, pagenum_t n_page_number, int64_t key, pagenum_t pointer);


/* INDEX MANAGER */

int64_t open_table (char *pathname);

int db_insert(int64_t table_id, int64_t key, char * value, uint16_t val_size);

int db_find (int64_t table_id, int64_t key, char * ret_val, uint16_t * val_size);

int db_find (int64_t table_id, int64_t key, char * ret_val, uint16_t * val_size, int trx_id);

int db_delete (int64_t table_id, int64_t key);

int db_update(int64_t table_id, int64_t key, char* values, uint16_t new_val_size, uint16_t* old_val_size, int trx_id);

int init_db (int num_buf);

int shutdown_db();


#endif // __NEW_BTP_H__