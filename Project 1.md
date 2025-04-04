# Disk Space Manager

## Enviroment

 - OS : Ubuntu 20.04.3 LTS
 - Compiler Version: g++ 9.3.0

## Configuration

1. [file&#46;h](#fileh)
2. [file&#46;cc](#filecc)
3. [file_test&#46;cc](#file_testcc)
4. [dberror&#46;h](#dberrorh)
5. [dberror&#46;cc](#dberrorcc)

---

## file&#46;h

<details>
<summary>Source Code</summary>
<div markdown = '1'>

```cpp
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

// These definitions are not requirements.
// You may build your own way to handle the constants.


#define INITIAL_DB_FILE_SIZE (10 * 1024 * 1024)  // 10 MiB
#define PAGE_SIZE (4 * 1024)                     // 4 KiB

#define HP_RESERVED_SIZE ((PAGE_SIZE) - ((sizeof(pagenum_t)*2)))
#define FP_RESERVED_SIZE ((PAGE_SIZE) - (sizeof(pagenum_t)))

#define P_NUMBER ((INITIAL_DB_FILE_SIZE)/(PAGE_SIZE))

typedef uint64_t pagenum_t;

struct _header_page
{
    pagenum_t free_page_number;
    pagenum_t number_of_pages;
    char reserved[HP_RESERVED_SIZE];
};

struct _free_page
{
    pagenum_t next_free_page_number;
    char reserved[FP_RESERVED_SIZE];
};

struct _normal_page
{
    char reserved[PAGE_SIZE];
};

struct page_t {
// in-memory page structure
    union
    {
        struct _header_page header;
        struct _free_page free;
        struct _normal_page normal;
    };
};



// Open existing database file or create one if it doesn't exist
int file_open_database_file(const char* pathname);

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page(int fd);

// Free an on-disk page to the free page list
void file_free_page(int fd, pagenum_t pagenum);

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int fd, pagenum_t pagenum, page_t* dest);

// Write an in-memory page(src) to the on-disk page
void file_write_page(int fd, pagenum_t pagenum, const page_t* src);

// Close the database file
void file_close_database_file();

#endif  // DB_FILE_H_

```

</div> 
</details>

### Variables & Macros

 - `INITIAL_DB_FILE_SIZE` : the size of file when created (10Mib)
 - `PAGE_SIZE` : the size of page (4Kib)
 - `HP_RESERVED_SIZE` : the size of reserved area for other layers in header page (4088 Bytes)
 - `FP_RESERVED_SIZE` : the size of reserved area for DBMS in free page (4080 Bytes)
 - `P_NUMBER` : the number of pages in initialized file (2560)

 - `pagenum_t` : the page number in database file
 - `page_t` : the page structrue in database file
 - `_header_page` : the header page structure consist of `free_page_number`, `number_of_pages`, `reserved[HP_RESERVED_SIZE]`
 - `_free_page` : the free or allocated page structure consist of `next_free_page_number` and `reserved[FP_RESERVED_SIZE]`
 - `_normal_page` : the default page structure only consist of `reserved[PAGE_SIZE]`

### Functions

 - [`file_open_database_file`](#file_open_database_file)
 - [`file_alloc_page`](#file_alloc_page)
 - [`file_free_page`](#file_free_page)
 - [`file_read_page`](#file_read_page)
 - [`file_write_page`](#file_write_page)
 - [`file_close_database_file`](#file_close_database_file)



---

## file&#46;cc

<details>
<summary>Source Code</summary>
<div markdown = '1'>

```cpp
#include "file.h"
#include "dberror.h"

std::vector<int> opened_files;

// Open existing database file or create one if it doesn't exist
int file_open_database_file(const char* pathname){
    int fd = open(pathname, O_RDWR | O_SYNC, 0644);
    if(fd == -1) {
        errno = 0;
        fd = open(pathname, O_RDWR | O_CREAT | O_SYNC, 0644);
        // fd = creat(pathname, 0644);
       
        if(fd == -1){ // creat() is failed
            Error file_create_error("file create error", "creat() failed");
            file_create_error.printError(__FILE__, __func__, __LINE__);
        }

        for(int i = 1; i < P_NUMBER; i++){
            page_t free_page;
            free_page.free.next_free_page_number = i - 1;
            ssize_t pw = pwrite(fd, &free_page, PAGE_SIZE, i * PAGE_SIZE);
            
            if(pw == -1){ // pwrite() is failed
                Error file_write_error("file write error", "pwrite() failed");
                file_write_error.printError(__FILE__, __func__, __LINE__);
            }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
                Error file_write_error("file write error", "pwrite() end of file");
                file_write_error.printError(__FILE__, __func__, __LINE__);
            }
            int fs = fsync(fd);

            if(fs == -1){ // fsync() failed
                Error file_sync_error("file sync error", "fsync() failed");
                file_sync_error.printError(__FILE__, __func__, __LINE__);
            }
        }

        page_t header_page;
        header_page.header.free_page_number = P_NUMBER - 1;
        header_page.header.number_of_pages = P_NUMBER;
        ssize_t pw = pwrite(fd, &header_page, PAGE_SIZE, 0);

        if(pw == -1){ // pwrite() is failed
            Error file_write_error("file write error", "pwrite() failed");
            file_write_error.printError(__FILE__, __func__, __LINE__);
        }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
            Error file_write_error("file write error", "pwrite() end of file");
            file_write_error.printError(__FILE__, __func__, __LINE__);
        }

        int fs = fsync(fd);

        if(fs == -1){ // fsync() failed
            Error file_sync_error("file sync error", "fsync() failed");
            file_sync_error.printError(__FILE__, __func__, __LINE__);
        }
    }
    opened_files.push_back(fd);
    return fd;
}

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page(int fd){
    page_t header_page;
    int pr = pread(fd, &header_page, PAGE_SIZE, 0);
    
    if(pr == -1){ // pread() failed
        Error file_read_error("file read error", "pread() failed");
        file_read_error.printError(__FILE__, __func__, __LINE__);
    }else if(pr != PAGE_SIZE){ // pread() didn't read whole data
        Error file_read_error("file read error", "pread() end of file");
        file_read_error.printError(__FILE__, __func__, __LINE__);
    }

    pagenum_t next_fpn = header_page.header.free_page_number;

    if(next_fpn != 0){
        page_t next_free_page;
        int pr = pread(fd, &next_free_page, PAGE_SIZE, next_fpn * PAGE_SIZE);

        if(pr == -1){ // pread() failed
            Error file_read_error("file read error", "pread() failed");
            file_read_error.printError(__FILE__, __func__, __LINE__);
        }else if(pr != PAGE_SIZE){ // pread() didn't read whole data
            Error file_read_error("file read error", "pread() end of file");
            file_read_error.printError(__FILE__, __func__, __LINE__);
        }

        header_page.header.free_page_number = next_free_page.free.next_free_page_number;
        ssize_t pw = pwrite(fd, &header_page, PAGE_SIZE, 0);

        if(pw == -1){ // pwrite() is failed
            Error file_write_error("file write error", "pwrite() failed");
            file_write_error.printError(__FILE__, __func__, __LINE__);
        }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
            Error file_write_error("file write error", "pwrite() end of file");
            file_write_error.printError(__FILE__, __func__, __LINE__);
        }

        int fs = fsync(fd);

        if(fs == -1){ // fsync() failed
            Error file_sync_error("file sync error", "fsync() failed");
            file_sync_error.printError(__FILE__, __func__, __LINE__);
        }
        return next_fpn;
    }else{
        pagenum_t cur_number_of_pages = header_page.header.number_of_pages;
        page_t free_page;
        free_page.free.next_free_page_number = 0;
        ssize_t pw = pwrite(fd, &free_page, PAGE_SIZE, cur_number_of_pages * PAGE_SIZE);
        if(pw == -1){ // pwrite() is failed
            Error file_write_error("file write error", "pwrite() failed");
            file_write_error.printError(__FILE__, __func__, __LINE__);
        }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
            Error file_write_error("file write error", "pwrite() end of file");
            file_write_error.printError(__FILE__, __func__, __LINE__);
        }

        int fs = fsync(fd);

        if(fs == -1){ // fsync() failed
            Error file_sync_error("file sync error", "fsync() failed");
            file_sync_error.printError(__FILE__, __func__, __LINE__);
        }

        for(int i = cur_number_of_pages + 1; i < cur_number_of_pages * 2; i++){
            page_t free_page;
            free_page.free.next_free_page_number = i - 1;
            ssize_t pw = pwrite(fd, &free_page, PAGE_SIZE, i * PAGE_SIZE);
            if(pw == -1){ // pwrite() is failed
                Error file_write_error("file write error", "pwrite() failed");
                file_write_error.printError(__FILE__, __func__, __LINE__);
            }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
                Error file_write_error("file write error", "pwrite() end of file");
                file_write_error.printError(__FILE__, __func__, __LINE__);
            }

            int fs = fsync(fd);

            if(fs == -1){ // fsync() failed
                Error file_sync_error("file sync error", "fsync() failed");
                file_sync_error.printError(__FILE__, __func__, __LINE__);
            }
        }

        header_page.header.free_page_number = cur_number_of_pages * 2 - 2;
        header_page.header.number_of_pages *= 2;
        pw = pwrite(fd, &header_page, PAGE_SIZE, 0);
        if(pw == -1){ // pwrite() is failed
            Error file_write_error("file write error", "pwrite() failed");
            file_write_error.printError(__FILE__, __func__, __LINE__);
        }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
            Error file_write_error("file write error", "pwrite() end of file");
            file_write_error.printError(__FILE__, __func__, __LINE__);
        }

        return cur_number_of_pages * 2 - 1;
    }
}

// Free an on-disk page to the free page list
void file_free_page(int fd, pagenum_t pagenum){
    page_t header_page;
    ssize_t pr = pread(fd, &header_page, PAGE_SIZE, 0);
    if(pr == -1){ // pread() failed
        Error file_read_error("file read error", "pread() failed");
        file_read_error.printError(__FILE__, __func__, __LINE__);
    }else if(pr != PAGE_SIZE){ // pread() didn't read whole data
        Error file_read_error("file read error", "pread() end of file");
        file_read_error.printError(__FILE__, __func__, __LINE__);
    }

    pagenum_t next_fpn = header_page.header.free_page_number;

    page_t target_page;
    pr = pread(fd, &target_page, PAGE_SIZE, pagenum * PAGE_SIZE);
    if(pr == -1){ // pread() failed
        Error file_read_error("file read error", "pread() failed");
        file_read_error.printError(__FILE__, __func__, __LINE__);
    }else if(pr != PAGE_SIZE){ // pread() didn't read whole data
        Error file_read_error("file read error", "pread() end of file");
        file_read_error.printError(__FILE__, __func__, __LINE__);
    }

    target_page.free.next_free_page_number = next_fpn;

    header_page.header.free_page_number = pagenum;
    ssize_t pw = pwrite(fd, &header_page, PAGE_SIZE, 0);
    if(pw == -1){ // pwrite() is failed
        Error file_write_error("file write error", "pwrite() failed");
        file_write_error.printError(__FILE__, __func__, __LINE__);
    }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
        Error file_write_error("file write error", "pwrite() end of file");
        file_write_error.printError(__FILE__, __func__, __LINE__);
    }

    int fs = fsync(fd);

    if(fs == -1){ // fsync() failed
        Error file_sync_error("file sync error", "fsync() failed");
        file_sync_error.printError(__FILE__, __func__, __LINE__);
    }
}

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int fd, pagenum_t pagenum,page_t* dest){
    ssize_t pr = pread(fd, dest, PAGE_SIZE, pagenum * PAGE_SIZE);
    if(pr == -1){ // pread() failed
        Error file_read_error("file read error", "pread() failed");
        file_read_error.printError(__FILE__, __func__, __LINE__);
    }else if(pr != PAGE_SIZE){ // pread() didn't read whole data
        Error file_read_error("file read error", "pread() end of file");
        file_read_error.printError(__FILE__, __func__, __LINE__);
    }
}

// Write an in-memory page(src) to the on-disk page
void file_write_page(int fd, pagenum_t pagenum,const page_t* src){
    ssize_t pw = pwrite(fd, src, PAGE_SIZE, pagenum * PAGE_SIZE);
    if(pw == -1){ // pwrite() is failed
        Error file_write_error("file write error", "pwrite() failed");
        file_write_error.printError(__FILE__, __func__, __LINE__);
    }else if(pw != PAGE_SIZE){ // pwrite() didn't write whole data
        Error file_write_error("file write error", "pwrite() end of file");
        file_write_error.printError(__FILE__, __func__, __LINE__);
    }
}

// Close the database file
void file_close_database_file(){
    for(int opened : opened_files){
        int cl = close(opened);
        if(cl == -1){
            Error file_close_error("file close error", "close() failed");
            file_close_error.printError(__FILE__, __func__, __LINE__);
        }
    }
    opened_files.clear();
    
}
```

</div> 
</details>

### Variable
 - `opened_files` : A vector storing opened files' fd

### Functions

#### `file_open_databas_file`
 - Open existing database file or create one if it doesn't exist.
 - Parameter(s) : `const char* pathname`
 - Return type : `int`
 
 1. Open the file in `pathname` and store `fd`.
 2. If not exist, create, store `fd` and initialize it. Write the 1 header page and `P_NUMBER - 1` free pages.
 3. Insert the `fd` into `opened_files`. (Invoke `push_back()`)
 4. Return `fd`.

#### `file_alloc_page`
 - Allocate an on-disk page from the free page list
 - Parameter(s) : `int fd`
 - Return type : `pagenum_t`

 1. Read the header page using `fd`.
 2. If the header page's free page number is not 0, then set the header's free page number next free page number and return the header's free page number.
 3. If not, write the free pages so that the file size is twice than before, set the header page, and return new free page number.  

#### `file_free_page`
 - Free an on-disk page to the free page list
 - Parameter(s) : `int fd`, `pagenum_t pagenum`
 - Return type : `void`

 1. Read the header page using `fd`.
 2. Read the allocated page using `fd` and `pagenum`.
 3. Set the allocated page's next free page number to the header page's free page number.
 4. Set the header page's free page number to `pagenum`.

#### `file_read_page`
 - Read an on-disk page into the in-memory page structure(dest)
 - Parameter(s) : `int fd`, `pagenum_t pagenum`, `page_t* dest`
 - Return type : `void`

 1. Find the page using `fd`, `src` and read the data in `dest`.

#### `file_write_page`
 - Write an in-memory page(src) to the on-disk page
 - Parameter(s) : `int fd`, `pagenum_t pagenum`, `const page_t* src`
 - Return type : `void`

 1. Find the page using `fd`, `pagenum` and write the `src` into the page.

#### `file_close_database_file`
 - Close the database file
 - Parameter(s) : None
 - Return type : `void`

 1. Close all files that the `opened_files` element indicate.


---

## file_test&#46;cc

<details>
<summary>Source Code</summary>
<div markdown ='1'>

```cpp
#include <gtest/gtest.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <string>
#include <stdio.h>
#include "file.h"

#define FILE_PATH "./test.db"

    
off_t getFileSize(int fd){
    return lseek(fd, 0, SEEK_END);
}

int getNumOfPages(int fd){
    page_t header_page;
    file_read_page(fd, 0, &header_page);

    return header_page.header.number_of_pages;
}

bool findFreedPageNum(int fd, pagenum_t target){
    page_t header_page;
    file_read_page(fd, 0, &header_page);

    pagenum_t temp = header_page.header.free_page_number;
    while(temp != 0 && temp != target){
        page_t temp_free_page;
        file_read_page(fd, temp, &temp_free_page);
        temp = temp_free_page.free.next_free_page_number;
    }

    if(temp == target) return true;
    else return false;
}

void getRandomPage(char* temp, int size){
    srand(time(0));
    for(int i = 0; i < size; i++){
        int randomInt = rand();
        *(temp + i) = (randomInt % 26) + 97;
    }
}

bool checkSamePage(char* a, char* b, int size){
    for(int i = 0; i < size; i++){
        // std::cout << *(a + i) << " " << *(b + i) << std::endl;

        if(*(a + i) != *(b + i)) return false;
    }
    return true;
}



int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(DiskSpaceManager, FileInitialization){
    std::remove(FILE_PATH);
    int test_fd = file_open_database_file(FILE_PATH);
    EXPECT_EQ(getFileSize(test_fd), INITIAL_DB_FILE_SIZE);
    EXPECT_EQ(getNumOfPages(test_fd), P_NUMBER);
    file_close_database_file();
}

TEST(DiskSpaceManager, PageManagement){
    int test_fd = file_open_database_file(FILE_PATH);
    pagenum_t temp_1, temp_2;

    temp_1 = file_alloc_page(test_fd);
    temp_2 = file_alloc_page(test_fd);

    file_free_page(test_fd, temp_1);

    EXPECT_TRUE(findFreedPageNum(test_fd, temp_1));

    file_close_database_file();
}

TEST(DiskSpaceManager, PageIO){
    int test_fd = file_open_database_file(FILE_PATH);
    int epoch = 30;
    bool check = false;
    for(int i = 0; i < epoch; i++){
        pagenum_t temp = file_alloc_page(test_fd);
        page_t temp_page;

        getRandomPage((char*)&temp_page, PAGE_SIZE);
        file_write_page(test_fd, temp, &temp_page);

        page_t read_page;
        file_read_page(test_fd, temp, &read_page);
        check = checkSamePage((char*)&temp_page, (char*)&read_page, PAGE_SIZE);
        // printf("[ Epoch %2d ] Run checkSamePage : %s\n", i + 1, check ? "true" : "false");
        EXPECT_TRUE(check);
    }
    file_close_database_file();
}


TEST(DiskSpaceManager, ReallocPage){
    int test_fd = file_open_database_file(FILE_PATH);
    // std::vector<pagenum_t> allocatedPages;

    page_t header_page;
    file_read_page(test_fd, 0, &header_page);

    for(int i = 0; i < header_page.header.number_of_pages; i++){
        pagenum_t temp_page_num = file_alloc_page(test_fd);
        // allocatedPages.push_back(temp_page_num);
    }

    file_read_page(test_fd, 0, &header_page);
    EXPECT_EQ(getFileSize(test_fd), header_page.header.number_of_pages * PAGE_SIZE);

    file_close_database_file();
}
```

</div> 
</details>

### Functions

#### `getFileSize`
 - Return the file size using `fd`.
 - Parameter(s) : `int fd`
 - Return type : `off_t`

#### `getNumOfPages`
 - Read the header page and return the number of pages in header page
 - Parameter(s) : `int fd`
 - Return type : `int`

#### `findFreedPageNum`
 - Read the header page and traverse the free page while the `target` found.
 - Parameter(s) : `int fd`, `pagenum_t target`
 - Return type : `bool`

#### `getRandomPage`
 - Make the random string with length `size`, and write it into `temp`.
 - Parameter(s) : `char* temp`, `int size`
 - Return type : `void`

#### `checkSamePage`
 - Compare pages `a` and `b` and return true if they are same.
 - Parameter(s) : `char* a`, `char* b`, `int size`
 - Return type : `bool`
 - You must do type casting `page_t` to `char*`

### Google Tests

#### DiskSpaceManager_FileInitialization
 - Check the file's size when created calling `getFileSize`.
 - Check the number of pages in the header page calling `getNumOfPages`.

#### DiskSpaceManager_PageManagement
 - Allocate two pages and free one page. Find the freed page calling `findFreedPageNum` and check the freed page number exist.

#### DiskSpaceManager_PageIO
 - Allocate `epoch` pages (e.g. 30) and write each page in file. And read it again and compare the data calling `checkSamePage` to check the read and write operation executes currectly.
 - Print the result of each page.

#### DiskSpaceManager_ReallocPage (Extra Test)
 - Allocate a number of free pages (e.g. header page's number of pages) and check the increase of the size of database file calling `getFileSize`.

## dberror&#46;h


<details>
<summary>Source Code</summary>
<div markdown ='1'>

```cpp
#include <stdio.h>
#include <string>

#ifndef __DB_ERROR_H__
#define __DB_ERROR_H__

class Error {
    private:
    std::string error_name = "Error";
    std::string error_message = "None";

    public:
    Error(std::string name, std::string message);

    void printError(std::string filename, std::string funcname, int linenum);
    void printError();
};

// class FileCreateError : public Error{};
// class FileWriteError : public Error{};

#endif  // __DB_ERROR_H__
```

</div>
</details>

### Class Error

#### Constructor

`Error error_name(error_type_name, error_content);`

#### Member Functions
 - [`printError`](#printerror)
## dberror&#46;cc


<details>
<summary>Source Code</summary>
<div markdown ='1'>

```cpp
#include "dberror.h"

Error::Error(std::string name, std::string message){
    this->error_name = name;
    this->error_message = message;
}

void Error::printError(std::string filename, std::string funcname, int linenum){
    printf("ERROR : %s, %s\n[%s:%d]\n", this->error_name.c_str(), this->error_message.c_str(), filename.c_str(), linenum);
    perror(funcname.c_str());
    exit(0);
}

void Error::printError(){
    printf("ERROR : %s, %s\n",this->error_name.c_str(), this->error_message.c_str());
    exit(0);
}
```

</div>
</details>

### Member Functions

#### `printError`
 - Parameter(s) : None - method overloading
 - Parameter(s) : `std::string filename`, `std::string funcname`, `int linenum`
 - Return type : `void`
 - `filename` : `__FILE__`
 - `funcname` : `__func__`
 - `linenum` : `__LINE__`
