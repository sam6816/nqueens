/* Segmented BFS N-Queens with openmp */
// N14 in 45 ms, N16 in 1.2, N17 in 8.6 seconds 
// N14 single thread: 0.150 s 
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* 'mid' is available columns, 'le' and 'ri' are diagonal threats, cumulated */
/* the resulting candidates are a simple bitop, could also be a fourth field */
struct nqstate { 
    int mid, le, ri; 
};
#define lsbit(x) ((x) & -(x))

/* Write zero to N elems into 't' from elem 'i' from 's' */
int
gen_single(struct nqstate * s, int i, struct nqstate * t, int j) {

    int cand = s[i].mid & ~(s[i].le | s[i].ri),
        queen;
    while ((queen=lsbit(cand))) {

        cand ^= queen;

        t[j].mid =  s[i].mid ^ queen;
        t[j].le  = (s[i].le|queen) << 1;
        t[j].ri  = (s[i].ri|queen) >> 1;

        if (t[j].mid & ~(t[j].le | t[j].ri))  
            j++;
    }
    return j;
}

/* Some small branches/segments grow irregularly at medium multipliers (4.0-1.0) */
/* N17 has very big small branches -> better test + realloc */
#define FIX  10  // for small N
#define FIXS 1.2  // N17 needs 1.5 and does one realloc with 1.2
/* L3 size is still best, but LIM does not matter that much - never steady */
#define LIM 32000

/* Generate solutions by rotating 's' and 'gen' until last board row, or array gets too big */
int
do_nq_gen_limit(struct nqstate **sh, int sz, float *multip, int *nrows) { 

    int j, debut = 1;
    struct nqstate *gen, *s = *sh;
   
    for (;;) {
        int E = *multip * sz * FIXS + FIX;
        gen = malloc(E * sizeof*gen); 

        for (int i = j = 0; i < sz; i++) { 
            if (j > E - 10) {
                E *= 1.4;
                gen = realloc(gen, E * sizeof*gen);
                //printf("[%d] BIGJ, realloc! %p %d %d %f\n", omp_get_thread_num(), gen, *nrows, sz, *multip); 
            }
            j = gen_single(s, i, gen, j);            
        }
        if (!--*nrows) {
            free(gen);
            break;
        }  
        if (!j) { 
            //printf("ZEROJ low LIM and big N - dead segment [%d] nr %d sz %d %f\n", omp_get_thread_num(), *nrows, sz, *multip); 
            free(gen);
            break;
        }
        *multip = (float)j / sz;

        if (j > LIM && *multip > 2.0) {
            *sh = gen;
            break;
        }
        sz = j;
        if (debut)
            debut = 0; 
        else
            free(s); 
        s = gen;
    }
    if (!debut)
        free(s);
    return j;
}

/* Work-Block. Stack element for saved params plus chunk divisor and free flag */
struct wbloc { 
    struct nqstate *nqptr; 
    int sz, nrows;
    float multip;
    int chdiv, f;
};

/* Even big N and low LIM only reach third stack level */
#define SPMAX 10
/* Manage blocks on a stack. Take chunk (segment) and count solutions, 
   or push a new big block on the 'wblocs' stack.                   */
long
sum_wbloc(struct wbloc *wblocs) {

    int sz, nrows;
    float multip; 
    int sp = 0;
    long sum = 0;
    while (sp > -1) { 
        struct wbloc *w = &wblocs[sp];
        sz = w->sz / w->chdiv--;
        w->sz -= sz;
        struct nqstate *s = w->nqptr + w->sz;
        multip = w->multip, nrows = w->nrows;

        sz = do_nq_gen_limit(&s, sz, &multip, &nrows);
        if (!w->sz) {
            if (w->f)  
                free(w->nqptr);
            sp--;
        }
        if (nrows && sz) {
            int chdiv =  sz * multip * multip / LIM;
            wblocs[++sp] = (struct wbloc) {.nqptr=s, .sz=sz, .nrows=nrows, .multip=multip, .chdiv=chdiv, .f=1};
            //printf("[%d] WBLOC added at sp %d nr %2d split: %8d/%d m %2.1f %p\n", omp_get_thread_num(), sp, nrows, sz, chdiv, multip, s);
            continue;
        }
        //printf("[%d] SUM at %d sum: %ld m %f\n", omp_get_thread_num(), sp, sum, multip);
        sum += sz;
    
    }
    return sum;
}
/* Start generating using 'wblocs' for nesting.  
   A first manual run is used for parallelization, and is where the symmetric half can be suppressed */
/* Finally do use basic symmetry and do only half (plus uneven middle) - this gives just 8 unequal :( jobs for N16 */
void
nqueens_generate(int N, int runs) {

    struct nqstate midfull = { .mid = (1<<N)-1 }; 

    struct nqstate *s0 = malloc(N*sizeof*s0);
    int j_gen = gen_single(&midfull, 0, s0, 0);
    if (!runs)
        runs = N - 2; 
    long sumsum = 0;   
    #pragma omp parallel
    {
    struct wbloc wblocs[SPMAX];

    #pragma omp for reduction(+:sumsum) schedule(dynamic)
    for (int i = 0; i < (j_gen+1)/2; i++) {

        wblocs[0] = (struct wbloc) { .nqptr=&s0[i], .sz=1, .nrows=runs, .multip=N-3, .chdiv=1, .f=0 };  

        long sum = sum_wbloc(wblocs);

        printf("[%d] %2d. %5x %5x %5x --> %8ld\n", omp_get_thread_num(), i, s0[i].mid, s0[i].le & ((1<<N)-1), s0[i].ri, sum);
        if (i < j_gen / 2)
            sum *= 2;
        sumsum += sum;
    }
    }
    printf("N%d --> %ld\n", N, sumsum);
}

int main(int argc, char *argv[]){
    int N = atoi(argv[1]);
    int runs = 0;
    if (argc > 2)
        runs = atoi(argv[2]);
    nqueens_generate(N, runs);  
    return 0;
}
