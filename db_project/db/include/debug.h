#ifndef __DEBUG_H__
#define __DEBUG_H__


#define DEBUG // debug mode

#ifdef DEBUG

// #define DEBUG_FILE_H // print log in file.h
// #define DEBUG_BUFFER_H // print log in buffer.h
// #define DEBUG_NEWBPT_H // print log in newbpt.h
// #define DEBUG_PAGE_H // print log in page.h
#define DEBUG_LOCK_TABLE_H
#define DEBUG_TRX_H

// #define ERROR_EXIT // exit program with error


#define DEBUG_COLOR "\033[33m"
#define ERROR_COLOR "\033[31m"
#define NORMAL_COLOR "\033[0m"

void print_temp(const char* str);

// #define DEBUG_FILE_H
#ifdef DEBUG_FILE_H

void fileIO_init_stack();
void print_function_start(const char* func_name);
void print_function_end(const char* func_name);
void print_error(const char* file_name, const char* func_name, int line_number);
void fileIO_function_stack();

#endif

// #define DEBUG_BUFFER_H

#ifdef DEBUG_BUFFER_H

void print_buffer_function_start(const char* func_name);
void print_buffer_function_end(const char* func_name);
void print_buffer_error(const char* file_name, const char* func_name, int line_number);

#endif  

#ifdef DEBUG_NEWBPT_H

void print_bpt_function_start(const char* func_name);
void print_bpt_function_end(const char* func_name);
void print_bpt_error(const char* file_name, const char* func_name, int line_number);

#endif

#ifdef DEBUG_PAGE_H



#endif

#ifdef DEBUG_LOCK_TABLE_H

void print_lock_table_function_start(const char* func_name);
void print_lock_table_function_end(const char* func_name);
void print_lock_table_error(const char* file_name, const char* func_name, int line_number);

#endif

#ifdef DEBUG_TRX_H

void print_trx_function_start(const char* func_name);
void print_trx_function_end(const char* func_name);
void print_trx_error(const char* file_name, const char* func_name, int line_number);

#endif


#endif // DEBUG

#endif // __DEBUG_H__