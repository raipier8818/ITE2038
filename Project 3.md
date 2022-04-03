# Buffer Manager

## Enviroment

- OS : Ubuntu 20.04.3 LTS
- Compiler Version: g++ 9.3.0

## Configuration

- [buffer.cc](#buffer.cc)

- [page.h](#page.h)

- [Changes](#changes)

----

## buffer.cc

### Design of Buffer Manager

- Header page and Internal/Free page are in the same buffer.
- If the page in the buffer is dirty, then write it to the file.
- For maintaining the form in `Index Manager`, the APIs' call are similar to `Disk Space Manager`.
- But, because the file extension is under the `buffer manager`, `file_alloc_page` and `file_free_page` are deprecated.

- If there is no free page when `buffer_alloc_page` called, then make the new free page in the buffer like the `file_alloc_page` doing.
- If `buffer_free_page` called, then free the page in buffer to file like `file_free_page` doing.
- `find_index_from_page_buffer` is the function that find the index of page in buffer, so I design it like linear search but it takes bidirectional.
- If `buffer_read_page` called, you must call `unpin_page` to unpin when you don't need to reference the page.

#### Variables

- `page_buffer` : The buffer contained the page data.
- `page_buffer_control_block` : The control block contained the information of page.

- `number_of_pages` : The number of pages in buffer.
- `entry_point` : The start index of the buffer lastly referenced or loaded.

#### Functions

- API functions (called in upper layer)

| Return Type | Function Name | Parameter | Description |
|:----|:----|:----|:----|
| `int` | `init_buffer` | null | Initialize the buffer and global variables. |
| `uint64_t` | `buffer_alloc_page` | `int64_t` | Allocate the new free page (in the buffer). |
| `void` | `buffer_free_page` | `int64_t`, `uint64_t` | Free the allocated page (in the buffer). |
| `void` | `buffer_read_page` | `int64_t`, `uint64_t`, `char*` | Read the page in the buffer to upper layer and pin. |
| `void` | `buffer_write_page` | `int64_t`, `uint64_t`, `const char*` | Write the page in upper layer to the buffer and check `is_dirty`. |
| `void` | `unpin_page` | `int64_t`, `uint64_t` | Unpin the page if the page is pinned. |
| `int` | `shutdown_buffer` | null | Flush and clear the buffer and global variables. |

- Util functions

| Return Type | Function Name | Parameter | Description |
|:----|:----|:----|:----|
| `int` | `find_index_from_page_buffer` | `int64_t`, `uint64_t` | Find the index of page in buffer. |
| `int` | `read_new_page` | `int64_t`, `uint64_t` | Read the page from file to buffer and return 1 if succeed. |
| `void` | `flush_buffer` | null | Flush all buffer to file if dirty. |

## page.h

- The functions in the namespace `HEADER_PAGE`, `FREE_PAGE`, `INTERNAL_PAGE`, `LEAF_PAGE` are in this header file now.
- So, including this header file is essential if you access in the page data.

## Changes

- `Page.h` is new.
- `file_read_page` in `file.cc` can allocate page if the page number is bigger than number of pages in header page.
- `file_write_page` in `file.cc` can allocate page if the page number is bigger than number of pages in header page.

## TODO

- Debug mode (using `printf`)