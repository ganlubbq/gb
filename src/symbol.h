/* gb: Gröbner Basis
 * Copyright (C) 2015 Christian Eder <ederc@mathematik.uni-kl.de>
 * This file is part of gb.
 * gbla is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * gbla is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with gbla . If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file symbol.h
 * \brief Implementation of the symbolic pre- and postprocessing in gb.
 *
 * \author Christian Eder <ederc@mathematik.uni-kl.de>
 */
#ifndef GB_SYMBOL_H
#define GB_SYMBOL_H

#include "gb_config.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <omp.h>
#include "types.h"
#include "hash.h"
#include "spair.h"

#ifndef META_DATA_DEBUG
#define META_DATA_DEBUG 0
#endif

#ifndef SYMBOL_DEBUG
#define SYMBOL_DEBUG 0
#endif

/**
 * \brief Symbolic preprocessing searching for all possible reducers of all
 * ocurring monomials. These data are then used to construct the matrices for
 * gbla.
 *
 * \param current pair set ps
 *
 * \param intermediate groebner basis basis
 *
 * \param simplifier list sf
 *
 * \return full information from symbolic preprocessing for generationg corresponding
 * matrices out of polynomial data
 */
spd_t *symbolic_preprocessing(ps_t *ps, const gb_t *basis, const gb_t *sf);

/**
 * \brief Adjusts size of hash list for symbolic preprocessing to new_size.
 *
 * \note It is not only used to enlarge the size, but also for cutting down
 * memory once symbolic preprocessing is done.
 *
 * \param preprocessing hash list hl
 *
 * \param new size of hash list size
 */
static inline void adjust_size_of_preprocessing_hash_list(pre_t *hl, const nelts_t size)
{
  //printf("hl->size %u --> %u\n", hl->size, size);
  hl->hpos  = realloc(hl->hpos, size * sizeof(hash_t));
  hl->size  = size;
}

/**
 * \brief Enters the lower order monomials of the selected spair generators to
 * the preprocessing hash list.
 *
 * \param intermediate groebner basis basis
 *
 * \param selected list of elements to be used in the next round sel
 *
 * \param preprocessing hash list mon
 */
void enter_spairs_to_preprocessing_hash_list(const gb_t *basis, sel_t *sel, pre_t *mon);

/**
 * \brief Enters one monomial h1 to preprocessing hash list.
 *
 * \param monomial hash position h1
 *
 * \param preprocessing hash list mon
 */
static inline void enter_not_multiplied_monomial_to_preprocessing_hash_list(const hash_t h1,
    pre_t *mon)
{
  hash_t pos  = h1;
  // only in this case we have this monomial hash for the first time,
  // otherwise it has already been taken care of
#if SYMBOL_DEBUG
    for (int i=0; i<ht->nv; ++i)
      printf("%u ",ht->exp[h1][i]);
    printf("\n");
    for (int i=0; i<ht->nv; ++i)
      printf("%u ",ht->exp[pos][i]);
    printf("\n");
#endif
  if (ht->idx[pos] == 0) {
    ht->idx[pos]++;
    mon->hpos[mon->load]  = pos;
    mon->load++;
#if SYMBOL_DEBUG
    printf("new mon %u == %u\n", h1,mon->hpos[mon->load]);
#endif
  }
}

/**
 * \brief Enters one monomial (h1*h2) to preprocessing hash list.
 *
 * \param monomial hash position h1
 *
 * \param monomial hash position h2
 *
 * \param preprocessing hash list mon
 */
//void enter_monomial_to_preprocessing_hash_list(const hash_t h1, const hash_t h2, pre_t *mon);
static inline void enter_monomial_to_preprocessing_hash_list(const mpp_t mpp, pre_t *mon,
    mp_cf4_ht_t *ht)
{
  nelts_t i;
  const hash_t h1 = mpp.mul;
  
  for (i=0; i<mpp.nt; ++i) {
    const hash_t h2 = mpp.eh[i];
    hash_t pos = check_in_hash_table_product(h1, h2, ht);
    // only in this case we have this monomial hash for the first time,
    // otherwise it has already been taken care of
#if SYMBOL_DEBUG
    printf("h1 %lu -- h2 %lu -- pos %lu\n", h1, h2, pos);
    for (int i=0; i<ht->nv; ++i)
      printf("%u ",ht->exp[h1][i]);
    printf("\n");
    for (int i=0; i<ht->nv; ++i)
      printf("%u ",ht->exp[h2][i]);
    printf("\n");
    for (int i=0; i<ht->nv; ++i)
      printf("%u ",ht->exp[pos][i]);
    printf("\n");
#endif
    if (ht->idx[pos] == 0) {
      ht->idx[pos]++;
      mon->hpos[mon->load]  = pos;
#if SYMBOL_DEBUG
      printf("hash %lu at position %u\n", h1+h2,pos);
      printf("2 new mon %u + %u == %u\n", h1,h2,mon->hpos[mon->load]);
#endif
      mon->load++;
    }
  }
}

/**
 * \brief Initializes a hash list for symbolic preprocessing.
 *
 * \param size of hash list size
 *
 * \return hash list
 */
static inline pre_t *init_preprocessing_hash_list(const nelts_t size)
{
  // allocate a list for hashes of monomials to be checked in the symbolic
  // preprocessing
  pre_t *mon  = (pre_t *)malloc(sizeof(pre_t));
  mon->hpos   = (hash_t *)malloc(size * sizeof(hash_t));
  mon->size   = size;
  mon->load   = 0;
  mon->nlm    = 0;
  
  return mon;
}


/**
 * \brief Frees hash list for symbolic preprocessing.
 *
 * \param preprocessing hash list hl
 */
static inline void free_preprocessing_hash_list(pre_t **hl_in)
{
  pre_t *hl = *hl_in;
  free(hl->hpos);
  free(hl);
  hl      = NULL;
  *hl_in  = hl;
}


/**
 * \brief Frees data structure for symbolic preprocessing.
 *
 * \param symbolic preprocessing data structure spd
 */
static inline void free_symbolic_preprocessing_data(spd_t **spd_in)
{
  spd_t *spd = *spd_in;
  free_preprocessing_hash_list(&(spd->col));
  free_selection(&(spd->selu));
  free_selection(&(spd->sell));
  free(spd);
  spd     = NULL;
  *spd_in = spd;
}


/**
 * \brief Comparison function of monomials for quicksort. Compares the property
 * of being lead monomial or not.
 *
 * \param value a
 *
 * \param value b
 *
 * \returns negative value if a is non lead and b is lead; 0 if both are lead or
 * both are non lead; a positive value if a is lead and b is non lead
 */
static inline int cmp_symbolic_preprocessing_monomials_by_lead(const void *a,
    const void *b)
{
  hash_t h1 = *((hash_t *)a);
  hash_t h2 = *((hash_t *)b);

  return (ht->idx[h2] - ht->idx[h1]);
}

/**
 * \brief Comparison function of monomials for quicksort. Compares w.r.t. the
 * given monomial order lex.
 *
 * \note This is used in symbolic preprocessing, thus we know already that a =/=
 * b and we do not have to check this again.
 *
 * \param value a
 *
 * \param value b
 *
 * \returns negative value if a is non lead and b is lead; 0 if both are lead or
 * both are non lead; a positive value if a is lead and b is non lead
 */
static inline int cmp_symbolic_preprocessing_monomials_by_lex(const void *a,
    const void *b)
{
  hash_t ha = *((hash_t *)a);
  hash_t hb = *((hash_t *)b);

  // else we have to check lexicographical
#if __GB_HAVE_SSE2
  nvars_t i;
  exp_t expa[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t expb[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t tmp[ht->vl] __attribute__ ((aligned (16)));
  for (i=0; i<ht->nev; ++i) {
    _mm_store_si128((exp_v *)tmp, ht->ev[ha][i]);
    memcpy(expa+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
    _mm_store_si128((exp_v *)tmp, ht->ev[hb][i]);
    memcpy(expb+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
  }
  return memcmp(expb, expa, sizeof(expa));
#else
  return memcmp(ht->exp[hb], ht->exp[ha], sizeof(exp_t) * ht->nv);
#endif
}

/**
 * \brief Comparison function of monomials for quicksort. Compares w.r.t. the
 * given monomial order lex and inverts the order.
 *
 * \note This is used in symbolic preprocessing, thus we know already that a =/=
 * b and we do not have to check this again.
 *
 * \note We invert lex since we have to align the A and C parts of the gbla
 * matrix in order to efficiently compute.
 *
 * \param value a
 *
 * \param value b
 *
 * \returns negative value if a is non lead and b is lead; 0 if both are lead or
 * both are non lead; a positive value if a is lead and b is non lead
 */
static inline int cmp_symbolic_preprocessing_monomials_by_inverse_lex(const void *a,
    const void *b)
{
  hash_t ha = *((hash_t *)a);
  hash_t hb = *((hash_t *)b);

  // else we have to check lexicographical
#if __GB_HAVE_SSE2
  nvars_t i;
  exp_t expa[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t expb[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t tmp[ht->vl] __attribute__ ((aligned (16)));
  for (i=0; i<ht->nev; ++i) {
    _mm_store_si128((exp_v *)tmp, ht->ev[ha][i]);
    memcpy(expa+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
    _mm_store_si128((exp_v *)tmp, ht->ev[hb][i]);
    memcpy(expb+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
  }
  return memcmp(expa, expb, sizeof(expa));
#else
  return memcmp(ht->exp[ha], ht->exp[hb], sizeof(exp_t) * ht->nv);
#endif
}

/**
 * \brief Comparison function of monomials for quicksort. Compares w.r.t. the
 * given monomial order grevlex.
 *
 * \note This is used in symbolic preprocessing, thus we know already that a =/=
 * b and we do not have to check this again.
 *
 * \param value a
 *
 * \param value b
 *
 * \returns negative value if a is non lead and b is lead; 0 if both are lead or
 * both are non lead; a positive value if a is lead and b is non lead
 */
static inline int cmp_symbolic_preprocessing_monomials_by_grevlex(const void *a,
    const void *b)
{
  hash_t ha = *((hash_t *)a);
  hash_t hb = *((hash_t *)b);

  // compare degree first
  if (ht->deg[hb] != ht->deg[ha])
    return ht->deg[hb]-ht->deg[ha];

  // else we have to check reverse lexicographical
  // NOTE: We store the exponents in reverse order in ht->exp and ht->ev
  // => we can use memcmp() here and still get reverse lexicographical ordering
#if __GB_HAVE_SSE2
  nvars_t i;
  exp_t expa[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t expb[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t tmp[ht->vl] __attribute__ ((aligned (16)));
  for (i=0; i<ht->nev; ++i) {
    _mm_store_si128((exp_v *)tmp, ht->ev[ha][i]);
    memcpy(expa+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
    _mm_store_si128((exp_v *)tmp, ht->ev[hb][i]);
    memcpy(expb+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
  }
  return memcmp(expa, expb, sizeof(expa));
#else
  return memcmp(ht->exp[ha], ht->exp[hb], sizeof(exp_t) * ht->nv);
#endif
}

/**
 * \brief Comparison function of monomials for quicksort. Compares w.r.t. the
 * given monomial order grevlex and inverts the order.
 *
 * \note This is used in symbolic preprocessing, thus we know already that a =/=
 * b and we do not have to check this again.
 *
 * \note We invert grevlex since we have to align the A and C parts of the gbla
 * matrix in order to efficiently compute.
 *
 * \param value a
 *
 * \param value b
 *
 * \returns negative value if a is non lead and b is lead; 0 if both are lead or
 * both are non lead; a positive value if a is lead and b is non lead
 */
static inline int cmp_symbolic_preprocessing_monomials_by_inverse_grevlex(const void *a,
    const void *b)
{
  hash_t ha = *((hash_t *)a);
  hash_t hb = *((hash_t *)b);

  // compare degree first
  if (ht->deg[hb] != ht->deg[ha])
    return ht->deg[ha]-ht->deg[hb];

  // else we have to check reverse lexicographical
#if __GB_HAVE_SSE2
  nvars_t i;
  exp_t expa[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t expb[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t tmp[ht->vl] __attribute__ ((aligned (16)));
  for (i=0; i<ht->nev; ++i) {
    _mm_store_si128((exp_v *)tmp, ht->ev[ha][i]);
    memcpy(expa+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
    _mm_store_si128((exp_v *)tmp, ht->ev[hb][i]);
    memcpy(expb+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
  }
  return memcmp(expb, expa, sizeof(expa));
#else
  return memcmp(ht->exp[hb], ht->exp[ha], sizeof(exp_t) * ht->nv);
#endif
}

/**
 * \brief Comparison function of polynomials for quicksort. Compares w.r.t. the
 * given monomial order lex.
 *
 * \param value a
 *
 * \param value b
 *
 * \returns negative value if a is non lead and b is lead; 0 if both are lead or
 * both are non lead; a positive value if a is lead and b is non lead
 */
static inline int cmp_polynomials_by_lex(const void *a,
    const void *b)
{
  poly_t pa = *((poly_t *)a);
  poly_t pb = *((poly_t *)b);

  hash_t ha = pa.eh[0];
  hash_t hb = pb.eh[0];

#if __GB_HAVE_SSE2
  nvars_t i;
  exp_t expa[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t expb[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t tmp[ht->vl] __attribute__ ((aligned (16)));
  for (i=0; i<ht->nev; ++i) {
    _mm_store_si128((exp_v *)tmp, ht->ev[ha][i]);
    memcpy(expa+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
    _mm_store_si128((exp_v *)tmp, ht->ev[hb][i]);
    memcpy(expb+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
  }
  return memcmp(expb, expa, sizeof(expa));
#else
  return memcmp(ht->exp[hb], ht->exp[ha], sizeof(exp_t) * ht->nv);
#endif
}

/**
 * \brief Comparison function of polynomials for quicksort. Compares w.r.t. the
 * given monomial order grevlex.
 *
 * \param value a
 *
 * \param value b
 *
 * \returns negative value if a is non lead and b is lead; 0 if both are lead or
 * both are non lead; a positive value if a is lead and b is non lead
 */
static inline int cmp_polynomials_by_grevlex(const void *a,
    const void *b)
{
  poly_t pa = *((poly_t *)a);
  poly_t pb = *((poly_t *)b);

  hash_t ha = pa.eh[0];
  hash_t hb = pb.eh[0];

  // compare degree first
  if (ht->deg[hb] != ht->deg[ha])
    return ht->deg[hb]-ht->deg[ha];

  // else we have to check reverse lexicographical
  // NOTE: We store the exponents in reverse order in ht->exp and ht->ev
  // => we can use memcmp() here and still get reverse lexicographical ordering
#if __GB_HAVE_SSE2
  nvars_t i;
  exp_t expa[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t expb[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t tmp[ht->vl] __attribute__ ((aligned (16)));
  for (i=0; i<ht->nev; ++i) {
    _mm_store_si128((exp_v *)tmp, ht->ev[ha][i]);
    memcpy(expa+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
    _mm_store_si128((exp_v *)tmp, ht->ev[hb][i]);
    memcpy(expb+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
  }
  return memcmp(expa, expb, sizeof(expa));
#else
  return memcmp(ht->exp[ha], ht->exp[hb], sizeof(exp_t) * ht->nv);
#endif
}

/**
 * \brief Comparison function of polynomials for quicksort. Compares inverted 
 * w.r.t. the given monomial order lex.
 *
 * \param value a
 *
 * \param value b
 *
 * \returns negative value if a is non lead and b is lead; 0 if both are lead or
 * both are non lead; a positive value if a is lead and b is non lead
 */
static inline int cmp_polynomials_by_inverse_lex(const void *a,
    const void *b)
{
  poly_t pa = *((poly_t *)a);
  poly_t pb = *((poly_t *)b);

  hash_t ha = pa.eh[0];
  hash_t hb = pb.eh[0];

#if __GB_HAVE_SSE2
  nvars_t i;
  exp_t expa[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t expb[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t tmp[ht->vl] __attribute__ ((aligned (16)));
  for (i=0; i<ht->nev; ++i) {
    _mm_store_si128((exp_v *)tmp, ht->ev[ha][i]);
    memcpy(expa+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
    _mm_store_si128((exp_v *)tmp, ht->ev[hb][i]);
    memcpy(expb+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
  }
  return memcmp(expa, expb, sizeof(expa));
#else
  return memcmp(ht->exp[ha], ht->exp[hb], sizeof(exp_t) * ht->nv);
#endif
}

/**
 * \brief Comparison function of polynomials for quicksort. Compares inverted
 * w.r.t. the given monomial order grevlex.
 *
 * \param value a
 *
 * \param value b
 *
 * \returns negative value if a is non lead and b is lead; 0 if both are lead or
 * both are non lead; a positive value if a is lead and b is non lead
 */
static inline int cmp_polynomials_by_inverse_grevlex(const void *a,
    const void *b)
{
  poly_t pa = *((poly_t *)a);
  poly_t pb = *((poly_t *)b);

  hash_t ha = pa.eh[0];
  hash_t hb = pb.eh[0];

  // compare degree first
  if (ht->deg[hb] != ht->deg[ha])
    return ht->deg[ha]-ht->deg[hb];

  // else we have to check reverse lexicographical
  // NOTE: We store the exponents in reverse order in ht->exp and ht->ev
  // => we can use memcmp() here and still get reverse lexicographical ordering
#if __GB_HAVE_SSE2
  nvars_t i;
  exp_t expa[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t expb[ht->nev * ht->vl] __attribute__ ((aligned (16)));
  exp_t tmp[ht->vl] __attribute__ ((aligned (16)));
  for (i=0; i<ht->nev; ++i) {
    _mm_store_si128((exp_v *)tmp, ht->ev[ha][i]);
    memcpy(expa+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
    _mm_store_si128((exp_v *)tmp, ht->ev[hb][i]);
    memcpy(expb+(i*ht->vl), tmp, ht->vl*sizeof(exp_t));
  }
  return memcmp(expb, expa, sizeof(expa));
#else
  return memcmp(ht->exp[hb], ht->exp[ha], sizeof(exp_t) * ht->nv);
#endif
}

/**
 * \brief Sorts columns resp. monomials found by symbolic preprocessing to get
 * two different parts: Monomials which are lead monomials and monomials which
 * are non lead monomials.
 *
 * \param symbolic preprocessing data spd
 */
static inline void sort_columns_by_lead(spd_t *spd)
{
  qsort(spd->col->hpos, spd->col->load, sizeof(hash_t),
      cmp_symbolic_preprocessing_monomials_by_lead);
}

/**
 * \brief Sorts lead monomials found by symbolic preprocessing w.r.t. the
 * given monomial order lex
 *
 * \note The list of columns resp. monomials was already presorted to
 * distinguish lead ones (at the beginning of the list) from non lead ones (at
 * the end of the list).
 *
 * \param symbolic preprocessing data spd
 */
static inline void sort_lead_columns_by_lex(spd_t *spd)
{
  if (spd->col->nlm != 0) {
    // sort the start of spd->col, i.e. the lead monomial list
    qsort(spd->col->hpos, spd->col->nlm, sizeof(hash_t),
        cmp_symbolic_preprocessing_monomials_by_lex);
  }
}

/**
 * \brief Sorts lead monomials found by symbolic preprocessing w.r.t. the
 * given monomial order grevlex
 *
 * \note The list of columns resp. monomials was already presorted to
 * distinguish lead ones (at the beginning of the list) from non lead ones (at
 * the end of the list).
 *
 * \param symbolic preprocessing data spd
 */
static inline void sort_lead_columns_by_grevlex(spd_t *spd)
{
  if (spd->col->nlm != 0) {
    // sort the start of spd->col, i.e. the lead monomial list
    qsort(spd->col->hpos, spd->col->nlm, sizeof(hash_t),
        cmp_symbolic_preprocessing_monomials_by_grevlex);
  }
}

/**
 * \brief Sorts lead monomials found by symbolic preprocessing w.r.t. the
 * given monomial order lex and inverts the order
 *
 * \note The list of columns resp. monomials was already presorted to
 * distinguish lead ones (at the beginning of the list) from non lead ones (at
 * the end of the list).
 *
 * \note We invert lex since we have to align the A and C parts of the gbla
 * matrix in order to efficiently compute.
 *
 * \param symbolic preprocessing data spd
 */
static inline void sort_lead_columns_by_inverse_lex(spd_t *spd)
{
  if (spd->col->nlm != 0) {
    // sort the start of spd->col, i.e. the lead monomial list
    qsort(spd->col->hpos, spd->col->nlm, sizeof(hash_t),
        cmp_symbolic_preprocessing_monomials_by_inverse_lex);
  }
}

/**
 * \brief Sorts lead monomials found by symbolic preprocessing w.r.t. the
 * given monomial order grevlex and inverts the order
 *
 * \note The list of columns resp. monomials was already presorted to
 * distinguish lead ones (at the beginning of the list) from non lead ones (at
 * the end of the list).
 *
 * \note We invert grevlex since we have to align the A and C parts of the gbla
 * matrix in order to efficiently compute.
 *
 * \param symbolic preprocessing data spd
 */
static inline void sort_lead_columns_by_inverse_grevlex(spd_t *spd)
{
  if (spd->col->nlm != 0) {
    // sort the start of spd->col, i.e. the lead monomial list
    qsort(spd->col->hpos, spd->col->nlm, sizeof(hash_t),
        cmp_symbolic_preprocessing_monomials_by_inverse_grevlex);
  }
}

/**
 * \brief Sorts non lead monomials found by symbolic preprocessing w.r.t. the
 * given monomial order lex
 *
 * \note The list of columns resp. monomials was already presorted to
 * distinguish lead ones (at the beginning of the list) from non lead ones (at
 * the end of the list).
 *
 * \param symbolic preprocessing data spd
 */
static inline void sort_non_lead_columns_by_lex(spd_t *spd)
{
  // sort the end of spd->col, i.e. the non lead monomial list
  qsort(spd->col->hpos+spd->col->nlm, (spd->col->load - spd->col->nlm),
      sizeof(hash_t), cmp_symbolic_preprocessing_monomials_by_lex);
}

/**
 * \brief Sorts non lead monomials found by symbolic preprocessing w.r.t. the
 * given monomial order grevlex
 *
 * \note The list of columns resp. monomials was already presorted to
 * distinguish lead ones (at the beginning of the list) from non lead ones (at
 * the end of the list).
 *
 * \param symbolic preprocessing data spd
 */
static inline void sort_non_lead_columns_by_grevlex(spd_t *spd)
{
  // sort the end of spd->col, i.e. the non lead monomial list
  qsort(spd->col->hpos+spd->col->nlm, (spd->col->load - spd->col->nlm),
      sizeof(hash_t), cmp_symbolic_preprocessing_monomials_by_grevlex);
}

/**
 * \brief Sorts already presorted list of monomials resp. columns w.r.t. the
 * given monomial order lex. The list is already presorted by lead and non
 * lead monomials. Those monomials correspond then to the columns of the gbla
 * matrix generated later on.
 *
 * \param symbolic preprocessing data spd
 *
 * \param number of threads to use in parallel nthreads
 */
static inline void sort_presorted_columns_by_lex(spd_t *spd,
    const int nthreads)
{
  const int t = 2<nthreads ? 2 : nthreads;
  #pragma omp parallel num_threads(t)
  {
    #pragma omp single
    {
      #pragma omp task
      sort_lead_columns_by_lex(spd);
      #pragma omp task
      sort_non_lead_columns_by_lex(spd);
      #pragma omp taskwait
    }
  }
}

/**
 * \brief Sorts already presorted list of monomials resp. columns w.r.t. the
 * given monomial order grevlex. The list is already presorted by lead and non
 * lead monomials. Those monomials correspond then to the columns of the gbla
 * matrix generated later on.
 *
 * \param symbolic preprocessing data spd
 *
 * \param number of threads to use in parallel nthreads
 */
static inline void sort_presorted_columns_by_grevlex(spd_t *spd,
    const int nthreads)
{
  const int t = 2<nthreads ? 2 : nthreads;
  #pragma omp parallel num_threads(t)
  {
    #pragma omp single
    {
      #pragma omp task
      sort_lead_columns_by_grevlex(spd);
      #pragma omp task
      sort_non_lead_columns_by_grevlex(spd);
      #pragma omp taskwait
    }
  }
}

/**
 * \brief Sorts already presorted list of monomials resp. columns w.r.t. the
 * given monomial order lex. The list is already presorted by lead and non
 * lead monomials. Those monomials correspond then to the columns of the gbla
 * matrix generated later on.
 *
 * \note On the lefthand side of the gbla matrix we invert the sorting of the
 * columns due to gbla's internal ordering for reducing A later on.
 *
 * \param symbolic preprocessing data spd
 *
 * \param number of threads to use in parallel nthreads
 */
static inline void sort_presorted_columns_by_lex_invert_left_side(spd_t *spd,
    const int nthreads)
{
  const int t = 2<nthreads ? 2 : nthreads;
  #pragma omp parallel num_threads(t)
  {
    #pragma omp single
    {
      #pragma omp task
      sort_lead_columns_by_inverse_lex(spd);
      #pragma omp task
      sort_non_lead_columns_by_lex(spd);
      #pragma omp taskwait
    }
  }
}

/**
 * \brief Sorts already presorted list of monomials resp. columns w.r.t. the
 * given monomial order grevlex. The list is already presorted by lead and non
 * lead monomials. Those monomials correspond then to the columns of the gbla
 * matrix generated later on.
 *
 * \note On the lefthand side of the gbla matrix we invert the sorting of the
 * columns due to gbla's internal ordering for reducing A later on.
 *
 * \param symbolic preprocessing data spd
 *
 * \param number of threads to use in parallel nthreads
 */
static inline void sort_presorted_columns_by_grevlex_invert_left_side(spd_t *spd,
    const int nthreads)
{
  const int t = 2<nthreads ? 2 : nthreads;
  #pragma omp parallel num_threads(t)
  {
    #pragma omp single
    {
      #pragma omp task
      sort_lead_columns_by_inverse_grevlex(spd);
      #pragma omp task
      sort_non_lead_columns_by_grevlex(spd);
      #pragma omp taskwait
    }
  }
}

/**
 * \brief Sets the index entry of the hash values to the corresponding columns
 * for the upcoming gbla matrix. The relation between hash values of monomials
 * and columns is given by the symbolic preprocessing data.
 *
 * \param hash table ht
 *
 * \param symbolic preprocessing data spd
 */
static inline void set_column_index_in_hash_table(mp_cf4_ht_t *ht, const spd_t *spd)
{
  nelts_t i;

  for (i=0; i<spd->col->load; ++i)
    ht->idx[spd->col->hpos[i]]  = i;
}

/**
 * \brief Implements the comparison function for quicksort used in the function
 * sort_selection_by_column_index(). Takes multiplied lead monomials and sorts
 * corresponding to the predefined column index that is stored in the idx entry
 * of the hash table.
 *
 * \param value a
 *
 * \param value b
 *
 * \returns ht->idx[a.mlm] - ht->idx[b.mlm]
 */
static inline int cmp_monomial_polynomial_pair(const void *a, const void *b)
{
  hash_t h1 = ((mpp_t *)a)->mlm;
  hash_t h2 = ((mpp_t *)b)->mlm;

  return (ht->idx[h1] - ht->idx[h2]);
}

/**
 * \brief Implements the comparison function for quicksort used in the function
 * sort_selection_by_inverted_column_index(). Takes multiplied lead monomials and
 * inverts sorting corresponding to the predefined column index that is stored in
 * the idx entry of the hash table.
 *
 * \param value a
 *
 * \param value b
 *
 * \returns ht->idx[a.mlm] - ht->idx[b.mlm]
 */
static inline int cmp_monomial_polynomial_pair_inverted(const void *a, const void *b)
{
  hash_t h1 = ((mpp_t *)a)->mlm;
  hash_t h2 = ((mpp_t *)b)->mlm;

  return (ht->idx[h2] - ht->idx[h1]);
}

/**
 * \brief Sorts upper and lower selection of polynomials from preprocessing to
 * by the position of the corresponding multiplied lead monomial w.r.t. to the
 * predefined column order of the gbla matrix to be generated next.
 *
 * \note The sorting is inverted since, if we keep A, gbla assumes such an
 * inverted ordering on the rows of the matrix.
 *
 * \note Both lists, upper and lower, can be sorted in parallel, thus the
 * implementation is done using open mp tasks.
 *
 * \param symbolic data structure spd
 *
 * \param hash table ht
 *
 * \param number of threads nthreads
 */
static inline void sort_selection_by_inverted_column_index(spd_t *spd, const mp_cf4_ht_t *ht,
  const int nthreads)
{
  const int t = 2<nthreads ? 2 : nthreads;
  #pragma omp parallel num_threads(t)
  {
    #pragma omp single
    {
      // upper selection
      #pragma omp task
      qsort(spd->selu->mpp, spd->selu->load, sizeof(mpp_t),
          cmp_monomial_polynomial_pair_inverted);
      // lower selection
      #pragma omp task
      qsort(spd->sell->mpp, spd->sell->load, sizeof(mpp_t),
          cmp_monomial_polynomial_pair_inverted);
      #pragma omp taskwait
    }
  }
}

/**
 * \brief Sorts upper and lower selection of polynomials from preprocessing to
 * by the position of the corresponding multiplied lead monomial w.r.t. to the
 * predefined column order of the gbla matrix to be generated next.
 *
 * \note Both lists, upper and lower, can be sorted in parallel, thus the
 * implementation is done using open mp tasks.
 *
 * \param symbolic data structure spd
 *
 * \param hash table ht
 *
 * \param number of threads nthreads
 */
static inline void sort_selection_by_column_index(spd_t *spd, const mp_cf4_ht_t *ht,
  const int nthreads)
{
  const int t = 2<nthreads ? 2 : nthreads;
  #pragma omp parallel num_threads(t)
  {
    #pragma omp single
    {
      // upper selection
      #pragma omp task
      qsort(spd->selu->mpp, spd->selu->load, sizeof(mpp_t),
          cmp_monomial_polynomial_pair);
      // lower selection
      #pragma omp task
      qsort(spd->sell->mpp, spd->sell->load, sizeof(mpp_t),
          cmp_monomial_polynomial_pair);
      #pragma omp taskwait
    }
  }
}

/**
 * \brief Does not perform a simplifier search, just an empty function call.
 *
 * \param multiplier polynomial pair mpp
 *
 * \param intermediate groebner basis basis
 *
 * \param simplifier list sf
 */
static inline void no_simplify(mpp_t *mpp, const gb_t *basis, const gb_t *sf)
{
  return;
}

/**
 * \brief Tries to find a simplifier for the given polynomial multiple.
 *
 * \param multiplier polynomial pair mpp
 *
 * \param intermediate groebner basis basis
 *
 * \param simplifier list sf
 */
static inline void try_to_simplify(mpp_t *mpp, const gb_t *basis, const gb_t *sf)
{
  nelts_t l     = 0;
  hash_t sf_mul = 0;
  const nelts_t load  = basis->sf[mpp->bi].load;
  for (l=0; l<load; ++l) {
    const nelts_t idx = basis->sf[mpp->bi].idx[load-1-l];
    // we start searching from the end of the list since those elements
    // might be best reduced
    if (sf->nt[idx] < 3* mpp->nt && check_monomial_division(mpp->mlm, sf->eh[idx][0], ht)) {
      sf_mul = get_multiplier(mpp->mlm, sf->eh[idx][0], ht);
      if (sf_mul != 0) {
#if SYMBOL_DEBUG
        printf("-- SIMPLIFY --\n");
        for (int ii=0; ii<basis->nv; ++ii)
          printf("%u ",ht->exp[mpp->mul][ii]);
        printf("\n");
        for (int ii=0; ii<basis->nv; ++ii)
          printf("%u ",ht->exp[basis->eh[mpp->bi][0]][ii]);
        printf("\n - - -\n");
        for (int ii=0; ii<basis->nv; ++ii)
          printf("%u ",ht->exp[sf_mul][ii]);
        printf("\n");
        for (int ii=0; ii<basis->nv; ++ii)
          printf("%u ",ht->exp[sf->eh[basis->sf[mpp->bi].idx[load-1-l]][0]][ii]);
        printf("\n");
#endif

        mpp->mul  = sf_mul;
        mpp->nt   = sf->nt[idx];
        mpp->eh   = sf->eh[idx];
        mpp->cf   = sf->cf[idx];
        return;
      }
    }
  }
}

/**
 * \brief Checks the given spair sp if it consists of generators that are
 * possible duplicates interpreted as rows of the to be constructed matrix. This
 * means if the spair has an already known lcm and a generator is already stored
 * in the corresponding list of duplicates. If such a duplicate is found the
 * generator index in the spair is set to zero such that it is no longer
 * considered in the symbolic preprocessing. If the lcm of the spair is a new
 * one, the duplicates list is reset. Thus both parameters are possibly updated.
 *
 * \param duplicates list duplicates
 *
 * \param spair sp
 */
static inline void mark_duplicates(dup_t *duplicates, spair_t *sp)
{
  int k = 0;
  // so pair is not redundant
  if (sp->lcm == duplicates->lcm) {
    // check gen1
    for (k=0; k<duplicates->load; ++k) {
      if (duplicates->idx[k] == sp->gen1) {
        sp->gen1  = 0;
        goto next_loop;
      }
    }
    // add gen1
    duplicates->idx[duplicates->load++]  = sp->gen1;
    // check gen2
next_loop:
    for (k=0; k<duplicates->load; ++k) {
      if (duplicates->idx[k] == sp->gen2) {
        sp->gen2  = 0;
        return;
      }
    }
    // add gen2
    duplicates->idx[duplicates->load++]  = sp->gen2;
  } else { // reset duplicates list
    duplicates->lcm   = sp->lcm;
    duplicates->load  = 0;
    memset(duplicates->idx, 0, duplicates->size*sizeof(nelts_t));
    duplicates->idx[duplicates->load++] = sp->gen1;
    duplicates->idx[duplicates->load++] = sp->gen2;
  }
}

/**
 * \brief Initializes a list for storing duplicates. The list's size is based on
 * the number of spairs selected in this step of the algorithm.
 *
 * \param number of spairs selected n_spairs_selected
 *
 * \return duplicated list
 */
static inline dup_t *init_duplicates_list(const nelts_t n_pairs_selected)
{
  dup_t *duplicates = (dup_t *)malloc(sizeof(dup_t));
  // allocating space for all generators of all selected pairs in order not
  // having to reallocate memory
  duplicates->idx   = (nelts_t *)malloc(2 * n_pairs_selected * sizeof(nelts_t));
  duplicates->lcm   = 0;
  duplicates->size  = 2 * n_pairs_selected;
  return duplicates;
}

/**
 * \brief Selects pairs due to the sorting and selection done by the previous
 * get_pairs_by_*() procedure and returns
 * a set of this selection that is later on filled with corresponding lower term
 * reducers.
 *
 * \note The function also already marks the hash values for the lcms of the
 * spairs with "2". So we know that there are 2 polynomials in the upcoming
 * matrix that hit that monomial. We use this in the following symbolic
 * preprocessing not to handle this monomial again and also, in the matrix
 * construction later on, for know the number of nonzero entries in the column
 * corresponding to this monomial. This will help splicing the matrix for gbla.
 *
 * \note There are two selection lists, one for the upper and one for the lower
 * part of the gbla matrix to be constructed. By default, the first generator of
 * an spair is added to the upper part, the second generator to the lower part.
 * All upcoming reducers are also added to the upper selection list.
 *
 * \param pair set ps
 *
 * \param selection set for upper part of gbla matrix sel_upp
 *
 * \param selection set for lower part of gbla matrix sel_low
 *
 * \param hash list of monomials mon
 *
 * \param intermediate grobner basis basis
 *
 * \param simplifier list sf
 *
 * \param last index of pair selection in pair list idx
 */
static inline void select_pairs(ps_t *ps, sel_t *selu, sel_t *sell, pre_t *mon,
    const gb_t *basis, const gb_t *sf, const nelts_t nsel)
{
  int i, j ,k, l;
  // we do not need to check for size problems in sel du to above comment: we
  // have allocated basis->load slots, so enough for each possible element from
  // the basis
#if SYMBOL_DEBUG
  printf("%5u selected pairs in this step of the algorithm:\n", nsel);
  for (int k=0; k<nsel; ++k) {
    if (k+1<nsel) {
      if (ps->pairs[k]->lcm == ps->pairs[k+1]->lcm) {
        printf("same lcms! %5u | %5u\n",k,k+1);
      }
    }
  }
#endif

#if 1
  // for each lcm we first detect which elements have to be added to the
  // symbolic preprocessing step, i.e. we remove duplicates
  i = 0;
  hash_t lcm    = 0;
  nelts_t *gens = (nelts_t *)malloc(2 * nsel * sizeof(nelts_t));
  nelts_t load  = 0;
  while (i<nsel) {
    lcm   = ps->pairs[i]->lcm;
    j = i;
    while (j<nsel && ps->pairs[j]->lcm == lcm) {
      // check first generator
      for (k=0; k<load; ++k) {
        if (ps->pairs[j]->gen1 == gens[k]) {
          break;
        }
      }
      if (k == load) {
        gens[load]  = ps->pairs[j]->gen1;
        load++;
      }
      // check second generator
      for (k=0; k<load; ++k) {
        if (ps->pairs[j]->gen2 == gens[k]) {
          break;
        }
      }
      if (k == load) {
        gens[load]  = ps->pairs[j]->gen2;
        load++;
      }
      j++;
    }
    i     = j;
    // now we have handled all poairs of the given lcm, we add them to the
    // symbolic preprocessing step in the following
    k = 0;
    // ht->idx is always at least 1
    /*
    for (int ii=0; ii<load; ++ii)
      printf("gens[%u] = %u\n",ii, gens[ii]);
    printf("\n");
    */
    //ht->idx[lcm]  = 1;
    if (load>1) {
      mon->nlm++;
      //mon->load++;
      add_spair_generator_to_selection(selu, basis, lcm, gens[k]);
      j = selu->load-1;
      // check for simplification
      // function pointer set correspondingly if simplify option is set or not
      ht->sf.simplify(&selu->mpp[j], basis, sf);
      //printf("[u] %u | %u || %u\n", mon->size, mon->load, selu->mpp[j].nt);
      if (mon->size-mon->load+1 < selu->mpp[j].nt) {
        const nelts_t max = 2*mon->size > selu->mpp[j].nt ? 2*mon->size : selu->mpp[j].nt;
        adjust_size_of_preprocessing_hash_list(mon, max);
      }
      enter_monomial_to_preprocessing_hash_list(selu->mpp[j], mon, ht);
      k++;
      ht->idx[lcm]  = 2;
    }
    for (l=k; l<load; l++) {
      add_spair_generator_to_selection(sell, basis, lcm, gens[l]);
      j = sell->load-1;
      // check for simplification
      // function pointer set correspondingly if simplify option is set or not
      ht->sf.simplify(&sell->mpp[j], basis, sf);
      //printf("[l] %u | %u || %u\n", mon->size, mon->load, sell->mpp[j].nt);
      if (mon->size-mon->load+1 < sell->mpp[j].nt) {
        const nelts_t max = 2*mon->size > sell->mpp[j].nt ? 2*mon->size : sell->mpp[j].nt;
        adjust_size_of_preprocessing_hash_list(mon, max);
      }
      enter_monomial_to_preprocessing_hash_list(sell->mpp[j], mon, ht);
    }
    // set data for next lcm round
    load  = 0;
    //printf("ht->idx[%u] = %u\n", lcm, ht->idx[lcm]);
  }

  free(gens);

#else
  nelts_t i, j, k;
  spair_t *sp = NULL;

  // keep a list for duplicates, i.e. for each lcm appearing we keep a list of
  // corresponding polynomials. if a polynomial is appearing again with the same
  // lcm, this would lead to a duplicate row and we can remove it, i.e. set the
  // corresponding gen1 or gen2 entry to 0.
  dup_t *duplicates = init_duplicates_list(nsel);

  for (i=0; i<nsel; ++i) {
    // remove duplicates if lcms and the first generators are the same
    sp  = ps->pairs[i];
    // we remove pairs that are redundant in the sense that a new basis element
    // has made an older one redundant. thus we only need to keep the one pair
    // that consists of the new basis element and the element that is redundant
    // due to it.
    /*
    if ((basis->red[sp->gen1] > 0 && basis->red[sp->gen1] != sp->gen2) || basis->red[sp->gen2] > 0) {
      meta_data->sel_pairs--;
      continue;
    }
    */
    // mark generators of sp that would give duplicate rows in the matrix
    mark_duplicates(duplicates, sp);
    // We have to distinguish between usual spairs and spairs consisting of one
    // initial input element: The latter ones have only 1 generator (gen2 has
    // the generator, gen1 is zero) and the
    // corresponding lead monomial is not part of the basis. Thus we cannot set
    // ht->idx[*] = 2 for these. The whole data for these special spairs is in
    // basis before basis->st, whereas the data of the usual spairs is
    // completely in basis starting at position basis->st.

    // gen2 is only 0 if it might be a duplicate, see above
    /*
     * sp->gen2 > sp->gen1 when constructing new spairs. so if sp->gen2 != 0
     * then we must have sp->gen2 - sp->gen1 > 0.
     * sp->gen2 - sp->gen1 = 0 can only hold if sp->gen2 = sp>gen1 = 0, so the
     * whole spair is useless
     * if sp->gen2 - sp->gen1 < 0 then sp->gen2 = 0 and sp->gen1 > 0.
     */

    if (sp->gen2 != 0) {
      add_spair_generator_to_selection(sell, basis, sp->lcm, sp->gen2);
      printf("gen %u || lcm %lu || sell %u\n", sp->gen2, sp->lcm, sell->load-1);
      j = sell->load-1;

      // check for simplification
      // function pointer set correspondingly if simplify option is set or not
      ht->sf.simplify(&sell->mpp[j], basis, sf);

      enter_monomial_to_preprocessing_hash_list(sell->mpp[j], mon, ht);
    }
    // now we distinguish cases for gen1
    if (sp->gen1 != 0) {
      // first generator for upper part of gbla matrix if there is no other pair
      // with the same lcm
      if (ht->idx[sp->lcm] == 1) {
        //mon->hpos[mon->load]  = sp->lcm;
        ht->idx[sp->lcm]      = 2;
#if SYMBOL_DEBUG
      printf("hpos[%u] = %u\n", mon->load, mon->hpos[mon->load]);
      for (int ii=0; ii<ht->nv; ++ii)
        printf("%u ", ht->exp[sp->lcm][ii]);
      printf("\n");
#endif
        mon->nlm++;
        //mon->load++;
        if (mon->load == mon->size)
          adjust_size_of_preprocessing_hash_list(mon, 2*mon->size);

        add_spair_generator_to_selection(selu, basis, sp->lcm, sp->gen1);
      printf("gen %u || lcm %lu || selu %u\n", sp->gen1, sp->lcm, selu->load-1);
        j = selu->load-1;
        // check for simplification
        // function pointer set correspondingly if simplify option is set or not
        ht->sf.simplify(&selu->mpp[j], basis, sf);
        enter_monomial_to_preprocessing_hash_list(selu->mpp[j],
            mon, ht);
      } else {
        add_spair_generator_to_selection(sell, basis, sp->lcm, sp->gen1);
      printf("gen %u || lcm %lu || sell %u\n", sp->gen1, sp->lcm, sell->load-1);
        j = sell->load-1;
        // check for simplification
        // function pointer set correspondingly if simplify option is set or not
        ht->sf.simplify(&sell->mpp[j], basis, sf);
        enter_monomial_to_preprocessing_hash_list(sell->mpp[j],
            mon, ht);
      }
    }
    // remove the selected pair from the pair set
    //free(sp);
  }

  // free duplicates list
  free(duplicates->idx);
  free(duplicates);
#endif
  // adjust pair set after removing the bunch of selected pairs
  k = 0;
  for (i=0; i<nsel; ++i)
    free(ps->pairs[i]);
  for (i=nsel; i<ps->load; ++i) {
    ps->pairs[k] = ps->pairs[i];
    k++;
  }
  ps->load  = k;
}
#endif
