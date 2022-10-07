/* from computerpuzzle.net, signed takaken */
/* fast and elegant; I added the explicit test for empty "cand", 
   changed the counting (for fun) and the bit operations. sam 23.9.2022 */
/* gcc makes the difference with huge 'constprop' inlinings: 
   N14 in 220 ms */
#include <stdio.h>
#include <stdlib.h>

int N;

int 
nqueen_rec(int row, int left, int down, int right) {
    int count = 0; 
    int cand = down & ~(left | right);

    if (!cand)
        return 0;
    if (++row == N)  
        return 1;

    while (cand) {
        int queen = -cand & cand;
        cand &= ~queen;
        count += nqueen_rec(row, (left|queen) << 1, down & ~queen, (right|queen) >> 1);
    }
    return count;
}

int main(int argc, char **argv) {
    N = atoi(argv[1]);
    int count = nqueen_rec(0, 0, (1<<N)-1, 0);
    printf("N=%d -> %d\n", N, count);
    return 0;
}



/* original recursion 
void Backtrack(int y, int left, int down, int right)
{
    int  bitmap, bit;

    if (y == SIZE) {
        COUNT++;
    } else {
        bitmap = MASK & ~(left | down | right);
        while (bitmap) {
            bit = -bitmap & bitmap;
            bitmap ^= bit;
            Backtrack(y+1, (left | bit)<<1, down | bit, (right | bit)>>1);
        }
    }
}
*/
