#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "bmm.h"



#ifdef DEBUG
#define LOG(fmt, args...)                            \
        fprintf(stdout,"\n%s:%s:%d: "fmt, __FILE__, __FUNCTION__, __LINE__, args)

#else
#define LOG(fmt, args...)
#endif


typedef struct block{
  size_t size;
  struct block *next;
}block_t;


typedef struct mpool {
  void *base;
  size_t total_mem, free_mem;
  block_t *head;
}mpool_t;


typedef struct {
  uint8_t used;
  size_t size;
} mem_header_t;

void block_coalesce(block_t *prev, block_t *free_node);
void block_insert(mpool_t *pool, block_t* free_node);


block_t *find_fit(mpool_t *pool, size_t size) {
  if (pool->head == NULL)
    return NULL;

  block_t *tmp = pool->head, *prev = NULL;  
  while(tmp != NULL) {

    if (tmp->size >= size + sizeof(mem_header_t))
    {
      // remove node from list
      if (prev == NULL) { // node will be head
        pool->head = tmp->next;
      } else {
        prev->next = tmp->next;
      }
      return tmp;
    }
    prev = tmp;
    tmp = tmp->next;
  }
  return NULL;
}

/* Insert freed block back on to the free list*/
void block_insert(mpool_t *pool, block_t* free_node) {
  block_t *node = pool->head;
  block_t *tmp = NULL;
  block_t *prev = NULL;
  
  while (node != NULL) {
    if (node + node->size == free_node) { // prev node
      tmp = node->next;
      node->next = free_node;
      free_node->next = tmp;
      block_coalesce(node, free_node);
    }
    else if (node == free_node + free_node->size) { // next node
      prev->next = free_node;
      free_node->next = node;
      block_coalesce(free_node, prev);
    }
    prev = node;
    node = node->next;
  }
}


/* block coalesce merges adjacent memory blocks to reduce fragmentation*/
void block_coalesce(block_t *node, block_t *next_node) {
  // nodes must be adjacent
  if (node + node->size != next_node) {
    return;
  }
  node->size += next_node->size;
  node->next = next_node->next;
  block_coalesce(node, node->next);
}


err_t _bmm_init(mpool_t **pool, size_t size) {
  if (size == 0)
    return FAILURE;

  *pool = malloc(sizeof(struct mpool));
  if (pool == NULL)
    return FAILURE;
  
  (*pool)->base = malloc(size);
  if ((*pool)->base == NULL)
    return FAILURE;
  (*pool)->total_mem = size;
  (*pool)->free_mem = size;
  (*pool)->head = NULL;
  return SUCCESS;
}

/**
 * Allocates memory
 * @param pool The memory pool to allocate from
 * @param size The size of memory to allocate
 * @param out The pointer to the allocated memory
 */
err_t _bmm_malloc(mpool_t **pool, size_t size, void **out) {  
  out = (void **)out;
  size_t offset = sizeof(mem_header_t);
  mem_header_t *hd;
  block_t *tmp = find_fit(*pool, size);

  if (size >= (*pool)->total_mem && size >= (*pool)->free_mem)
    return FAILURE;

  if ((*pool)->head == tmp){ // first fit is head
    hd = (mem_header_t*)(*pool)->base;
    (*pool)->head = (*pool)->base + offset + size;
    (*pool)->head->size = (*pool)->total_mem - (offset + size);
    (*pool)->head->next = NULL;
  } else if (tmp != NULL)
    hd = (mem_header_t*)tmp;
  else // failed to acquire enough memory for this alloc
    return FAILURE;

  hd->size = size;
  hd->used = 1;
  *out = hd + offset;  
  (*pool)->free_mem -= size + offset;
  return SUCCESS;
}

err_t _bmm_free(mpool_t *pool, void *ptr) {
  block_t *free_node = NULL;  
  mem_header_t* hd = NULL;
  
  if (ptr == NULL)
    return SUCCESS;

  // find header
  hd = (mem_header_t*)ptr - sizeof(mem_header_t);
  hd->used = 0;
  free_node = (block_t*)hd;
  free_node->size = hd->size;
  pool->free_mem += free_node->size;
  free_node->next = NULL;
  block_insert(pool, free_node);
  return SUCCESS;
}

err_t _bmm_deinit(mpool_t *pool) {
  pool->total_mem = 0;
  pool->free_mem = 0;
  pool->head = NULL;
  free(pool->base);
  free(pool);
  return SUCCESS;
}


/* Exposed API*/

static mpool_t *pool = NULL;

/**
 * Initialize memory pool with given size 
 * @param size The size of the whole memory pool
 */
err_t bmm_init(size_t size) {
  LOG("Initializing a pool of size %lu\n", size);
  return _bmm_init(&pool, size);
}

/**
 * Allocates memory
 * @param size The size of memory to allocate
 * @param out The pointer to the allocated memory
 */
err_t bmm_malloc(size_t size, void **out) {  
  return _bmm_malloc(&pool, size, out);
}

/**
 * Frees allocated memory
 * @param ptr Previously allocated pointer
 */
err_t bmm_free(void *ptr) {
  return _bmm_free(pool, ptr);
}

err_t bmm_deinit(void) {
  return _bmm_deinit(pool);
}

