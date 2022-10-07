/* nqueens adapted from Kenji Kise's "simple" program */
/* The bit operations in 'new_queen()' make the difference */
/* I introduced the 'up' direction to clarify the backtracking - it is not necessary */

#include <stdio.h>
#include <stdlib.h>

struct nqstate { 
    int cdt, mid, le, ri; 
};
#define lsbit(x) ((x) & -(x))
void
new_queen(int cand, int L, struct nqstate *s) {

    int queen = lsbit(cand);
    s[L]  .cdt = cand     ^ queen;
    s[L+1].mid = s[L].mid ^ queen;
    s[L+1].le  =(s[L].le  | queen) << 1;
    s[L+1].ri  =(s[L].ri  | queen) >> 1;
}
long
nqueens_upflag(int N, int L, struct nqstate *s) {
    
    int cand, up = 1;
    long sum = 0;
    
    while(L > -1) {
        if (up) {
            if ((cand = s[L].mid & ~(s[L].le | s[L].ri)))
                if (L < N - 1)  
                    prep: new_queen(cand, L++, s);
                else
                    sum++, up = 0, L--;
            else 
                up = 0, L--;         
        }
    	else {
            if ((cand = s[L].cdt)) {up = 1; goto prep;}
            else L--;
        }
    }
    
    return sum;
}
int main(int argc, char *argv[]){
    int N = atoi(argv[1]);
    struct nqstate *nqs = calloc(N, sizeof*nqs); 
    nqs[0].mid = (1<<N) - 1;
    long n_solu = nqueens_upflag(N, 0, nqs); 
    printf("%12ld\n", n_solu);
    return 0;
}
