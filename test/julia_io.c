#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../src/data.h"

int main(
    void
    )
{
  printf("[TEST] Julia IO\n");

  const int32_t lens[]  = {2,2,2}; 
  const int32_t cfs[]   = {1, 1, 1, 3, 5, 3};
  const int32_t exps[]  = {1, 0, 0, 1, 3, 0, 0, 1, 1, 1, 1, 0};

  const nr_vars = 2;
  const nr_gens = 3;
  const ht_size = 12;

  f4_julia(lens, cfs, exps, nr_vars, nr_gens, ht_size);
  return 0;
}