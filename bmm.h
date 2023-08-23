#ifndef MEM_POOL_H
#define MEM_POOL_H

#include <stddef.h>

/**
 * Error type
 */
typedef enum  {
  SUCCESS,
  FAILURE
}err_t;


/**
 * Initialize memory pool with given size 
 * @param size The size of the whole memory pool
 */
err_t bmm_init(size_t size);

/**
 * Allocates memory
 * @param size The size of memory to allocate
 * @param out The pointer to the allocated memory
 */
err_t bmm_malloc(size_t size, void **out);

/**
 * Frees a single block of allocated memory
 * @param ptr Previously allocated pointer
 */
err_t bmm_free(void *ptr);

/**
 * Destroy memory pool
 */
err_t bmm_deinit(void);

#endif
