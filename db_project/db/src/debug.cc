#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#ifdef DEBUG


void print_temp(const char* str) {
    printf(NORMAL_COLOR "[ DEBUG ] %s \n" NORMAL_COLOR, str);
}


#ifdef DEBUG_FILE_H

int file_alloc, file_free, file_read, file_write;

void fileIO_init_stack(){
    file_alloc = 0;
    file_free = 0;
    file_read = 0;
    file_write = 0;
}

void print_function_start(const char* func_name){
    printf(DEBUG_COLOR "[ DEBUG ] %s start.\n" NORMAL_COLOR, func_name);
    if(strcmp(func_name, "file_alloc_page") == 0){
        file_alloc++;
    }
    else if(strcmp(func_name, "file_free_page") == 0){
        file_free++;
    }
    else if(strcmp(func_name, "file_read_page") == 0){
        file_read++;
    }
    else if(strcmp(func_name, "file_write_page") == 0){
        file_write++;
    }
    
}

void print_function_end(const char* func_name){
    printf(DEBUG_COLOR "[ DEBUG ] %s end.\n" NORMAL_COLOR, func_name);
}

void print_error(const char* file_name, const char* func_name, int line_number){
    printf(ERROR_COLOR "[ ERROR ] %s %s %d\n" NORMAL_COLOR, file_name, func_name, line_number);

    #ifdef ERROR_EXIT
    exit(1);
    #endif
}

void fileIO_function_stack(){
    printf(DEBUG_COLOR "[ Disk Space Manager ]\n");
    printf(            "[ Alloc : %10d ]\n", file_alloc);
    printf(            "[ Free  : %10d ]\n", file_free);
    printf(            "[ Read  : %10d ]\n", file_read);
    printf(            "[ Write : %10d ]\n" NORMAL_COLOR, file_write);
}

#endif

#ifdef DEBUG_BUFFER_H


void print_buffer_function_start(const char* func_name){
    printf(DEBUG_COLOR "[ DEBUG ] %s start.\n" NORMAL_COLOR, func_name);
}

void print_buffer_function_end(const char* func_name){
    printf(DEBUG_COLOR "[ DEBUG ] %s end.\n" NORMAL_COLOR, func_name);
}

void print_buffer_error(const char* file_name, const char* func_name, int line_number){
    printf(ERROR_COLOR "[ ERROR ] %s %s %d\n" NORMAL_COLOR, file_name, func_name, line_number);

    #ifdef ERROR_EXIT
    exit(1);
    #endif
}

#endif

#ifdef DEBUG_NEWBPT_H

void print_bpt_function_start(const char* func_name){
    printf(DEBUG_COLOR "[ DEBUG ] %s start.\n" NORMAL_COLOR, func_name);
}

void print_bpt_function_end(const char* func_name){
    printf(DEBUG_COLOR "[ DEBUG ] %s end.\n" NORMAL_COLOR, func_name);
}

void print_bpt_error(const char* file_name, const char* func_name, int line_number){
    printf(ERROR_COLOR "[ ERROR ] %s %s %d\n" NORMAL_COLOR, file_name, func_name, line_number);

    #ifdef ERROR_EXIT
    exit(1);
    #endif
}

#endif


#ifdef DEBUG_LOCK_TABLE_H

void print_lock_table_function_start(const char* func_name){
    printf(DEBUG_COLOR "[ DEBUG ] %s start.\n" NORMAL_COLOR, func_name);
}

void print_lock_table_function_end(const char* func_name){
    printf(DEBUG_COLOR "[ DEBUG ] %s end.\n" NORMAL_COLOR, func_name);
}

void print_lock_table_error(const char* file_name, const char* func_name, int line_number){
    printf(ERROR_COLOR "[ ERROR ] %s %s %d\n" NORMAL_COLOR, file_name, func_name, line_number);

    #ifdef ERROR_EXIT
    exit(1);
    #endif
}

#ifdef DEBUG_TRX_H

void print_trx_function_start(const char* func_name){
    printf(DEBUG_COLOR "[ DEBUG ] %s start.\n" NORMAL_COLOR, func_name);
}

void print_trx_function_end(const char* func_name){
    printf(DEBUG_COLOR "[ DEBUG ] %s end.\n" NORMAL_COLOR, func_name);
}

void print_trx_error(const char* file_name, const char* func_name, int line_number){
    printf(ERROR_COLOR "[ ERROR ] %s %s %d\n" NORMAL_COLOR, file_name, func_name, line_number);

    #ifdef ERROR_EXIT
    exit(1);
    #endif
}

#endif


#endif

#endif