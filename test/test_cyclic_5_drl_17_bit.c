#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../src/gb.c"

int main(
        void
        )
{
    int32_t i, j, k;
    len_t *hcm;

    int32_t round = 0;

    const int32_t lens[]  = {5, 5, 5, 5, 2}; 
    const int32_t cfs[]   = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1};
    const int32_t exps[]  = {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};

    const int32_t nr_vars           = 5;
    const int32_t nr_gens           = 5;
    const int32_t ht_size           = 12;
    const int32_t field_char        = 101;
    const int32_t mon_order         = 0;
    const int32_t nr_threads        = 1;
    const int32_t la_option         = 1;
    const int32_t max_nr_pairs      = 0;
    const int32_t reset_hash_table  = 0;

    int32_t failure = 0;

    int32_t **basis = (int32_t **)malloc(sizeof(int32_t *));
    int64_t len     = f4_julia(
            basis, lens, cfs, exps, field_char, mon_order, nr_vars,
            nr_gens, ht_size, nr_threads, max_nr_pairs, reset_hash_table,
            la_option);

    if (len != 1473) {
        failure = 1;
        free(*basis);
        free(basis);
        basis = NULL;
        return failure;
    }
    int32_t val[1473]  = {20, 30, 36, 66, 78, 72, 102, 78, 90, 102, 90, 24, 84, 66, 60, 36, 90, 90, 90, 90, 78, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 2, 0, 0, 0, 1, 0, 1, 0, 1, 0, 100, 0, 0, 1, 1, 0, 2, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 2, 1, 0, 1, 2, 0, 0, 100, 0, 1, 1, 1, 0, 1, 0, 0, 2, 1, 0, 100, 0, 0, 2, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 2, 1, 100, 0, 1, 0, 0, 2, 99, 0, 0, 1, 0, 2, 1, 0, 0, 0, 1, 2, 100, 0, 0, 0, 0, 3, 1, 0, 1, 1, 2, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 2, 1, 1, 100, 0, 1, 0, 2, 1, 1, 0, 0, 1, 2, 1, 100, 0, 0, 0, 3, 1, 100, 0, 1, 1, 0, 2, 100, 0, 1, 0, 1, 2, 99, 0, 0, 0, 2, 2, 1, 0, 1, 0, 0, 3, 1, 0, 0, 1, 0, 3, 100, 0, 0, 0, 1, 3, 1, 0, 0, 0, 0, 4, 1, 0, 1, 1, 1, 2, 51, 0, 0, 2, 1, 2, 1, 0, 0, 1, 2, 2, 50, 0, 1, 1, 0, 3, 100, 0, 1, 0, 1, 3, 50, 0, 0, 1, 1, 3, 50, 0, 0, 0, 2, 3, 51, 0, 1, 0, 0, 4, 51, 0, 0, 1, 0, 4, 100, 0, 0, 0, 1, 4, 51, 0, 0, 0, 0, 5, 50, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 4, 30, 0, 0, 2, 0, 5, 61, 0, 1, 0, 1, 5, 100, 0, 0, 1, 1, 5, 12, 0, 0, 0, 2, 5, 60, 0, 1, 0, 0, 6, 90, 0, 0, 1, 0, 6, 91, 0, 0, 0, 1, 6, 60, 0, 0, 0, 0, 7, 99, 0, 1, 1, 0, 0, 69, 0, 0, 2, 0, 0, 42, 0, 1, 0, 1, 0, 92, 0, 0, 0, 2, 0, 41, 0, 1, 0, 0, 1, 8, 0, 0, 1, 0, 1, 10, 0, 0, 0, 1, 1, 43, 0, 0, 0, 0, 2, 1, 0, 0, 3, 0, 0, 1, 0, 1, 1, 1, 0, 99, 0, 1, 0, 2, 0, 100, 0, 0, 1, 2, 0, 100, 0, 0, 0, 3, 0, 3, 0, 0, 2, 0, 1, 99, 0, 1, 0, 1, 1, 99, 0, 0, 1, 1, 1, 98, 0, 0, 0, 2, 1, 3, 0, 1, 0, 0, 2, 3, 0, 0, 1, 0, 2, 99, 0, 0, 0, 1, 2, 2, 0, 0, 0, 0, 3, 1, 0, 0, 0, 2, 6, 28, 0, 1, 1, 1, 0, 14, 0, 0, 2, 1, 0, 80, 0, 1, 0, 2, 0, 89, 0, 0, 1, 2, 0, 89, 0, 0, 0, 3, 0, 90, 0, 1, 1, 0, 1, 3, 0, 0, 2, 0, 1, 9, 0, 1, 0, 1, 1, 17, 0, 0, 1, 1, 1, 71, 0, 0, 0, 2, 1, 96, 0, 1, 0, 0, 2, 29, 0, 0, 1, 0, 2, 73, 0, 0, 0, 1, 2, 18, 0, 0, 0, 0, 3, 1, 0, 1, 0, 3, 0, 3, 0, 0, 1, 3, 0, 1, 0, 0, 0, 4, 0, 3, 0, 1, 1, 1, 1, 1, 0, 0, 2, 1, 1, 99, 0, 1, 0, 2, 1, 3, 0, 0, 1, 2, 1, 3, 0, 0, 0, 3, 1, 97, 0, 1, 1, 0, 2, 99, 0, 0, 2, 0, 2, 99, 0, 1, 0, 1, 2, 94, 0, 0, 1, 1, 2, 1, 0, 0, 0, 2, 2, 4, 0, 1, 0, 0, 3, 100, 0, 0, 1, 0, 3, 97, 0, 0, 0, 1, 3, 2, 0, 0, 0, 0, 4, 1, 0, 0, 2, 2, 0, 5, 0, 0, 1, 3, 0, 1, 0, 0, 0, 4, 0, 5, 0, 1, 1, 1, 1, 1, 0, 0, 2, 1, 1, 97, 0, 1, 0, 2, 1, 4, 0, 0, 1, 2, 1, 4, 0, 0, 0, 3, 1, 95, 0, 1, 1, 0, 2, 98, 0, 0, 2, 0, 2, 99, 0, 1, 0, 1, 2, 90, 0, 0, 1, 1, 2, 7, 0, 1, 0, 0, 3, 95, 0, 0, 0, 1, 3, 4, 0, 0, 0, 0, 4, 1, 0, 1, 0, 0, 5, 100, 0, 0, 1, 0, 5, 100, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 2, 3, 68, 0, 0, 0, 3, 3, 66, 0, 1, 1, 0, 4, 66, 0, 0, 2, 0, 4, 34, 0, 1, 0, 1, 4, 32, 0, 0, 1, 1, 4, 2, 0, 0, 0, 2, 4, 86, 0, 0, 1, 0, 5, 20, 0, 0, 0, 1, 5, 94, 0, 0, 0, 0, 6, 1, 0, 1, 0, 0, 0, 13, 0, 0, 1, 0, 0, 81, 0, 0, 0, 1, 0, 42, 0, 0, 0, 0, 1, 1, 0, 1, 0, 2, 3, 34, 0, 0, 0, 3, 3, 34, 0, 1, 1, 0, 4, 34, 0, 0, 2, 0, 4, 67, 0, 1, 0, 1, 4, 68, 0, 0, 1, 1, 4, 33, 0, 0, 1, 0, 5, 67, 0, 0, 0, 0, 6, 100, 0, 1, 0, 0, 0, 68, 0, 0, 1, 0, 0, 100, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 5, 51, 0, 0, 0, 2, 5, 4, 0, 0, 1, 0, 6, 51, 0, 0, 0, 1, 6, 52, 0, 0, 0, 0, 7, 100, 0, 0, 1, 1, 0, 50, 0, 0, 0, 2, 0, 97, 0, 0, 1, 0, 1, 50, 0, 0, 0, 1, 1, 49, 0, 0, 0, 0, 2, 1, 0, 0, 2, 0, 5, 3, 0, 0, 1, 0, 6, 1, 0, 0, 0, 0, 7, 100, 0, 0, 2, 0, 0, 98, 0, 0, 1, 0, 1, 100, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 8, 42, 0, 1, 1, 1, 0, 21, 0, 0, 2, 1, 0, 37, 0, 1, 0, 2, 0, 42, 0, 0, 1, 2, 0, 46, 0, 0, 0, 3, 0, 25, 0, 1, 1, 0, 1, 46, 0, 0, 2, 0, 1, 13, 0, 1, 0, 1, 1, 71, 0, 0, 1, 1, 1, 80, 0, 0, 0, 2, 1, 85, 0, 1, 0, 0, 2, 21, 0, 0, 1, 0, 2, 59, 0, 0, 0, 1, 2, 17, 0, 0, 0, 0, 3, 1, 0, 0, 0, 1, 7, 92, 0, 1, 1, 1, 0, 46, 0, 0, 2, 1, 0, 52, 0, 1, 0, 2, 0, 60, 0, 0, 1, 2, 0, 39, 0, 0, 0, 3, 0, 29, 0, 1, 1, 0, 1, 75, 0, 0, 2, 0, 1, 67, 0, 1, 0, 1, 1, 100, 0, 0, 1, 1, 1, 19, 0, 0, 0, 2, 1, 63, 0, 1, 0, 0, 2, 82, 0, 0, 1, 0, 2, 8, 0, 0, 0, 1, 2, 75, 0, 0, 0, 0, 3, 1, 0, 0, 1, 0, 7, 85, 0, 1, 1, 1, 0, 93, 0, 0, 2, 1, 0, 63, 0, 1, 0, 2, 0, 85, 0, 0, 1, 2, 0, 21, 0, 0, 0, 3, 0, 29, 0, 1, 1, 0, 1, 21, 0, 0, 2, 0, 1, 96, 0, 1, 0, 1, 1, 50, 0, 0, 1, 1, 1, 8, 0, 0, 0, 2, 1, 30, 0, 1, 0, 0, 2, 92, 0, 0, 1, 0, 2, 16, 0, 0, 0, 1, 2, 17, 0, 0, 0, 0, 3, 1, 0, 0, 0, 4, 0, 14, 0, 1, 1, 1, 1, 6, 0, 0, 2, 1, 1, 74, 0, 1, 0, 2, 1, 2, 0, 0, 1, 2, 1, 86, 0, 0, 0, 3, 1, 100, 0, 1, 1, 0, 2, 7, 0, 0, 2, 0, 2, 91, 0, 1, 0, 1, 2, 92, 0, 0, 1, 1, 2, 68, 0, 0, 0, 2, 2, 24, 0, 1, 0, 0, 3, 33, 0, 0, 1, 0, 3, 87, 0, 0, 0, 1, 3, 22, 0, 0, 0, 0, 4, 1, 0, 0, 1, 3, 0, 99, 0, 1, 1, 1, 1, 100, 0, 0, 2, 1, 1, 5, 0, 1, 0, 2, 1, 4, 0, 0, 0, 3, 1, 100, 0, 1, 1, 0, 2, 99, 0, 0, 2, 0, 2, 2, 0, 1, 0, 1, 2, 7, 0, 0, 0, 2, 2, 97, 0, 1, 0, 0, 3, 94, 0, 0, 1, 0, 3, 2, 0, 0, 0, 1, 3, 97, 0, 0, 0, 0, 4};

    for (i = 0; i < len; ++i) {
        if (val[i] != (*basis)[i]) {
            printf("difference at position %d: %d -- %d\n", i, val[i], (*basis)[i]);
            failure = 1;
            break;
        }
    }

    free(*basis);
    free(basis);
    basis = NULL;

    return failure;
}
