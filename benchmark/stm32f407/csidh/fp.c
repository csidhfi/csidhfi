
#include <stddef.h>
#include <string.h>

#include "params.h"
#include "uint.h"
#include "fp.h"
#include "randombytes.h"

#ifdef CM
void fpcmov(fp *dst, const fp *src, uint8_t b)
{
    b = -b;
    uint8_t *s = (uint8_t *)src;
    uint8_t *d = (uint8_t *)dst;
    for (size_t ii = 0; ii < sizeof(fp); ii++)
    {
        d[ii] ^= b & (d[ii] ^ s[ii]);
    }
}

bool fp_cmp_ct(const fp *n1, const fp *n2)
{

    const uint8_t *s1 = (const uint8_t *)n1;
    const uint8_t *s2 = (const uint8_t *)n2;

    uint8_t ret = 0;
    for (size_t i = 0; i < sizeof(fp); i++)
    {
        ret |= s1[i] ^ s2[i];
    }
    return (bool)ret;
}

// conditional set if(m>0) x = z else x = 0
void fp_cset(fp *x, const fp *z, uint64_t m)
{
    m = -m; /* Now m is either 0 or 0xffffffff */
    for (int i = 0; i < 8; ++i)
    {
        x->c[i] = z->c[i] & m;
    }
}

// conditional add 2C <- add(b .C, b. C)
void fp_cadd2(fp *x, const fp *y, const fp *z, uint64_t m)
{
    fp tmp0, tmp1;
    fp_cset(&tmp0, z, m);
    fp_cset(&tmp1, y, m);
    fp_add3(x, &tmp0, &tmp1);
}

// conditional add x = y + b . Z
void fp_cadd(fp *x, const fp *y, const fp *z, uint64_t m)
{
    fp tmp0;
    fp_cset(&tmp0, z, m);
    fp_add3(x, y, &tmp0);
}

// conditional sub x = y - b . Z
void fp_csub(fp *x, const fp *y, const fp *z, uint64_t m)
{
    fp tmp0;
    fp_cset(&tmp0, z, m);
    fp_sub3(x, y, &tmp0);
}

#endif

void fp_set(fp *x, uint64_t y)
{
    uint_set((uint_c *)x, y);
    fp_enc(x, (uint_c *)x);
}

static void reduce_once(uint_c *x)
{
    uint_c t;
    if (!uint_sub3(&t, x, &p))
        *x = t;
}

void fp_add3(fp *x, fp const *y, fp const *z)
{
    uint_add3((uint_c *)x, (uint_c *)y, (uint_c *)z);
    reduce_once((uint_c *)x);
}

void fp_add2(fp *x, fp const *y)
{
    fp_add3(x, x, y);
}

void fp_sub3(fp *x, fp const *y, fp const *z)
{
    if (uint_sub3((uint_c *)x, (uint_c *)y, (uint_c *)z))
        uint_add3((uint_c *)x, (uint_c *)x, &p);
}

void fp_sub2(fp *x, fp const *y)
{
    fp_sub3(x, x, y);
}

/* Montgomery arithmetic */

void fp_enc(fp *x, uint_c const *y)
{
    fp_mul3(x, (fp *)y, &r_squared_mod_p);
}

void fp_dec(uint_c *x, fp const *y)
{
    fp_mul3((fp *)x, y, (fp *)&uint_1);
}

void fp_mul3(fp *x, fp const *y, fp const *z)
{
    uint64_t t[LIMBS + 1] = {0};
    for (size_t k = 0; k < LIMBS; ++k)
    {
#define r(i) t[(k + (i)) % (LIMBS + 1)]

        uint64_t m = inv_min_p_mod_r * (y->c[k] * z->c[0] + r(0));
        bool c = 0, o = 0;
        uint128 u;
        for (size_t i = 0; i < LIMBS; ++i)
        {
            //__uint128_t u = (__uint128_t) m * p.c[i];
            mult64to128(m, p.c[i], &u);

            o = __builtin_add_overflow(r(i), o, &r(i));

            //o |= __builtin_add_overflow(r(i), (uint64_t) u, &r(i));
            o |= __builtin_add_overflow(r(i), u.Lo, &r(i));

            c = __builtin_add_overflow(r(i + 1), c, &r(i + 1));

            //c |= __builtin_add_overflow(r(i+1), (uint64_t) (u >> 64), &r(i+1));
            c |= __builtin_add_overflow(r(i + 1), u.Hi, &r(i + 1));
        }
        r(LIMBS) += o;

        c = o = 0;
        for (size_t i = 0; i < LIMBS; ++i)
        {
            //__uint128_t u = (__uint128_t) y->c[k] * z->c[i];
            mult64to128(y->c[k], z->c[i], &u);

            o = __builtin_add_overflow(r(i), o, &r(i));

            //o |= __builtin_add_overflow(r(i), (uint64_t) u, &r(i));
            o |= __builtin_add_overflow(r(i), u.Lo, &r(i));

            c = __builtin_add_overflow(r(i + 1), c, &r(i + 1));

            //c |= __builtin_add_overflow(r(i+1), (uint64_t) (u >> 64), &r(i+1));
            c |= __builtin_add_overflow(r(i + 1), u.Hi, &r(i + 1));
        }
        r(LIMBS) += o;
#undef r
    }

    for (size_t i = 0; i < LIMBS; ++i)
        x->c[i] = t[(LIMBS + i) % (LIMBS + 1)];

    reduce_once((uint_c *)x);
}

void fp_cswap(fp *x, fp *y, bool c)
{
    if (c)
    {
        fp_add3(x, x, y);
        fp_sub3(y, x, y);
        fp_sub3(x, x, y);
    }
}

void fp_mul2(fp *x, fp const *y)
{
    fp_mul3(x, x, y);
}

void fp_sq2(fp *x, fp const *y)
{
    fp_mul3(x, y, y);
}

void fp_sq1(fp *x)
{
    fp_sq2(x, x);
}

/* (obviously) not constant time in the exponent */
static void fp_pow(fp *x, uint_c const *e)
{
    fp y = *x;
    *x = fp_1;
    for (size_t k = 0; k < LIMBS; ++k)
    {
        uint64_t t = e->c[k];
        for (size_t i = 0; i < 64; ++i, t >>= 1)
        {
            if (t & 1)
                fp_mul2(x, &y);
            fp_sq1(&y);
        }
    }
}

void fp_inv(fp *x)
{
    fp_pow(x, &p_minus_2);
}

bool fp_issquare(fp *x)
{
    fp_pow(x, &p_minus_1_halves);
    return !memcmp(x, &fp_1, sizeof(fp));
}

void fp_random(fp *x)
{
    while (1)
    {
        randombytes((unsigned char *)x, sizeof(fp));
        uint64_t m = ((uint64_t)1 << pbits % 64) - 1;
        x->c[LIMBS - 1] &= m;

        for (size_t i = LIMBS - 1; i < LIMBS; --i)
            if (x->c[i] < p.c[i])
                return;
            else if (x->c[i] > p.c[i])
                break;
    }
}
