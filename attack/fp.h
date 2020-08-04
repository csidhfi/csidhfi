#ifndef FP_H
#define FP_H

#include <stdbool.h>

#include "params.h"

void fp_set(fp *x, uint64_t y);
void fp_cswap(fp *x, fp *y, bool c);

void fp_enc(fp *x, uint_c const *y); /* encode to Montgomery representation */
void fp_dec(uint_c *x, fp const *y); /* decode from Montgomery representation */

void fp_add2(fp *x, fp const *y);
void fp_sub2(fp *x, fp const *y);
void fp_mul2(fp *x, fp const *y);

void fp_add3(fp *x, fp const *y, fp const *z);
void fp_sub3(fp *x, fp const *y, fp const *z);
void fp_mul3(fp *x, fp const *y, fp const *z);

void fp_sq1(fp *x);
void fp_sq2(fp *x, fp const *y);
void fp_inv(fp *x);
bool fp_issquare(fp *x); /* destroys input! */

void fp_random(fp *x);

#ifdef CM

void fp_cadd(fp *x, const fp *y, const fp *z, uint64_t m);
void fp_csub(fp *x, const fp *y, const fp *z, uint64_t m);

#ifdef CM
static inline int compare(const fp *x, const uint_c *y)
{
    const uint8_t *s1 = (const uint8_t *) x;
    const uint8_t *s2 = (const uint8_t *) y;

    for(int i = sizeof(fp) - 1; i >= 0; i--)
    {
		if (s1[i] != s2[i])
            return s1[i] > s2[i] ? 1 : -1;
	}
	return 0;
}
#endif

void fp_cset(fp *x, const fp *z, uint64_t m);
void fp_cadd2(fp *x, const fp *y, const fp *z, uint64_t m);
bool fp_cmp_ct(const fp *n1, const fp *n2);
void fpcmov(fp *dst, const fp *src, uint8_t b);

#endif

#endif
