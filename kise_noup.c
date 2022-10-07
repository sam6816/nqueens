/* w/o explicit direction; "s[L].cand" is "r" in Kise's and serves directly 
   as up or down flag. In fact, this  amounts to Kise's version with other var names  */

#include <stdio.h>
#include <stdlib.h>

struct nqstate { 
    int cand, mid, le, ri; 
};
#define lsbit(x) ((x) & -(x))

long
nqueens_cand(int N, int L, struct nqstate *s) {
    
    int cand, TOP = N - 1;
    long sum = 0;
    
    for (;;) {
        if (s[L].cand) 
            if (L == TOP)   
                sum++, L--;
            else {
                int queen = lsbit(s[L].cand);
                s[L].cand ^= queen;
                s[L+1].mid = s[L].mid ^ queen;
                s[L+1].le  =(s[L].le  | queen) << 1;
                s[L+1].ri  =(s[L].ri  | queen) >> 1;
                L++;
                s[L].cand = s[L].mid & ~(s[L].le | s[L].ri);
            }
        else 
            if (!L--)
                break;
    }
    
    return sum;
}
int main(int argc, char *argv[]){
    int N = atoi(argv[1]);
    struct nqstate *nqs = calloc(N, sizeof*nqs); 
    nqs[0].cand = (1<<N) - 1;
    nqs[0].mid = (1<<N) - 1;
    long n_solu = nqueens_cand(N, 0, nqs); 
    printf("%12ld\n", n_solu);
    return 0;
}
