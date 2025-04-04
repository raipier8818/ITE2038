# Files and Index Manager

## Enviroment

- OS : Ubuntu 20.04.3 LTS
- Compiler Version: g++ 9.3.0

## Configuration

- [Page Structure](#page-structure)
- [B+ Tree](#b+-tree)
- [Index Manager](#index-manager)

----

## Page Structure

- The variables constituting all pages are divided into memory addresses. So I define the function to set and get the variables in Disk Space Manager using namespace. These getter functions use `page_t *` to access the page. And setters use `page_t *` and `target value` to set the variable.
- If you use these function, you must fit the type of page to the name of namespace. (e.g. If you call the `HEADER_PAGE`'s function in leaf page, it can occur fatal error.)

### Header Page


<details>
<summary>Code</summary>
<div markdown="1">

```cpp
// file.cc

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
```

</div>
</details>

#### `namespace HEADER_PAGE`

| Return Type | Function Name | Parameter | Description |
|:----|:----|:----|:----|
| `pagenum_t` | `get_free_page_number` | `page_t*` | return the header page's free page number |
| `void` | `set_free_page_number` | `page_t*`, `pagenum_t` | set the header page's free page number |
| `pagenum_t` | `get_number_of_pages` | `page_t*` | return the header page's number of pages |
| `void` | `set_number_of_pages` | `page_t*`, `pagenum_t` | set the header page's number of pages |
| `pagenum_t` | `get_root_page_number` | `page_t*` | return the header page's root page number |
| `void` | `set_free_page_number` | `page_t*`, `pagenum_t` | set the header page's root page number |

### Free Page


<details>
<summary>Code</summary>
<div markdown="1">

```cpp
// file.cc

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
```

</div>
</details>

#### `namespace FREE_PAGE`

| Return Type | Function Name | Parameter | Description |
|:----|:----|:----|:----|
| `pagenum_t` | `get_next_free_page_number` | `page_t*` | return the free page's next free page number |
| `void` | `set_next_free_page_number` | `page_t*`, `pagenum_t` | set the free page's next free page number |

### Leaf Page

<details>
<summary>Code</summary>
<div markdown="1">

```cpp
// file.cc

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
```

</div>
</details>

#### `namespace PAGE_HEADER` in `namespace LEAF_PAGE`

| Return Type | Function Name | Parameter | Description |
|:----|:----|:----|:----|
| `pagenum_t` | `get_parent_page_number` | `page_t*` | return the free page's parent page number |
| `void` | `set_parent_page_number` | `page_t*`, `pagenum_t` | set the free page's parent page number |
| `int` | `get_is_leaf` | `page_t*` | return 1 if the page is leaf page |
| `void` | `set_is_leaf` | `page_t*`, `int` | set the free page's is_leaf variable |
| `int` | `get_number_of_keys` | `page_t*` | return the number of keys of free page |
| `void` | `set_number_of_keys` | `page_t*`, `int` | set the number of keys of free page |
| `pagenum_t` | `get_amount_of_free_space` | `page_t*` | get the amount of free space of free page |
| `void` | `set_amount_of_free_space` | `page_t*`, `pagenum_t` | set the amount of free space of free page |
| `pagenum_t` | `get_right_sibling_page_number` | `page_t*` | get the right sibling page number of free page |
| `void` | `set_right_sibling_page_number` | `page_t*`, `pagenum_t` | set the right sibling page number of free page |

#### `namespace LEAF_PAGE`

| Return Type | Function Name | Parameter | Description |
|:----|:----|:----|:----|
| `slot` | `get_slot` | `page_t*`, `int` | return the slot<sub>i</sub> of the free page |
| `void` | `set_slot` | `page_t*`, `slot`, `int` | set the slot<sub>i</sub> of the free page |
| `char*` | `get_value` | `page_t*`, `slot` | return the value corresponding to the slot |
| `void` | `set_value` | `page_t*`, `slot`, `char*` | set the value corresponding to the slot |

### Internal Page

<details>
<summary>Code</summary>
<div markdown="1">

```cpp
// file.cc

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

```

</div>
</details>

#### `namespace PAGE_HEADER` in `namespace INTERNAL_PAGE`

| Return Type | Function Name | Parameter | Description |
|:----|:----|:----|:----|
| `pagenum_t` | `get_parent_page_number` | `page_t*` | return the internal page's parent page number |
| `void` | `set_parent_page_number` | `page_t*`, `pagenum_t` | set the internal page's parent page number |
| `int` | `get_is_leaf` | `page_t*` | return 0 if the page is internal page |
| `void` | `set_is_leaf` | `page_t*`, `int` | set the internal page's is_leaf variable |
| `int` | `get_number_of_keys` | `page_t*` | return the number of keys of internal page |
| `void` | `set_number_of_keys` | `page_t*`, `int` | set the number of keys of internal page |

#### `namespace INTERNAL_PAGE`

| Return Type | Function Name | Parameter | Description |
|:----|:----|:----|:----|
| `slot` | `get_key` | `page_t*`, `int` | return the key<sub>i</sub> of the internal page |
| `void` | `set_key` | `page_t*`, `pagenum_t`, `int` | set the key<sub>i</sub> of the internal page |
| `pagenum_t` | `get_left_page_number` | `page_t*`, `int` | return i-th left page number |
| `void` | `set_left_page_number` | `page_t*`, `pagenum_t`, `int` | set i-th left page number |
| `pagenum_t` | `get_right_page_number` | `page_t*`, `int` | return i-th right page number |
| `void` | `set_right_page_number` | `page_t*`, `pagenum_t`, `int` | set i-th right page number |

----

## B+ Tree

### Insertion (parameter : key)

1. Find the `key` in B+tree. If exist, return.
2. Find the leaf node that `key` will be inserted.
3. Insert `key` in leaf node.
   1. If the leaf node has enough space to insert, then just insert.
   2. If the leaf node has no space, then split the leaf node.
4. Check internal node
   1. If the child node splitted and internal node has no space, then split the internal node.
   2. Else, insert the key created by splitting in the internal node.
5. Repeat 4 until the root node. 

### Deletion (parameter : key)

1. Find the `key` in B+tree. If not exist, return.
2. Find the leaf node that `key` will be deleted.
3. Delete `key` in leaf node.
   1. Find the neighbor node of the leaf node.
   2. If the neighbor node has no space to move all keys in the leaf node, then redistribute.
   3. Else, merge the leaf node into the neighbor node.
4. Check internal node
   1. If the child node was merged, then delete the merged leaf node's key.
   2. Else, return.
5. Repeat 4 until the root node.

----

## Index Manager

- One B+ tree is in one file. All nodes of B+ tree is consist of page.
- Header page is unique page and it contains some information of table.
- Root page is the root of B+ tree.
- Internal page is the page that contains `n` key and `n+1` page number. `n` is smaller or equal 64.
- Leaf page is the page that contains slot and value, the maximum number of slot(value) is depends of the amount of free space.
- When merge or split, copy all key in the page to temporary array because the offset of the value in leaf page can have random value.

### Insertion Call Path

![IndexManager_Insert](uploads/e4179ebe63710530a64916e6a9c7366a/IndexManager_Insert.png)

### Deletion Call Path

![IndexManager_Delete](uploads/5dadf755ff31cdeb1f15cd2ae6cce2bb/IndexManager_Delete.png)

### Split in insert

1. If the leaf page splits, then allocate temporary array to store all slots and values in the leaf page.
2. Make a new leaf page and divide the temporary array to seperate the slots and values.
3. Set the original leaf page has free space greater than half of page size and insert the slots and values.
4. The rest of slots and values are inserted in the new leaf page.
5. Insert the new child page number and the first slot's key in the parent page.
6. Repeat 5 until the page number is header page's page number.

### Merge in delete

1. If the leaf page has less free space than 2500 (threshold) bytes, then merge or redistribute it.
2. Find the sibling page, and if the free space is enough then merge or redistribute.
3. If merged, allocate temporary array to store all slots and value (leaf page) or key and page number (internal page).
4. Move all slots and value (leaf page) or key and page number (internal page) to sibling page.
5. Delete the original page number in the parent page
6. Free the original page.


<details>
<summary>Code</summary>
<div markdown="1">

```cpp
#include "file.h"
#include "newbpt.h"

#include <string.h>
#include <stdlib.h>

int64_t open_table (char *pathname){
    int64_t fd = file_open_table_file(pathname);
    return fd;
}

void print_page(int64_t table_id, pagenum_t page_number){
    if(page_number == 0) return;
    page_t page;
    file_read_page(table_id, page_number, (char*)&page);

    print_page(table_id, LEAF_PAGE::PAGE_HEADER::get_parent_page_number(&page));


    printf("---< PAGE %ld >---\n", page_number);

    if(INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&page)){
        printf("parent page number : %ld\n", LEAF_PAGE::PAGE_HEADER::get_parent_page_number(&page));
        printf("is leaf : %d\n", LEAF_PAGE::PAGE_HEADER::get_is_leaf(&page));
        printf("number of keys : %d\n", LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&page));
        printf("amount of free space : %ld\n", LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&page));
        printf("right sibling page number : %ld\n", LEAF_PAGE::PAGE_HEADER::get_right_sibling_page_number(&page));

        for(int i = 0; i < LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&page); i++){
            slot temp = LEAF_PAGE::get_slot(&page, i);
            printf("slot : {%ld, %hd, %hd}\n", temp.key, temp.offset, temp.size);
        }

        printf("----------------\n");

    }else{
        printf("parent page number : %ld\n", INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&page));
        printf("is leaf : %d\n", INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&page));
        printf("number of keys : %d\n", INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&page));
        
        int i;
        for(i = 0; i < INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&page); i++){
            printf("left pagenum : %ld\n", INTERNAL_PAGE::get_left_page_number(&page, i));
            printf("key : %ld\n", INTERNAL_PAGE::get_key(&page, i));
        }
        printf("last pagenum : %ld\n", INTERNAL_PAGE::get_left_page_number(&page, i));



        printf("----------------\n");
    }
}


int db_insert(int64_t table_id, int64_t key, char * value, uint16_t val_size){

    // printf("%s start.\n", __func__);
    page_t header_page;

    file_read_page(table_id, 0, (char*)&header_page);


    // check the key existence
    if(db_find(table_id, key, value , &val_size) == 0){

        // printf("%s end.\n", __func__);
        return -1;
    }


    pagenum_t root_page_number = HEADER_PAGE::get_root_page_number(&header_page);


    page_t leaf_page;
    pagenum_t leaf_page_number;

    if(root_page_number == 0){    // Case 1 : no root in b+ tree
        // make root page as leaf page
        leaf_page_number = file_alloc_page(table_id);
        file_read_page(table_id, 0, (char*)&header_page);
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
        file_write_page(table_id, leaf_page_number, (char*)&leaf_page);
        file_write_page(table_id, 0, (char*)&header_page);

        return 0;
    }else{  // Case 2 : root in b+ tree

        // read leaf page
        leaf_page_number = db_find_leaf_page_number(table_id, root_page_number, key);
        file_read_page(table_id, leaf_page_number, (char*)&leaf_page);
    }

    if(LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&leaf_page) >= sizeof(slot) + val_size){
        // printf("%s end.\n", __func__);
        db_insert_into_leaf(table_id, leaf_page_number, key, value, val_size);
        return 0;
    }


    // printf("%s end.\n", __func__);
    db_insert_into_leaf_page_after_splitting(table_id, root_page_number, leaf_page_number, key, value, val_size);
    return 0;
}

// node * insert_into_leaf
int db_insert_into_leaf(int64_t table_id, pagenum_t leaf_page_number, int64_t key, char* value, uint16_t val_size){
    // printf("%s start.\n", __func__);
    
    page_t leaf_page;
    file_read_page(table_id, leaf_page_number, (char*)&leaf_page);
    
    int64_t number_of_keys = LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&leaf_page);
    
    

    int i, j, insertion_point = 0;
    while(insertion_point < number_of_keys && LEAF_PAGE::get_slot(&leaf_page, insertion_point).key < key){
        insertion_point++;
    }
    

    slot* temp_slot = (slot*)malloc(sizeof(slot)*MAX_SLOT_NUMBER);
    char** temp_value = (char**)malloc(sizeof(char*)*MAX_SLOT_NUMBER);

    for(i = 0, j = 0; i < number_of_keys; i++, j++){
        if(j == insertion_point) j++;
        temp_slot[j] = LEAF_PAGE::get_slot(&leaf_page, i);
        temp_value[j] = (char*)malloc(sizeof(char)*temp_slot[j].size);
        temp_value[j] = LEAF_PAGE::get_value(&leaf_page, temp_slot[j]);
    }

    temp_slot[insertion_point].key = key;
    temp_slot[insertion_point].size = val_size;

    temp_value[insertion_point] = (char*)malloc(sizeof(char)*val_size);
    temp_value[insertion_point] = value;

    uint16_t new_offset = PAGE_SIZE;

    for(i = 0; i < number_of_keys + 1; i++){
        temp_slot[i].offset = new_offset - temp_slot[i].size;
        LEAF_PAGE::set_slot(&leaf_page, temp_slot[i], i);
        LEAF_PAGE::set_value(&leaf_page, temp_slot[i], temp_value[i]);

        new_offset -= temp_slot[i].size;
    }
    

    // slot temp_slot;
    // temp_slot.offset = PAGE_SIZE;

    // for(i = number_of_keys; i > insertion_point; i--){
    //     temp_slot = LEAF_PAGE::get_slot(&leaf_page, i - 1);
    //     temp_slot.offset -= val_size;
    //     LEAF_PAGE::set_slot(&leaf_page, temp_slot, i);
    //     LEAF_PAGE::set_value(&leaf_page, temp_slot, value);
    // }

    // temp_slot.key = key;
    // temp_slot.size = val_size;
    // temp_slot.offset -= val_size;

    // LEAF_PAGE::set_slot(&leaf_page, temp_slot, insertion_point);
    // LEAF_PAGE::set_value(&leaf_page, temp_slot, value);


    LEAF_PAGE::PAGE_HEADER::set_number_of_keys(&leaf_page, number_of_keys + 1);
    LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&leaf_page, LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&leaf_page) - sizeof(slot) - val_size);


    file_write_page(table_id, leaf_page_number, (char*)&leaf_page);

    // printf("%s end.\n", __func__);
    return 0;
}

pagenum_t db_insert_into_leaf_page_after_splitting(int64_t table_id, pagenum_t root_page_number, pagenum_t leaf_page_number, int64_t key, char* value, uint16_t val_size){
    // printf("%s start.\n", __func__);

    page_t leaf_page, new_leaf_page;
    file_read_page(table_id, leaf_page_number, (char*)&leaf_page);

    slot* temp_slot = (slot*)malloc((MAX_SLOT_NUMBER + 1) * sizeof(slot));
    char** temp_value = (char**)malloc((MAX_SLOT_NUMBER + 1) * sizeof(char*));
    pagenum_t new_leaf_page_number = file_alloc_page(table_id);
    uint64_t number_of_keys = LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&leaf_page);


    int insertion_index = 0, i, j;
    
    while(insertion_index < number_of_keys &&  LEAF_PAGE::get_slot(&leaf_page, insertion_index).key < key){
        insertion_index++;
    }

    for(i = 0, j = 0; i < number_of_keys; i++, j++){
        if(j == insertion_index) j++;
        temp_slot[j] = LEAF_PAGE::get_slot(&leaf_page, i);
        temp_value[j] = (char*)malloc(temp_slot[j].size);
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
    }

    LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&new_leaf_page, MAX_FREE_SPACE - temp_alloc_space);
    LEAF_PAGE::PAGE_HEADER::set_is_leaf(&new_leaf_page, 1);
    LEAF_PAGE::PAGE_HEADER::set_number_of_keys(&new_leaf_page, temp_number_of_keys);
    LEAF_PAGE::PAGE_HEADER::set_parent_page_number(&new_leaf_page, LEAF_PAGE::PAGE_HEADER::get_parent_page_number(&leaf_page));
    LEAF_PAGE::PAGE_HEADER::set_right_sibling_page_number(&new_leaf_page, LEAF_PAGE::PAGE_HEADER::get_right_sibling_page_number(&leaf_page));
    LEAF_PAGE::PAGE_HEADER::set_right_sibling_page_number(&leaf_page, new_leaf_page_number);



    file_write_page(table_id, leaf_page_number, (char*)&leaf_page);
    file_write_page(table_id, new_leaf_page_number, (char*)&new_leaf_page);

    free(temp_slot);
    free(temp_value);

    // printf("%s end.\n", __func__);
    return db_insert_into_parent(table_id, root_page_number, leaf_page_number, LEAF_PAGE::get_slot(&new_leaf_page, 0).key, new_leaf_page_number);
}



pagenum_t db_insert_into_parent(int64_t table_id, pagenum_t root_page_number, pagenum_t left_page_number, int64_t key, pagenum_t right_page_number){
    // printf("%s start.\n", __func__);
    page_t left_page;
    file_read_page(table_id, left_page_number, (char*)&left_page);

    int64_t parent_page_number = INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&left_page);

    if(parent_page_number == 0){
        // printf("%s end.\n", __func__);
        return db_insert_into_new_root(table_id, left_page_number, key, right_page_number);
    }



    int left_index = get_left_index(table_id, parent_page_number, left_page_number);


    page_t parent_page;
    file_read_page(table_id, parent_page_number, (char*)&parent_page);

    
    if(INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&parent_page) < MAX_KEY_NUMBER){
        // printf("%s end.\n", __func__);
        
        return db_insert_into_internal(table_id, root_page_number, parent_page_number, left_index, key, right_page_number);
    }
    
    // printf("%s end.\n", __func__);
    return insert_into_node_after_splitting(table_id, root_page_number, parent_page_number, left_index, key, right_page_number);
}

pagenum_t insert_into_node_after_splitting(int64_t table_id, pagenum_t root_page_number, pagenum_t parent_page_number, int left_index, int64_t key, pagenum_t right_page_number){
    
    // printf("%s start.\n", __func__);

    int64_t* temp_keys = (int64_t*)malloc((MAX_KEY_NUMBER + 1) * sizeof(int64_t));
    pagenum_t* temp_page = (pagenum_t*)malloc((MAX_KEY_NUMBER + 2) * sizeof(int64_t));

    page_t parent_page;
    file_read_page(table_id, parent_page_number, (char*)&parent_page);
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
    pagenum_t new_parent_page_number = file_alloc_page(table_id);

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
        file_read_page(table_id, INTERNAL_PAGE::get_left_page_number(&new_parent_page, i), (char*)&child_page);
        INTERNAL_PAGE::PAGE_HEADER::set_parent_page_number(&child_page, new_parent_page_number);
        file_write_page(table_id, INTERNAL_PAGE::get_left_page_number(&new_parent_page, i), (char*)&child_page);
    }

    file_write_page(table_id, parent_page_number, (char*)&parent_page);
    file_write_page(table_id,new_parent_page_number, (char*)&new_parent_page);

    free(temp_keys);
    free(temp_page);

    // printf("%s end.\n", __func__);
    return db_insert_into_parent(table_id, root_page_number, parent_page_number, k_prime, new_parent_page_number);
}



pagenum_t db_insert_into_internal(int64_t table_id, pagenum_t root_page_number, pagenum_t parent_page_number, int left_index, int64_t key, pagenum_t right_page_number){
    
    // printf("%s start.\n", __func__);
    page_t parent_page;
    file_read_page(table_id, parent_page_number, (char*)&parent_page);

    uint64_t number_of_keys = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&parent_page);
    int i = number_of_keys;
    for(; i > left_index; i--){
        INTERNAL_PAGE::set_left_page_number(&parent_page, INTERNAL_PAGE::get_left_page_number(&parent_page, i), i + 1);
        INTERNAL_PAGE::set_key(&parent_page, INTERNAL_PAGE::get_key(&parent_page, i - 1), i);
    }

    INTERNAL_PAGE::set_left_page_number(&parent_page, right_page_number, left_index + 1);
    INTERNAL_PAGE::set_key(&parent_page, key, left_index);
    INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&parent_page, number_of_keys + 1);

    file_write_page(table_id, parent_page_number, (char*)&parent_page);
    
    // printf("%s end.\n", __func__);
    return root_page_number;
}


int get_left_index(int64_t table_id, pagenum_t parent_page_number, pagenum_t left_page_number){
    
    // printf("%s start.\n", __func__);
    page_t parent_page;
    file_read_page(table_id, parent_page_number, (char*)&parent_page);

    int left_index = 0;
    while(left_index <= INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&parent_page) && INTERNAL_PAGE::get_left_page_number(&parent_page, left_index) != left_page_number){
        left_index++;
    }
    // printf("%s end.\n", __func__);
    return left_index;
}


pagenum_t db_insert_into_new_root(int64_t table_id, pagenum_t left, int64_t key, pagenum_t right){
    
    // printf("%s start.\n", __func__);
    pagenum_t root_page_number = file_alloc_page(table_id);
    page_t root_page, header_page, left_page, right_page;
    file_read_page(table_id, 0, (char*)&header_page);
    file_read_page(table_id, left, (char*)&left_page);
    file_read_page(table_id, right, (char*)&right_page);

    INTERNAL_PAGE::PAGE_HEADER::set_is_leaf(&root_page, 0);
    INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&root_page, 1);
    INTERNAL_PAGE::PAGE_HEADER::set_parent_page_number(&root_page, 0);


    

    LEAF_PAGE::PAGE_HEADER::set_parent_page_number(&left_page, root_page_number);
    LEAF_PAGE::PAGE_HEADER::set_parent_page_number(&right_page, root_page_number);

    HEADER_PAGE::set_root_page_number(&header_page, root_page_number);

    INTERNAL_PAGE::set_key(&root_page, key, 0);
    INTERNAL_PAGE::set_left_page_number(&root_page, left, 0);
    INTERNAL_PAGE::set_right_page_number(&root_page, right, 0);

    file_write_page(table_id, left, (char*)&left_page);
    file_write_page(table_id, right, (char*)&right_page);
    file_write_page(table_id, root_page_number, (char*)&root_page);
    file_write_page(table_id, 0, (char*)&header_page);

    // printf("%s end.\n", __func__);
    return root_page_number;
}

// node* find_leaf
pagenum_t db_find_leaf_page_number(int64_t table_id, pagenum_t root_page_number, int key){

    // printf("%s start.\n", __func__);

    if(root_page_number == 0){
        // printf("%s end.\n", __func__);
        return 0;
    }

    page_t temp_page;
    file_read_page(table_id, root_page_number, (char*)&temp_page);   
    pagenum_t next_page_number = root_page_number;

    while(!INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&temp_page)){
        int i = 0;
        int number_of_keys = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&temp_page);
        
        while(i < number_of_keys){
            if(key >= INTERNAL_PAGE::get_key(&temp_page, i)) i++;
            else break;
        }

        next_page_number = INTERNAL_PAGE::get_left_page_number(&temp_page, i);
        file_read_page(table_id, next_page_number, (char*)&temp_page);
    }
    // printf("%s end.\n", __func__);
    return next_page_number;
}

int db_find (int64_t table_id, int64_t key, char * ret_val, uint16_t * val_size){

    // printf("%s start.\n", __func__);
    page_t header_page;
    file_read_page(table_id, 0, (char*)&header_page);


    pagenum_t root_page_number = HEADER_PAGE::get_root_page_number(&header_page);



    if(root_page_number == 0){
        // printf("%s end.\n", __func__);
        return -1;
    }

    pagenum_t leaf_page_number = db_find_leaf_page_number(table_id, root_page_number, key);
    page_t leaf_page;

    file_read_page(table_id, leaf_page_number, (char*)&leaf_page);   

    uint64_t number_of_keys = LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&leaf_page);
    
    for(int i = 0; i < number_of_keys; i++){
        slot temp_slot = LEAF_PAGE::get_slot(&leaf_page, i);

        if(temp_slot.key == key){
            *val_size = temp_slot.size;
            memmove(ret_val, LEAF_PAGE::get_value(&leaf_page, temp_slot), temp_slot.size);

            // printf("%s end.\n", __func__);
            return 0;
        }
    }
    
    // printf("%s end.\n", __func__);
    return -1;
}



int db_delete (int64_t table_id, int64_t key){
    // printf("< Delete %d >\n", key);

    // printf("%s start.\n", __func__);

    page_t header_page;
    file_read_page(table_id, 0, (char*)&header_page);

    pagenum_t root_page_number = HEADER_PAGE::get_root_page_number(&header_page);

    char* temp_value = (char*)malloc(MAX_VALUE_SIZE);
    uint16_t temp_value_size;
    int find_key = db_find(table_id, key, temp_value, &temp_value_size);
    pagenum_t leaf_page_number = db_find_leaf_page_number(table_id, root_page_number, key);

    if(find_key == 0 && leaf_page_number != 0){
        root_page_number = db_delete_entry(table_id, root_page_number, leaf_page_number, key, 0);
        free(temp_value);

        file_read_page(table_id, 0, (char*)&header_page);
        HEADER_PAGE::set_root_page_number(&header_page, root_page_number);
        file_write_page(table_id, 0, (char*)&header_page);
        // printf("%s end.\n", __func__);
        return 0;
    }
    // printf("%s end.\n", __func__);
    return -1;
}

pagenum_t db_delete_entry(int64_t table_id, pagenum_t root_page_number, pagenum_t n_page_number, int64_t key, pagenum_t pointer){
    // printf("%s start.\n", __func__);

    n_page_number = db_remove_entry_from_node(table_id, n_page_number, key, pointer);



    if(n_page_number == root_page_number){
        // printf("%s end.\n", __func__);
        return adjust_root(table_id, root_page_number);
    }


    page_t n_page, n_parent_page;
    file_read_page(table_id, n_page_number, (char*)&n_page);


    if(INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&n_page)){ // leaf page
        int threshold = 2500;
        
        if(threshold > LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&n_page)){
            // printf("%s end.\n", __func__);
            return root_page_number;
        }



        int neighbor_index = get_neighbor_index(table_id, n_page_number);
        int k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
        

        file_read_page(table_id, LEAF_PAGE::PAGE_HEADER::get_parent_page_number(&n_page), (char*)&n_parent_page);
        int64_t k_prime = INTERNAL_PAGE::get_key(&n_parent_page, k_prime_index);        
        
        page_t neighbor_page;
        pagenum_t neighbor_page_number = INTERNAL_PAGE::get_left_page_number(&n_parent_page, neighbor_index == -1 ? 1 : neighbor_index);
        file_read_page(table_id, neighbor_page_number, (char*)&neighbor_page);

        if(LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&n_page) >= PAGE_SIZE - 128 - LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&neighbor_page)){
            /* Coalescence */
            // printf("%s end.\n", __func__);
            return coalesce_nodes(table_id, root_page_number, n_page_number, neighbor_page_number, neighbor_index, k_prime);
        
        }else{
            /* Redistribution */
            // printf("%s end.\n", __func__);
            return redistribute_nodes(table_id, root_page_number, n_page_number, neighbor_page_number, neighbor_index, k_prime_index, k_prime);

        }
    }else{ // internal page
        int min_keys = MAX_KEY_NUMBER / 2;

        if(min_keys <= INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_page)){
            // printf("%s end.\n", __func__);
            return root_page_number;
        }


        int neighbor_index = get_neighbor_index(table_id, n_page_number);
        int k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;

        file_read_page(table_id, INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&n_page), (char*)&n_parent_page);
        int64_t k_prime = INTERNAL_PAGE::get_key(&n_parent_page, k_prime_index);

        page_t neighbor_page;
        pagenum_t neighbor_page_number = INTERNAL_PAGE::get_left_page_number(&n_parent_page, neighbor_index == -1 ? 1 : neighbor_index);
        file_read_page(table_id, neighbor_page_number, (char*)&neighbor_page);

        if(INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&neighbor_page) + INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_page) < MAX_KEY_NUMBER){
            /* Coalescence */
            // printf("%s end.\n", __func__);
            return coalesce_nodes(table_id, root_page_number, n_page_number, neighbor_page_number, neighbor_index, k_prime);
        }else{
            /* Redistribution */
            // printf("%s end.\n", __func__);
            return redistribute_nodes(table_id, root_page_number, n_page_number, neighbor_page_number, neighbor_index, k_prime_index, k_prime);
        }
    }
}

pagenum_t coalesce_nodes(int64_t table_id, pagenum_t root_page_number, pagenum_t n_page_number, pagenum_t neighbor_page_number, int neighbor_index, int64_t k_prime){
    // printf("%s start.\n", __func__);

    int i, j, neighbor_insertion_index, n_end;
    page_t neighbor_page, n_page, temp_page;
    pagenum_t temp_page_number;

    if(neighbor_index == -1){
        temp_page_number = n_page_number;
        n_page_number = neighbor_page_number;
        neighbor_page_number = temp_page_number;
    }

    file_read_page(table_id, n_page_number, (char*)&n_page);
    file_read_page(table_id, neighbor_page_number, (char*)&neighbor_page);

    neighbor_insertion_index = INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&neighbor_page);

    if(INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&n_page)){ // leaf page
        uint64_t n_number_of_key = LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&n_page);
        uint64_t neighbor_number_of_key = LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&neighbor_page);
        uint16_t new_offset = PAGE_SIZE;
        uint64_t new_free_space = PAGE_SIZE - 128;

        slot* temp_slots = (slot*)malloc(sizeof(slot) * MAX_SLOT_NUMBER);
        char** temp_value = (char**)malloc(sizeof(char*) * MAX_SLOT_NUMBER);

        for(i = 0; i < neighbor_number_of_key; i++){
            temp_slots[i] = LEAF_PAGE::get_slot(&neighbor_page, i);
            temp_value[i] = (char*)malloc(sizeof(char) * temp_slots[i].size);
            temp_value[i] = LEAF_PAGE::get_value(&neighbor_page, temp_slots[i]);

            temp_slots[i].offset = new_offset - temp_slots[i].size;
            new_offset -= temp_slots[i].size;
        }

        for(j = 0; j < n_number_of_key; j++, i++){
            temp_slots[i] = LEAF_PAGE::get_slot(&n_page, j);
            temp_value[i] = (char*)malloc(sizeof(char) * temp_slots[i].size);
            temp_value[i] = LEAF_PAGE::get_value(&n_page, temp_slots[i]);

            temp_slots[i].offset = new_offset - temp_slots[i].size;
            new_offset -= temp_slots[i].size;
        }

        for(i = 0; i < neighbor_number_of_key + n_number_of_key; i++){
            LEAF_PAGE::set_slot(&neighbor_page, temp_slots[i], i);
            LEAF_PAGE::set_value(&neighbor_page, temp_slots[i], temp_value[i]);

            new_free_space -= (sizeof(slot) + temp_slots[i].size);
        }

        LEAF_PAGE::PAGE_HEADER::set_right_sibling_page_number(&neighbor_page, LEAF_PAGE::PAGE_HEADER::get_right_sibling_page_number(&n_page));
        LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&neighbor_page, new_free_space);
        LEAF_PAGE::PAGE_HEADER::set_number_of_keys(&neighbor_page, n_number_of_key + neighbor_number_of_key);
        file_write_page(table_id, neighbor_page_number, (char*)&neighbor_page);

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
            file_read_page(table_id, INTERNAL_PAGE::get_left_page_number(&neighbor_page, i), (char*)&temp_page);
            INTERNAL_PAGE::PAGE_HEADER::set_parent_page_number(&temp_page, neighbor_page_number);
            file_write_page(table_id, INTERNAL_PAGE::get_left_page_number(&neighbor_page, i), (char*)&temp_page);
        }

        file_write_page(table_id, neighbor_page_number, (char*)&neighbor_page);
    }


    // print_page(table_id, n_page_number);
    // print_page(table_id, neighbor_page_number);

    root_page_number = db_delete_entry(table_id, root_page_number, INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&n_page), k_prime, n_page_number);
    
    file_free_page(table_id, n_page_number);
    // printf("%s end.\n", __func__);
    return root_page_number;
}

pagenum_t redistribute_nodes(int64_t table_id, pagenum_t root_page_number, pagenum_t n_page_number, pagenum_t neighbor_page_number, int neighbor_index, int k_prime_index, int64_t k_prime){
    // printf("%s start.\n", __func__);
    
    int i;
    page_t temp_page, n_page, neighbor_page, n_parent_page;
    pagenum_t parent_page_number;
    file_read_page(table_id, n_page_number, (char*)&n_page);
    file_read_page(table_id, neighbor_page_number, (char*)&neighbor_page);

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
            file_read_page(table_id, INTERNAL_PAGE::get_left_page_number(&n_page, 0), (char*)&temp_page);
            INTERNAL_PAGE::PAGE_HEADER::set_parent_page_number(&temp_page, n_page_number);
            file_write_page(table_id, INTERNAL_PAGE::get_left_page_number(&n_page, 0), (char*)&temp_page);
            INTERNAL_PAGE::set_key(&n_page, k_prime, 0);
            
            parent_page_number = INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&n_page);

            file_read_page(table_id, parent_page_number, (char*)&n_parent_page);
            INTERNAL_PAGE::set_key(&n_parent_page, INTERNAL_PAGE::get_key(&neighbor_page, neighbor_number_of_keys - 1), k_prime_index);

            INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&n_page, n_number_of_keys + 1);
            INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&neighbor_page, neighbor_number_of_keys - 1);

        }else{ // leaf page
            while(LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&n_page) >= 2500){
                slot* temp_slots = (slot*)malloc(sizeof(slot) * MAX_SLOT_NUMBER);
                char** temp_values = (char**)malloc(sizeof(slot) * MAX_SLOT_NUMBER);
                uint64_t new_offset = PAGE_SIZE;
                temp_slots[0] = LEAF_PAGE::get_slot(&neighbor_page, 0);
                temp_values[0] = (char*)malloc(sizeof(char) * temp_slots[0].size);
                temp_values[0] = LEAF_PAGE::get_value(&neighbor_page, temp_slots[0]);

                temp_slots[0].offset = new_offset - temp_slots[0].size;
                new_offset -= temp_slots[0].size;

                int j;
                for(i = 1, j = 0; j < n_number_of_keys; j++, i++){
                    temp_slots[i] = LEAF_PAGE::get_slot(&n_page, j);
                    temp_values[i] = (char*)malloc(sizeof(char) * temp_slots[i].size);
                    temp_values[i] = LEAF_PAGE::get_value(&n_page, temp_slots[i]);

                    temp_slots[i].offset = new_offset - temp_slots[i].size;
                    new_offset -= temp_slots[i].size;
                }

                for(i = 0; i < n_number_of_keys + 1; i++){
                    LEAF_PAGE::set_slot(&n_page, temp_slots[i], i);
                    LEAF_PAGE::set_value(&n_page, temp_slots[i], temp_values[i]);
                }

                LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&n_page, LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&n_page) - sizeof(slot) - temp_slots[0].size);
                LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&neighbor_page, LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&neighbor_page) + sizeof(slot) + temp_slots[0].size);

                parent_page_number = LEAF_PAGE::PAGE_HEADER::get_parent_page_number(&n_page);
                file_read_page(table_id, parent_page_number, (char*)&n_parent_page);
                INTERNAL_PAGE::set_key(&n_parent_page, LEAF_PAGE::get_slot(&n_page, 0).key, k_prime_index);


                INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&n_page, n_number_of_keys + 1);
                INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&neighbor_page, neighbor_number_of_keys - 1);
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
                    temp_values[i] = (char*)malloc(sizeof(char) * temp_slots[i].size);
                    temp_values[i] = LEAF_PAGE::get_value(&n_page, temp_slots[i]);

                    temp_slots[i].offset = new_offset - temp_slots[i].size;
                    new_offset -= temp_slots[i].size;
                }

                temp_slots[i] = LEAF_PAGE::get_slot(&neighbor_page, 0);
                temp_values[i] = (char*)malloc(sizeof(char) * temp_slots[i].size);
                temp_values[i] = LEAF_PAGE::get_value(&neighbor_page, temp_slots[i]);

                temp_slots[i].offset = new_offset - temp_slots[i].size;
                new_offset -= temp_slots[i].size;

                int j;
                for(j = 0; j < n_number_of_keys + 1; j++){
                    LEAF_PAGE::set_slot(&n_page, temp_slots[j], j);
                    LEAF_PAGE::set_value(&n_page, temp_slots[j], temp_values[j]);
                }

                for(j = 0; j < neighbor_number_of_keys - 1; j++){
                    LEAF_PAGE::set_slot(&neighbor_page, LEAF_PAGE::get_slot(&neighbor_page, j + 1), j);
                }

                LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&n_page, LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&n_page) - sizeof(slot) - temp_slots[i].size);
                LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&neighbor_page, LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&neighbor_page) + sizeof(slot) + temp_slots[i].size);

                parent_page_number = LEAF_PAGE::PAGE_HEADER::get_parent_page_number(&n_page);
                file_read_page(table_id, parent_page_number, (char*)&n_parent_page);
                INTERNAL_PAGE::set_key(&n_parent_page, LEAF_PAGE::get_slot(&neighbor_page, 0).key, k_prime_index);

                INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&n_page, n_number_of_keys + 1);
                INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&neighbor_page, neighbor_number_of_keys - 1);
            }
        }else{ // internal page
            INTERNAL_PAGE::set_key(&n_page, k_prime, n_number_of_keys);
            INTERNAL_PAGE::set_left_page_number(&n_page, INTERNAL_PAGE::get_left_page_number(&neighbor_page, 0), n_number_of_keys + 1);

            file_read_page(table_id, INTERNAL_PAGE::get_left_page_number(&n_page, n_number_of_keys + 1), (char*)&temp_page);
            INTERNAL_PAGE::PAGE_HEADER::set_parent_page_number(&temp_page, n_page_number);
            file_write_page(table_id, INTERNAL_PAGE::get_left_page_number(&n_page, n_number_of_keys + 1), (char*)&temp_page);

            parent_page_number = INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&n_page);
            file_read_page(table_id, parent_page_number, (char*)&n_parent_page);
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

    file_write_page(table_id, n_page_number, (char*)&n_page);
    file_write_page(table_id, neighbor_page_number, (char*)&neighbor_page);
    file_write_page(table_id, parent_page_number, (char*)&n_parent_page);

    // printf("%s end.\n", __func__);
    return root_page_number;
}



int get_neighbor_index(int64_t table_id, pagenum_t n_page_number){
    // printf("%s start.\n", __func__);

    int i;
    page_t n_page, n_parent_page;
    file_read_page(table_id, n_page_number, (char*)&n_page);
    file_read_page(table_id, INTERNAL_PAGE::PAGE_HEADER::get_parent_page_number(&n_page), (char*)&n_parent_page);



    for(i = 0; i <= INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_parent_page); i++){
        if(INTERNAL_PAGE::get_left_page_number(&n_parent_page, i) == n_page_number) {
            // printf("%s end.\n", __func__);
            return i - 1;
        }
    }

    printf("\nFatal Error\n");
    exit(1);
}

pagenum_t adjust_root(int64_t table_id, pagenum_t root_page_number){
    // printf("%s start.\n", __func__);


    page_t root_page;
    file_read_page(table_id, root_page_number, (char*)&root_page);

    if(INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&root_page) > 0){
        // printf("%s end.\n", __func__);
        return root_page_number;
    }

    pagenum_t new_root_page_number;
    page_t new_root_page;

    if(!INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&root_page)){
        new_root_page_number = INTERNAL_PAGE::get_left_page_number(&root_page, 0);
        file_read_page(table_id, new_root_page_number, (char*)&new_root_page);

        INTERNAL_PAGE::PAGE_HEADER::set_parent_page_number(&new_root_page, 0);
        
        file_write_page(table_id, new_root_page_number, (char*)&new_root_page);
    }
    else{
        new_root_page_number = 0;
    }

    file_free_page(table_id, root_page_number);
    // printf("%s end.\n", __func__);
    return new_root_page_number;
}

pagenum_t db_remove_entry_from_node(int64_t table_id, pagenum_t n_page_number, int64_t key, pagenum_t pointer){
    // printf("%s start.\n", __func__);

    // if(key == 3220)
    //     print_page(table_id, n_page_number);

    page_t n_page;
    file_read_page(table_id, n_page_number, (char*)&n_page);
    
    if(INTERNAL_PAGE::PAGE_HEADER::get_is_leaf(&n_page)){
        int i = 0;


        while(LEAF_PAGE::get_slot(&n_page, i).key != key){
            i++;
        }

        slot target = LEAF_PAGE::get_slot(&n_page, i);
        int number_of_keys = LEAF_PAGE::PAGE_HEADER::get_number_of_keys(&n_page);

        slot* temp_slots = (slot*)malloc(sizeof(slot)*MAX_SLOT_NUMBER);
        char** temp_values = (char**)malloc(sizeof(char*)*MAX_SLOT_NUMBER);
        uint16_t new_offset = PAGE_SIZE;

        for(int j = 0, k = 0; j < number_of_keys; j++, k++){
            if(i == j) j++;
            temp_slots[k] = LEAF_PAGE::get_slot(&n_page, j);
            temp_values[k] = (char*)malloc(sizeof(char)*temp_slots[k].size);
            temp_values[k] = LEAF_PAGE::get_value(&n_page, temp_slots[k]);

            temp_slots[k].offset = new_offset - temp_slots[k].size;
            new_offset -= temp_slots[k].size;
        }

        for(int j = 0; j < number_of_keys - 1; j++){
            LEAF_PAGE::set_slot(&n_page, temp_slots[j], j);
            LEAF_PAGE::set_value(&n_page, temp_slots[j], temp_values[j]);
        }


        LEAF_PAGE::PAGE_HEADER::set_amount_of_free_space(&n_page, LEAF_PAGE::PAGE_HEADER::get_amount_of_free_space(&n_page) + sizeof(slot) + target.size);
        LEAF_PAGE::PAGE_HEADER::set_number_of_keys(&n_page, number_of_keys - 1);

        file_write_page(table_id, n_page_number, (char*)&n_page);

        // printf("%s end.\n", __func__);
        return n_page_number;
    }else{
        int i = 0;

        while(INTERNAL_PAGE::get_key(&n_page, i) != key){
            i++;
        }
        for(++i; i < INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_page); i++){
            INTERNAL_PAGE::set_key(&n_page, INTERNAL_PAGE::get_key(&n_page, i), i - 1);
        }

        i = 0;
        while(INTERNAL_PAGE::get_left_page_number(&n_page, i) != pointer){
            i++;
        }
        for(++i; i < INTERNAL_PAGE::PAGE_HEADER::get_number_of_keys(&n_page) + 1; i++){
            INTERNAL_PAGE::set_left_page_number(&n_page, INTERNAL_PAGE::get_left_page_number(&n_page, i), i - 1);
        }


        INTERNAL_PAGE::PAGE_HEADER::set_number_of_keys(&n_page, i - 2);
        
        file_write_page(table_id, n_page_number, (char*)&n_page);
        // printf("%s end.\n", __func__);
        return n_page_number;
    }
}


int init_db (){
    // TODO : turn on buffer manager


    return 0;   
}

int shutdown_db(){
    // TODO : turn off buffer manager

    file_close_table_files();
    return 0;
};
```

</div>
</details>

### Functions in Index Manager API

| Return Type | Function Name | Parameter | Description |
|:----|:----|:----|:----|
| `int` | `init_db` | `null` | Initialize the DBMS. Return 0 if success. |
| `int` | `shutdown_db` | `null` | Shutdown the DBMS. Return 0 if success. |
| `int64_t` | `open_table` | `char*` | Open existing data file or create. Return 0 if success. |
| `int` | `db_insert` | `int64_t`, `int64_t`, `char*`, `uint16_t` | Insert input key and value. Return 0 if success. |
| `int` | `db_find` | `int64_t`, `int64_t`, `char*`, `uint16_t*` | Find the value corresponding the key. Return 0 if success. |
| `int` | `db_delete` | `int64_t`, `int64_t` | Delete the value corresponding the key. Return 0 if success. |
