#include "page.h"


namespace HEADER_PAGE{
    pagenum_t get_free_page_number(page_t* page_pointer){
        pagenum_t free_page_number = -1;
        memmove(&free_page_number, page_pointer->data, sizeof(pagenum_t));
        return free_page_number;
    }

    void set_free_page_number(page_t* page_pointer, pagenum_t free_page_number){
        memmove(page_pointer->data, &free_page_number, sizeof(pagenum_t));
    }

    pagenum_t get_number_of_pages(page_t* page_pointer){
        pagenum_t number_of_pages = -1;
        memmove(&number_of_pages, (page_pointer->data + 8), sizeof(pagenum_t));
        return number_of_pages;
    }

    void set_number_of_pages(page_t* page_pointer, pagenum_t number_of_pages){
        memmove(page_pointer->data + 8, &number_of_pages, sizeof(pagenum_t));
    }

    pagenum_t get_root_page_number(page_t* page_pointer){
        pagenum_t root_page_number = -1;
        memmove(&root_page_number, (page_pointer->data + 16), sizeof(pagenum_t));
        return root_page_number;
    }

    void set_root_page_number(page_t* page_pointer, pagenum_t root_page_number){
        memmove(page_pointer->data + 16, &root_page_number, sizeof(pagenum_t));
    }
}

// free page
namespace FREE_PAGE{
    pagenum_t get_next_free_page_number(page_t* page_pointer){
        pagenum_t next_free_page_number = -1;
        memmove(&next_free_page_number, page_pointer->data, sizeof(pagenum_t));
        return next_free_page_number;
    }

    void set_next_free_page_number(page_t* page_pointer, pagenum_t next_free_page_number){
        memmove(page_pointer->data, &next_free_page_number, sizeof(pagenum_t));
    }
}


// leaf page
namespace LEAF_PAGE{
    // page header
    namespace PAGE_HEADER{
        pagenum_t get_parent_page_number(page_t* page_pointer){
            pagenum_t next_free_page_number = -1;
            memmove(&next_free_page_number, page_pointer->data, sizeof(pagenum_t));
            return next_free_page_number;
        }

        void set_parent_page_number(page_t* page_pointer, pagenum_t parent_page_number){
            memmove(page_pointer->data, &parent_page_number, sizeof(pagenum_t));
        }

        int get_is_leaf(page_t* page_pointer){
            int is_leaf = -1;
            memmove(&is_leaf, page_pointer->data + 8, sizeof(int));
            return is_leaf;
        }

        void set_is_leaf(page_t* page_pointer, int is_leaf){
            memmove(page_pointer->data + 8, &is_leaf, sizeof(int));
        }

        int get_number_of_keys(page_t* page_pointer){
            int number_of_keys = -1;
            memmove(&number_of_keys, page_pointer->data + 12, sizeof(int));
            return number_of_keys;
        }

        void set_number_of_keys(page_t* page_pointer, int number_of_keys){
            memmove(page_pointer->data + 12, &number_of_keys, sizeof(int));
        }

        pagenum_t get_amount_of_free_space(page_t* page_pointer){
            pagenum_t amount_of_free_space = -1;
            memmove(&amount_of_free_space, page_pointer->data + 112, sizeof(pagenum_t));
            return amount_of_free_space;
        }

        void set_amount_of_free_space(page_t* page_pointer, pagenum_t amount_of_free_space){
            // std::cout << " // " << amount_of_free_space << std::endl;
            memmove(page_pointer->data + 112, &amount_of_free_space, sizeof(pagenum_t));
        }

        pagenum_t get_right_sibling_page_number(page_t* page_pointer){
            pagenum_t right_sibling_page_number = -1;
            memmove(&right_sibling_page_number, page_pointer->data + 120, sizeof(pagenum_t));
            return right_sibling_page_number;
        }

        void set_right_sibling_page_number(page_t* page_pointer, pagenum_t right_sibling_page_number){
            memmove(page_pointer->data + 120, &right_sibling_page_number, sizeof(pagenum_t));
        }
    }

    slot get_slot(page_t* page_pointer, int idx){
        slot target;
        memmove(&target, page_pointer->data + 128 + idx * sizeof(slot), sizeof(slot));
        return target;
    }

    void set_slot(page_t* page_pointer, slot target, int idx){
        memmove(page_pointer->data + 128 + idx * sizeof(slot), &target, sizeof(slot));
    }

    char* get_value(page_t* page_pointer, slot target){
        char* value = (char*)malloc(target.size);
        memmove(value, page_pointer->data + target.offset, target.size);
        
        return value;
    }

    void set_value(page_t* page_pointer, slot target, char* value){
        // std::cout << (value) << std::endl;
        memmove(page_pointer->data + target.offset, value, target.size);
    }

}


// internal page
namespace INTERNAL_PAGE{
    namespace PAGE_HEADER{
        pagenum_t get_parent_page_number(page_t* page_pointer){
            pagenum_t next_free_page_number = -1;
            memmove(&next_free_page_number, page_pointer->data, sizeof(pagenum_t));
            return next_free_page_number;
        }

        void set_parent_page_number(page_t* page_pointer, pagenum_t parent_page_number){
            memmove(page_pointer->data, &parent_page_number, sizeof(pagenum_t));
        }

        int get_is_leaf(page_t* page_pointer){
            int is_leaf = -1;
            memmove(&is_leaf, page_pointer->data + 8, sizeof(int));
            return is_leaf;
        }

        void set_is_leaf(page_t* page_pointer, int is_leaf){
            memmove(page_pointer->data + 8, &is_leaf, sizeof(int));
        }

        int get_number_of_keys(page_t* page_pointer){
            int number_of_keys = -1;
            memmove(&number_of_keys, page_pointer->data + 12, sizeof(int));
            return number_of_keys;
        }

        void set_number_of_keys(page_t* page_pointer, int number_of_keys){
            memmove(page_pointer->data + 12, &number_of_keys, sizeof(int));
        }

        pagenum_t get_page_A_number(page_t* page_pointer){
            pagenum_t page_A_number = -1;
            memmove(&page_A_number, page_pointer->data + 120, sizeof(pagenum_t));
            return page_A_number;
        }

        void set_page_A_number(page_t* page_pointer, pagenum_t page_A_number){
            memmove(page_pointer->data + 120, &page_A_number, sizeof(pagenum_t));
        }        
    }

    int64_t get_key(page_t* page_pointer, int idx){
        int64_t key;
        memmove(&key, page_pointer->data + 128 + idx * 16, sizeof(int64_t));
        return key; 
    }

    void set_key(page_t* page_pointer, pagenum_t key, int idx){
        memmove(page_pointer->data + 128 + idx * 16, &key, sizeof(int64_t));
    }

    pagenum_t get_left_page_number(page_t* page_pointer, int key_idx){
        pagenum_t left_page_number;
        memmove(&left_page_number, page_pointer->data + 128 + key_idx * 16 - 8, sizeof(pagenum_t));
        return left_page_number;
    }

    void set_left_page_number(page_t* page_pointer, pagenum_t left_page_number, int key_idx){
        memmove(page_pointer->data + 128 + key_idx * 16 - 8, &left_page_number, sizeof(pagenum_t));
    }

    pagenum_t get_right_page_number(page_t* page_pointer, int key_idx){
        pagenum_t right_page_number;
        memmove(&right_page_number, page_pointer->data + 128 + key_idx * 16 + 8, sizeof(pagenum_t));
        return right_page_number;
    }

    void set_right_page_number(page_t* page_pointer, pagenum_t right_page_number, int key_idx){
        memmove(page_pointer->data + 128 + key_idx * 16 + 8, &right_page_number, sizeof(pagenum_t));
    }
}
