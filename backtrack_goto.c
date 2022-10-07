/* N-Queens with goto */
/* this seems to be the fastest simple backtracking approach;
   the problem is the inner loop to check the threats from previous rows */

#include <stdio.h>
#include <stdlib.h>

//const int N = 8;  //92 solutions in 0.003 s 
const int N = 12;   //14200 in 0.120 s
//int N = 13; // 0.900 s

void init_board(int bq[], int v) {
	for (int i = 0; i < N; i++)
		bq[i] = v;
}
int main() {
	int bq[N];
	init_board(bq, -1);	
	int row, col, rd, old;
	int cnt = 0;
	for (row = 0; row >= 0;) {
		col = bq[row];
		NEXT_col:
		if (++col == N) 
			bq[row--] = -1;
		else {
			for (rd = 1; rd <= row; rd++)
				if ((old = bq[row-rd]) == col || abs(old - col) == rd)
					goto NEXT_col;
			if (row == N - 1) {
				cnt++;
				row--;
			}
			else  
		    	bq[row++] = col;
		}
	}	
	printf("%d SOLUTIONS (%d-Queens)\n", cnt, N);
	return 0;
}
