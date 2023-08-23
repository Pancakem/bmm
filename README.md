# First Fit Allocator

A simple block memory manager that uses the first fit algorithm to allocate memory to caller modules.
The allocator simply searches a list of free blocks and returns
the first block that is big enough to hold the requested memory.

## Design

The memory manager relies on the already existing memory management system provided by
the C standard library, `malloc` and `free` functions to request memory from the operating system.
After memory is acquired from the OS it needs to be kept track of.
That means everytime a caller module requests for memory the allocator needs to bookkeep, for example
it would need to know where memory given to a caller module starts from and how big it is and
when it is returned (freed) we would like to have enough information to know where to place the
returned memory. To bookkeep, the memory manager holds a `singly linked list` of the unused memory
blocks. This singly linked list can be defined as:
```
struct ll {
    size_t size;
    struct ll *next;
};
```
`|head|--free block--|--------in use memory--------|--free block--|-----in use memory-----|`

The beauty of this is we can use the the unused memory to hold this list, so there is no
memory overhead of maintaining this list as it uses no additional memory. The memory manager only
keeps track of the head of the unused linked list of the chain of unused blocks.
This simple bookkeeping algorithm requires that each unused block holds the its size.

We only add new blocks to the unused linked list during allocation. Another data structure employed
in the memory manager is a `header`,
```c
struct header {
    size_t size;
};
```
it contains the `size` of the allocated memory.

`|header|---allocated memory---|header|------allocated memory-----|block|--allocated memory--|`

The allocated memory address returned is calculated using the an offset that is
`sizeof(struct header)`. The memory header data structure is used by the memory manager when
freeing memory to reconstruct the free block. It brings on a small overhead as we have to add
size of allocated memory to accomodate it.


The implementation does not expose this memory manager


## API
The memory manager exposes the following API:

The error type, `err_t`, that all the exposed functions of the API return is defined as:
```c
typedef enum {
    SUCCESS,
    FAILURE
} err_t;
```

### Creating the memory pool
```c
/**
 * Initialize memory pool with given size
 * @param size The size of the whole memory pool
 */
err_t bmm_init(size_t size);
```

### Destroying the memory pool
```c
/**
 * Destroy memory pool
 */
err_t bmm_deinit(void);

```

### Malloc
```c
/**
 * Allocates memory
 * @param size The size of memory to allocate
 * @param out The pointer to the allocated memory
 */
err_t bmm_malloc(size_t size, void **out);
```

### Free
```c
/**
 * Frees a single block of allocated memory
 * @param ptr Previously allocated pointer
 */
err_t bmm_free(void *ptr);
```


## Usage
The memory manager implementation is in the `bmm.h` and `bmm.c`.

A test-bed is provided in `test.c`. It can simply be run by:
`make test`
