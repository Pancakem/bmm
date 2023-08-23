#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bmm.h"


#define MB_2 (2 * 1024 * 1024) // 2MB
#define MB_1024 (1024 * 1024 * 1024) // 1024MB
#define KB_5 (1024 * 5) // 5KB

int main(void) {
  clock_t begin, end;  
  err_t err;
  size_t num_allocs = 0, num_deallocs = 0;
  double total_alloc_time = 0, total_dealloc_time = 0;
  size_t num_failed = 0;
  size_t test_size = 0;


  puts("Testing 512MB allocation");
  err = bmm_init(MB_1024 / 2);
  if (err != SUCCESS)
    puts("512MB allocation fails");

  err = bmm_deinit();
 
  puts("Starting test 2MB block with fixed 5KB allocations\n");

  err = bmm_init(MB_2);
  if (err == FAILURE) {
    fprintf(stderr, "Could not get reserve for 2MB");
    return 1;
  }  

  // possible test size
  test_size = MB_2 / KB_5;
  
  for (size_t i = 0; i < test_size; ++i) {
    char *out = NULL;
    begin = clock();
    err = bmm_malloc(5 * 1024, (void **)&out);
    end = clock();
    if (err == SUCCESS) {
      *out = 5; // making sure we get valid memory
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
  printf("\nEnd 2MB block test\n\n\n");


  puts("Start 1024MB block with fixed 2MB allocations test\n");
  num_allocs = 0;
  num_deallocs = 0;
  total_alloc_time = 0, total_dealloc_time = 0;
  num_failed = 0; 
  
  err = bmm_init(MB_1024);
  if (err == FAILURE) {
    fprintf(stderr, "Could not get reserve for 2MB\n");
    return 1;
  }

  test_size = MB_1024 / MB_2;
  
  for (size_t i = 0; i < test_size; ++i) {
    char *out = NULL;
    begin = clock();
    err = bmm_malloc(MB_2, (void **)&out);
    end = clock();
    if (err == SUCCESS) {
      
      *out = 5;
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
  printf("\nEnd 1024MB block with fixed 2MB allocations test\n");

  puts("\nStart 1024MB block with variable sized allocations test\n");

  num_allocs = 0;
  num_deallocs = 0;
  total_alloc_time = 0, total_dealloc_time = 0;
  num_failed = 0;
  
  err = bmm_init(MB_1024);
  if (err == FAILURE) {
    fprintf(stderr, "Could not get reserve for 1024MB\n");
    return 1;
  }

  #define TESTS 10000
  char *pointers[TESTS] = {0};
  srand(time(NULL));
  size_t i;  
  for (i = 0; i < TESTS / 5; ++i) {
    size_t alloc_size = rand()/((RAND_MAX + 1u)/MB_1024);
    char *out = NULL;
    begin = clock();
    err = bmm_malloc(alloc_size, (void **)&out);
    end = clock();
    if (err == SUCCESS) {      
      *out = 5;
      total_alloc_time += (double)(end - begin) / CLOCKS_PER_SEC;
      // num_allocs++;
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
  printf("\nEnd 1024MB block with variable sized allocations test\n");
  
  
  return 0;
}
