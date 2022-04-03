#ifndef DB_FILE_H_
#define DB_FILE_H_


#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <error.h>
#include <string.h>

// These definitions are not requirements.
// You may build your own way to handle the constants.


#define INITIAL_DB_FILE_SIZE (10 * 1024 * 1024)  // 10 MiB
#define PAGE_SIZE (4 * 1024)                     // 4 KiB
#define P_NUMBER ((INITIAL_DB_FILE_SIZE)/(PAGE_SIZE))

typedef uint64_t pagenum_t;

struct page_t{
    char data[4096];
};

#pragma pack(2)
struct slot{
    int64_t key;
    uint16_t size;
    uint16_t offset;
};
#pragma pack()


void fileIO_init_stack();

// Open existing database file or create one if it doesn't exist
int64_t file_open_table_file(const char* pathname);
// Allocate an on-disk page from the free page list
uint64_t file_alloc_page(int64_t table_id);
// Free an on-disk page to the free page list
void file_free_page(int64_t table_id, uint64_t page_number);
// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int64_t table_id, uint64_t page_number, char* dest);
// Write an in-memory page(src) to the on-disk page
void file_write_page(int64_t table_id, uint64_t page_number, const char* src);
// Close the database file
void file_close_table_files();

void fileIO_function_stack();

#endif  // DB_FILE_H_
