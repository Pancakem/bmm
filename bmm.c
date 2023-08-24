#include "bmm.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG
#define LOG(msg) fprintf(stdout, "\e[1;32m\n[DEBUG] " msg " \e[0m\n")
#define LOGF(fmt, args...)                                                     \
  fprintf(stdout, "\e[1;32m\n[DEBUG] " fmt " \e[0m\n", args)
#else
#define LOG(fmt)
#define LOGF(fmt, args...)
#endif

#define OFFSET_SZ sizeof(mem_header_t)

typedef struct block {
  size_t size;
  struct block *next;
} block_t;

typedef struct mpool {
  void *base;
  size_t total_mem, free_mem;
  block_t *head;
} mpool_t;

typedef struct {
  size_t size;
} mem_header_t;

block_t *find_fit(mpool_t *pool, size_t size);
void block_coalesce(block_t *prev, block_t *free_node);
void block_insert(mpool_t *pool, block_t *free_node);

/**
 * Find the first block that fits the size request
 * The block returned has a size that accomodates
 * requested size and the sizeof of the memory header
 */
block_t *find_fit(mpool_t *pool, size_t size) {
  if (pool->head == NULL)
    return NULL;

  block_t *tmp = pool->head, *prev = NULL;
  while (tmp != NULL) {

    if (tmp->size >= size + sizeof(mem_header_t)) {
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

/**
 * Insert freed block back on to the free list
 * It also calls the coalescing function that
 * will merge the freed block if any adjacent is
 * also freed
 */
void block_insert(mpool_t *pool, block_t *free_node) {
  block_t *node = pool->head;
  block_t *tmp = NULL;
  block_t *prev = node;

  while (node != NULL) {
    if (node + node->size == free_node) { // prev node
      tmp = node->next;
      node->next = free_node;
      free_node->next = tmp;
      block_coalesce(node, free_node);
    } else if (node == free_node + free_node->size) { // next node
      prev->next = free_node;
      free_node->next = node;
      block_coalesce(free_node, prev);
    }
    prev = node;
    node = node->next;
  }
}

/**
 * Merges adjacent memory blocks to reduce fragmentation
 */
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

err_t _bmm_malloc(mpool_t **pool, size_t size, void **out) {
  out = (void **)out;
  mem_header_t *hd;
  block_t *tmp = find_fit(*pool, size);

  LOGF("Allocating memory of size %lu", size);

  if (size >= (*pool)->total_mem && size >= (*pool)->free_mem)
    return FAILURE;

  if ((*pool)->head == tmp) { // first fit is head
    hd = (mem_header_t *)(*pool)->base;
    (*pool)->head = (*pool)->base + OFFSET_SZ + size;
    (*pool)->head->size = (*pool)->total_mem - (OFFSET_SZ + size);
    (*pool)->head->next = NULL;
  } else if (tmp != NULL)
    hd = (mem_header_t *)tmp;
  else // failed to acquire enough memory for this alloc
    return FAILURE;

  hd->size = size;
  *out = hd + OFFSET_SZ;
  (*pool)->free_mem -= size + OFFSET_SZ;
  return SUCCESS;
}

err_t _bmm_free(mpool_t *pool, void *ptr) {
  block_t *free_node = NULL;
  mem_header_t *hd = NULL;
  if (ptr == NULL)
    return SUCCESS;

  // find header
  hd = (mem_header_t *)ptr - OFFSET_SZ;
  free_node = (block_t *)hd;
  free_node->size = hd->size;
  LOGF("Freeing memory of size %lu", free_node->size);
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

static mpool_t *pool = NULL;

/* Exposed API*/

/**
 * Initialize memory pool with given size
 * @param size The size of the whole memory pool
 */
err_t bmm_init(size_t size) { return _bmm_init(&pool, size); }

/**
 * Allocates memory
 * @param size The size of memory to allocate
 * @param out The pointer to the allocated memory
 */
err_t bmm_malloc(size_t size, void **out) {
  return _bmm_malloc(&pool, size, out);
}

/**
 * Frees a single block of allocated memory
 * @param ptr Previously allocated pointer
 */
err_t bmm_free(void *ptr) { return _bmm_free(pool, ptr); }

/**
 * Destroy memory pool
 */
err_t bmm_deinit(void) { return _bmm_deinit(pool); }
