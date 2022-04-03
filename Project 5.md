# Concurrency Control - Milestone 1

## Enviroment

- OS : Ubuntu 20.04.3 LTS
- Compiler Version: g++ 9.3.0

## Configuration

- [lock_table.cc](#lock_table.cc)
- [trx.cc](#trx.cc)
- [newbpt.cc](#newbpt.cc)

----

## lock_table.cc

### Design of Lock Table

- Using `pthread.h`, the lock table manager can handle multiple thread.
- So, it must make the multiple transaction operate like serializable.
- When the operation such as `find` and `update` called, it must call `lock_acquire` to acquire the permission to access or modify value.
- If the conflict occurs, it must wait when the previous operation released.

### Functions

#### `lock_acquire`

- parameter : `int64_t table_id`, `pagenum_t page_id`, `int64_t key`, `int trx_id`, `int lock_mode`
- return : `lock_t*`

1. If the `hash_table_entry` matching `table_id` and `page_id` doesn't exist, then create it.
2. Make a new `lock_t`.
3. If there is no `lock_t` in the `hash_table_entry`, then put it and return the new `lock_t`'s pointer.
4. Else, find the condition to acquire new `lock_t`.
   1. If there is a `lock_t` with same `trx_id`.
      1. If new `lock_t` is shared or found `lock_t` is exclusive, then return that `lock_t`'s pointer.
      2. Else, save that address at `same_trx_lock`.
   2. Else, save that address at `other_trx_lock`.
      1. If `lock_mode` is exclusive, then save that address at `x_other_trx_lock`.
5. If `same_trx_lock` exists and `other_trx_lock` do not, then change the `lock_mode` to exclusive.
6. Else, see the last `lock_t`'s `lock_mode`.
   1. If it is waiting, then new `lock_t` is waiting
   2. Else, see the new `lock_t`'s `lock_mode`.
      1. If `lock_mode` is shared, then find the `lock_t` having other `trx_id` and `lock_mode` is exclusive. Then if exist, acquire new `lock_t`.
      2. Else, wait.
7. Add new `lock_t` to the `lock_table_entry` and `trx_table_entry`.
8. Check deadlock(maybe it is implemented in `trx_add`).
9. If `lock_t`'s `is_acquired` is `false`, then wait. 

#### `lock_release`

- parameter : `lock_t* lock_obj`
- return : `int`

1. Delete the target `lock_t` in the `lock_table_entry`.
2. If, target `lock_t`'s next `lock_t` is not acquired and target `lock_t` is first, then call `pthread_cond_signal`.
3. If, next `lock_t` is shared lock, then find and send signal to all shared lock until find exclusive lock.


## trx.cc

### Design of Trx

- Connect `lock_t` in the `lock_table_entry`, and one `trx_table_entry` handles one transaction.
- When transaction begins, then call `trx_begin` to get `trx_id`.
- If transaction is committed, then call `trx_commit`.
- When new `lock_t` added to the `trx_table_entry`, call `trx_add` and check deadlock.
- If find deadlock, then call `trx_abort`. (Not implemented)

### `trx_commit`

- parameter : `int trx_id`
- return : `int`

1. Check the `trx_id` if valid. If not valid, then kill the program.
2. Call `lock_release` for every `lock_t` in the `trx_table_entry`.
3. Delete `trx_table_entry` in `trx_table`
4. Return `trx_id`

### `trx_add`

- parameter : `int trx_id`, `lock_t* lock_ptr`
- return : `lock_t*`

1. Check the `trx_id` is valid.
2. Put the `lock_t` in the tail of the `trx_table_entry`
3. Check deadlock. (Not implemented)
4. If deadlock, then call `trx_abort` and return `nullptr`.

### `is_deadlock` (Not implemented)

- parameter : `lock_t* lock_ptr`
- return : `bool`

1. If `lock_t` is acquired, return false.
2. Traverse `lock_table_entry`.
3. Find the `lock_t`'s `trx_table_entry` and why it is waiting.
4. If it is waiting same trx, then abort.

### `trx_abort` (Not implemented)

- Before update value, the original value will be saved in log.
- If abort, undo all process to change original value.

## newbpt.cc

### `db_find`

- parameter : `int64_t table_id`, `int64_t key`, `char* ret_val`, `uint16_t* val_size`, `int trx_id`
- return : `int`

1. Find the leaf page that has key.
2. If it finds key, then call `lock_acquire`
3. Copy the value of key then return. 

### `db_update`

- parameter : `int64_t table_id`, `int64_t key`, `char* values`, `uint16_t new_val_size`, `uint16_t* old_val_size`, `int trx_id`
- return : `int`

1. Find the leaf page that has key.
2. If it finds key, then call `lock_acquire`
3. Update the value of key then return. 

# Concurrency Control - Milestone 2

- I found fatal error when I call `db_update`, but I couldn't fix it. (Literally I spent all time to find it...) So I can not implement any part of milestone2.

## Implicit locking

## Locking compression