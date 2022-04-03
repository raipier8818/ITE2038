#include "file.h"

#ifndef __PAGE_H__
#define __PAGE_H__


// header page
namespace HEADER_PAGE{
    pagenum_t get_free_page_number(page_t* page_pointer);
    void set_free_page_number(page_t* page_pointer, pagenum_t free_page_number);
    pagenum_t get_number_of_pages(page_t* page_pointer);
    void set_number_of_pages(page_t* page_pointer, pagenum_t number_of_pages);
    pagenum_t get_root_page_number(page_t* page_pointer);
    void set_root_page_number(page_t* page_pointer, pagenum_t root_page_number);
}
// free page
namespace FREE_PAGE{
    pagenum_t get_next_free_page_number(page_t* page_pointer);
    void set_next_free_page_number(page_t* page_pointer, pagenum_t next_free_page_number);
}
// leaf page
namespace LEAF_PAGE{
    // page header
    namespace PAGE_HEADER{
        pagenum_t get_parent_page_number(page_t* page_pointer);
        void set_parent_page_number(page_t* page_pointer, pagenum_t parent_page_number);
        int get_is_leaf(page_t* page_pointer);
        void set_is_leaf(page_t* page_pointer, int is_leaf);
        int get_number_of_keys(page_t* page_pointer);
        void set_number_of_keys(page_t* page_pointer, int number_of_keys);
        pagenum_t get_amount_of_free_space(page_t* page_pointer);
        void set_amount_of_free_space(page_t* page_pointer, pagenum_t amount_of_free_space);
        pagenum_t get_right_sibling_page_number(page_t* page_pointer);
        void set_right_sibling_page_number(page_t* page_pointer, pagenum_t right_sibling_page_number);
    }
    slot get_slot(page_t* page_pointer, int idx);
    void set_slot(page_t* page_pointer, slot target, int idx);
    char* get_value(page_t* page_pointer, slot target);
    void set_value(page_t* page_pointer, slot target, char* value);
}
// internal page
namespace INTERNAL_PAGE{
    namespace PAGE_HEADER{
        pagenum_t get_parent_page_number(page_t* page_pointer);
        void set_parent_page_number(page_t* page_pointer, pagenum_t parent_page_number);
        int get_is_leaf(page_t* page_pointer);
        void set_is_leaf(page_t* page_pointer, int is_leaf);
        int get_number_of_keys(page_t* page_pointer);
        void set_number_of_keys(page_t* page_pointer, int number_of_keys);
        pagenum_t get_page_A_number(page_t* page_pointer);
        void set_page_A_number(page_t* page_pointer, pagenum_t page_A_number);
    }
    int64_t get_key(page_t* page_pointer, int idx);
    void set_key(page_t* page_pointer, pagenum_t key, int idx);
    pagenum_t get_left_page_number(page_t* page_pointer, int key_idx);
    void set_left_page_number(page_t* page_pointer, pagenum_t left_page_number, int key_idx);
    pagenum_t get_right_page_number(page_t* page_pointer, int key_idx);
    void set_right_page_number(page_t* page_pointer, pagenum_t right_page_number, int key_idx);
}



#endif // __PAGE_H__