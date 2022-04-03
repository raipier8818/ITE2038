# Lock Table

## Enviroment

- OS : Ubuntu 20.04.3 LTS
- Compiler Version: g++ 9.3.0

## Configuration

- [lock_table.cc](#lock_table.cc)

----

## lock_table.cc

### Design of Lock Table

- Lock table is the tool that handles with the multi-users' accesses.
- So, all users access the database through threads.
- If other user already uses the same key, then new user to access that key must wait until the front user ends the process. 
- When user wants to access the key, then it calls `lock_acquire()` and calls `lock_release()` if it ends.
- To process multi-thread, I use the `pthread.h`.

### Structs

#### `hash_table_entry`

- `int64_t table_id` : Table ID of database (for identification).
- `int64_t record_id` : Record ID of database (for identification).
- `lock_t* head` : Point first lock struct.
- `lock_t* tail` : Point last lock struct.

#### `lock_t`

- `lock_t *prev` : Point front lock struct.
- `lock_t *next` : Point next lock struct.
- `hash_table_entry* sentinel_pointer` : Point the hash table entry that it belongs to.
- `pthread_cond_t cond` : Certain condition for wake up when `lock_release()` called.

#### `hash`

- `bool operator() const` : Hash function.

### Functions

| Return Type | Function Name | Parameter | Description |
|:----|:----|:----|:----|
| `int` | `init_lock_table` | null | Initialize the lock table.  |
| `lock_t*` | `lock_acquire` | `int64_t`, `int64_t` | Allocate and append a new lock struct to the lock list of the record having the key. |
| `int` | `lock_release` | `lock_t*` | Remove the lock struct from the lock list and wake up successor if exist. |