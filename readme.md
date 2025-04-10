# ITE2038 Database System 

- 2021.09 ~ 2021.12

## Projects

- Disk Space Manager : This is a Disk Space Manager that handles fixed-size pages on a database file. It supports creating/opening files, allocating/freeing pages, and reading/writing pages between disk and memory. Functionality is verified using GoogleTest.

- Index Manager : This is a B+ Tree-based index manager system. It defines a page structure for different page types (header, internal, leaf, free), provides utility functions to manipulate page content using memory operations, and supports B+ Tree operations such as insertion, deletion, and search. Each operation handles splitting or merging pages as needed to maintain B+ Tree properties.

- Buffer Manager : This is a Buffer Manager for a database system. It caches pages in memory, tracks dirty pages for write-back, and replaces direct disk access (from the Disk Space Manager) with in-memory operations. Pages are pinned while in use, and unpinned when done. If no free page exists, it allocates more; if flushed or shut down, dirty pages are written to disk.

- Lock Table : This Lock Table manages concurrent access to records in a multi-threaded environment. It ensures mutual exclusion using a lock list per record (identified by table and record ID). Threads acquire locks via `lock_acquire()` and release them with `lock_release()`. Each lock uses condition variables for blocking and waking threads.

- Concurrency Control : This concurrency control implementation (Milestone 1) introduces a lock table and transaction system to ensure serializability in multi-threaded access. Each transaction acquires locks using lock_acquire() and releases them with lock_release(), while transaction lifecycle is handled via trx_begin, trx_commit, and (in future) trx_abort. Deadlock detection is planned but not yet implemented. db_find and db_update now use locks to protect concurrent access. Milestone 2 was not completed due to a persistent issue in db_update.
