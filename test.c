#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bmm.h"

#define MB_2 (2 * 1024 * 1024)       // 2MB
#define MB_1024 (1024 * 1024 * 1024) // 1024MB
#define KB_5 (1024 * 5)              // 5KB

struct test_statistics {
  size_t num_allocs, num_deallocs;
  size_t num_failed;
  size_t min_alloc_size, max_alloc_size;
  size_t avg_alloc_size;
  double total_alloc_time, total_dealloc_time;
};

void print_test_statistics(struct test_statistics *ts) {
  printf("\tNumber of allocations: %25lu\n", ts->num_allocs);
  printf("\tAverage allocation time: %23.8f\n",
         (ts->total_alloc_time / (double)ts->num_allocs));
  printf("\tNumber of de-allocations: %22lu\n", ts->num_deallocs);
  printf("\tAverage de-allocation time: %20.8f\n",
         (ts->total_dealloc_time / (double)ts->num_deallocs));
  printf("\tNumber of failed allocations: %18lu\n", ts->num_failed);
  printf("\tMinimum allocation size: %18lu\n", ts->min_alloc_size);
  printf("\tMaximum allocation size: %18lu\n", ts->max_alloc_size);
  printf("\tAverage allocation size: %18lu\n", ts->avg_alloc_size);
}

int test_fixed_sized_allocs(struct test_statistics *ts, size_t block_size,
                            size_t alloc_size) {
  clock_t begin, end;
  err_t err;
  size_t test_size = 0;

  // fixed size test
  ts->min_alloc_size = ts->max_alloc_size = ts->avg_alloc_size = alloc_size;

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
      ts->total_alloc_time += (double)(end - begin) / CLOCKS_PER_SEC;
      ts->num_allocs++;
    } else {
      ts->num_failed++;
      continue;
    }

    begin = clock();
    err = bmm_free(out);
    end = clock();
    if (err == SUCCESS) {
      ts->total_dealloc_time += (double)(end - begin) / CLOCKS_PER_SEC;
      ts->num_deallocs++;
    }
  }

  bmm_deinit();
  return 0;
}

int test_variable_sized_allocs(struct test_statistics *ts, size_t block_size) {
  clock_t begin, end;
  err_t err;

  err = bmm_init(block_size);
  if (err == FAILURE) {
    fprintf(stderr, "Could not get reserve for 1024MB\n");
    return 1;
  }

#define TESTS 10000
  char *pointers[TESTS] = {0};
  srand(time(NULL));
  size_t i;
  size_t alloc_size = rand() / ((RAND_MAX + 1u) / MB_1024);
  ts->max_alloc_size = ts->min_alloc_size = alloc_size;
  for (i = 0; i < TESTS / 5; ++i) {
    char *out = NULL;
    begin = clock();
    err = bmm_malloc(alloc_size, (void **)&out);
    end = clock();
    if (err == SUCCESS) {
      ts->avg_alloc_size += alloc_size;
      if (alloc_size > ts->max_alloc_size)
        ts->max_alloc_size = alloc_size;
      else if (alloc_size < ts->min_alloc_size)
        ts->min_alloc_size = alloc_size;
      *out = 5;
      assert(*out == 5);
      ts->total_alloc_time += (double)(end - begin) / CLOCKS_PER_SEC;
      pointers[ts->num_allocs++] = out;
    } else
      ts->num_failed++;

    alloc_size = rand() / ((RAND_MAX + 1u) / MB_1024);
  }

  for (i = 0; i < ts->num_allocs; ++i) {
    begin = clock();
    err = bmm_free(pointers[i]);
    end = clock();
    if (err == SUCCESS) {
      ts->total_dealloc_time += (double)(end - begin) / CLOCKS_PER_SEC;
      ts->num_deallocs++;
    }
  }

  bmm_deinit();
  return 0;
}

int main(void) {
  struct test_statistics ts = {0};
  printf("\nStart 2MB block with fixed 5KB allocations test\n");
  test_fixed_sized_allocs(&ts, MB_2, KB_5);
  print_test_statistics(&ts);
  printf("\nEnd 2MB block with fixed 5KB allocations test\n");

  printf("\nStart 1024MB block with fixed 2MB allocations test\n");
  test_fixed_sized_allocs(&ts, MB_1024, MB_2);
  print_test_statistics(&ts);
  printf("\nEnd 1024MB block with fixed 2MB allocations test\n");

  printf("\nStart 1024MB block with variable sized allocations test\n");
  test_variable_sized_allocs(&ts, MB_1024);
  print_test_statistics(&ts);
  printf("\nEnd 1024MB block with variable sized allocations test\n");
  return 0;
}
