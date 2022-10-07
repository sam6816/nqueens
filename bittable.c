/* N-Queens counting with threats as bits in a lookup table */
/* Slightly faster than the non-bit "cumulative threats": N12 in 0.045s */

#include <stdio.h>
#include <stdlib.h>

/* N is board dimension, global, run-time */
int N;
/* Array of N*N integers holding the bits of N queens and their threats */
int *qtbits;
 
/* Fill the array "board" with queens and threats */
void init_qtbits (int *b) {
	int i, j, base;
	int q, thlo, thhi;
	for (i = 0; i < N; i++) { 
		base = i * N;			 			/* Queen */
		q = b[base] = 1 << i;
		thlo = thhi = q;		 			/* Threats */
		for (j = 1; j < N; j++) {
			b[base+j] = q | (thlo >>= 1);
			if (thhi < 1<<N-1)
				b[base+j] |= (thhi <<= 1);
		}
	}
}
int
next_node(int q, int thr) {
	while (++q < N) 
		if (!(qtbits[q*N] & thr))
			return q;
	return -1;
}

int
next_state(int LVL, int *nodes ) {
	int thr = 0, i;
	for (i = 0; i <= LVL; i++) 
		thr |= qtbits[ nodes[LVL-i]*N + i + 1 ];
	return thr;
	
}

void nqb_test(void) {
	
	qtbits = malloc(N * N * sizeof*qtbits);
	init_qtbits(qtbits);	

	int nodes[N], 
		states[N],
		i;
	for (i = 0; i < N; i++)
		nodes[i] = -1;
	states[0] = 0;
	int LVL = 0, cnt = 0;
	while (LVL >= 0) {
		nodes[LVL] = next_node(nodes[LVL], states[LVL]);
		if (nodes[LVL] == -1) 
			LVL--;
		else
			if (LVL == N-1) {
				cnt++;
				nodes[LVL--] = -1;
			}
			else {
				states[LVL+1] = next_state(LVL, nodes);
				LVL++;
			}
	}

	printf("%d \n", cnt);
	
}

int main(int argc, char **argv) {
	if (argc >= 2) 
		if ((N = atoi(argv[1])) > 0 && N < 32) 
			nqb_test();
	return 0;
}
