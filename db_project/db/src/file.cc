#include "file.h"
#include "page.h"
#include "debug.h"
std::vector<int> opened_files;


// Open existing database file or create one if it doesn't exist
int64_t file_open_table_file(const char* pathname){
    #ifdef DEBUG_FILE_H
    print_function_start(__func__);
    #endif

    int64_t fd = open(pathname, O_RDWR, 0644);
    if(fd == -1) {
        fd = open(pathname, O_RDWR | O_CREAT, 0644);
       
        if(fd == -1){ // creat() is failed
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
            return -1;
        }

        for(int i = 1; i < P_NUMBER; i++){
            page_t free_page;
            FREE_PAGE::set_next_free_page_number(&free_page, i - 1);
            // free_page.free.next_free_page_number = i - 1;
            ssize_t pw = pwrite(fd, &free_page, PAGE_SIZE, i * PAGE_SIZE);
            
            if(pw == -1){ // pwrite() is failed
                #ifdef DEBUG_FILE_H
                    print_error(__FILE__, __func__, __LINE__);
                #endif
                return -1;
            }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
                #ifdef DEBUG_FILE_H
                print_error(__FILE__, __func__, __LINE__);
                #endif
                return -1;
            }
            // int fs = fsync(fd);

            // if(fs == -1){ // fsync() failed
            //     #ifdef DEBUG_FILE_H
            //     print_error(__FILE__, __func__, __LINE__);
            //     #endif    
            //     return -1;
            // }
        }

        page_t header_page;
        HEADER_PAGE::set_free_page_number(&header_page, P_NUMBER - 1);
        HEADER_PAGE::set_number_of_pages(&header_page, P_NUMBER);
        HEADER_PAGE::set_root_page_number(&header_page, 0);
        // header_page.header.free_page_number = P_NUMBER - 1;
        // header_page.header.number_of_pages = P_NUMBER;
        ssize_t pw = pwrite(fd, &header_page, PAGE_SIZE, 0);

        if(pw == -1){ // pwrite() is failed
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
            return -1;
        }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
            return -1;
        }

        // int fs = fsync(fd);

        // if(fs == -1){ // fsync() failed
        //     #ifdef DEBUG_FILE_H
        //     print_error(__FILE__, __func__, __LINE__);
        //     #endif
        // }
    }

    int fs = fsync(fd);

    if(fs == -1){ // fsync() failed
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
    }
    opened_files.push_back(fd);

    #ifdef DEBUG_FILE_H
    print_function_end(__func__);
    #endif
    return fd;
}

// Allocate an on-disk page from the free page list
uint64_t file_alloc_page(int64_t table_id){
    #ifdef DEBUG_FILE_H
    print_function_start(__func__);
    #endif

    page_t header_page;
    int pr = pread(table_id, &header_page, PAGE_SIZE, 0);
    
    if(pr == -1){ // pread() failed
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
    }else if(pr != PAGE_SIZE){ // pread() didn't read whole data
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
    }

    pagenum_t next_fpn = HEADER_PAGE::get_free_page_number(&header_page) ;// header_page.header.free_page_number;
    pagenum_t cur_number_of_pages = HEADER_PAGE::get_number_of_pages(&header_page);
    // printf("alloc fpn : %ld\n", next_fpn);
    if(next_fpn != 0){
        page_t next_free_page;
        int pr = pread(table_id, &next_free_page, PAGE_SIZE, next_fpn * PAGE_SIZE);

        if(pr == -1){ // pread() failed
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
        }else if(pr != PAGE_SIZE){ // pread() didn't read whole data
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
        }

        HEADER_PAGE::set_free_page_number(&header_page, FREE_PAGE::get_next_free_page_number(&next_free_page));

        ssize_t pw = pwrite(table_id, &header_page, PAGE_SIZE, 0);

        if(pw == -1){ // pwrite() is failed
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
        }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
        }

        int fs = fsync(table_id);

        if(fs == -1){ // fsync() failed
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
        }
        #ifdef DEBUG_FILE_H
        print_function_end(__func__);
        #endif
            
        return next_fpn;
    }else{
        page_t free_page;
        FREE_PAGE::set_next_free_page_number(&free_page, HEADER_PAGE::get_free_page_number(&header_page));
        // free_page.free.next_free_page_number = 0;
        ssize_t pw = pwrite(table_id, &free_page, PAGE_SIZE, cur_number_of_pages * PAGE_SIZE);
        if(pw == -1){ // pwrite() is failed
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
        }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
        }

        // int fs = fsync(table_id);

        // if(fs == -1){ // fsync() failed
        //     #ifdef DEBUG_FILE_H
        //     print_error(__FILE__, __func__, __LINE__);
        //     #endif
        // }

        for(int i = cur_number_of_pages + 1; i < cur_number_of_pages * 2; i++){
            page_t free_page;
            FREE_PAGE::set_next_free_page_number(&free_page, i - 1);
            
            ssize_t pw = pwrite(table_id, &free_page, PAGE_SIZE, i * PAGE_SIZE);
            if(pw == -1){ // pwrite() is failed
                #ifdef DEBUG_FILE_H
                print_error(__FILE__, __func__, __LINE__);
                #endif
            }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
                #ifdef DEBUG_FILE_H
                print_error(__FILE__, __func__, __LINE__);
                #endif    
            }

            // int fs = fsync(table_id);

            // if(fs == -1){ // fsync() failed
            //     #ifdef DEBUG_FILE_H
            //     print_error(__FILE__, __func__, __LINE__);
            //     #endif    
            // }
        }
        HEADER_PAGE::set_free_page_number(&header_page, cur_number_of_pages * 2 - 2);
        HEADER_PAGE::set_number_of_pages(&header_page, HEADER_PAGE::get_number_of_pages(&header_page)*2);
        pw = pwrite(table_id, &header_page, PAGE_SIZE, 0);
        if(pw == -1){ // pwrite() is failed
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
        }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
        }

        int fs = fsync(table_id);

        if(fs == -1){ // fsync() failed
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
        }

        #ifdef DEBUG_FILE_H
        print_function_end(__func__);
        #endif
        
        return cur_number_of_pages * 2 - 1;
    }
    #ifdef DEBUG_FILE_H
    print_error(__FILE__, __func__, __LINE__);
    #endif
}

// Free an on-disk page to the free page list
void file_free_page(int64_t table_id, uint64_t page_number){
    #ifdef DEBUG_FILE_H
    print_function_start(__func__);
    #endif
    
    page_t header_page;
    ssize_t pr = pread(table_id, &header_page, PAGE_SIZE, 0);
    if(pr == -1){ // pread() failed
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;
    }else if(pr != PAGE_SIZE){ // pread() didn't read whole data
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif 
        return;   
    }

    pagenum_t next_fpn = HEADER_PAGE::get_free_page_number(&header_page) ; //header_page.header.free_page_number;

    page_t target_page;
    pr = pread(table_id, &target_page, PAGE_SIZE, page_number * PAGE_SIZE);
    if(pr == -1){ // pread() failed
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;
    }else if(pr != PAGE_SIZE){ // pread() didn't read whole data
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;
    }

    FREE_PAGE::set_next_free_page_number(&target_page, next_fpn);

    HEADER_PAGE::set_free_page_number(&header_page, page_number);
    // header_page.header.free_page_number = pagenum;
    ssize_t pw = pwrite(table_id, &header_page, PAGE_SIZE, 0);
    if(pw == -1){ // pwrite() is failed
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;    
    }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;        
    }

    // int fs = fsync(table_id);

    // if(fs == -1){ // fsync() failed
    //     #ifdef DEBUG_FILE_H
    //     print_error(__FILE__, __func__, __LINE__);
    //     #endif
    //     return;    
    // }

    pw = pwrite(table_id, &target_page, PAGE_SIZE, page_number * PAGE_SIZE);
    if(pw == -1){ // pwrite() is failed
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;
    }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;    
    }

    int fs = fsync(table_id);

    if(fs == -1){ // fsync() failed
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;    
    }

    #ifdef DEBUG_FILE_H
    print_function_end(__func__);
    #endif
            
}

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int64_t table_id, uint64_t page_number, char* dest){
    #ifdef DEBUG_FILE_H
    print_function_start(__func__);
    #endif
    
    

    page_t header_page;
    ssize_t pr = pread(table_id, &header_page, PAGE_SIZE, 0);
    if(pr == -1){ // pread() failed
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;
    }else if(pr != PAGE_SIZE){ // pread() didn't read whole data
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;
    }

    pagenum_t number_of_pages = HEADER_PAGE::get_number_of_pages(&header_page);

    while(page_number >= number_of_pages){
        // printf("read extension\n");
        for(int i = number_of_pages; i < number_of_pages * 2; i++){
            page_t free_page;
            if(i == number_of_pages){
                FREE_PAGE::set_next_free_page_number(&free_page, HEADER_PAGE::get_free_page_number(&header_page));
            }else{
                FREE_PAGE::set_next_free_page_number(&free_page, i - 1);
            }
            ssize_t pw = pwrite(table_id, &free_page, PAGE_SIZE, i * PAGE_SIZE);
            if(pw == -1){ // pwrite() is failed
                #ifdef DEBUG_FILE_H
                print_error(__FILE__, __func__, __LINE__);
                #endif
                return;
            }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
                #ifdef DEBUG_FILE_H
                print_error(__FILE__, __func__, __LINE__);
                #endif
                return;
            }

            // int fs = fsync(table_id);

            // if(fs == -1){ // fsync() failed
            //     #ifdef DEBUG_FILE_H
            //     print_error(__FILE__, __func__, __LINE__);
            //     #endif
            //     return;
            // }
        }
        HEADER_PAGE::set_free_page_number(&header_page, number_of_pages * 2 - 1);
        HEADER_PAGE::set_number_of_pages(&header_page, number_of_pages * 2);
        int pw = pwrite(table_id, &header_page, PAGE_SIZE, 0);
        if(pw == -1){ // pwrite() is failed
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
            return;
        }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
            return;
        }

        // int fs = fsync(table_id);

        // if(fs == -1){ // fsync() failed
        //     #ifdef DEBUG_FILE_H
        //     print_error(__FILE__, __func__, __LINE__);
        //     #endif
        //     return;
        // }
        number_of_pages = HEADER_PAGE::get_number_of_pages(&header_page);
    }
    
    int fs = fsync(table_id);

    if(fs == -1){ // fsync() failed
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;
    }
    

    pr = pread(table_id, dest, PAGE_SIZE, page_number * PAGE_SIZE);
    if(pr == -1){ // pread() failed
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;
    }else if(pr != PAGE_SIZE){ // pread() didn't read whole data
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;
    }

    #ifdef DEBUG_FILE_H
    print_function_end(__func__);
    #endif
}

// Write an in-memory page(src) to the on-disk page
void file_write_page(int64_t table_id, uint64_t page_number, const char* src){
    #ifdef DEBUG_FILE_H
    print_function_start(__func__);
    #endif


    // TODO --------------
    page_t header_page;
    ssize_t pr = pread(table_id, &header_page, PAGE_SIZE, 0);
    if(pr == -1){ // pread() failed
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;
    }else if(pr != PAGE_SIZE){ // pread() didn't read whole data
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;
    }

    pagenum_t number_of_pages = HEADER_PAGE::get_number_of_pages(&header_page);

    while(page_number >= number_of_pages){
        for(int i = number_of_pages; i < number_of_pages * 2; i++){
            page_t free_page;
            if(i == number_of_pages){
                FREE_PAGE::set_next_free_page_number(&free_page, HEADER_PAGE::get_free_page_number(&header_page));
            }else{
                FREE_PAGE::set_next_free_page_number(&free_page, i - 1);
            }

            ssize_t pw = pwrite(table_id, &free_page, PAGE_SIZE, i * PAGE_SIZE);
            if(pw == -1){ // pwrite() is failed
                #ifdef DEBUG_FILE_H
                print_error(__FILE__, __func__, __LINE__);
                #endif
                return;    
            }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
                #ifdef DEBUG_FILE_H
                print_error(__FILE__, __func__, __LINE__);
                #endif
                return;    
            }

            // int fs = fsync(table_id);

            // if(fs == -1){ // fsync() failed
            //     #ifdef DEBUG_FILE_H
            //     print_error(__FILE__, __func__, __LINE__);
            //     #endif
            //     return;    
            // }
        }

        HEADER_PAGE::set_free_page_number(&header_page, number_of_pages * 2 - 1);
        HEADER_PAGE::set_number_of_pages(&header_page, number_of_pages * 2);
        int pw = pwrite(table_id, &header_page, PAGE_SIZE, 0);
        if(pw == -1){ // pwrite() is failed
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
            return;    
        }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
            return;    
        }

        // int fs = fsync(table_id);

        // if(fs == -1){ // fsync() failed
        //     #ifdef DEBUG_FILE_H
        //     print_error(__FILE__, __func__, __LINE__);
        //     #endif
        //     return;    
        // }
        // number_of_pages = HEADER_PAGE::get_number_of_pages(&header_page);
    }
    // --------------------

    ssize_t pw = pwrite(table_id, src, PAGE_SIZE, page_number * PAGE_SIZE);
    if(pw == -1){ // pwrite() is failed
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;
    }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;    
    }

    int fs = fsync(table_id);

    if(fs == -1){ // fsync() failed
        #ifdef DEBUG_FILE_H
        print_error(__FILE__, __func__, __LINE__);
        #endif
        return;
    }

    #ifdef DEBUG_FILE_H
    print_function_end(__func__);
    #endif
    
}

// Close the database file
void file_close_table_files(){
    #ifdef DEBUG_FILE_H
    print_function_start(__func__);
    #endif
    
    for(int opened : opened_files){
        int cl = close(opened);
        if(cl == -1){
            #ifdef DEBUG_FILE_H
            print_error(__FILE__, __func__, __LINE__);
            #endif
            return;    
        }
    }
    opened_files.clear();
    #ifdef DEBUG_FILE_H
    print_function_end(__func__);
    #endif    
    
}