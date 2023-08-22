#ifndef MEM_POOL_H
#define MEM_POOL_H

#include <stddef.h>


typedef enum  {
  SUCCESS,
  FAILURE
}err_t;

err_t bmm_init(size_t size);
err_t bmm_malloc(size_t size, void **out);
err_t bmm_free(void *ptr);
err_t bmm_deinit(void);


#endif
