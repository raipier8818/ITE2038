#include "buffer.h"

std::vector<page_t> page_buffer;
std::vector<control_block> page_buffer_control_block;
std::map<std::pair<int64_t, pagenum_t>, int> page_buffer_map;

pthread_mutex_t buffer_mutex;

int number_of_pages;
int entry_point;
int NUM_OF_BUFFER;

#ifdef DEBUG_BUFFER_H

void print_buffer(){
    int temp = entry_point;
    printf(" < buffer : %d > \n", number_of_pages);
    for(int i = 0; i < number_of_pages; i++){
        printf(" %d(%d) ->", page_buffer_control_block[temp].page_number, page_buffer_control_block[temp].is_pinned);
        temp = page_buffer_control_block[temp].next;
    }
    printf("\n");
}

#endif

int init_buffer(int num_buf){
    #ifdef DEBUG_BUFFER_H
    print_buffer_function_start(__func__);
    #endif

    NUM_OF_BUFFER = num_buf > (MIN_BUFFER_SIZE) ? num_buf : (MIN_BUFFER_SIZE); 

    buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

    page_buffer.resize(NUM_OF_BUFFER);
    page_buffer_control_block.resize(NUM_OF_BUFFER);

    for(int i = 0; i < NUM_OF_BUFFER; i++){
        page_buffer_control_block[i].is_dirty = 0;
        page_buffer_control_block[i].is_pinned = 0;
    }


    number_of_pages = 0;
    entry_point = -1;

    #ifdef DEBUG_BUFFER_H
    print_buffer_function_end(__func__);
    #endif

    return 0;
}

int find_index_from_page_buffer(int64_t table_id, uint64_t page_number){
    #ifdef DEBUG_BUFFER_H
    print_buffer_function_start(__func__);
    #endif

    // if(page_buffer_map.find(std::make_pair(table_id, page_number)) == page_buffer_map.end()){
    //     #ifdef DEBUG_BUFFER_H
    //     print_buffer_function_end(__func__);
    //     #endif
    //     return -1;
    // }

    // return page_buffer_map[std::make_pair(table_id, page_number)];

    int temp1 = page_buffer_control_block[entry_point].next;
    int temp2 = entry_point;
    for(int i = 0; i < number_of_pages; i++){
        if(page_buffer_control_block[temp1].table_id == table_id && page_buffer_control_block[temp1].page_number == page_number){
            
            #ifdef DEBUG_BUFFER_H
            print_buffer_function_end(__func__);
            #endif

            return temp1;
        }
        if(page_buffer_control_block[temp2].table_id == table_id && page_buffer_control_block[temp2].page_number == page_number){
            
            #ifdef DEBUG_BUFFER_H
            print_buffer_function_end(__func__);
            #endif
            
            return temp2;
        }
        temp1 = page_buffer_control_block[temp1].next;
        temp2 = page_buffer_control_block[temp2].prev;
    }

    #ifdef DEBUG_BUFFER_H
    print_buffer_function_end(__func__);
    #endif

    return -1;
}

void buffer_read_control_block(int64_t table_id, uint64_t page_number, char* dest){
    #ifdef DEBUG_BUFFER_H
    print_buffer_function_start(__func__);
    #endif

    int index = find_index_from_page_buffer(table_id, page_number);

    if(index == -1){
        #ifdef DEBUG_BUFFER_H
        print_buffer_function_end(__func__);
        #endif
        
        exit(1);
    }

    memmove(dest, &page_buffer_control_block[index], sizeof(page_buffer_control_block));

    #ifdef DEBUG_BUFFER_H
    print_buffer_function_end(__func__);
    #endif
}

int read_new_page(int64_t table_id, uint16_t page_number){
    // printf("%d %d\n", table_id, page_number);
    #ifdef DEBUG_BUFFER_H
    print_buffer_function_start(__func__);
    #endif

    page_t new_page;
    file_read_page(table_id, page_number, (char*)&new_page);
    
    if(number_of_pages == 0){
        control_block temp;
        temp.table_id = table_id;
        temp.page_number = page_number;
        temp.next = 0;
        temp.prev = 0;

        temp.is_dirty = 0;
        temp.is_pinned = 0;
        temp.page_mutex = PTHREAD_MUTEX_INITIALIZER;

        page_buffer[number_of_pages] = new_page;
        page_buffer_control_block[number_of_pages] = temp;

        page_buffer_map.insert(std::make_pair(std::make_pair(table_id, page_number), number_of_pages));

        number_of_pages++;
        entry_point++;
    }else if(number_of_pages < NUM_OF_BUFFER){
        page_buffer_control_block[number_of_pages].table_id = table_id;
        page_buffer_control_block[number_of_pages].page_number = page_number;

        page_buffer_control_block[number_of_pages].is_dirty = 0;
        page_buffer_control_block[number_of_pages].is_pinned = 0;

        page_buffer_control_block[number_of_pages].page_mutex = PTHREAD_MUTEX_INITIALIZER;

        int next_idx = page_buffer_control_block[entry_point].next;

        page_buffer_control_block[number_of_pages].next = next_idx;
        page_buffer_control_block[number_of_pages].prev = entry_point;

        page_buffer_control_block[entry_point].next = number_of_pages;
        page_buffer_control_block[next_idx].prev = number_of_pages;
        
        page_buffer[number_of_pages] = new_page;

        page_buffer_map.insert(std::make_pair(std::make_pair(table_id, page_number), number_of_pages));

        entry_point = number_of_pages;
        number_of_pages++;
    }else{
        int i = page_buffer_control_block[entry_point].next;
        int j = 0;
        for(; j < number_of_pages; j++){
            if(page_buffer_control_block[i].is_pinned == 0) break;
            i = page_buffer_control_block[i].next;
        }

        if(j == NUM_OF_BUFFER){
            printf("No Buffer.\n");

            pthread_mutex_unlock(&buffer_mutex);
            #ifdef DEBUG_BUFFER_H
            print_buffer_error(__FILE__,__func__,__LINE__);
            #endif
            exit(1);
        }

        page_buffer_map.erase(std::make_pair(page_buffer_control_block[i].table_id, page_buffer_control_block[i].page_number));

        if(page_buffer_control_block[i].is_dirty == 1){
            file_write_page(page_buffer_control_block[i].table_id, page_buffer_control_block[i].page_number, (char*)&page_buffer[i]);
        }

        page_buffer_control_block[i].table_id = table_id;
        page_buffer_control_block[i].page_number = page_number;
        page_buffer_control_block[i].is_dirty = 0;
        page_buffer_control_block[i].is_pinned = 0;
        page_buffer_control_block[i].page_mutex = PTHREAD_MUTEX_INITIALIZER;

        int prev_idx = page_buffer_control_block[i].prev;
        int next_idx = page_buffer_control_block[i].next;

        page_buffer_control_block[prev_idx].next = next_idx;
        page_buffer_control_block[next_idx].prev = prev_idx;

        next_idx = page_buffer_control_block[entry_point].next;

        page_buffer_control_block[entry_point].next = i;
        page_buffer_control_block[next_idx].prev = i;

        page_buffer_control_block[i].next = next_idx;
        page_buffer_control_block[i].prev = entry_point;

        page_buffer[i] = new_page;
        page_buffer_map.insert(std::make_pair(std::make_pair(table_id, page_number), i));

        entry_point = i;
    }


    #ifdef DEBUG_BUFFER_H
    print_buffer_function_end(__func__);
    #endif
    return 1;
}

uint64_t buffer_alloc_page(int64_t table_id){    
    // pthread_mutex_lock(&buffer_mutex);
    #ifdef DEBUG_BUFFER_H
    print_buffer_function_start(__func__);
    #endif

    uint64_t new_page_number = file_alloc_page(table_id);
    page_t header_page, free_page;
    buffer_read_page(table_id, 0, (char*)&header_page);
    buffer_read_page(table_id, new_page_number, (char*)&free_page);


    if(new_page_number >= HEADER_PAGE::get_number_of_pages(&header_page)){
        HEADER_PAGE::set_number_of_pages(&header_page, HEADER_PAGE::get_number_of_pages(&header_page) * 2);
    }

    HEADER_PAGE::set_free_page_number(&header_page, FREE_PAGE::get_next_free_page_number(&free_page));


    buffer_write_page(table_id, 0, (char*)&header_page);
    unpin_page(table_id, 0);
    unpin_page(table_id, new_page_number);

    #ifdef DEBUG_BUFFER_H
    print_buffer_function_end(__func__);
    #endif

    // pthread_mutex_unlock(&buffer_mutex);
    return new_page_number;
}

void buffer_free_page(int64_t table_id, uint64_t page_number){
    #ifdef DEBUG_BUFFER_H
    print_buffer_function_start(__func__);
    #endif

    // pthread_mutex_lock(&buffer_mutex);
    int i = find_index_from_page_buffer(table_id, 0);

    if(i == -1){
        file_free_page(table_id, page_number);
    }

    int freed_page_i = find_index_from_page_buffer(table_id, page_number);
    
    page_t header_page, target_page;
    buffer_read_page(table_id, 0, (char*)&header_page);
    buffer_read_page(table_id, page_number, (char*)&target_page);

    pagenum_t next_fpn = HEADER_PAGE::get_free_page_number(&header_page);

    FREE_PAGE::set_next_free_page_number(&target_page, next_fpn);
    HEADER_PAGE::set_free_page_number(&header_page, page_number);

    buffer_write_page(table_id, 0, (char*)&header_page);
    buffer_write_page(table_id, page_number, (char*)&target_page);
    unpin_page(table_id, 0);
    unpin_page(table_id, page_number);

    // pthread_mutex_unlock(&buffer_mutex);

    #ifdef DEBUG_BUFFER_H
    print_buffer_function_end(__func__);
    #endif
}

void buffer_read_page(int64_t table_id, uint64_t page_number, char* dest){
    /* direct connection to disk space manager with header page. */
    // if(page_number == 0){
    //     file_read_page(table_id, 0, dest);
    //     return;
    // }   
    #ifdef DEBUG_BUFFER_H
    print_buffer_function_start(__func__);
    #endif


    int i = find_index_from_page_buffer(table_id, page_number);


    if(i == -1){
        int temp = read_new_page(table_id, page_number);
        if(temp != 1){
            // Handle page with file directly.
            printf("No unpinned buffer.\n");
            #ifdef DEBUG_BUFFER_H
            print_buffer_error(__FILE__,__func__,__LINE__);
            #endif
            exit(1);
            return;
        }

        i = find_index_from_page_buffer(table_id, page_number);

        if(i == -1){
            printf("%ld\n", page_number);
            printf("Fatal Error. %d\n", i);
            #ifdef DEBUG_BUFFER_H
            print_buffer_error(__FILE__,__func__,__LINE__);
            #endif
            exit(1);
        }

        page_buffer_control_block[i].is_pinned += 1;
        
        memmove(dest, &page_buffer[i], PAGE_SIZE);

        #ifdef DEBUG_BUFFER_H
        print_buffer_function_end(__func__);
        #endif
        
        return;
    }

    page_buffer_control_block[i].is_pinned += 1;
    
    memmove(dest, &page_buffer[i], PAGE_SIZE);

    if(entry_point == i){ 
        #ifdef DEBUG_BUFFER_H
        print_buffer_function_end(__func__);
        #endif

        return;
    }

    int next_idx = page_buffer_control_block[i].next;
    int prev_idx = page_buffer_control_block[i].prev;

    page_buffer_control_block[next_idx].prev = prev_idx;
    page_buffer_control_block[prev_idx].next = next_idx;

    next_idx = page_buffer_control_block[entry_point].next;
    page_buffer_control_block[entry_point].next = i;
    page_buffer_control_block[next_idx].prev = i;

    page_buffer_control_block[i].next = next_idx;
    page_buffer_control_block[i].prev = entry_point;

    entry_point = i;

    #ifdef DEBUG_BUFFER_H
    print_buffer_function_end(__func__);
    #endif
}

void buffer_write_page(int64_t table_id, uint64_t page_number, const char* src){
    /* direct connection to disk space manager with header page */
    // if(page_number == 0){
    //     file_write_page(table_id, 0, src);
    //     return;
    // }

    #ifdef DEBUG_BUFFER_H
    print_buffer_function_start(__func__);
    #endif

    int i = find_index_from_page_buffer(table_id, page_number);
    if(page_buffer_control_block[i].is_pinned == 0){
        printf("Unpinned page cannot be written.\n");

        #ifdef DEBUG_BUFFER_H
        print_buffer_error(__FILE__,__func__,__LINE__);
        #endif

        exit(1);
    }
    page_buffer_control_block[i].is_dirty = 1;
    memmove(&page_buffer[i], src, PAGE_SIZE);

    #ifdef DEBUG_BUFFER_H
    print_buffer_function_end(__func__);
    #endif
}

void unpin_page(int64_t table_id, uint64_t page_number){
    #ifdef DEBUG_BUFFER_H
    print_buffer_function_start(__func__);
    #endif

    // if(page_number == 0) return;
    // pthread_mutex_lock(&buffer_mutex);
    int i = find_index_from_page_buffer(table_id, page_number);
    page_buffer_control_block[i].is_pinned -= 1;
    // pthread_mutex_unlock(&buffer_mutex);

    #ifdef DEBUG_BUFFER_H
    print_buffer_function_end(__func__);
    #endif
}

int shutdown_buffer(){
    #ifdef DEBUG_BUFFER_H
    print_buffer_function_start(__func__);
    #endif

    flush_buffer();
    number_of_pages = 0;
    page_buffer.clear();
    page_buffer_control_block.clear();

    #ifdef DEBUG_BUFFER_H
    print_buffer_function_end(__func__);
    #endif
    
    return 0;
}

void flush_buffer(){
    for(int i = 0; i < number_of_pages; i++){
        if(page_buffer_control_block[i].is_dirty == 0) continue;
        file_write_page(page_buffer_control_block[i].table_id, page_buffer_control_block[i].page_number, (char*)&page_buffer[i]);
    }
}