/* N-Queens with threat-table -> twice as fast, N=15 takes 7 sec */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//const int N = 8;  //92 solutions
const int N = 12;   //12: 14200 in 0.050 s
#define SHOWLIM  12000

/* Print excluding last row, plus last col */
//pr_solution(bq, col); 
void pr_solution(int *bq, int col) {
	for (int i = 0; i < N - 1; i++)
		printf("%d:", bq[i]);
	printf("%d\n", col);
}
void init_board(int bq[], int v) {
	for (int i = 0; i < N; i++)
		bq[i] = v;
}
void show_board(int *q, int **t, int row, int cnt) {
	int i, j;
	char s[N+2];
   	
	printf("\033[H");
	printf("\n");
	for (i = 0; i < N; i++) {
		if (i == row)
			s[0] = '-';
		else
			s[0] = ' ';
		for (j = 0; j < N; j++) {
			s[j+1] = q[i] == j ? '#': ' ';
			if (t[i][j] == 1)
				s[j+1] = '.';
			else 
				if (t[i][j] == 2)
					s[j+1] = 'o';
			else 
				if (t[i][j] > 2)
					s[j+1] = 'O';
		}
		s[j+1] = '\0';
		printf("%s\n", s);
	}
    printf("\n%d\n", cnt);
	usleep(row*row*400);
	return;	
}
/* Returns zeroed n*n array */
int **init_threats(int n) {
	int r;
	int **t2d = malloc(n * sizeof*t2d);
	for (r = 0; r < n; r++)
		t2d[r] = calloc(n, sizeof**t2d);
	return t2d;
}
/* Only needs a row from "threats[][]" */
int next_free(int *r, int curr) {
	for (int q = curr + 1; q < N; q++)
		if (r[q] == 0)
			return q;
	return -1;
}
/* Cumulative threats - they can overlap */
void change_thr(int chg, int r, int c, int **threats) {
	int tr, diag;
	for (tr = r + 1; tr < N; tr++) { 
		threats[tr][c] += chg;
		diag = c + tr - r;
		if (diag < N)
			threats[tr][diag] += chg;  
		diag = c - (tr - r);
		if (diag >= 0)
			threats[tr][diag] += chg;  
	}
}
int main() {
	int queen[N];
	init_board(queen, -1);	
	int **threats = init_threats(N);
	int row = 0, cnt = 0;
	int col;
	
	while (row >= 0) {

		col = next_free(threats[row], queen[row]);

		if (row == N - 1) { 
			if (col >= 0) {
				cnt++;
                //if (cnt > SHOWLIM)
				    //show_board(queen, threats, row, cnt);
			}
			row--;

		} else {
            //if (cnt > SHOWLIM)
    			//show_board(queen, threats, row, cnt);
			if (queen[row] >= 0)
				change_thr(-1, row, queen[row], threats);	
			if ((queen[row] = col) >= 0) 
				change_thr(1, row++, col, threats);	
			else 
		    	row--;
		}
	}	
	printf("%d SOLUTIONS (%d-Queens)\n", cnt, N);
	return 0;
}
