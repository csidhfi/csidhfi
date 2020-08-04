
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "hal.h"
#include "uint.h"
#include "fp.h"
#include "mont.h"
#include "csidh.h"
#include "randombytes.h"
#include "constants.h"

#if defined(ATTACK1) || defined(ATTACK1_D) || defined(ATTACK2) || defined(ATTACK2_D)
#define REDUCEDSK
#endif

extern const unsigned primes[num_primes];

#if defined(ATTACK1) || defined(ATTACK1_D) || defined(ATTACK2) || defined(ATTACK2_D)
void uart_puts(char *s)
{
    while (*s)
    {
        putch(*(s++));
    }
}
#endif

#if defined(ATTACK1) || defined(ATTACK1_D)
const unsigned primes[num_primes] = {5, 3, 349, 347, 337, 331, 317, 313, 311,
                                     307, 293, 283, 281, 277, 271, 269, 263, 257, 251, 241, 239, 233, 229,
                                     227, 223, 211, 199, 197, 193, 191, 181, 179, 173, 167, 163, 157, 151,
                                     149, 139, 137, 131, 127, 113, 109, 107, 103, 101, 97, 89, 83, 79, 73,
                                     71, 67, 61, 59, 53, 47, 43, 41, 37, 31, 29, 23, 19, 17, 13, 11, 7, 353, 359,
                                     587, 373, 367};
#elif defined(ATTACK2) || defined(ATTACK2_D)
const unsigned primes[num_primes] = {359, 353, 349, 347, 337, 331, 317, 313, 311,
                                     307, 293, 283, 281, 277, 271, 269, 263, 257, 251, 241, 239, 233, 229,
                                     227, 223, 211, 199, 197, 193, 191, 181, 179, 173, 167, 163, 157, 151,
                                     149, 139, 137, 131, 127, 113, 109, 107, 103, 101, 97, 89, 83, 79, 73,
                                     71, 67, 61, 59, 53, 47, 43, 41, 37, 31, 29, 23, 19, 17, 13, 11, 7, 5, 3,
                                     587, 373, 367};
#endif

const public_key base = {0}; /* A = 0 */

const uint_c p_plus_one = {{0x1b81b90533c6c87c, 0xc2721bf457aca835, 0x516730cc1f0b4f25, 0xa7aac6c567f35507, 0x5afbfcc69322c9cd, 0xb42d083aedc88c42, 0xfc8ab0d15e3e4c4a, 0x65b48e8f740f89bf}};

//TODO remove
//int8_t error = 0;

extern unsigned long long overflowcnt;
extern unsigned long long startcnt;

/* get priv[pos] in constant time  */
int32_t lookup(size_t pos, int8_t const *priv)
{
    int b;
    int8_t r = priv[0];
    for (size_t i = 1; i < num_primes; i++)
    {
        b = isequal(i, pos);
        //ISEQUAL(i, pos, b);
        //b = (uint8_t)(1-((-(i ^ pos)) >> 31));
        cmov(&r, &priv[i], b);
        //CMOV(&r, &priv[i], b);
    }
    return r;
}

/* check if a and b are equal in constant time  */
uint32_t isequal(uint32_t a, uint32_t b)
{
    //size_t i;
    uint32_t r = 0;
    unsigned char *ta = (unsigned char *)&a;
    unsigned char *tb = (unsigned char *)&b;
    r = (ta[0] ^ tb[0]) | (ta[1] ^ tb[1]) | (ta[2] ^ tb[2]) | (ta[3] ^ tb[3]);
    r = (-r);
    r = r >> 31;
    return (int)(1 - r);
}

/* decision bit b has to be either 0 or 1 */
void cmov(int8_t *r, const int8_t *a, uint32_t b)
{
    uint32_t t;
    b = -b; /* Now b is either 0 or 0xffffffff */
    t = (*r ^ *a) & b;
    *r ^= t;
}

void csidh_private(private_key *priv, const int8_t *max_exponent)
{
    memset(&priv->e, 0, sizeof(priv->e));
    for (size_t i = 0; i < num_primes;)
    {
        int8_t buf[64];
        randombytes((unsigned char *)buf, sizeof(buf));
        for (size_t j = 0; j < sizeof(buf); ++j)
        {
            if (buf[j] <= max_exponent[i] && buf[j] >= -max_exponent[i])
            {
                priv->e[i] = lookup(j, buf);
                if (++i >= num_primes)
                    break;
            }
        }
    }
}

/* compute [(p+1)/l] P for all l in our list of primes. */
/* divide and conquer is much faster than doing it naively,
 * but uses more memory. */
static void cofactor_multiples(proj *P, const proj *A, size_t lower,
                               size_t upper)
{
    assert(lower < upper);

    if (upper - lower == 1)
        return;

    size_t mid = lower + (upper - lower + 1) / 2;

    uint_c cl = uint_1, cu = uint_1;
    for (size_t i = lower; i < mid; ++i)
        uint_mul3_64(&cu, &cu, primes[i]);
    for (size_t i = mid; i < upper; ++i)
        uint_mul3_64(&cl, &cl, primes[i]);

    xMUL(&P[mid], A, &P[lower], &cu);
    xMUL(&P[lower], A, &P[lower], &cl);

    cofactor_multiples(P, A, lower, mid);
    cofactor_multiples(P, A, mid, upper);
}

/* never accepts invalid keys. */
bool validate(public_key const *in)
{
    const proj A = {in->A, fp_1};

    do
    {

        proj P[num_primes];
        fp_random(&P->x);
        P->z = fp_1;

        /* maximal 2-power in p+1 */
        xDBL(P, &A, P);
        xDBL(P, &A, P);

        cofactor_multiples(P, &A, 0, num_primes);

        uint_c order = uint_1;

        for (size_t i = num_primes - 1; i < num_primes; --i)
        {

            /* we only gain information if [(p+1)/l] P is non-zero */
            if (memcmp(&P[i].z, &fp_0, sizeof(fp)))
            {

                uint_c tmp;
                uint_set(&tmp, primes[i]);
                xMUL(&P[i], &A, &P[i], &tmp);

                if (memcmp(&P[i].z, &fp_0, sizeof(fp)))
                    /* P does not have order dividing p+1. */
                    return false;

                uint_mul3_64(&order, &order, primes[i]);

                if (uint_sub3(&tmp, &four_sqrt_p, &order)) /* returns borrow */
                    /* order > 4 sqrt(p), hence definitely supersingular */
                    return true;
            }
        }

        /* P didn't have big enough order to prove supersingularity. */
    } while (1);
}

/* compute x^3 + Ax^2 + x */
/*
static void montgomery_rhs(fp *rhs, fp const *A, fp const *x) {
	fp tmp;
	*rhs = *x;
	fp_sq1(rhs);
	fp_mul3(&tmp, A, x);
	fp_add2(rhs, &tmp);
	fp_add2(rhs, &fp_1);
	fp_mul2(rhs, x);
}
*/

/* generates curve points */
void elligator(proj *P, proj *Pd, const fp *A)
{

    fp u2m1, tmp, rhs;
    bool issquare;

#if defined(ATTACK1) || defined(ATTACK1_D) || defined(ATTACK2) || defined(ATTACK2_D)
    fp u2 = {{0xf73849b0ce4e064b, 0x94bbfb03237b4a47, 0x467d743c736b034f, 0xb3fee59267e9b9e8, 0x036bafb7d4af3814, 0x05b62c28c87084ce, 0x620a625431f0111e, 0x03d7f790ac52fd83}};
#else
    fp u2;
    fp_random(&u2);
#endif

    fp_sq1(&u2);                // u^2
    fp_sub3(&u2m1, &u2, &fp_1); // u^2 - 1
                                //uart_puts("3\n");
    fp_sq2(&tmp, &u2m1);        // (u^2 - 1)^2
    fp_sq2(&rhs, A);            // A^2
    fp_mul2(&rhs, &u2);         // A^2u^2
    fp_add2(&rhs, &tmp);        // A^2u^2 + u(u^2 - 1)^2
    fp_mul2(&rhs, A);           // (A^2u^2 + u(u^2 - 1)^2)A
    fp_mul2(&rhs, &u2m1);       // (A^2u^2 + u(u^2 - 1)^2)A(u^2 - 1)
                                // uart_puts("4\n");
    fp_set(&P->x, 0);
    fp_add2(&P->x, A);
    fp_set(&P->z, 0);
    fp_add2(&P->z, &u2m1);
    fp_set(&Pd->x, 0);
    fp_sub2(&Pd->x, A);
    fp_mul2(&Pd->x, &u2);
    fp_set(&Pd->z, 0);
    fp_add2(&Pd->z, &u2m1);

    issquare = fp_issquare(&rhs);
    fp_cswap(&P->x, &Pd->x, !issquare);
    fp_cswap(&P->z, &Pd->z, !issquare);
}

#ifdef CM
// Input: A supersingular curve E : Cy^2 = Cx^3 + Ax^2 + Cx over F_p,  and an element u in {2,..., p−1}.
// Output: A pair of points T+ in E[pi - 1] and T- in E[pi + 1], error variable error.
/* generates curve points */
static bool new_elligator(proj *P, proj *Pd, const proj *A, const fp *u, int8_t *ps)
{
    bool error = 0;
    bool b = false, s1, s2;
    fp a, tmp0, tmp1, tmp2, u2m1, xz;
    //fp_set(&a, 0);
    // A->x = A and A->z = C
    b = fp_cmp_ct(&A->x, &fp_0); // b <- isequal(A, 0)
    //fp_cadd(&a, &fp_0, u, !b);                // a <- cadd(0, bu)
    fp_cset(&a, u, !b);           // a <- cadd(0, bu)
    fp_sq2(&tmp0, u);             // u^2
    fp_sub3(&u2m1, &tmp0, &fp_1); // u^2 - 1
    fp_mul3(&P->z, &u2m1, &A->z); // Z = C(u^2 - 1)
    fp_mul3(&tmp0, &P->z, &a);    // aC(u^2 - 1)
    fp_add3(&P->x, &A->x, &tmp0); // X = A +aC(u^2 −1)
    fp_cset(&Pd->z, &P->z, 1);    // Z' = C(u^2 - 1)

    //fp_sub3(&tmp0, u, &fp_1);                   // u - 1
    fp_mul3(&tmp2, &a, &A->z);     // aC
    fp_mul3(&tmp1, &u2m1, &tmp2);  // aC (u^2 − 1)
    fp_sq2(&tmp0, u);              // u^2
    fp_mul3(&tmp2, &A->x, &tmp0);  // Au^2
    fp_sub3(&tmp0, &fp_0, &tmp2);  // -Au^2
    fp_sub3(&Pd->x, &tmp0, &tmp1); // X' = −Au^2 −aC (u^2 −1)

    fp_sq2(&tmp0, &P->x);         // X^2
    fp_mul3(&tmp1, &A->z, &tmp0); // CX^2
    fp_mul3(&xz, &P->x, &P->z);   // XZ
    fp_mul3(&tmp0, &A->x, &xz);   // AXZ
    fp_sq2(&a, &P->z);            // Z^2
    fp_mul3(&tmp2, &A->z, &a);    // CZ^2
    fp_add3(&a, &tmp1, &tmp0);    // CX^2 + AXZ
    fp_add3(&tmp0, &a, &tmp2);    // CX^2 + AXZ + + CZ^2
    fp_mul3(&tmp1, &xz, &tmp0);   // XZ(CX^2 + AXZ + + CZ^2)

    s1 = fp_issquare(&tmp1); // s1 <- Legendre symbol(XZ(CX^2 + AXZ + + CZ^2), p)

    fp_sq2(&tmp0, &Pd->x);        // X'^2
    fp_mul3(&tmp1, &A->z, &tmp0); // CX'^2
    fp_mul3(&xz, &Pd->x, &Pd->z); // X'Z'
    fp_mul3(&tmp0, &A->x, &xz);   // AX'Z'
    fp_sq2(&a, &Pd->z);           // Z'^2
    fp_mul3(&tmp2, &A->z, &a);    // CZ'^2
    fp_add3(&a, &tmp1, &tmp0);    // CX'^2 + AX'Z'
    fp_add3(&tmp0, &a, &tmp2);    // CX'^2 + AX'Z' + + CZ'^2
    fp_mul3(&tmp1, &xz, &tmp0);   // X'Z'(CX'^2 + AX'Z' + + CZ'^2)

    s2 = fp_issquare(&tmp1); // s2 <- Legendre symbol(X'Z'(CX'^2 + AX'Z' + CZ'^2), p)

    // fp_cswap(&P->x, &Pd->x, !s1);
    // fp_cswap(&P->z, &Pd->z, !s1);

    error |= !(s1 ^ s2);

    *ps = !(s1 ^ 1); //ps stores information which point is stored in P

    error |= (*ps ^ !(s1 ^ 1)); //check that ps has not been manipulated
    return error;
}
#endif

/* constant-time. */
bool action(public_key *out, public_key const *in, private_key const *priv,
            uint8_t num_batches, int8_t const *max_exponent, unsigned int const num_isogenies, uint8_t const my)
{
    //factors k for different batches
    uint_c k[3] = {{{0x1b5933af628d005c, 0x9d4af02b1d7b7f56, 0x8977a8435092262a, 0xb86302ff54a37ca2, 0xd6e09db2af04d095, 0x5c73f, 0x0, 0x0}},
                 {{0xd97b8b6bc6f6be1c, 0x315872c44ea6e448, 0x1aae7c54fd380c86, 0x237ec4cf2da454a2, 0x3733f9e3d9fea1b4, 0x1fdc0e, 0x0, 0x0}},
                 {{0x629ea97b02169a84, 0xc4b9616a12d48d22, 0x492a10278ad7b45a, 0xc44ac4dce55b87f8, 0x9e12876886632d6e, 0xe0c0c5, 0x0, 0x0}}};

    uint_c p_order = {{0x24403b2c196b9323, 0x8a8759a31723c208, 0xb4a93a543937992b, 0xcdd1f791dc7eb773, 0xff470bd36fd7823b, 0xfbcf1fc39d553409, 0x9478a78dd697be5c, 0x0ed9b5fb0f251816}};

    int8_t ec = 0, m = 0;
    uint8_t count = 0;
    //uint8_t elligator_index = 0;
    uint8_t last_iso[3], bc, ss;
    proj P, Pd, K;
    uint_c cof, l;
    bool finished[num_primes] = {0};
#ifdef REDUCEDSK
    memset(finished, 1, sizeof(finished));
    finished[0] = 0;
    finished[1] = 0;
    // finished[2] = 0;
    // finished[3] = 0;
    // finished[4] = 0;
#endif

    int8_t e[num_primes] = {0};

#ifdef CM
    bool error = 0;
    (void)p_order;
    uint_c lastOrder;
    proj lastA = {in->A, fp_1};
#endif

    int8_t counter[num_primes] = {0};
    int8_t s, ps;
    unsigned int isog_counter = 0;

    //index for skipping point evaluations
    last_iso[0] = 72;
    last_iso[1] = 73;
    last_iso[2] = 71;

    memcpy(e, priv->e, sizeof(priv->e));

    memcpy(counter, max_exponent, sizeof(counter));

    proj A = {in->A, fp_1};

    #if defined(ATTACK1) || defined(ATTACK1_D) || defined(ATTACK2) || defined(ATTACK2_D)
    char str[100];
    #endif

#if defined(ATTACK1) || defined(ATTACK1_D)
    trigger_high();
#endif

    while (isog_counter < num_isogenies)
    {
        #if defined(ATTACK1) || defined(ATTACK1_D) || defined(ATTACK2) || defined(ATTACK2_D)
        uart_puts("1\n");
        #endif

        m = (m + 1) % num_batches;

        if (count == my * num_batches)
        { //merge the batches after my rounds
            m = 0;
            last_iso[0] = 73;   //doesn't skip point evaluations anymore after merging batches
            uint_set(&k[m], 4); //recompute factor k
            num_batches = 1;

            // no need for constant-time, depends only on randomness
            for (uint8_t i = 0; i < num_primes; i++)
            {
                if (counter[i] == 0)
                {
                    uint_mul3_64(&k[m], &k[m], primes[i]);
                }
            }
        }

#ifdef CM
        #if defined(ATTACK1) || defined(ATTACK1_D) || defined(ATTACK2) || defined(ATTACK2_D)
        fp u = {{0x9b9499cf4466e265, 0x1890b03f447fc2c2, 0x59200afefa8db817, 0x9d45d54e769e45a7, 0x97ef18c3efd52e4f, 0xf20393bd845656fe, 0xda07eea2333775f1, 0x1b808024fce7c39d}};
        #else
         fp u;
         fp_random(&u);
         while (compare(&u, &p_minus_1_halves) > 0)
             fp_random(&u);
        #endif
        error |= new_elligator(&P, &Pd, &A, &u, &ps);

#else

        if (memcmp(&A.x, &fp_0, sizeof(fp)))
        {

            elligator(&P, &Pd, &A.x);
        }
        else
        {
            fp_enc(&P.x, &p_order); // point of full order on E_a with a=0
            fp_sub3(&Pd.x, &fp_0, &P.x);
            P.z = fp_1;
            Pd.z = fp_1;
        }
#endif
        #if defined(ATTACK1) || defined(ATTACK1_D) || defined(ATTACK2) || defined(ATTACK2_D)
        uart_puts("2\n");
        #endif
        xMUL(&P, &A, &P, &k[m]);

        xMUL(&Pd, &A, &Pd, &k[m]);
        #if defined(ATTACK1) || defined(ATTACK1_D) || defined(ATTACK2) || defined(ATTACK2_D)
        uart_puts("3\n");
        #endif
#ifndef CM
        ps = 1; //initialized in elligator
#endif

        for (uint8_t i = m; i < num_primes; i = i + num_batches)
        {
#ifdef CM
            uint_set(&lastOrder, primes[i]);

#endif
            if (finished[i] == true)
            { //depends only on randomness
                continue;
            }
            else
            {
                #if defined(ATTACK1) || defined(ATTACK1_D) || defined(ATTACK2) || defined(ATTACK2_D)
                uart_puts("4\n");
                #endif
                cof = uint_1;
                for (uint8_t j = i + num_batches; j < num_primes; j = j + num_batches)
                {
                    if (finished[j] == false) //depends only on randomness
                        uint_mul3_64(&cof, &cof, primes[j]);
                }

                ec = lookup(i, e); //check in constant-time if normal or dummy isogeny must be computed

                bc = isequal(ec, 0);

                s = (uint8_t)ec >> 7;
                ss = !isequal(s, ps);

#ifdef CM
                error |= (ec ^ e[i]);

                error |= (bc ^ isequal(ec, 0));

                error |= (s ^ ((uint8_t)ec >> 7));

                error |= (ss ^ (s ^ ps));
#endif

                ps = s;

#ifdef CM
                error |= (ps ^ s);
#endif

                fp_cswap(&P.x, &Pd.x, ss);

                fp_cswap(&P.z, &Pd.z, ss);

                xMUL(&K, &A, &P, &cof);

                uint_set(&l, primes[i]);
                xMUL(&Pd, &A, &Pd, &l);
                #if defined(ATTACK1) || defined(ATTACK1_D) || defined(ATTACK2) || defined(ATTACK2_D)
                uart_puts("5\n");
                #endif

#ifdef CM
                fp_add3(&lastA.x, &A.x, &fp_0);
                fp_add3(&lastA.z, &A.z, &fp_0);
#endif

                if (memcmp(&K.z, &fp_0, sizeof(fp)))
                { //depends only on randomness
                    #if defined(ATTACK1) || defined(ATTACK1_D) || defined(ATTACK2) || defined(ATTACK2_D)
                    uart_puts("6\n");
                    #endif
#ifdef TWIST_ATTACK_CM
                    fp XZ, AC, XZ2, Z2, C2, C2XZ, ACXZ2, X2, C2X3Z, C2XZ3, check;

                    // 1M
                    fp_mul3(&XZ, &K.x, &K.z);           // XZ
                    // 4S
                    fp_sq2(&XZ2, &XZ);                  // XZ^2
                    fp_sq2(&X2, &K.x);                  // X^2
                    fp_sq2(&Z2, &K.z);                  // Z^2
                    fp_sq2(&C2, &A.z);                  // C^2
                    // 3M
                    fp_mul3(&C2XZ, &C2, &XZ);           // C^2XZ
                    fp_mul3(&AC, &A.x, &A.z);           // AC
                    fp_mul3(&ACXZ2, &AC, &XZ2);         // ACXZ2
                    // 2M
                    fp_mul3(&C2X3Z, &C2XZ, &X2);        // C^2X^3Z
                    fp_mul3(&C2XZ3, &C2XZ, &Z2);        // C^2XZ^3
                    // 2a
                    fp_add3(&check, &C2X3Z, &ACXZ2);    // C^2X^3Z + ACXZ2
                    fp_add3(&check, &check, &C2XZ3);    // C^2X^3Z + ACXZ2 + C^2XZ^3
                    // Legendre
                    error |= (s) != (fp_issquare(&check));

#endif
                    if (i == last_iso[m])
                    {
#ifdef CM
                        error |= lastxISOG(&A, &K, primes[i], bc); // doesn't compute the images of points
#else
                        lastxISOG(&A, &K, primes[i], bc); // doesn't compute the images of points
#endif
                    }
                    else
                    {
#ifdef ATTACK2
                        char c = 'A';
                        trigger_high();
#ifdef CM
                        error |= xISOG(&A, &P, &Pd, &K, primes[i], bc);
#else
                        xISOG(&A, &P, &Pd, &K, primes[i], bc);
#endif
                        trigger_low();
                        // wait for next "signal"
                        while ((c != 'n'))
                        {
                            c = getch();
                        }
                        c = 'A';
#else

#ifdef CM
                        error |= xISOG(&A, &P, &Pd, &K, primes[i], bc);
#else
                        xISOG(&A, &P, &Pd, &K, primes[i], bc);
#endif

#endif
                    }

                    e[i] = ec - (1 ^ bc) + (s << 1);
#ifdef CM
                    error |= (e[i] ^ (ec - (1 ^ bc) + (s << 1)));
#endif

                    counter[i] = counter[i] - 1;
                    isog_counter = isog_counter + 1;
                    #if defined(ATTACK1) || defined(ATTACK1_D) || defined(ATTACK2) || defined(ATTACK2_D)
                    sprintf(str, "isog_counter = %d, k = %d\n", isog_counter, primes[i]);
                    uart_puts(str);
                    #endif
                }
            }

            if (counter[i] == 0)
            { //depends only on randomness
                finished[i] = true;
                uint_mul3_64(&k[m], &k[m], primes[i]);
            }
        }

#ifdef CM
        // check P = 0 and Pd = 0 at the end of a batch run
        xMUL(&P, &lastA, &P, &lastOrder);
        error |= fp_cmp_ct(&P.z, &fp_0);
        error |= fp_cmp_ct(&Pd.z, &fp_0);
#endif

#ifndef CM
        fp_inv(&A.z);
        fp_mul2(&A.x, &A.z);
        A.z = fp_1;
#endif
        count = count + 1;
    }
#ifdef CM
    fp_inv(&A.z);
    fp_mul2(&A.x, &A.z);
#endif
    #if defined(ATTACK1) || defined(ATTACK1_D) || defined(ATTACK2) || defined(ATTACK2_D)
    uart_puts("7\n");
    #endif
    out->A = A.x;

#if defined(ATTACK1) || defined(ATTACK1_D)
    trigger_low();
#endif

#ifdef CM
    error |= (isog_counter ^ 404);
    for (int8_t i = 0; i < num_primes; i++)
        error |= (counter[i] ^ 0);
    error |= validate_cheaper(out);
    #if defined(ATTACK1) || defined(ATTACK1_D) || defined(ATTACK2) || defined(ATTACK2_D)
    sprintf(str, "error = %d\n", error);
    uart_puts(str);
    #endif
    return error;
#else
        return 0;
#endif

}

#ifdef CM
bool validate_cheaper(const public_key *in)
{
    const proj A = {in->A, fp_1};
    #if defined(ATTACK1) || defined(ATTACK1_D) || defined(ATTACK2) || defined(ATTACK2_D)
    proj P = {{{0x9b9499cf4466e265, 0x1890b03f447fc2c2, 0x59200afefa8db817, 0x9d45d54e769e45a7, 0x97ef18c3efd52e4f, 0xf20393bd845656fe, 0xda07eea2333775f1, 0x1b808024fce7c39d}}, {{0xc8fc8df598726f0a, 0x7b1bc81750a6af95, 0x5d319e67c1e961b4, 0xb0aa7275301955f1, 0x4a080672d9ba6c64, 0x97a5ef8a246ee77b, 0x06ea9e5d4383676a, 0x3496e2e117e0ec80,}}};
    #else
     proj P;
     fp_random(&P.x);
     P.z = fp_1;
    #endif

    xMUL(&P, &A, &P, &p_plus_one);
    return fp_cmp_ct(&P.z, &fp_0);
}
#endif

/* includes public-key validation. */
bool csidh(public_key *out, public_key const *in, private_key const *priv,
           uint8_t const num_batches, int8_t const *max_exponent, unsigned int const num_isogenies, uint8_t const my)
{
    int8_t error;
    /*
	if (!validate(in)) {
		fp_random(&out->A);
		return false;
	}
    */
    error = action(out, in, priv, num_batches, max_exponent, num_isogenies, my);
#ifdef CM
    fpcmov(&out->A, &fp_0, error);
#endif

    return error;
}
