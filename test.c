#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "bmm.h"

#define MB_2 (2 * 1024 * 1024) // 2MB
#define MB_1024 (1024 * 1024 * 1024) // 1024MB
#define KB_5 (1024 * 5) // 5KB


int test_fixed_sized_allocs(size_t block_size, size_t alloc_size) {
  clock_t begin, end;  
  err_t err;
  size_t num_allocs = 0, num_deallocs = 0;
  double total_alloc_time = 0, total_dealloc_time = 0;
  size_t num_failed = 0;
  size_t test_size = 0; 
  
  err = bmm_init(block_size);
  if (err == FAILURE) {
    fprintf(stderr, "Could not get reserve for %luMB\n", block_size);
    fprintf(stderr, "TEST FAILED");
    return 1;
  }

  test_size = block_size / alloc_size;
  for (size_t i = 0; i < test_size; ++i) {
    char *out = NULL;
    begin = clock();
    err = bmm_malloc(alloc_size, (void **)&out);
    end = clock();
    if (err == SUCCESS) {
      *out = 5;
      assert(*out == 5);
      total_alloc_time += (double)(end - begin) / CLOCKS_PER_SEC;
      num_allocs++;
    }else{
      num_failed++;
      continue;
    }
      
    begin = clock();
    err = bmm_free(out);
    end = clock();
    if (err == SUCCESS) {     
      total_dealloc_time += (double)(end - begin) / CLOCKS_PER_SEC;
      num_deallocs++;
    }    
  }

  bmm_deinit();
  printf("\tNumber of allocations: %25lu\n", num_allocs);  
  printf("\tAverage allocation time: %23.8f\n", (total_alloc_time / (double)num_allocs));
  printf("\tNumber of de-allocations: %22lu\n", num_deallocs);
  printf("\tAverage de-allocation time: %20.8f\n", (total_dealloc_time / (double)num_deallocs));
  printf("\tNumber of failed allocations: %18lu\n", num_failed);
  printf("\tMinimum allocation size: %18lu\n", alloc_size);
  printf("\tMaximum allocation size: %18lu\n", alloc_size);
  printf("\tAverage allocation size: %18lu\n", alloc_size);
  return 0;
}


int test_variable_sized_allocs(size_t block_size) {
  clock_t begin, end;  
  err_t err;
  size_t num_allocs = 0, num_deallocs = 0;
  double total_alloc_time = 0, total_dealloc_time = 0;
  size_t num_failed = 0;

  size_t max_alloc_size = 0, min_alloc_size = 0, avg_alloc_size = 0;
  
  err = bmm_init(block_size);
  if (err == FAILURE) {
    fprintf(stderr, "Could not get reserve for 1024MB\n");
    return 1;
  }

  #define TESTS 10000
  char *pointers[TESTS] = {0};
  srand(time(NULL));
  size_t i;
  size_t alloc_size = rand()/((RAND_MAX + 1u)/MB_1024);
  max_alloc_size = min_alloc_size = alloc_size;
  for (i = 0; i < TESTS / 5; ++i) {    
    char *out = NULL;
    alloc_size = rand()/((RAND_MAX + 1u)/MB_1024);    
    begin = clock();
    err = bmm_malloc(alloc_size, (void **)&out);
    end = clock();
    if (err == SUCCESS) {      
      avg_alloc_size += alloc_size;
      if (alloc_size > max_alloc_size)
        max_alloc_size = alloc_size;
      else if (alloc_size < min_alloc_size)
        min_alloc_size = alloc_size;
      *out = 5;
      assert(*out == 5);
      total_alloc_time += (double)(end - begin) / CLOCKS_PER_SEC;      
      pointers[num_allocs++] = out;
    }else
      num_failed++; 
  }

  for(i = 0; i < num_allocs; ++i) {
    begin = clock();
    err = bmm_free(pointers[i]);
    end = clock();
    if (err == SUCCESS) {     
      total_dealloc_time += (double)(end - begin) / CLOCKS_PER_SEC;
      num_deallocs++;
    }      
      
  }
  
  bmm_deinit();
  printf("\tNumber of allocations: %25lu\n", num_allocs);  
  printf("\tAverage allocation time: %23.8f\n", (total_alloc_time / (double)num_allocs));
  printf("\tNumber of de-allocations: %22lu\n", num_deallocs);
  printf("\tAverage de-allocation time: %20.8f\n", (total_dealloc_time / (double)num_deallocs));
  printf("\tNumber of failed allocations: %18lu\n", num_failed);
  printf("\tMinimum allocation size: %18lu\n", min_alloc_size);
  printf("\tMaximum allocation size: %18lu\n", max_alloc_size);
  printf("\tAverage allocation size: %18lu\n", avg_alloc_size / num_allocs);
  
  return 0;
}

int main(void) {
  printf("\nStart 2MB block with fixed 5KB allocations test\n");
  test_fixed_sized_allocs(MB_2, KB_5);
  printf("\nEnd 2MB block with fixed 5KB allocations test\n");

  printf("\nStart 1024MB block with fixed 2MB allocations test\n");
  test_fixed_sized_allocs(MB_1024, MB_2);
  printf("\nEnd 1024MB block with fixed 2MB allocations test\n");

  printf("\nStart 1024MB block with variable sized allocations test\n");
  test_variable_sized_allocs(MB_1024);
  printf("\nEnd 1024MB block with variable sized allocations test\n");
  return 0;
}
