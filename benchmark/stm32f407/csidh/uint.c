
#include <stddef.h>

#include "params.h"
#include "uint.h"

/* assumes little-endian throughout. */

uint_c const uint_1 = {{1}};


void uint_set(uint_c *x, uint64_t y)
{
    x->c[0] = y;
    for (size_t i = 1; i < LIMBS; ++i)
        x->c[i] = 0;
}

bool uint_bit(uint_c const *x,  uint64_t k)
{
    return 1 & (x->c[k / 64] >> k % 64);
}


bool uint_add3(uint_c *x, uint_c const *y, uint_c const *z)
{
    bool c = 0;
    for (size_t i = 0; i < LIMBS; ++i) {
        uint64_t t;
        c = __builtin_add_overflow(y->c[i], c, &t);
        c |= __builtin_add_overflow(t, z->c[i], &x->c[i]);
    }
    return c;
}

bool uint_sub3(uint_c *x, uint_c const *y, uint_c const *z)
{
    bool b = 0;
    for (size_t i = 0; i < LIMBS; ++i) {
        uint64_t t;
        b = __builtin_sub_overflow(y->c[i], b, &t);
        b |= __builtin_sub_overflow(t, z->c[i], &x->c[i]);
    }
    return b;
}

inline void uint_mul3_64(uint_c *x, const uint_c* const y, const uint64_t z)
{
	uint128 c;
	c.Hi = 0;
	c.Lo = 0;
    uint128 t;
    t.Hi = 0;
    t.Lo = 0;
    for (size_t i = 0; i < LIMBS; ++i) {

    	if((y->c[i]>0) || (z>0)) {
    			mult64to128(y->c[i], z, &t);
    			add128(&t, &t, &c);

			    //c = t >> 64;
  	    	c.Lo = t.Hi;
  	    	c.Hi = 0;

			    //x->c[i] = t;
			    x->c[i] = t.Lo;
    	} else {

        	c.Lo = t.Hi;
        	c.Hi = 0;

			    //x->c[i] = t;
			    x->c[i] = t.Lo;

    	}



    }
//    uint64_t c = 0;
//    for (size_t i = 0; i < LIMBS; ++i) {
//        __uint128_t t = y->c[i] * (__uint128_t) z + c;
//        c = t >> 64;
//        x->c[i] = t;
//    }
}


inline void add128(uint128* out, const uint128* const op1, const uint128* const op2)
{
	uint64_t carry = (((op1->Lo & op2->Lo) & 1) + (op1->Lo >> 1) + (op2->Lo >> 1)) >> 63;
    out->Hi = op1->Hi + op2->Hi + carry;
    out->Lo = op1->Lo + op2->Lo;
}


inline void mult64to128(const uint64_t op1, const uint64_t op2, uint128* out)
{
    uint64_t u1 = (op1 & 0xffffffff);
    uint64_t v1 = (op2 & 0xffffffff);
    uint64_t t = (u1 * v1);
    uint64_t w3 = (t & 0xffffffff);
    uint64_t k = (t >> 32);

    //op1 >>= 32;
    t = ((op1 >> 32) * v1) + k;
    k = (t & 0xffffffff);
    uint64_t w1 = (t >> 32);

    //op2 >>= 32;
    t = (u1 * (op2 >> 32)) + k;
    k = (t >> 32);

    out->Hi = ((op1 >> 32)* (op2 >> 32)) + w1 + k;
    out->Lo = (t << 32) + w3;
}



void shiftright128(uint128* op1, unsigned shift, uint128* out)
{
	shift &= 127;

    if(shift != 0)
    {
        if(shift > 64)
        {
            out->Hi = op1->Hi >> (shift - 64);
            out->Lo = 0;
        }
        else if(shift < 64)
        {
            out->Lo = (op1->Lo >> shift) | (op1->Hi << (64 - shift));
            out->Hi = op1->Hi >> shift;
        }
        else
        {
        	out->Lo = op1->Hi;
            out->Hi = 0;
        }
    }
    else
    {
    	out->Hi = op1->Hi;
    	out->Lo = op1->Lo;
    }
}
