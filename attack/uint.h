#ifndef UINT_H
#define UINT_H

#include <stdbool.h>
#include <stdint.h>

#include "params.h"

extern const uint_c uint_1;

typedef struct uint128
{
	uint64_t Hi;
	uint64_t Lo;
} uint128;

void uint_set(uint_c *x, uint64_t y);

bool uint_bit(uint_c const *x, uint64_t k);

bool uint_add3(uint_c *x, uint_c const *y, uint_c const *z); /* returns carry */
bool uint_sub3(uint_c *x, uint_c const *y, uint_c const *z); /* returns borrow */

void uint_mul3_64(uint_c *x, const uint_c* const y, const uint64_t z);

void mult64to128(const uint64_t op1, const uint64_t op2, uint128* out);

void add128(uint128* out, const uint128* const op1, const uint128* const op2);

void shiftright128(uint128* op1, unsigned shift, uint128* out);

#endif
