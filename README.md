# First Fit Allocator

A simple block memory manager that uses the first fit algorithm to allocate memory to caller modules.
The allocator simply searches a list of free blocks and returns
the first block that is big enough to hold the requested memory.


## API

The A
```c

```

### Creating the memory pool
`err_t

### Malloc
`err_t bmm_malloc(size_t size, void **out);`

### Free
`err_t bmm_free()




## Usage
