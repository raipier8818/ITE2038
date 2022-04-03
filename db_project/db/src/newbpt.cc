
#include "newbpt.h"

std::vector<std::string> paths;

int64_t open_table (char *pathname){
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    std::string temp(pathname);
    if(paths.size() >= 20) return -1;
    auto iter = std::find(paths.begin(), paths.end(), temp);

    if(iter != paths.end()){
        #ifdef DEBUG_NEWBPT_H
        print_bpt_function_end(__func__);
        #endif
        return -1;
    }

    int64_t fd = file_open_table_file(pathname);

    if(fd != -1) paths.push_back(temp);

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif

    return fd;
}

#ifdef DEBUG_NEWBPT_H

void print_page(int64_t table_id, pagenum_t page_number){
    if(page_number == 0){
        page_t header_page;
        buffer_read_page(table_id, 0, (char*)&header_page);

        printf("<<<< PAGE %ld >>>>\n", page_number);
        printf("free page number : %ld\n", HEADER_PAGE::get_free_page_number(&header_page));
        printf("number of pages : %d\n", HEADER_PAGE::get_number_of_pages(&header_page));
        printf("root page number : %d\n", HEADER_PAGE::get_root_page_number(&header_page));
        printf("----------------\n");

        if(HEADER_PAGE::get_root_page_number(&header_page) == 0) return;

        unpin_page(table_id, 0);
        print_page(table_id, HEADER_PAGE::get_root_page_number(&header_page));
        
        return;
    }

    page_t page;
    buffer_read_page(table_id, page_number, (char*)&page);

    printf("<<<< PAGE %ld >>>>\n", page_number);

    if(INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&page)){
        printf("parent page number : %ld\n", LEAF_PAGE::PAGE_HEADER::get_parent_page_number(&page));
        printf("is leaf : %d\n", LEAF_PAGE::PAGE_HEADER::get_is_leaf(&page));
        printf("number of keys : %d\n", LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&page));
        printf("amount of free space : %ld\n", LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&page));
        printf("right sibling page number : %ld\n", LEAF_PAGE::PAGE_HEADER::get_right_sibling_page_number(&page));

        for(int i = 0; i < LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&page); i++){
            slot temp = LEAF_PAGE::get_slot(&page, i);
            printf(" {%ld, %hd, %hd} ", temp.key, temp.offset, temp.size);
        }

        printf("\n----------------\n");
        unpin_page(table_id, page_number);
        return;
    }else{
        printf("parent page number : %ld\n", INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&page));
        printf("is leaf : %d\n", INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&page));
        printf("number of keys : %d\n", INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&page));
        
        int i;
        for(i = 0; i < INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&page); i++){
            printf(" %ld ", INTERNAL_PAGE::get_left_page_number(&page, i));
            printf(" [%ld] ", INTERNAL_PAGE::get_key(&page, i));
        }
        printf(" %ld \n", INTERNAL_PAGE::get_left_page_number(&page, i));



        printf("----------------\n");

        unpin_page(table_id, page_number);
        for(int i = 0; i < INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&page) + 1; i++){
            print_page(table_id, INTERNAL_PAGE::get_left_page_number(&page, i));
        }
    }

}

#endif

int db_insert(int64_t table_id, int64_t key, char * value, uint16_t val_size){
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    // printf("%s start.\n", __func__);
    page_t header_page;

    buffer_read_page(table_id, 0, (char*)&header_page);


    // check the key existence
    if(db_find(table_id, key, value , &val_size) == 0){

        // printf("%s end.\n", __func__);
        unpin_page(table_id, 0);

        #ifdef DEBUG_NEWBPT_H
        print_bpt_function_end(__func__);
        #endif

        return -1;
    }


    pagenum_t root_page_number = HEADER_PAGE::get_root_page_number(&header_page);


    page_t leaf_page;
    pagenum_t leaf_page_number;

    if(root_page_number == 0){    // Case 1 : no root in b+ tree
        // make root page as leaf page
        unpin_page(table_id, 0);
        leaf_page_number = buffer_alloc_page(table_id);
        buffer_read_page(table_id, leaf_page_number, (char*)&leaf_page);
        // file_read_page(table_id, 0, (char*)&header_page);
        buffer_read_page(table_id, 0, (char*)&header_page);
        
        HEADER_PAGE::set_root_page_number(&header_page, leaf_page_number);
        slot temp_slot = {key, val_size, (MAX_FREE_SPACE - val_size)};

        LEAF_PAGE::set_slot(&leaf_page, temp_slot, 0);
        LEAF_PAGE::set_value(&leaf_page, temp_slot, value);

        LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&leaf_page, MAX_FREE_SPACE - val_size - sizeof(slot));
        LEAF_PAGE::PAGE_HEADER::set_is_leaf(&leaf_page, 1);
        LEAF_PAGE::PAGE_HEADER::set_number_of_keys(&leaf_page, 1);
        LEAF_PAGE::PAGE_HEADER::set_parent_page_number(&leaf_page, 0);
        LEAF_PAGE::PAGE_HEADER::set_right_sibling_page_number(&leaf_page, 0);


        // printf("lfn : %ld, hpn : %ld\n", leaf_page_number, HEADER_PAGE::get_free_page_number(&header_page));

        // write root page
        // file_write_page(table_id, leaf_page_number, (char*)&leaf_page);
        buffer_write_page(table_id, leaf_page_number, (char*)&leaf_page);
        unpin_page(table_id, leaf_page_number);

        buffer_write_page(table_id, 0, (char*)&header_page);
        unpin_page(table_id, 0);
        // print_page(table_id, 0);

        #ifdef DEBUG_NEWBPT_H
        print_bpt_function_end(__func__);
        #endif

        return 0;
    }else{  // Case 2 : root in b+ tree

        // read leaf page
        unpin_page(table_id, 0);
        leaf_page_number = db_find_leaf_page_number(table_id, root_page_number, key);
        buffer_read_page(table_id, leaf_page_number, (char*)&leaf_page);
    }

    if(LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&leaf_page) >= sizeof(slot) + val_size){
        // printf("%s end.\n", __func__);
        unpin_page(table_id, leaf_page_number);
        db_insert_into_leaf(table_id, leaf_page_number, key, value, val_size);

        #ifdef DEBUG_NEWBPT_H
        print_bpt_function_end(__func__);
        #endif

        return 0;
    }


    // printf("%s end.\n", __func__);
    unpin_page(table_id, leaf_page_number);
    db_insert_into_leaf_page_after_splitting(table_id, root_page_number, leaf_page_number, key, value, val_size);

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif

    return 0;
}

// node * insert_into_leaf
int db_insert_into_leaf(int64_t table_id, pagenum_t leaf_page_number, int64_t key, char* value, uint16_t val_size){
    // printf("%s start.\n", __func__);
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    page_t leaf_page;
    // file_read_page(table_id, leaf_page_number, (char*)&leaf_page);
    buffer_read_page(table_id, leaf_page_number, (char*)&leaf_page);
    
    int64_t number_of_keys = LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&leaf_page);
    

    int i, j, insertion_point = 0;
    // while(insertion_point < number_of_keys && LEAF_PAGE::get_slot(&leaf_page, insertion_point).key < key){
    //     insertion_point++;
    // }

    int l = 0;
    int r = number_of_keys - 1;
    while(l <= r){
        int m = (l + r) / 2;
        if(LEAF_PAGE::get_slot(&leaf_page, m).key < key){
            l = m + 1;
        }else{
            r = m - 1;
        }
    }

    insertion_point = l;

    slot* temp_slot = (slot*)malloc(sizeof(slot)*MAX_SLOT_NUMBER);
    char** temp_value = (char**)malloc(sizeof(char*)*MAX_SLOT_NUMBER);

    for(i = 0, j = 0; i < number_of_keys; i++, j++){
        if(j == insertion_point) j++;
        temp_slot[j] = LEAF_PAGE::get_slot(&leaf_page, i);
        // temp_value[j] = (char*)malloc(sizeof(char)*temp_slot[j].size);
        temp_value[j] = LEAF_PAGE::get_value(&leaf_page, temp_slot[j]);
    }

    temp_slot[insertion_point].key = key;
    temp_slot[insertion_point].size = val_size;

    temp_value[insertion_point] = (char*)malloc(sizeof(char)*val_size);
    memmove(temp_value[insertion_point], value, val_size);

    uint16_t new_offset = PAGE_SIZE;

    for(i = 0; i < number_of_keys + 1; i++){
        temp_slot[i].offset = new_offset - temp_slot[i].size;
        LEAF_PAGE::set_slot(&leaf_page, temp_slot[i], i);
        LEAF_PAGE::set_value(&leaf_page, temp_slot[i], temp_value[i]);

        new_offset -= temp_slot[i].size;
        free(temp_value[i]);
        temp_value[i] = NULL;
    }

    LEAF_PAGE::PAGE_HEADER::set_number_of_keys(&leaf_page, number_of_keys + 1);
    LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&leaf_page, LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&leaf_page) - sizeof(slot) - val_size);


    // file_write_page(table_id, leaf_page_number, (char*)&leaf_page);
    buffer_write_page(table_id, leaf_page_number, (char*)&leaf_page);
    unpin_page(table_id, leaf_page_number);
    
    // printf("%s end.\n", __func__);

    free(temp_slot);
    free(temp_value);

    temp_slot = NULL;
    temp_value = NULL;

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif

    return 0;
}

pagenum_t db_insert_into_leaf_page_after_splitting(int64_t table_id, pagenum_t root_page_number, pagenum_t leaf_page_number, int64_t key, char* value, uint16_t val_size){
    // printf("%s start.\n", __func__);
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    page_t leaf_page, new_leaf_page;
    buffer_read_page(table_id, leaf_page_number, (char*)&leaf_page);
    pagenum_t new_leaf_page_number = buffer_alloc_page(table_id);

    buffer_read_page(table_id, new_leaf_page_number, (char*)&new_leaf_page);
    uint64_t number_of_keys = LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&leaf_page);

    slot* temp_slot = (slot*)malloc((MAX_SLOT_NUMBER + 1) * sizeof(slot));
    char** temp_value = (char**)malloc((MAX_SLOT_NUMBER + 1) * sizeof(char*));

    int insertion_index = 0, i, j;
    
    // while(insertion_index < number_of_keys &&  LEAF_PAGE::get_slot(&leaf_page, insertion_index).key < key){
    //     insertion_index++;
    // }

    int l = 0;
    int r = number_of_keys - 1;

    while(l <= r){
        int m = (l + r) / 2;
        if(LEAF_PAGE::get_slot(&leaf_page, m).key < key){
            l = m + 1;
        }else{
            r = m - 1;
        }
    }

    insertion_index = l;

    for(i = 0, j = 0; i < number_of_keys; i++, j++){
        if(j == insertion_index) j++;
        temp_slot[j] = LEAF_PAGE::get_slot(&leaf_page, i);
        // temp_value[j] = (char*)malloc(temp_slot[j].size);
        temp_value[j] = LEAF_PAGE::get_value(&leaf_page, temp_slot[j]);
    }

    temp_value[insertion_index] = (char*)malloc(val_size);
    memmove(temp_value[insertion_index], value, val_size);
    temp_slot[insertion_index] = {key, val_size, 0};

    uint64_t temp_alloc_space = 0;
    uint64_t temp_number_of_keys = 0;
    uint64_t new_offset = PAGE_SIZE;


    for(i = 0; i < number_of_keys + 1; i++){
        if(temp_alloc_space + sizeof(slot) + temp_slot[i].size >= MAX_FREE_SPACE/2) break;
        new_offset -= temp_slot[i].size;
        temp_slot[i].offset = new_offset;

        LEAF_PAGE::set_slot(&leaf_page, temp_slot[i], i);
        LEAF_PAGE::set_value(&leaf_page, temp_slot[i], temp_value[i]);

        temp_alloc_space += sizeof(slot) + temp_slot[i].size;
        temp_number_of_keys++;

        free(temp_value[i]);
        temp_value[i] = NULL;
    }


    LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&leaf_page, MAX_FREE_SPACE - temp_alloc_space);
    LEAF_PAGE::PAGE_HEADER::set_number_of_keys(&leaf_page, temp_number_of_keys);

    temp_alloc_space = 0;
    temp_number_of_keys = 0;
    new_offset = PAGE_SIZE;

    for(j = 0; i < number_of_keys + 1; i++, j++){
        new_offset -= temp_slot[i].size;
        temp_slot[i].offset = new_offset;

        LEAF_PAGE::set_slot(&new_leaf_page, temp_slot[i], j);
        LEAF_PAGE::set_value(&new_leaf_page, temp_slot[i], temp_value[i]);
        

        temp_alloc_space += sizeof(slot) + temp_slot[i].size;
        temp_number_of_keys++;
        free(temp_value[i]);
        temp_value[i] = NULL;
    }

    LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&new_leaf_page, MAX_FREE_SPACE - temp_alloc_space);
    LEAF_PAGE::PAGE_HEADER::set_is_leaf(&new_leaf_page, 1);
    LEAF_PAGE::PAGE_HEADER::set_number_of_keys(&new_leaf_page, temp_number_of_keys);
    LEAF_PAGE::PAGE_HEADER::set_parent_page_number(&new_leaf_page, LEAF_PAGE::PAGE_HEADER::get_parent_page_number(&leaf_page));
    LEAF_PAGE::PAGE_HEADER::set_right_sibling_page_number(&new_leaf_page, LEAF_PAGE::PAGE_HEADER::get_right_sibling_page_number(&leaf_page));
    LEAF_PAGE::PAGE_HEADER::set_right_sibling_page_number(&leaf_page, new_leaf_page_number);

    buffer_write_page(table_id, leaf_page_number, (char*)&leaf_page);
    buffer_write_page(table_id, new_leaf_page_number, (char*)&new_leaf_page);

    unpin_page(table_id, leaf_page_number);
    unpin_page(table_id, new_leaf_page_number);

    free(temp_slot);
    free(temp_value);

    temp_slot = NULL;
    temp_value = NULL;

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif
    // printf("%s end.\n", __func__);
    return db_insert_into_parent(table_id, root_page_number, leaf_page_number, LEAF_PAGE::get_slot(&new_leaf_page, 0).key, new_leaf_page_number);
}



pagenum_t db_insert_into_parent(int64_t table_id, pagenum_t root_page_number, pagenum_t left_page_number, int64_t key, pagenum_t right_page_number){
    // printf("%s start.\n", __func__);
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif
    
    page_t left_page;
    buffer_read_page(table_id, left_page_number, (char*)&left_page);

    int64_t parent_page_number = INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&left_page);

    if(parent_page_number == 0){
        // printf("%s end.\n", __func__);
        unpin_page(table_id, left_page_number);
        
        #ifdef DEBUG_NEWBPT_H
        print_bpt_function_end(__func__);
        #endif

        return db_insert_into_new_root(table_id, left_page_number, key, right_page_number);
    }

    int left_index = get_left_index(table_id, parent_page_number, left_page_number);
    page_t parent_page;
    buffer_read_page(table_id, parent_page_number, (char*)&parent_page);

    if(INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&parent_page) < MAX_KEY_NUMBER){
        // printf("%s end.\n", __func__);
        unpin_page(table_id, left_page_number);
        unpin_page(table_id, parent_page_number);

        #ifdef DEBUG_NEWBPT_H
        print_bpt_function_end(__func__);
        #endif

        return db_insert_into_internal(table_id, root_page_number, parent_page_number, left_index, key, right_page_number);
    }
    
    unpin_page(table_id, left_page_number);
    unpin_page(table_id, parent_page_number);
    // printf("%s end.\n", __func__);

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif

    return insert_into_node_after_splitting(table_id, root_page_number, parent_page_number, left_index, key, right_page_number);
}

pagenum_t insert_into_node_after_splitting(int64_t table_id, pagenum_t root_page_number, pagenum_t parent_page_number, int left_index, int64_t key, pagenum_t right_page_number){
    
    // printf("%s start.\n", __func__);
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    int64_t* temp_keys = (int64_t*)malloc((MAX_KEY_NUMBER + 1) * sizeof(int64_t));
    pagenum_t* temp_page = (pagenum_t*)malloc((MAX_KEY_NUMBER + 2) * sizeof(int64_t));

    page_t parent_page;
    buffer_read_page(table_id, parent_page_number, (char*)&parent_page);
    int number_of_keys = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&parent_page);

    int i, j;
    for(i = 0, j = 0; i < number_of_keys + 1; i++, j++){
        if(j == left_index + 1) j++;
        temp_page[j] = INTERNAL_PAGE::get_left_page_number(&parent_page, i);
    }

    for(i = 0, j = 0; i < number_of_keys; i++, j++){
        if(j == left_index) j++;
        temp_keys[j] = INTERNAL_PAGE::get_key(&parent_page, i);
    }

    temp_page[left_index + 1] = right_page_number;
    temp_keys[left_index] = key;

    int split = MAX_KEY_NUMBER / 2; // TODO
    page_t new_parent_page;
    pagenum_t new_parent_page_number = buffer_alloc_page(table_id);
    buffer_read_page(table_id, new_parent_page_number, (char*)&new_parent_page);

    number_of_keys = 0;
    for(i = 0; i < split - 1; i++){
        INTERNAL_PAGE::set_left_page_number(&parent_page, temp_page[i], i);
        INTERNAL_PAGE::set_key(&parent_page, temp_keys[i], i);
        number_of_keys++;
    }

    INTERNAL_PAGE::set_left_page_number(&parent_page, temp_page[i], i);
    INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&parent_page, number_of_keys);

    int64_t k_prime = temp_keys[split - 1];
    number_of_keys = 0;
    for(++i, j = 0; i <= MAX_KEY_NUMBER; i++, j++){
        INTERNAL_PAGE::set_left_page_number(&new_parent_page, temp_page[i], j);
        INTERNAL_PAGE::set_key(&new_parent_page, temp_keys[i], j);
        number_of_keys++;
    }
    INTERNAL_PAGE::set_left_page_number(&new_parent_page, temp_page[i], j);

    INTERNAL_PAGE::PAGE_HEADER::set_is_leaf(&new_parent_page, 0);
    INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&new_parent_page, number_of_keys);
    INTERNAL_PAGE::PAGE_HEADER::set_parent_page_number(&new_parent_page, INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&parent_page));

    for(i = 0; i <= number_of_keys; i++){
        page_t child_page;
        buffer_read_page(table_id, INTERNAL_PAGE::get_left_page_number(&new_parent_page, i), (char*)&child_page);
        INTERNAL_PAGE::PAGE_HEADER::set_parent_page_number(&child_page, new_parent_page_number);
        buffer_write_page(table_id, INTERNAL_PAGE::get_left_page_number(&new_parent_page, i), (char*)&child_page);
        unpin_page(table_id, INTERNAL_PAGE::get_left_page_number(&new_parent_page, i));
    }

    buffer_write_page(table_id, parent_page_number, (char*)&parent_page);
    buffer_write_page(table_id, new_parent_page_number, (char*)&new_parent_page);
    unpin_page(table_id, parent_page_number);
    unpin_page(table_id, new_parent_page_number);

    free(temp_keys);
    free(temp_page);
    
    temp_keys = NULL;
    temp_page = NULL;

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif

    // printf("%s end.\n", __func__);
    return db_insert_into_parent(table_id, root_page_number, parent_page_number, k_prime, new_parent_page_number);
}



pagenum_t db_insert_into_internal(int64_t table_id, pagenum_t root_page_number, pagenum_t parent_page_number, int left_index, int64_t key, pagenum_t right_page_number){
    
    // printf("%s start.\n", __func__);
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    page_t parent_page;
    buffer_read_page(table_id, parent_page_number, (char*)&parent_page);

    uint64_t number_of_keys = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&parent_page);
    int i = number_of_keys;
    for(; i > left_index; i--){
        INTERNAL_PAGE::set_left_page_number(&parent_page, INTERNAL_PAGE::get_left_page_number(&parent_page, i), i + 1);
        INTERNAL_PAGE::set_key(&parent_page, INTERNAL_PAGE::get_key(&parent_page, i - 1), i);
    }

    INTERNAL_PAGE::set_left_page_number(&parent_page, right_page_number, left_index + 1);
    INTERNAL_PAGE::set_key(&parent_page, key, left_index);
    INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&parent_page, number_of_keys + 1);

    buffer_write_page(table_id, parent_page_number, (char*)&parent_page);    
    unpin_page(table_id, parent_page_number);
    // printf("%s end.\n", __func__);
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif

    return root_page_number;
}


int get_left_index(int64_t table_id, pagenum_t parent_page_number, pagenum_t left_page_number){
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    // printf("%s start.\n", __func__);
    page_t parent_page;
    // file_read_page(table_id, parent_page_number, (char*)&parent_page);
    buffer_read_page(table_id, parent_page_number, (char*)&parent_page);

    int left_index = 0;
    while(left_index <= INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&parent_page) && INTERNAL_PAGE::get_left_page_number(&parent_page, left_index) != left_page_number){
        left_index++;
    }

    // int l = 0;
    // int r = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&parent_page);
    // while(l <= r){
    //     int m = (l + r) / 2;
    //     if(INTERNAL_PAGE::get_left_page_number(&parent_page, m) == left_page_number){
    //         left_index = m;
    //         break;
    //     }
    //     else if(INTERNAL_PAGE::get_left_page_number(&parent_page, m) < left_page_number){
    //         l = m + 1;
    //     }
    //     else{
    //         r = m - 1;
    //     }
    // }
    // left_index = l;

    // printf("%s end.\n", __func__);
    unpin_page(table_id, parent_page_number);

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif
    return left_index;
}


pagenum_t db_insert_into_new_root(int64_t table_id, pagenum_t left, int64_t key, pagenum_t right){
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif
    // printf("%s start.\n", __func__);
    page_t root_page, header_page, left_page, right_page;
    pagenum_t root_page_number = buffer_alloc_page(table_id);
    buffer_read_page(table_id, root_page_number, (char*)&root_page);
    buffer_read_page(table_id, 0, (char*)&header_page);
    buffer_read_page(table_id, left, (char*)&left_page);
    buffer_read_page(table_id, right, (char*)&right_page);

    INTERNAL_PAGE::PAGE_HEADER::set_is_leaf(&root_page, 0);
    INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&root_page, 1);
    INTERNAL_PAGE::PAGE_HEADER::set_parent_page_number(&root_page, 0);

    LEAF_PAGE::PAGE_HEADER::set_parent_page_number(&left_page, root_page_number);
    LEAF_PAGE::PAGE_HEADER::set_parent_page_number(&right_page, root_page_number);

    HEADER_PAGE::set_root_page_number(&header_page, root_page_number);

    INTERNAL_PAGE::set_key(&root_page, key, 0);
    INTERNAL_PAGE::set_left_page_number(&root_page, left, 0);
    INTERNAL_PAGE::set_right_page_number(&root_page, right, 0);

    // file_write_page(table_id, left, (char*)&left_page);
    // file_write_page(table_id, right, (char*)&right_page);
    // file_write_page(table_id, root_page_number, (char*)&root_page);


    buffer_write_page(table_id, left, (char*)&left_page);
    buffer_write_page(table_id, right, (char*)&right_page);
    buffer_write_page(table_id, root_page_number, (char*)&root_page);
    buffer_write_page(table_id, 0, (char*)&header_page);

    unpin_page(table_id, left);
    unpin_page(table_id, right);
    unpin_page(table_id, root_page_number);
    unpin_page(table_id, 0);

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif
    // printf("%s end.\n", __func__);
    return root_page_number;
}

// node* find_leaf
pagenum_t db_find_leaf_page_number(int64_t table_id, pagenum_t root_page_number, int key){

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    // printf("%s start.\n", __func__);

    if(root_page_number == 0){
        // printf("%s end.\n", __func__);
        #ifdef DEBUG_NEWBPT_H
        print_bpt_function_end(__func__);
        #endif

        return 0;
    }

    page_t temp_page;
    // file_read_page(table_id, root_page_number, (char*)&temp_page);
    buffer_read_page(table_id, root_page_number, (char*)&temp_page);   
    pagenum_t next_page_number = root_page_number;

    while(!INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&temp_page)){
        int i = 0;
        int number_of_keys = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&temp_page);
        
        // while(i < number_of_keys){
        //     if(key >= INTERNAL_PAGE::get_key(&temp_page, i)) i++;
        //     else break;
        // }

        int l = 0;
        int r = number_of_keys;
        while(l < r){
            int m = (l + r) / 2;
            if(key >= INTERNAL_PAGE::get_key(&temp_page, m)){
                l = m + 1;
            }
            else{
                r = m;
            }
        }
        i = l;        

        unpin_page(table_id, next_page_number);
        next_page_number = INTERNAL_PAGE::get_left_page_number(&temp_page, i);
        buffer_read_page(table_id, next_page_number, (char*)&temp_page);
    }
    // printf("%s end.\n", __func__);
    unpin_page(table_id, next_page_number);

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif

    return next_page_number;
}

int db_find (int64_t table_id, int64_t key, char * ret_val, uint16_t * val_size){
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    // printf("%s start.\n", __func__);
    page_t header_page;
    buffer_read_page(table_id, 0, (char*)&header_page);

    pagenum_t root_page_number = HEADER_PAGE::get_root_page_number(&header_page);



    if(root_page_number == 0){
        // printf("%s end.\n", __func__);
        unpin_page(table_id, 0);

        #ifdef DEBUG_NEWBPT_H
        print_bpt_function_end(__func__);
        #endif
        return -1;
    }

    pagenum_t leaf_page_number = db_find_leaf_page_number(table_id, root_page_number, key);
    page_t leaf_page;
    // file_read_page(table_id, leaf_page_number, (char*)&leaf_page);   
    buffer_read_page(table_id, leaf_page_number, (char*)&leaf_page);

    uint64_t number_of_keys = LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&leaf_page);
    
    for(int i = 0; i < number_of_keys; i++){
        slot temp_slot = LEAF_PAGE::get_slot(&leaf_page, i);

        if(temp_slot.key == key){
            *val_size = temp_slot.size;
            char* target = LEAF_PAGE::get_value(&leaf_page, temp_slot);
            memmove(ret_val, target, temp_slot.size);
            free(target);

            // printf("%s end.\n", __func__);
            unpin_page(table_id, leaf_page_number);
            unpin_page(table_id, 0);
            return 0;
        }
    }
    
    unpin_page(table_id, leaf_page_number);
    unpin_page(table_id, 0);
    // printf("%s end.\n", __func__);
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif
    return -1;
}



int db_delete (int64_t table_id, int64_t key){
    // printf("< Delete %d >\n", key);

    // printf("%s start.\n", __func__);

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    page_t header_page;
    buffer_read_page(table_id, 0, (char*)&header_page);

    pagenum_t root_page_number = HEADER_PAGE::get_root_page_number(&header_page);
    unpin_page(table_id, 0);

    char* temp_value = (char*)malloc(MAX_VALUE_SIZE);
    uint16_t temp_value_size;
    int find_key = db_find(table_id, key, temp_value, &temp_value_size);
    pagenum_t leaf_page_number = db_find_leaf_page_number(table_id, root_page_number, key);

    if(find_key == 0 && leaf_page_number != 0){
        root_page_number = db_delete_entry(table_id, root_page_number, leaf_page_number, key, 0);
        free(temp_value);
        temp_value = NULL;

        buffer_read_page(table_id, 0, (char*)&header_page);
        HEADER_PAGE::set_root_page_number(&header_page, root_page_number);
        buffer_write_page(table_id, 0, (char*)&header_page);
        unpin_page(table_id, 0);

        #ifdef DEBUG_NEWBPT_H
        print_bpt_function_end(__func__);
        #endif
        // printf("%s end.\n", __func__);
        return 0;
    }
    free(temp_value);
    // printf("%s end.\n", __func__);

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif
    return -1;
}

pagenum_t db_delete_entry(int64_t table_id, pagenum_t root_page_number, pagenum_t n_page_number, int64_t key, pagenum_t pointer){
    // printf("%s start.\n", __func__);
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    n_page_number = db_remove_entry_from_node(table_id, n_page_number, key, pointer);



    if(n_page_number == root_page_number){
        // printf("%s end.\n", __func__);
        #ifdef DEBUG_NEWBPT_H
        print_bpt_function_end(__func__);
        #endif

        return adjust_root(table_id, root_page_number);
    }

    page_t n_page, n_parent_page;
    buffer_read_page(table_id, n_page_number, (char*)&n_page);


    if(INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&n_page)){ // leaf page
        int threshold = 2500;
        
        if(threshold > LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&n_page)){
            // printf("%s end.\n", __func__);
            
            unpin_page(table_id, n_page_number);

            #ifdef DEBUG_NEWBPT_H
            print_bpt_function_end(__func__);
            #endif

            return root_page_number;
        }

        int neighbor_index = get_neighbor_index(table_id, n_page_number);
        int k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
        

        // file_read_page(table_id, LEAF_PAGE::PAGE_HEADER::get_parent_page_number(&n_page), (char*)&n_parent_page);
        buffer_read_page(table_id, LEAF_PAGE::PAGE_HEADER::get_parent_page_number(&n_page), (char*)&n_parent_page);
        int64_t k_prime = INTERNAL_PAGE::get_key(&n_parent_page, k_prime_index);        
        
        page_t neighbor_page;
        pagenum_t neighbor_page_number = INTERNAL_PAGE::get_left_page_number(&n_parent_page, neighbor_index == -1 ? 1 : neighbor_index);
        // file_read_page(table_id, neighbor_page_number, (char*)&neighbor_page);
        buffer_read_page(table_id, neighbor_page_number, (char*)&neighbor_page);

        if(LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&neighbor_page) >= PAGE_SIZE - 128 - LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&n_page)){
            /* Coalescence */
            // printf("%s end.\n", __func__);
            // printf(">> %ld >= %d\n", LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&neighbor_page), PAGE_SIZE - 128 - LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&n_page));
            unpin_page(table_id, n_page_number);
            unpin_page(table_id, LEAF_PAGE::PAGE_HEADER::get_parent_page_number(&n_page));
            unpin_page(table_id, neighbor_page_number);

            // print_page(table_id, n_page_number);
            // print_page(table_id, neighbor_page_number);
            #ifdef DEBUG_NEWBPT_H
            print_bpt_function_end(__func__);
            #endif

            return coalesce_nodes(table_id, root_page_number, n_page_number, neighbor_page_number, neighbor_index, k_prime);
        
        }else{
            /* Redistribution */
            // printf("%s end.\n", __func__);
            unpin_page(table_id, n_page_number);
            unpin_page(table_id, LEAF_PAGE::PAGE_HEADER::get_parent_page_number(&n_page));
            unpin_page(table_id, neighbor_page_number);


            // print_page(table_id, n_page_number);
            // print_page(table_id, neighbor_page_number);
            #ifdef DEBUG_NEWBPT_H
            print_bpt_function_end(__func__);
            #endif

            return redistribute_nodes(table_id, root_page_number, n_page_number, neighbor_page_number, neighbor_index, k_prime_index, k_prime);

        }
    }else{ // internal page
        int min_keys = MAX_KEY_NUMBER / 2;

        if(min_keys <= INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_page)){
            // printf("%s end.\n", __func__);
            unpin_page(table_id, n_page_number);

            #ifdef DEBUG_NEWBPT_H
            print_bpt_function_end(__func__);
            #endif

            return root_page_number;
        }


        int neighbor_index = get_neighbor_index(table_id, n_page_number);
        int k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;

        buffer_read_page(table_id, INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&n_page), (char*)&n_parent_page);
        int64_t k_prime = INTERNAL_PAGE::get_key(&n_parent_page, k_prime_index);

        page_t neighbor_page;
        pagenum_t neighbor_page_number = INTERNAL_PAGE::get_left_page_number(&n_parent_page, neighbor_index == -1 ? 1 : neighbor_index);
        buffer_read_page(table_id, neighbor_page_number, (char*)&neighbor_page);

        if(INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&neighbor_page) + INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_page) < MAX_KEY_NUMBER){
            /* Coalescence */
            // printf("%s end.\n", __func__);
            unpin_page(table_id, n_page_number);
            unpin_page(table_id, INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&n_page));
            unpin_page(table_id, neighbor_page_number);
            
            #ifdef DEBUG_NEWBPT_H
            print_bpt_function_end(__func__);
            #endif

            return coalesce_nodes(table_id, root_page_number, n_page_number, neighbor_page_number, neighbor_index, k_prime);
        }else{
            /* Redistribution */
            // printf("%s end.\n", __func__);
            unpin_page(table_id, n_page_number);
            unpin_page(table_id, INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&n_page));
            unpin_page(table_id, neighbor_page_number);

            #ifdef DEBUG_NEWBPT_H
            print_bpt_function_end(__func__);
            #endif

            return redistribute_nodes(table_id, root_page_number, n_page_number, neighbor_page_number, neighbor_index, k_prime_index, k_prime);
        }
    }
}

pagenum_t coalesce_nodes(int64_t table_id, pagenum_t root_page_number, pagenum_t n_page_number, pagenum_t neighbor_page_number, int neighbor_index, int64_t k_prime){
    // printf("%s start.\n", __func__);
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif


    int i, j, neighbor_insertion_index, n_end;
    page_t neighbor_page, n_page, temp_page;
    pagenum_t temp_page_number;

    if(neighbor_index == -1){
        temp_page_number = n_page_number;
        n_page_number = neighbor_page_number;
        neighbor_page_number = temp_page_number;
    }

    // file_read_page(table_id, n_page_number, (char*)&n_page);
    // file_read_page(table_id, neighbor_page_number, (char*)&neighbor_page);
    buffer_read_page(table_id, n_page_number, (char*)&n_page);
    buffer_read_page(table_id, neighbor_page_number, (char*)&neighbor_page);

    neighbor_insertion_index = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&neighbor_page);

    if(INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&n_page)){ // leaf page
        uint64_t n_number_of_key = LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&n_page);
        uint64_t neighbor_number_of_key = LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&neighbor_page);
        uint16_t new_offset = PAGE_SIZE;
        uint64_t new_free_space = PAGE_SIZE - 128;

        slot* temp_slots = (slot*)malloc(sizeof(slot) * (n_number_of_key + neighbor_number_of_key + 1));
        char** temp_value = (char**)malloc(sizeof(char*) * (n_number_of_key + neighbor_number_of_key + 1));

        for(i = 0; i < neighbor_number_of_key; i++){
            // printf("// i : %d\n", i);
            temp_slots[i] = LEAF_PAGE::get_slot(&neighbor_page, i);
            // temp_value[i] = (char*)malloc(sizeof(char) * temp_slots[i].size);
            temp_value[i] = LEAF_PAGE::get_value(&neighbor_page, temp_slots[i]);

            temp_slots[i].offset = new_offset - temp_slots[i].size;
            new_offset -= temp_slots[i].size;
        }

        // print_page(table_id, neighbor_page_number);
        // print_page(table_id, n_page_number);

        for(j = 0; j < n_number_of_key; j++, i++){
            // printf("// %d : ", j);
            temp_slots[i] = LEAF_PAGE::get_slot(&n_page, j);
            // temp_value[i] = (char*)malloc(sizeof(char) * temp_slots[i].size);
            temp_value[i] = LEAF_PAGE::get_value(&n_page, temp_slots[i]);

            // printf("{%ld %d %d} ", temp_slots[i].key, temp_slots[i].offset, temp_slots[i].size);
            temp_slots[i].offset = new_offset - temp_slots[i].size;
            new_offset -= temp_slots[i].size;
        }
        // printf("\n");

        for(i = 0; i < neighbor_number_of_key + n_number_of_key; i++){
            LEAF_PAGE::set_slot(&neighbor_page, temp_slots[i], i);
            LEAF_PAGE::set_value(&neighbor_page, temp_slots[i], temp_value[i]);

            new_free_space -= (sizeof(slot) + temp_slots[i].size);
            free(temp_value[i]);
            temp_value[i] = NULL;
        }

        LEAF_PAGE::PAGE_HEADER::set_right_sibling_page_number(&neighbor_page, LEAF_PAGE::PAGE_HEADER::get_right_sibling_page_number(&n_page));
        LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&neighbor_page, new_free_space);
        LEAF_PAGE::PAGE_HEADER::set_number_of_keys(&neighbor_page, n_number_of_key + neighbor_number_of_key);
        // file_write_page(table_id, neighbor_page_number, (char*)&neighbor_page);
        buffer_write_page(table_id, neighbor_page_number, (char*)&neighbor_page);
        unpin_page(table_id, neighbor_page_number);
        
        free(temp_slots);
        free(temp_value);

        temp_slots = NULL;
        temp_value = NULL;

    }else{ // internal page
        INTERNAL_PAGE::set_key(&neighbor_page, k_prime, neighbor_insertion_index);
        INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&neighbor_page, INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&neighbor_page) + 1);

        n_end = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_page);

        uint64_t neighbor_number_of_keys = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&neighbor_page);
        uint64_t n_number_of_keys = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_page);

        for(i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++){
            INTERNAL_PAGE::set_key(&neighbor_page, INTERNAL_PAGE::get_key(&n_page, j), i);
            INTERNAL_PAGE::set_left_page_number(&neighbor_page, INTERNAL_PAGE::get_left_page_number(&n_page, j), i);
            neighbor_number_of_keys++;
            n_number_of_keys--;
        }
        INTERNAL_PAGE::set_left_page_number(&neighbor_page, INTERNAL_PAGE::get_left_page_number(&n_page, j), i);
        INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&neighbor_page, neighbor_number_of_keys);

        for(i = 0; i < INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&neighbor_page) + 1; i++){
            // file_read_page(table_id, INTERNAL_PAGE::get_left_page_number(&neighbor_page, i), (char*)&temp_page);
            buffer_read_page(table_id, INTERNAL_PAGE::get_left_page_number(&neighbor_page, i), (char*)&temp_page);
            INTERNAL_PAGE::PAGE_HEADER::set_parent_page_number(&temp_page, neighbor_page_number);
            // file_write_page(table_id, INTERNAL_PAGE::get_left_page_number(&neighbor_page, i), (char*)&temp_page);
            buffer_write_page(table_id, INTERNAL_PAGE::get_left_page_number(&neighbor_page, i), (char*)&temp_page);
            unpin_page(table_id, INTERNAL_PAGE::get_left_page_number(&neighbor_page, i));
        }

        buffer_write_page(table_id, neighbor_page_number, (char*)&neighbor_page);
        unpin_page(table_id, neighbor_page_number);
    }


    // print_page(table_id, n_page_number);
    // print_page(table_id, neighbor_page_number);
    pagenum_t n_parent_page_number = INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&n_page);

    unpin_page(table_id, n_page_number);
    buffer_free_page(table_id, n_page_number);

    root_page_number = db_delete_entry(table_id, root_page_number, n_parent_page_number, k_prime, n_page_number);
    
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif

    return root_page_number;
}

pagenum_t redistribute_nodes(int64_t table_id, pagenum_t root_page_number, pagenum_t n_page_number, pagenum_t neighbor_page_number, int neighbor_index, int k_prime_index, int64_t k_prime){
    // printf("%s start.\n", __func__);
    
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif
    

    int i;
    page_t temp_page, n_page, neighbor_page, n_parent_page;
    pagenum_t parent_page_number;
    // file_read_page(table_id, n_page_number, (char*)&n_page);
    // file_read_page(table_id, neighbor_page_number, (char*)&neighbor_page);
    buffer_read_page(table_id, n_page_number, (char*)&n_page);
    buffer_read_page(table_id, neighbor_page_number, (char*)&neighbor_page);

    int threshold = 2500;

    uint64_t n_number_of_keys = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_page);
    uint64_t neighbor_number_of_keys = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&neighbor_page);

    if(neighbor_index != -1){
        if(!INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&n_page)){ // internal page
            INTERNAL_PAGE::set_left_page_number(&n_page, INTERNAL_PAGE::get_left_page_number(&n_page, n_number_of_keys), n_number_of_keys + 1); 
        
            for(i = n_number_of_keys; i > 0; i--){
                INTERNAL_PAGE::set_key(&n_page, INTERNAL_PAGE::get_key(&n_page, i - 1), i);
                INTERNAL_PAGE::set_left_page_number(&n_page, INTERNAL_PAGE::get_left_page_number(&n_page, i - 1), i);
            }

            INTERNAL_PAGE::set_left_page_number(&n_page, INTERNAL_PAGE::get_left_page_number(&neighbor_page, neighbor_number_of_keys), 0);
            buffer_read_page(table_id, INTERNAL_PAGE::get_left_page_number(&n_page, 0), (char*)&temp_page);
            INTERNAL_PAGE::PAGE_HEADER::set_parent_page_number(&temp_page, n_page_number);
            buffer_write_page(table_id, INTERNAL_PAGE::get_left_page_number(&n_page, 0), (char*)&temp_page);
            unpin_page(table_id, INTERNAL_PAGE::get_left_page_number(&n_page, 0));
            
            INTERNAL_PAGE::set_key(&n_page, k_prime, 0);
            
            parent_page_number = INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&n_page);

            // file_read_page(table_id, parent_page_number, (char*)&n_parent_page);
            buffer_read_page(table_id, parent_page_number, (char*)&n_parent_page);
            INTERNAL_PAGE::set_key(&n_parent_page, INTERNAL_PAGE::get_key(&neighbor_page, neighbor_number_of_keys - 1), k_prime_index);

            INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&n_page, n_number_of_keys + 1);
            INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&neighbor_page, neighbor_number_of_keys - 1);

        }else{ // leaf page
            while(LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&n_page) >= 2500){
                slot* temp_slots = (slot*)malloc(sizeof(slot) * MAX_SLOT_NUMBER);
                char** temp_values = (char**)malloc(sizeof(slot) * MAX_SLOT_NUMBER);
                uint64_t new_offset = PAGE_SIZE;
                temp_slots[0] = LEAF_PAGE::get_slot(&neighbor_page, 0);
                // temp_values[0] = (char*)malloc(sizeof(char) * temp_slots[0].size);
                temp_values[0] = LEAF_PAGE::get_value(&neighbor_page, temp_slots[0]);

                temp_slots[0].offset = new_offset - temp_slots[0].size;
                new_offset -= temp_slots[0].size;

                int j;
                for(i = 1, j = 0; j < n_number_of_keys; j++, i++){
                    temp_slots[i] = LEAF_PAGE::get_slot(&n_page, j);
                    // temp_values[i] = (char*)malloc(sizeof(char) * temp_slots[i].size);
                    temp_values[i] = LEAF_PAGE::get_value(&n_page, temp_slots[i]);

                    temp_slots[i].offset = new_offset - temp_slots[i].size;
                    new_offset -= temp_slots[i].size;
                }

                for(i = 0; i < n_number_of_keys + 1; i++){
                    LEAF_PAGE::set_slot(&n_page, temp_slots[i], i);
                    LEAF_PAGE::set_value(&n_page, temp_slots[i], temp_values[i]);

                    free(temp_values[i]);
                    temp_values[i] = NULL;
                }

                LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&n_page, LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&n_page) - sizeof(slot) - temp_slots[0].size);
                LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&neighbor_page, LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&neighbor_page) + sizeof(slot) + temp_slots[0].size);

                parent_page_number = LEAF_PAGE::PAGE_HEADER::get_parent_page_number(&n_page);
                // file_read_page(table_id, parent_page_number, (char*)&n_parent_page);
                buffer_read_page(table_id, parent_page_number, (char*)&n_parent_page);
                INTERNAL_PAGE::set_key(&n_parent_page, LEAF_PAGE::get_slot(&n_page, 0).key, k_prime_index);


                INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&n_page, n_number_of_keys + 1);
                INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&neighbor_page, neighbor_number_of_keys - 1);
            
                free(temp_slots);
                free(temp_values);

                temp_slots = NULL;
                temp_values = NULL;
            }
        }
    }else{
        if(INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&n_page)){ // leaf page
            while(LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&n_page) >= 2500){
                slot* temp_slots = (slot*)malloc(sizeof(slot) * MAX_SLOT_NUMBER);
                char** temp_values = (char**)malloc(sizeof(char*) * MAX_SLOT_NUMBER);
                uint16_t new_offset = PAGE_SIZE;

                for(i = 0; i < n_number_of_keys; i++){
                    temp_slots[i] = LEAF_PAGE::get_slot(&n_page, i);
                    // temp_values[i] = (char*)malloc(sizeof(char) * temp_slots[i].size);
                    temp_values[i] = LEAF_PAGE::get_value(&n_page, temp_slots[i]);

                    temp_slots[i].offset = new_offset - temp_slots[i].size;
                    new_offset -= temp_slots[i].size;
                }

                temp_slots[i] = LEAF_PAGE::get_slot(&neighbor_page, 0);
                // temp_values[i] = (char*)malloc(sizeof(char) * temp_slots[i].size);
                temp_values[i] = LEAF_PAGE::get_value(&neighbor_page, temp_slots[i]);

                temp_slots[i].offset = new_offset - temp_slots[i].size;
                new_offset -= temp_slots[i].size;

                int j;
                for(j = 0; j < n_number_of_keys + 1; j++){
                    LEAF_PAGE::set_slot(&n_page, temp_slots[j], j);
                    LEAF_PAGE::set_value(&n_page, temp_slots[j], temp_values[j]);
                    
                    free(temp_values[j]);
                    temp_values[j] = NULL;
                }

                for(j = 0; j < neighbor_number_of_keys - 1; j++){
                    LEAF_PAGE::set_slot(&neighbor_page, LEAF_PAGE::get_slot(&neighbor_page, j + 1), j);
                }

                LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&n_page, LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&n_page) - sizeof(slot) - temp_slots[i].size);
                LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&neighbor_page, LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&neighbor_page) + sizeof(slot) + temp_slots[i].size);

                parent_page_number = LEAF_PAGE::PAGE_HEADER::get_parent_page_number(&n_page);
                // file_read_page(table_id, parent_page_number, (char*)&n_parent_page);
                buffer_read_page(table_id, parent_page_number, (char*)&n_parent_page);
                INTERNAL_PAGE::set_key(&n_parent_page, LEAF_PAGE::get_slot(&neighbor_page, 0).key, k_prime_index);

                INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&n_page, n_number_of_keys + 1);
                INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&neighbor_page, neighbor_number_of_keys - 1);

                free(temp_slots);
                free(temp_values);

                temp_slots = NULL;
                temp_values = NULL;
            }
        }else{ // internal page
            INTERNAL_PAGE::set_key(&n_page, k_prime, n_number_of_keys);
            INTERNAL_PAGE::set_left_page_number(&n_page, INTERNAL_PAGE::get_left_page_number(&neighbor_page, 0), n_number_of_keys + 1);

            buffer_read_page(table_id, INTERNAL_PAGE::get_left_page_number(&n_page, n_number_of_keys + 1), (char*)&temp_page);
            INTERNAL_PAGE::PAGE_HEADER::set_parent_page_number(&temp_page, n_page_number);
            buffer_write_page(table_id, INTERNAL_PAGE::get_left_page_number(&n_page, n_number_of_keys + 1), (char*)&temp_page);
            unpin_page(table_id, INTERNAL_PAGE::get_left_page_number(&n_page, n_number_of_keys + 1));
            

            parent_page_number = INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&n_page);
            // file_read_page(table_id, parent_page_number, (char*)&n_parent_page);
            buffer_read_page(table_id, parent_page_number, (char*)&n_parent_page);
            INTERNAL_PAGE::set_key(&n_parent_page, INTERNAL_PAGE::get_key(&neighbor_page, 0), k_prime_index);
        
            for(i = 0; i < INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&neighbor_page) - 1; i++){
                INTERNAL_PAGE::set_key(&neighbor_page, INTERNAL_PAGE::get_key(&neighbor_page, i + 1), i);
                INTERNAL_PAGE::set_left_page_number(&neighbor_page, INTERNAL_PAGE::get_left_page_number(&neighbor_page, i + 1), i);
            }

            INTERNAL_PAGE::set_left_page_number(&neighbor_page, INTERNAL_PAGE::get_left_page_number(&neighbor_page, i + 1), i);

            INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&n_page, n_number_of_keys + 1);
            INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&neighbor_page, neighbor_number_of_keys - 1);

        }
    }

    buffer_write_page(table_id, n_page_number, (char*)&n_page);
    buffer_write_page(table_id, neighbor_page_number, (char*)&neighbor_page);
    buffer_write_page(table_id, parent_page_number, (char*)&n_parent_page);

    unpin_page(table_id, n_page_number);
    unpin_page(table_id, neighbor_page_number);
    unpin_page(table_id, parent_page_number);

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif
    // printf("%s end.\n", __func__);
    return root_page_number;
}



int get_neighbor_index(int64_t table_id, pagenum_t n_page_number){
    // printf("%s start.\n", __func__);
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    int i;
    page_t n_page, n_parent_page;
    // file_read_page(table_id, n_page_number, (char*)&n_page);
    // file_read_page(table_id, INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&n_page), (char*)&n_parent_page);
    buffer_read_page(table_id, n_page_number, (char*)&n_page);
    buffer_read_page(table_id, INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&n_page), (char*)&n_parent_page);



    for(i = 0; i <= INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_parent_page); i++){
        if(INTERNAL_PAGE::get_left_page_number(&n_parent_page, i) == n_page_number) {
            // printf("%s end.\n", __func__);
            unpin_page(table_id, n_page_number);
            unpin_page(table_id, INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&n_page));
            
            #ifdef DEBUG_NEWBPT_H
            print_bpt_function_end(__func__);
            #endif

            return i - 1;
        }
    }

    unpin_page(table_id, n_page_number);
    unpin_page(table_id, INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&n_page));
    
    #ifdef DEBUG_NEWBPT_H
    print_bpt_error(__FILE__, __func__, __LINE__);
    #endif
    exit(1);
}

pagenum_t adjust_root(int64_t table_id, pagenum_t root_page_number){
    // printf("%s start.\n", __func__);
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    page_t root_page;
    // file_read_page(table_id, root_page_number, (char*)&root_page);
    buffer_read_page(table_id, root_page_number, (char*)&root_page);
    

    if(INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&root_page) > 0){
        // printf("%s end.\n", __func__);

        unpin_page(table_id, root_page_number);

        #ifdef DEBUG_NEWBPT_H
        print_bpt_function_end(__func__);
        #endif

        return root_page_number;
    }

    pagenum_t new_root_page_number;
    page_t new_root_page;

    if(!INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&root_page)){
        new_root_page_number = INTERNAL_PAGE::get_left_page_number(&root_page, 0);
        // file_read_page(table_id, new_root_page_number, (char*)&new_root_page);
        buffer_read_page(table_id, new_root_page_number, (char*)&new_root_page);

        INTERNAL_PAGE::PAGE_HEADER::set_parent_page_number(&new_root_page, 0);
        
        // file_write_page(table_id, new_root_page_number, (char*)&new_root_page);
        buffer_write_page(table_id, new_root_page_number, (char*)&new_root_page);
        unpin_page(table_id, new_root_page_number);
    }
    else{
        new_root_page_number = 0;
    }

    unpin_page(table_id, root_page_number);
    buffer_free_page(table_id, root_page_number);

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif

    return new_root_page_number;
}

pagenum_t db_remove_entry_from_node(int64_t table_id, pagenum_t n_page_number, int64_t key, pagenum_t pointer){
    // printf("%s start.\n", __func__);
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    page_t n_page;
    // file_read_page(table_id, n_page_number, (char*)&n_page);
    buffer_read_page(table_id, n_page_number, (char*)&n_page);
    
    
    if(INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&n_page)){
        int i = 0;

        // while(LEAF_PAGE::get_slot(&n_page, i).key != key){
        //     i++;
        // }

        int l = 0;
        int r = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_page) - 1;
        while(l <= r){
            int m = (l + r) / 2;
            if(LEAF_PAGE::get_slot(&n_page, m).key == key){
                i = m;
                break;
            }
            else if(LEAF_PAGE::get_slot(&n_page, m).key < key){
                l = m + 1;
            }
            else{
                r = m - 1;
            }
        }

        slot target = LEAF_PAGE::get_slot(&n_page, i);
        int number_of_keys = LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&n_page);

        slot* temp_slots = (slot*)malloc(sizeof(slot)*MAX_SLOT_NUMBER);
        char** temp_values = (char**)malloc(sizeof(char*)*MAX_SLOT_NUMBER);
        uint16_t new_offset = PAGE_SIZE;

        for(int j = 0, k = 0; j < number_of_keys; j++, k++){
            if(i == j) j++;
            temp_slots[k] = LEAF_PAGE::get_slot(&n_page, j);
            // temp_values[k] = (char*)malloc(sizeof(char)*temp_slots[k].size);
            temp_values[k] = LEAF_PAGE::get_value(&n_page, temp_slots[k]);

            temp_slots[k].offset = new_offset - temp_slots[k].size;
            new_offset -= temp_slots[k].size;
        }

        for(int j = 0; j < number_of_keys - 1; j++){
            LEAF_PAGE::set_slot(&n_page, temp_slots[j], j);
            LEAF_PAGE::set_value(&n_page, temp_slots[j], temp_values[j]);

            free(temp_values[j]);
            temp_values[j] = NULL;
        }


        LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&n_page, LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&n_page) + sizeof(slot) + target.size);
        LEAF_PAGE::PAGE_HEADER::set_number_of_keys(&n_page, number_of_keys - 1);

        // file_write_page(table_id, n_page_number, (char*)&n_page);
        buffer_write_page(table_id, n_page_number, (char*)&n_page);
        unpin_page(table_id, n_page_number);

        free(temp_slots);
        free(temp_values);

        temp_slots = NULL;
        temp_values = NULL;

        // printf("%s end.\n", __func__);

        #ifdef DEBUG_NEWBPT_H
        print_bpt_function_end(__func__);
        #endif

        return n_page_number;
    }else{
        int i = 0;

        // while(INTERNAL_PAGE::get_key(&n_page, i) != key){
        //     i++;
        // }

        int l = 0;
        int r = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_page) - 1;
        while(l <= r){
            int m = (l + r) / 2;
            if(INTERNAL_PAGE::get_key(&n_page, m) == key){
                i = m;
                break;
            }
            else if(INTERNAL_PAGE::get_key(&n_page, m) < key){
                l = m + 1;
            }
            else{
                r = m - 1;
            }
        }

        for(++i; i < INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_page); i++){
            INTERNAL_PAGE::set_key(&n_page, INTERNAL_PAGE::get_key(&n_page, i), i - 1);
        }

        i = 0;
        // while(INTERNAL_PAGE::get_left_page_number(&n_page, i) != pointer){
        //     i++;
        // }
        l = 0;
        r = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_page) - 1;
        while(l <= r){
            int m = (l + r) / 2;
            if(INTERNAL_PAGE::get_left_page_number(&n_page, m) == pointer){
                i = m;
                break;
            }
            else if(INTERNAL_PAGE::get_left_page_number(&n_page, m) < pointer){
                l = m + 1;
            }
            else{
                r = m - 1;
            }
        }

        for(++i; i < INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_page) + 1; i++){
            INTERNAL_PAGE::set_left_page_number(&n_page, INTERNAL_PAGE::get_left_page_number(&n_page, i), i - 1);
        }


        INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&n_page, INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_page) - 1);
        
        // file_write_page(table_id, n_page_number, (char*)&n_page);
        
        buffer_write_page(table_id, n_page_number, (char*)&n_page);
        unpin_page(table_id, n_page_number);

        // printf("%s end.\n", __func__);
        #ifdef DEBUG_NEWBPT_H
        print_bpt_function_end(__func__);
        #endif

        return n_page_number;
    }
}


int init_db (int num_buf){
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    // TODO : turn on buffer manager
    int temp = init_lock_table();
    temp &= trx_init();
    temp &= init_buffer(num_buf);

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif

    return temp;  
}

int shutdown_db(){
    // TODO : turn off buffer manager

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    int temp = shutdown_buffer();
    paths.clear();
    file_close_table_files();

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif

    return temp;
};


int db_find (int64_t table_id, int64_t key, char * ret_val, uint16_t * val_size, int trx_id){
    // printf("%s start.\n", __func__);
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    pthread_mutex_lock(&buffer_mutex);

    page_t header_page;
    buffer_read_page(table_id, 0, (char*)&header_page);

    pagenum_t root_page_number = HEADER_PAGE::get_root_page_number(&header_page);

    if(root_page_number == 0){
        // printf("%s end.\n", __func__);
        unpin_page(table_id, 0);
        pthread_mutex_unlock(&buffer_mutex);

        #ifdef DEBUG_NEWBPT_H
        print_bpt_function_end(__func__);
        #endif

        return 0;
    }

    pagenum_t leaf_page_number = db_find_leaf_page_number(table_id, root_page_number, key);
    control_block leaf_page_control_block;

    page_t leaf_page;
    buffer_read_page(table_id, leaf_page_number, (char*)&leaf_page);

    buffer_read_control_block(table_id, leaf_page_number, (char*)&leaf_page_control_block);
    pthread_mutex_lock(&leaf_page_control_block.page_mutex);
    
    pthread_mutex_unlock(&buffer_mutex);

    uint64_t number_of_keys = LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&leaf_page);
    
    int l = 0;
    int r = number_of_keys - 1;
    while(l <= r){
        int m = (l + r) / 2;
        slot temp_slot = LEAF_PAGE::get_slot(&leaf_page, m);
        if(temp_slot.key == key){
            pthread_mutex_unlock(&leaf_page_control_block.page_mutex);
            lock_t* lock_obj = lock_acquire(table_id, leaf_page_number, temp_slot.key, trx_id, 0);
            pthread_mutex_lock(&leaf_page_control_block.page_mutex);

            if(lock_obj == nullptr){
                // ABORTED!
                printf("ABORTED\n");
                exit(0);
                trx_abort(trx_id);
                unpin_page(table_id, leaf_page_number);
                unpin_page(table_id, 0);
                
                pthread_mutex_unlock(&leaf_page_control_block.page_mutex);

                #ifdef DEBUG_NEWBPT_H
                print_bpt_function_end(__func__);
                #endif

                return -1;
            }

            // trx_add(trx_id, lock_obj);
            
            *val_size = temp_slot.size;
            char* target = LEAF_PAGE::get_value(&leaf_page, temp_slot);
            memmove(ret_val, target, temp_slot.size);
            free(target);

            // printf("%s end.\n", __func__);
            unpin_page(table_id, leaf_page_number);
            unpin_page(table_id, 0);

            pthread_mutex_unlock(&leaf_page_control_block.page_mutex);

            #ifdef DEBUG_NEWBPT_H
            print_bpt_function_end(__func__);
            #endif

            return 0;
        }
        else if(temp_slot.key > key){
            r = m - 1;
        }
        else{
            l = m + 1;
        }
    }

    unpin_page(table_id, leaf_page_number);
    unpin_page(table_id, 0);

    pthread_mutex_unlock(&leaf_page_control_block.page_mutex);

    // printf("%s end.\n", __func__);
    // NOT FOUND
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif

    return 0;
}


int db_update(int64_t table_id, int64_t key, char* values, uint16_t new_val_size, uint16_t* old_val_size, int trx_id){
    // printf("%s start.\n", __func__);

    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_start(__func__);
    #endif

    pthread_mutex_lock(&buffer_mutex);

    page_t header_page;
    buffer_read_page(table_id, 0, (char*)&header_page);

    pagenum_t root_page_number = HEADER_PAGE::get_root_page_number(&header_page);

    if(root_page_number == 0){
        // printf("%s end.\n", __func__);
        unpin_page(table_id, 0);
        pthread_mutex_unlock(&buffer_mutex);
    
        #ifdef DEBUG_NEWBPT_H
        print_bpt_function_end(__func__);
        #endif

        return 0;
    }

    pagenum_t leaf_page_number = db_find_leaf_page_number(table_id, root_page_number, key);
    control_block leaf_page_control_block;

    page_t leaf_page;
    buffer_read_page(table_id, leaf_page_number, (char*)&leaf_page);

    buffer_read_control_block(table_id, leaf_page_number, (char*)&leaf_page_control_block);
    pthread_mutex_lock(&leaf_page_control_block.page_mutex);

    pthread_mutex_unlock(&buffer_mutex);

    uint64_t number_of_keys = LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&leaf_page);
    
    int l = 0;
    int r = number_of_keys - 1;
    while(l <= r){
        int m = (l + r) / 2;
        slot temp_slot = LEAF_PAGE::get_slot(&leaf_page, m);
        if(temp_slot.key == key){
            pthread_mutex_unlock(&leaf_page_control_block.page_mutex);
            lock_t* lock_obj = lock_acquire(table_id, leaf_page_number, temp_slot.key, trx_id, 1);
            pthread_mutex_lock(&leaf_page_control_block.page_mutex);

            if(lock_obj == nullptr){
                // ABORTED!
                printf("ABORTED\n");
                exit(0);

                unpin_page(table_id, leaf_page_number);
                unpin_page(table_id, 0);

                pthread_mutex_unlock(&leaf_page_control_block.page_mutex);

                #ifdef DEBUG_NEWBPT_H
                print_bpt_function_end(__func__);
                #endif

                return -1;
            }

            
            *old_val_size = temp_slot.size;
            temp_slot.size = new_val_size;
            LEAF_PAGE::set_value(&leaf_page, temp_slot, values);

            buffer_write_page(table_id, leaf_page_number, (char*)&leaf_page);
            // printf("%s end.\n", __func__);
            unpin_page(table_id, leaf_page_number);
            unpin_page(table_id, 0);

            pthread_mutex_unlock(&leaf_page_control_block.page_mutex);

            #ifdef DEBUG_NEWBPT_H
            print_bpt_function_end(__func__);
            #endif
            
            return 0;
        }
        else if(temp_slot.key > key){
            r = m - 1;
        }
        else{
            l = m + 1;
        }
    }

    unpin_page(table_id, leaf_page_number);
    unpin_page(table_id, 0);

    pthread_mutex_unlock(&leaf_page_control_block.page_mutex);
    printf("NOT FOUND\n");
    // NOT FOUND
    #ifdef DEBUG_NEWBPT_H
    print_bpt_function_end(__func__);
    #endif

    return 0;   
}