#ifndef MONT_H
#define MONT_H

#include "params.h"

void xDBL(proj *Q, proj const *A, proj const *P);
void xADD(proj *S, proj const *P, proj const *Q, proj const *PQ);
void xDBLADD(proj *R, proj *S, proj const *P, proj const *Q, proj const *PQ, proj const *A);
void xMUL(proj *Q, proj const *A, proj const *P, uint_c const *k);
bool xISOG(proj *A, proj *P, proj *Pd, proj *K, uint64_t k, int mask);
bool lastxISOG(proj *A, proj const *K, uint64_t k, int bit);
void exp_by_squaring_(fp* x, fp* y, uint64_t exp);

#endif
