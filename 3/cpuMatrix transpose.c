#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
clock_t sum_time = 0;
int main(){
    int n, it, i, j;
	int * mat, *to, *d_mat;
	n = 1024;
	mat = (int*)malloc(n*n*sizeof(int));
	to = (int*)malloc(n*n*sizeof(int));
	printf("input the iter times:\n");
	scanf("%d", &it);
	for (i = 0; i<n; i++){
		for (j = 0; j<n; j++){
			mat[i*n + j] = 1;
		}
	}
    clock_t t1,t2;
    t1 = clock();
    for (int a = 0; a<it; a++){
		for (int i = 0; i < n; i++){
			for (int j = 0; j < n; j++){
				to[j*1024+i] = mat[i*1024+j];
			}
		}
	}
    t2 = clock();
    printf("The total running time is: %f seconds\n", (double)(t2-t1)/CLOCKS_PER_SEC);
    printf("input any to exit\n");
	scanf("%d", &it);
    return 0;
}
