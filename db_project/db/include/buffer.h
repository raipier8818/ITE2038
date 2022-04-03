#ifndef __BUFFER_H__
#define __BUFFER_H__

#define MIN_BUFFER_SIZE 10
#define MAX_NUM_OF_FILE 20

#include <stdlib.h>
#include <vector>
#include <pthread.h>
#include <map>
#include "file.h"
#include "debug.h"
#include "page.h"

extern pthread_mutex_t buffer_mutex;

/*
 * Control block's prev and next are used for iterating all pages in buffer.
 * They are connected like double linked list.
 * The entry point is equal to the oldest page's index in buffer.
 * So, if you want to find the page that accessed latest, then you can find that page by using the prev of entry point.
 * Header page only uses table_id and page_number. 
*/
struct control_block{
    int64_t table_id;
    pagenum_t page_number;
    int is_dirty;
    int is_pinned;
    pthread_mutex_t page_mutex = PTHREAD_MUTEX_INITIALIZER;
    size_t prev;
    size_t next;
};


int init_buffer(int num_buf);

int find_index_from_page_buffer(int64_t table_id, uint64_t page_number);

int read_new_page(int64_t table_id, uint16_t page_number);

uint64_t buffer_alloc_page(int64_t table_id);

void buffer_free_page(int64_t table_id, uint64_t page_number);

void buffer_read_page(int64_t table_id, uint64_t page_number, char* dest);

void buffer_write_page(int64_t table_id, uint64_t page_number, const char* src);

void unpin_page(int64_t table_id, uint64_t page_number);

int shutdown_buffer();

void flush_buffer();

void print_buffer();

void buffer_read_control_block(int64_t table_id, uint64_t page_number, char* dest);


#endif // __BUFFER_H__