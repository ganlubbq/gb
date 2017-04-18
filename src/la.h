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
 * \file la.h
 * \brief Implementation of the linear algebra parts.
 *
 * \author Christian Eder <ederc@mathematik.uni-kl.de>
 */
#ifndef GB_LA_H
#define GB_LA_H

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <omp.h>
#include <config.h>
#include "types.h"

static inline void copy_first_row_from_dense(mat_gb_block_t *bl,
    const mat_gb_block_t *obl, const mat_gb_meta_data_t *meta)
{
  nelts_t i;

  bl->len[0]  = 0;
  bl->len[1]  = bl->len[0];
  
  for (i=0; i<meta->bs; ++i) {
    if (obl->val[i] != 0) {
      bl->pos[bl->len[1]] = (bs_t)i;
      bl->val[bl->len[1]] = obl->val[i];
      bl->len[1]++;
    }
  }
}

static inline void copy_first_row_from_sparse(mat_gb_block_t *bl,
    const mat_gb_block_t *obl)
{
  bl->len[0]  = obl->len[0];
  bl->len[1]  = obl->len[1];
  memcpy(bl->pos, obl->pos, (bl->len[1]-bl->len[0]) * sizeof(bs_t));
  memcpy(bl->val, obl->val, (bl->len[1]-bl->len[0]) * sizeof(cf_t));
}

static inline void load_dense_row_for_update_from_dense(bf_t *dr,
    const nelts_t idx, const mat_gb_block_t *bl,
    const mat_gb_meta_data_t *meta)
{
  nelts_t i;

  memset(dr, 0, meta->bs * sizeof(bf_t));

  for (i=0; i<meta->bs; ++i)
    dr[i]  = (bf_t)bl->val[idx*meta->bs+i];
}

static inline void load_dense_block_from_sparse(bf_t *db,
    const mat_gb_block_t *bl, const mat_gb_meta_data_t *meta)
{
  nelts_t i, j;

  memset(db, 0, (meta->bs*meta->bs) * sizeof(bf_t));

  if (bl->len != NULL) {
    for (i=0; i<bl->nr; ++i) {
      for (j=bl->len[i]; j<bl->len[i+1]; ++j) {
        db[i*meta->bs + bl->pos[j]]  = (bf_t)bl->val[j];
      }
    }
  }
}

static inline void load_dense_row_for_update(bf_t *dr,
    const nelts_t idx, const mat_gb_block_t *bl,
    const mat_gb_meta_data_t *meta)
{
  nelts_t i;

  memset(dr, 0, meta->bs * sizeof(bf_t));

  if (bl->len != NULL) {
    for (i=bl->len[idx]; i<bl->len[idx+1]; ++i)
      dr[bl->pos[i]]  = (bf_t)bl->val[i];
  } else {
    if (bl->val != NULL) {
      for (i=0; i<meta->bs; ++i)
        dr[i]  = (bf_t)bl->val[idx*meta->bs+i];
    }
  }
}

static inline void load_dense_row_for_update_from_sparse(bf_t *dr,
    const nelts_t idx, const mat_gb_block_t *bl,
    const mat_gb_meta_data_t *meta)
{
  nelts_t i;

  memset(dr, 0, meta->bs * sizeof(bf_t));

  if (bl->len != NULL) {
    for (i=bl->len[idx]; i<bl->len[idx+1]; ++i)
      dr[bl->pos[i]]  = (bf_t)bl->val[i];
  }
}

static inline void write_updated_row_to_sparse_format(mat_gb_block_t *bl,
    const bf_t *dr, const nelts_t idx, const mat_gb_meta_data_t *meta)
{
  nelts_t i;
  bf_t tmp;

  bl->len[idx+1]  = bl->len[idx];
  for (i=0; i<meta->bs; ++i) {
    if (dr[i] != 0) {
      tmp = dr[i] % meta->mod;
      if (tmp != 0) {
        bl->pos[bl->len[idx+1]] = (bs_t)i;
        bl->val[bl->len[idx+1]] = (cf_t)tmp;
        bl->len[idx+1]++;
      }
    }
  }
}

static inline void set_updated_block(mat_gb_block_t **bl, mat_gb_block_t* new_bl)
{

  mat_gb_block_t *blin  = *bl;
  free(blin->len);
  free(blin->pos);
  free(blin->val);

  blin->len  = new_bl->len;
  blin->pos  = new_bl->pos;
  blin->val  = new_bl->val;

  free(new_bl);
}

static inline void update_dense_row_dense(bf_t *dr, const nelts_t idx,
    const mat_gb_block_t *bl, const mat_gb_block_t *mbl,
    const mat_gb_meta_data_t *meta)
{
  nelts_t i, j;

  /* printf("bl->val %p\n", bl->val);
   * for (nelts_t k=0; k< meta->bs; ++k) {
   *   printf("%p -- ",bl->val+(k*meta->bs));
   *   for (nelts_t l=0; l< meta->bs; ++l) {
   *     printf("%u ", bl->val[k*meta->bs+l]);
   *   }
   *   printf("\n");
   * }
   * printf("\n"); */

  /* printf("val %p, pos %p\n", mbl->val, mbl->pos); */
  /* printf("idx %u\n", idx);
   * for (int ii=0; ii<meta->bs; ++ii)
   *   printf("%lu ",dr[ii]);
   * printf("\n"); */
  for (i=mbl->len[idx]; i<mbl->len[idx+1]-3; i=i+4) {
    const bf_t mul1   = (bf_t)meta->mod - mbl->val[i];
    const nelts_t ri1 = mbl->pos[i];
    const bf_t mul2   = (bf_t)meta->mod - mbl->val[i+1];
    const nelts_t ri2 = mbl->pos[i+1];
    const bf_t mul3   = (bf_t)meta->mod - mbl->val[i+2];
    const nelts_t ri3 = mbl->pos[i+2];
    const bf_t mul4   = (bf_t)meta->mod - mbl->val[i+3];
    const nelts_t ri4 = mbl->pos[i+3];
    /* const nelts_t ri  = bl->nr -1 - mbl->pos[i]; */
    const cf_t *red1  = bl->val+(ri1*meta->bs);
    const cf_t *red2  = bl->val+(ri2*meta->bs);
    const cf_t *red3  = bl->val+(ri3*meta->bs);
    const cf_t *red4  = bl->val+(ri4*meta->bs);
/*     printf("red %p\n",red);
 *     printf("ROW INDEX %u = %u - %u\n", ri, bl->nr, mbl->pos[i]);
 *
 *     printf("RED1: ");
 *     for (j=0; j<meta->bs; j++) {
 *       printf("%u ", red[j]);
 *     }
 *     printf("\n");
 *     printf("BS %u\n", meta->bs); */
    for (j=0; j<meta->bs; j=j+8) {
      /* printf("j %u\n", j);
       * printf("dr %lu\n", dr[j]);
       * printf("mul %lu\n", mul);
       * printf("red %u\n", red[j]); */
      dr[j]   +=  mul1 * red1[j] + mul2 * red2[j] + mul3 * red3[j] + mul4 * red4[j];
      dr[j+1] +=  mul1 * red1[j+1] + mul2 * red2[j+1] + mul3 * red3[j+1] + mul4 * red4[j+1];
      dr[j+2] +=  mul1 * red1[j+2] + mul2 * red2[j+2] + mul3 * red3[j+2] + mul4 * red4[j+2];
      dr[j+3] +=  mul1 * red1[j+3] + mul2 * red2[j+3] + mul3 * red3[j+3] + mul4 * red4[j+3];
      dr[j+4] +=  mul1 * red1[j+4] + mul2 * red2[j+4] + mul3 * red3[j+4] + mul4 * red4[j+4];
      dr[j+5] +=  mul1 * red1[j+5] + mul2 * red2[j+5] + mul3 * red3[j+5] + mul4 * red4[j+5];
      dr[j+6] +=  mul1 * red1[j+6] + mul2 * red2[j+6] + mul3 * red3[j+6] + mul4 * red4[j+6];
      dr[j+7] +=  mul1 * red1[j+7] + mul2 * red2[j+7] + mul3 * red3[j+7] + mul4 * red4[j+7];
      /* printf("%lu\n", dr[bl->pos[j]]); */
    }
  }
  for (; i<mbl->len[idx+1]; ++i) {
    const bf_t mul    = (bf_t)meta->mod - mbl->val[i];
    const nelts_t ri  = mbl->pos[i];
    /* const nelts_t ri  = bl->nr -1 - mbl->pos[i]; */
    const cf_t *red   = bl->val+(ri*meta->bs);
/*     printf("red %p\n",red);
 *     printf("ROW INDEX %u = %u - %u\n", ri, bl->nr, mbl->pos[i]);
 *
 *     printf("RED1: ");
 *     for (j=0; j<meta->bs; j++) {
 *       printf("%u ", red[j]);
 *     }
 *     printf("\n");
 *     printf("BS %u\n", meta->bs); */
    for (j=0; j<meta->bs; j=j+8) {
      /* printf("j %u\n", j);
       * printf("dr %lu\n", dr[j]);
       * printf("mul %lu\n", mul);
       * printf("red %u\n", red[j]); */
      dr[j]   +=  mul * red[j];
      dr[j+1] +=  mul * red[j+1];
      dr[j+2] +=  mul * red[j+2];
      dr[j+3] +=  mul * red[j+3];
      dr[j+4] +=  mul * red[j+4];
      dr[j+5] +=  mul * red[j+5];
      dr[j+6] +=  mul * red[j+6];
      dr[j+7] +=  mul * red[j+7];
      /* printf("%lu\n", dr[bl->pos[j]]); */
    }
  /*   printf("ri %u\n", ri);
   * for (int ii=0; ii<meta->bs; ++ii)
   *   printf("%lu ",dr[ii]);
   * printf("\n"); */
  }
}

static inline void update_dense_block_sparse(bf_t *db,
    mat_gb_block_t *bl, const mat_gb_block_t *mbl,
    const nelts_t max, const mat_gb_meta_data_t *meta)
{
  nelts_t i, j, k;

  for (i=0; i<max; ++i) {
    for (j=mbl->len[i]; j<mbl->len[i+1]; ++j) {
      const bf_t mul    = (bf_t)meta->mod - mbl->val[j];
      /* printf("mul %u\n", mul); */
      const nelts_t ri  = mbl->pos[j];
      /* const nelts_t ri  = bl->nr -1 - mbl->pos[j]; */
      const nelts_t shift = (bl->len[ri+1]-bl->len[ri]) % 8;
      /* printf("bl->len[ri] %u | bl->len[ri+1] %u | shift %u\n",
       *    bl->len[ri], bl->len[ri+1], shift); */
      for (k=bl->len[ri]; k<bl->len[ri]+shift; ++k) {
        db[i*meta->bs + bl->pos[k]] +=  mul * bl->val[k];
      }
      for (; k<bl->len[ri+1]; k=k+8) {
        db[i*meta->bs + bl->pos[k]]    +=  mul * bl->val[k];
        db[i*meta->bs + bl->pos[k+1]]  +=  mul * bl->val[k+1];
        db[i*meta->bs + bl->pos[k+2]]  +=  mul * bl->val[k+2];
        db[i*meta->bs + bl->pos[k+3]]  +=  mul * bl->val[k+3];
        db[i*meta->bs + bl->pos[k+4]]  +=  mul * bl->val[k+4];
        db[i*meta->bs + bl->pos[k+5]]  +=  mul * bl->val[k+5];
        db[i*meta->bs + bl->pos[k+6]]  +=  mul * bl->val[k+6];
        db[i*meta->bs + bl->pos[k+7]]  +=  mul * bl->val[k+7];
      }
    }
/*     for (int ii=0; ii<meta->bs; ++ii)
 *       printf("%lu ",db[i*meta->bs+ii]);
 *     printf("\n");
 *
 *     write_updated_row_to_sparse_format(bl, db+i*meta->bs, i, meta); */
  }
}

static inline void update_dense_block_sparse_self(bf_t *db,
    mat_gb_block_t *bl, const mat_gb_block_t *mbl,
    const mat_gb_meta_data_t *meta)
{
  nelts_t i, j, k;

  for (i=0; i<bl->nr; ++i) {
    for (j=mbl->len[i]; j<mbl->len[i+1]; ++j) {
      const bf_t mul    = (bf_t)meta->mod - mbl->val[j];
      /* printf("mul %u\n", mul); */
      const nelts_t ri  = bl->nr -1 - mbl->pos[j];
      const nelts_t shift = (bl->len[ri+1]-bl->len[ri]) % 4;
      /* printf("bl->len[ri] %u | bl->len[ri+1] %u | shift %u\n",
      *     bl->len[ri], bl->len[ri+1], shift); */
      for (k=bl->len[ri]; k<bl->len[ri]+shift; ++k) {
        db[i*meta->bs + bl->pos[k]] +=  mul * bl->val[k];
      }
      for (; k<bl->len[ri+1]; k=k+4) {
        db[i*meta->bs + bl->pos[k]]    +=  mul * bl->val[k];
        db[i*meta->bs + bl->pos[k+1]]  +=  mul * bl->val[k+1];
        db[i*meta->bs + bl->pos[k+2]]  +=  mul * bl->val[k+2];
        db[i*meta->bs + bl->pos[k+3]]  +=  mul * bl->val[k+3];
      }
    }
    /* for (int ii=0; ii<meta->bs; ++ii)
     *   printf("%lu ",db[i*meta->bs+ii]);
     * printf("\n"); */

    write_updated_row_to_sparse_format(bl, db+i*meta->bs, i, meta);
  }
}

static inline void update_dense_row_sparse(bf_t *dr, const nelts_t idx,
    const mat_gb_block_t *bl, const mat_gb_block_t *mbl,
    const mat_gb_meta_data_t *meta)
{
  nelts_t i, j;

  /* printf("idx %u\n", idx);
   * for (int ii=0; ii<meta->bs; ++ii)
   *   printf("%lu ",dr[ii]);
   * printf("\n"); */
  for (i=mbl->len[idx]; i<mbl->len[idx+1]; ++i) {
    const bf_t mul    = (bf_t)meta->mod - mbl->val[i];
      /* printf("mul %u\n", mul); */
    const nelts_t ri  = mbl->pos[i];
    /* const nelts_t ri  = bl->nr -1 - mbl->pos[i]; */
    const nelts_t shift = (bl->len[ri+1]-bl->len[ri]) % 4;
    /* printf("bl->len[ri] %u | bl->len[ri+1] %u | shift %u\n",
     *     bl->len[ri], bl->len[ri+1], shift); */
    for (j=bl->len[ri]; j<bl->len[ri]+shift; ++j) {
      dr[bl->pos[j]] +=  mul * bl->val[j];
    }
    for (j; j<bl->len[ri+1]; j=j+4) {
      dr[bl->pos[j]]    +=  mul * bl->val[j];
      dr[bl->pos[j+1]]  +=  mul * bl->val[j+1];
      dr[bl->pos[j+2]]  +=  mul * bl->val[j+2];
      dr[bl->pos[j+3]]  +=  mul * bl->val[j+3];
      /* printf("j %u | pos[j] %u | val[j] %u\n", j, bl->pos[j], bl->val[j]);
       * printf("%lu --> + %u * %u ====> ", dr[bl->pos[j]], mul, bl->val[j]); */
      /* printf("%lu\n", dr[bl->pos[j]]); */
    }
  /*   printf("ri %u\n", ri);
   * for (int ii=0; ii<meta->bs; ++ii)
   *   printf("%lu ",dr[ii]);
   * printf("\n"); */
  }
}

static inline void update_single_block_dense(mat_gb_block_t *mat,
    const nelts_t shift, const nelts_t idx, const mat_gb_meta_data_t *meta)
{
  nelts_t i;

  /* first block used as lookup table for updates */
  const mat_gb_block_t *fbl  = mat+shift;
  /* block to be updated */
  mat_gb_block_t *ubl  = mat+idx;
  /* row to be updated in dense format */
  bf_t *dr  = (bf_t *)malloc(meta->bs * sizeof(bf_t));

  mat_gb_block_t *nbl = generate_mat_gb_block(meta, ubl->nr);

  for (i=1; i<ubl->nr; ++i) {
    /* load dense row to be updated */
    load_dense_row_for_update_from_dense(dr, i, ubl, meta);

    /* find corresponding row and multiplier */
    update_dense_row_sparse(dr, i, nbl, fbl, meta);

    /* write updated row to new storage holders */
    write_updated_row_to_sparse_format(nbl, dr, i, meta);
  }

  set_updated_block(&ubl, nbl);
}

static inline void update_single_block_sparse_2(mat_gb_block_t *mat,
    const nelts_t shift, const nelts_t idx, const mat_gb_meta_data_t *meta)
{
  nelts_t i;

  /* first block used as lookup table for updates */
  const mat_gb_block_t *fbl  = mat+shift;
  /* block to be updated */
  mat_gb_block_t *ubl  = mat+idx;
  /* row to be updated in dense format */
  bf_t *db  = (bf_t *)malloc((meta->bs*meta->bs) * sizeof(bf_t));

  load_dense_block_from_sparse(db, ubl, meta);

  mat_gb_block_t *nbl = generate_mat_gb_block(meta, ubl->nr);
  update_dense_block_sparse(db, nbl, fbl, ubl->nr, meta);
  free(db);

  set_updated_block(&ubl, nbl);
}

static inline void update_single_block_sparse(mat_gb_block_t *mat,
    const nelts_t shift, const nelts_t idx, const mat_gb_meta_data_t *meta)
{
  nelts_t i;

  /* first block used as lookup table for updates */
  const mat_gb_block_t *fbl  = mat+shift;
  /* block to be updated */
  mat_gb_block_t *ubl  = mat+idx;
  /* row to be updated in dense format */
  bf_t *dr  = (bf_t *)malloc(meta->bs * sizeof(bf_t));

  mat_gb_block_t *nbl = generate_mat_gb_block(meta, ubl->nr);

  for (i=0; i<ubl->nr; ++i) {
    /* load dense row to be updated */
    load_dense_row_for_update_from_sparse(dr, i, ubl, meta);

    /* find corresponding row and multiplier */
    /* printf("UPDATE ROW %u\n", i); */
    update_dense_row_sparse(dr, i, nbl, fbl, meta);

    /* write updated row to new storage holders */
    write_updated_row_to_sparse_format(nbl, dr, i, meta);
  }
  free(dr);

  set_updated_block(&ubl, nbl);
}

static inline void update_single_block(mat_gb_block_t *mat,
    const nelts_t shift, const nelts_t idx, const mat_gb_meta_data_t *meta)
{
  /* printf("in %u || %u\n", idx, shift); */
  if (mat[idx].len != NULL) {
    update_single_block_sparse(mat, shift, idx, meta);
  } else {
    if (mat[idx].val != NULL)
      update_single_block_dense(mat, shift, idx, meta);
    else
      return;
  }
}

static inline void update_upper_row_block(mat_gb_block_t *mat,
    const nelts_t shift, const mat_gb_meta_data_t *meta, const int t)
{
  /* printf("mat %p\n", mat);
   * printf("ncb %u\n", meta->ncb); */
#pragma omp parallel num_threads(t)
  {
#pragma omp single nowait
    {
      /* the first block is used for updated the remaining ones,
       * i.e. we start at block index i=shift+1 */
      /* printf("shift %u +1 < %u ncb?\n", shift, meta->ncb); */
      for (nelts_t i=shift+1; i<meta->ncb; ++i) {
#pragma omp task
        update_single_block(mat, shift, i, meta);
      }
    }
  }
  /* check density of blocks */
  adjust_block_row_types(mat, meta);
  /* adjust_block_row_types_including_dense(mat, meta); */
}

static inline void update_block(mat_gb_block_t **bl, const bf_t *db,
    const mat_gb_meta_data_t *meta)
{
  nelts_t i, j;
  const nelts_t bs_square = meta->bs * meta->bs;
  bf_t tmp;

  mat_gb_block_t *blin  = *bl;
  blin->len     = realloc(blin->len, (meta->bs+1) * sizeof(nelts_t));
  blin->pos     = realloc(blin->pos, bs_square * sizeof(bs_t));
  blin->val     = realloc(blin->val, bs_square * sizeof(cf_t));
  blin->len[0]  = 0;

  for (i=0; i<meta->bs; ++i) {
    blin->len[i+1]  = blin->len[i];
    for (j=0; j<meta->bs; ++j) {
      tmp = db[i*meta->bs+j] %  meta->mod;
      if (tmp != 0) {
        blin->pos[blin->len[i+1]] = (bs_t)j;
        blin->val[blin->len[i+1]] = (cf_t)tmp;
        blin->len[i+1]++;
      }
    }
  }
  if (blin->len[meta->bs] == 0) {
    free(blin->len);
    blin->len = NULL;
    free(blin->pos);
    blin->pos = NULL;
    free(blin->val);
    blin->val = NULL;
  } else {
    blin->pos = realloc(blin->pos, blin->len[meta->bs] * sizeof(bs_t));
    blin->val = realloc(blin->val, blin->len[meta->bs] * sizeof(cf_t));
  }
}

static inline void sparse_update_lower_block_by_upper_block_2(mat_gb_block_t *l,
    const mat_gb_block_t *u, const nelts_t shift, const nelts_t rbi,
    const nelts_t cbi, const mat_gb_meta_data_t *meta)
{
  /* first multiplier block used as lookup table for updates */
  const mat_gb_block_t *mbl  = l + rbi*meta->ncb+shift;
  /* block to be updated */
  /* printf("rbi %u | cbi %u | ncb %u | nrb %u | shift %u\n",
   *     rbi, cbi, meta->ncb, meta->nrb_CD, shift); */
  mat_gb_block_t *ubl  = l + rbi*meta->ncb+cbi;
  /* row to be updated in dense format */
  bf_t *db  = (bf_t *)malloc((meta->bs*meta->bs) * sizeof(bf_t));

  load_dense_block_from_sparse(db, ubl, meta);

  update_dense_block_sparse(db, u+cbi, mbl, ubl->nr, meta);

  update_block(&ubl, db, meta);
  free(db);

}

static inline void sparse_update_lower_block_by_upper_block(mat_gb_block_t *l,
    const mat_gb_block_t *u, const nelts_t shift, const nelts_t rbi,
    const nelts_t cbi, const mat_gb_meta_data_t *meta)
{
  nelts_t i;

  /* first multiplier block used as lookup table for updates */
  const mat_gb_block_t *mbl  = l + rbi*meta->ncb+shift;
  /* block to be updated */
  /* printf("rbi %u | cbi %u | ncb %u | nrb %u | shift %u\n",
   *     rbi, cbi, meta->ncb, meta->nrb_CD, shift); */
  mat_gb_block_t *ubl  = l + rbi*meta->ncb+cbi;
  /* row to be updated in dense format */
  bf_t *dr  = (bf_t *)malloc(meta->bs * sizeof(bf_t));

  mat_gb_block_t *nbl = generate_mat_gb_block(meta, ubl->nr);

  /* printf("cbi %u | rbi %u\n", cbi, rbi); */
  for (i=0; i<ubl->nr; ++i) {
    /* load dense row to be updated */
    load_dense_row_for_update(dr, i, ubl, meta);
    /* load_dense_row_for_update_from_sparse(dr, i, ubl, meta); */

    /* find corresponding row and multiplier */

    /* we know already that u[cbi] is not the empty block,
     * so it is either sparse or dense */
    if (u[cbi].len != NULL)
      update_dense_row_sparse(dr, i, u+cbi, mbl, meta);
    else
      update_dense_row_dense(dr, i, u+cbi, mbl, meta);

    /* write updated row to new storage holders */
    write_updated_row_to_sparse_format(nbl, dr, i, meta);
  }

  set_updated_block(&ubl, nbl);
  
  free(dr);
}

static inline void compute_multiples_in_first_block(bf_t *dr,
    const mat_gb_block_t *bl, const mat_gb_meta_data_t *meta)
{
  nelts_t i, j, k;

  /* printf("bl->nr %u | bl->len[1] %u\n", bl->nr, bl->len[1]);
   * printf("\n ---------------start------------------------------\n");
   * for (i=0; i<bl->nr; ++i) {
   *   for (j=bl->len[i]; j<bl->len[i+1]; ++j) {
   *   printf("%u at %u | ", bl->val[j], bl->pos[j]);
   *   }
   *   printf("\n");
   * }
   * printf("\n- -\n");
   * for (i=0; i<bl->nr; ++i)
   *   printf("%lu ", dr[i]);
   * printf("\n"); */
  for (i=0; i<bl->nr; ++i) {
    /* printf("%u / %u -- %lu\n", i, bl->nr, dr[i]); */
    if (dr[i] != 0) {
      dr[i] = dr[i] % meta->mod;
      if (dr[i] != 0) {
        const bf_t mul  = (bf_t)meta->mod - dr[i];
        /* printf("mul %u\n", mul); */
        const nelts_t ri  = i;
        const nelts_t shift = (bl->len[ri+1]-bl->len[ri]-1) % 4;
        for (j=bl->len[ri]+1; j<bl->len[ri]+shift+1; ++j) {
          dr[bl->pos[j]] +=  mul * bl->val[j];
        }
        for (; j<bl->len[ri+1]; j=j+4) {
          dr[bl->pos[j]] +=  mul * bl->val[j];
          dr[bl->pos[j+1]] +=  mul * bl->val[j+1];
          dr[bl->pos[j+2]] +=  mul * bl->val[j+2];
          dr[bl->pos[j+3]] +=  mul * bl->val[j+3];
        }
      }
    }
  }
  /* printf("\n- -\n");
   * for (int ii=0; ii<bl->nr; ++ii)
   *   printf("%lu ", dr[ii]);
   * printf("\n");
   * }
   * printf("-----------------done----------------------------\n"); */
}

static inline void sparse_update_first_block_by_upper_block(mat_gb_block_t *l,
    const mat_gb_block_t *u, const nelts_t shift, const nelts_t rbi,
   const mat_gb_meta_data_t *meta)
{
  nelts_t i;

  /* block to be updated */
  mat_gb_block_t *ubl  = l + rbi*meta->ncb+shift;
  /* row to be updated in dense format */
  bf_t *dr  = (bf_t *)malloc(meta->bs * sizeof(bf_t));

  mat_gb_block_t *nbl = generate_mat_gb_block(meta, ubl->nr);

  for (i=0; i<ubl->nr; ++i) {
    /* load dense row to be updated */
    load_dense_row_for_update(dr, i, ubl, meta);
    /* load_dense_row_for_update_from_sparse(dr, i, ubl, meta); */

    /* find corresponding row and multiplier */

    /* we know already that u[cbi] is not the empty block,
     * so it is either sparse or dense */
    /* printf("shift %u || %p --> %p\n", shift, u, u+shift); */
    compute_multiples_in_first_block(dr, u+shift, meta);

    /* write updated row to new storage holders */
    write_updated_row_to_sparse_format(nbl, dr, i, meta);
  }

  set_updated_block(&ubl, nbl);
  
  free(dr);
}

static inline void update_lower_by_upper_row_block_not_prereduced(
    mat_gb_block_t *l, const mat_gb_block_t *u, const nelts_t shift,
    const mat_gb_meta_data_t *meta, const int t)
{
#pragma omp parallel num_threads(t)
  {
#pragma omp single nowait
    {
      /* the first block is used for updated the remaining ones,
       * i.e. we start at block index i=1 */
      for (nelts_t i=0; i<meta->nrb_CD; ++i) {
        /* need to look at the first block in each row in
        * order to decide which algorithm to be chosen */
        /* printf("NEXT REDUCTION STEP %p %u\n", l[i*meta->ncb+shift].len, i); */
        if (l[i*meta->ncb+shift].val != NULL) {
          sparse_update_first_block_by_upper_block(l, u, shift, i, meta);
          for (nelts_t j=shift+1; j<meta->ncb; ++j) {
            /* printf("we reduce j %u of length %u\n", j, l[i*meta->ncb+j].len[l[i*meta->ncb+j].nr]); */
            if (u[j].val != NULL) {
              /* reduce first block */
#pragma omp task
              {
                sparse_update_lower_block_by_upper_block(l, u, shift, i, j, meta);
              }
            }
            /* else {
             *   printf("lower block NULL\n");
             * } */
            /* printf("reduced j %u of length %u\n", j, l[i*meta->ncb+j].len[l[i*meta->ncb+j].nr]); */
          }
        } else {
          continue;
        }
      }
    }
  }
    /* remove the first block in each block row */
    for (nelts_t i=0; i<meta->nrb_CD; ++i) {
      /* printf("len before %u * %u + %u || %p\n", i, meta->ncb, shift, l[i*meta->ncb+shift].len); */
      free_mat_gb_block(l+i*meta->ncb+shift);
      /* printf("len after %p\n", l[i*meta->ncb+shift].len); */
    }

/* at the moment we only work with sparse blocks */
#if 1
#pragma omp parallel num_threads(t)
  {
#pragma omp single nowait
    {
      for (nelts_t i=0; i<meta->nrb_CD; ++i) {
#pragma omp task
        {
          /* check density of blocks */
          /* printf("adjusting C|D blocks\n"); */
          /* adjust_block_row_types(l+i*meta->ncb, meta); */
          adjust_block_row_types_including_dense(l+i*meta->ncb, meta);
        }
      }
    }
  }
#endif
}

static inline void update_lower_by_upper_row_block(mat_gb_block_t *l,
    const mat_gb_block_t *u, const nelts_t shift,
    const mat_gb_meta_data_t *meta, const int t)
{
#pragma omp parallel num_threads(t)
  {
#pragma omp single nowait
    {
      /* the first block is used for updated the remaining ones,
       * i.e. we start at block index i=1 */
      for (nelts_t j=shift+1; j<meta->ncb; ++j) {
        /* printf("we reduce j %u of length %u\n", j, l[i*meta->ncb+j].len[l[i*meta->ncb+j].nr]); */
        if (u[j].val != NULL) {
          for (nelts_t i=0; i<meta->nrb_CD; ++i) {
            /* need to look at the first block in each row in
            * order to decide which algorithm to be chosen */
            /* printf("NEXT REDUCTION STEP %p %u\n", l[i*meta->ncb+shift].len, i); */
            if (l[i*meta->ncb+shift].len != NULL) {
#pragma omp task
              {
                sparse_update_lower_block_by_upper_block(l, u, shift, i, j, meta);
              }
            }
            /* else {
             *   printf("lower block NULL\n");
             * } */
            /* printf("reduced j %u of length %u\n", j, l[i*meta->ncb+j].len[l[i*meta->ncb+j].nr]); */
          }
        } else {
          continue;
        }
      }
    }
  }
    /* remove the first block in each block row */
    for (nelts_t i=0; i<meta->nrb_CD; ++i) {
      /* printf("len before %u * %u + %u || %p\n", i, meta->ncb, shift, l[i*meta->ncb+shift].len); */
      free_mat_gb_block(l+i*meta->ncb+shift);
      /* printf("len after %p\n", l[i*meta->ncb+shift].len); */
    }

/* at the moment we only work with sparse blocks */
#if 1
#pragma omp parallel num_threads(t)
  {
#pragma omp single nowait
    {
      for (nelts_t i=0; i<meta->nrb_CD; ++i) {
#pragma omp task
        {
          /* check density of blocks */
          /* printf("adjusting C|D blocks\n"); */
          adjust_block_row_types(l+i*meta->ncb, meta);
        }
      }
    }
  }
#endif
}
#endif
