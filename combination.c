#include "combination.h"
#include <stdlib.h>

void Combination_create(struct Combination *c, int n, int r)
{
	int i;
	c->n = n;
	c->r = r;
	c->k = malloc(sizeof(*c->k) * r);
	for (i=0; i < r; i++) {
		c->k[i] = i;
	}
}

void Combination_destroy(struct Combination *c)
{
	free(c->k);
}

int Combination_next(struct Combination *c)
{
	int i, j;

	for (i=c->r-1; i >= 0; i--) {
		c->k[i]++;
		for (j=i+1; j<c->r; j++) c->k[j] = c->k[j-1]+1;
		if (c->k[c->r-1] != c->n) break;
		if (!i) return 0;
	}

	return 1;
}
