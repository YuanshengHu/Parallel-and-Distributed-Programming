
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
clock_t sum_time = 0;
__global__ void kernel(int *or_mat, int * to){
	__shared__ int mat[16][17];
	int bx = blockIdx.x * 16;
	int by = blockIdx.y * 16;
	int i = by + threadIdx.y; int j = bx + threadIdx.x;
	int ti = bx + threadIdx.y; int tj = by + threadIdx.x;
	if (i < 1024 && j < 1024){
		mat[threadIdx.x][threadIdx.y] = or_mat[i * 1024 + j];
	}
	__syncthreads();
	if (tj < 1024 && ti < 1024){
		to[ti * 1024 + tj] = mat[threadIdx.y][threadIdx.x];
	}
}
int main(){
	int n, it, i, j;
	int * mat, *to, *d_mat, *d_to;
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
	cudaMalloc((void**)&d_mat, n*n*sizeof(int));
	cudaMalloc((void**)&d_to, n*n*sizeof(int));
	cudaMemcpy(d_mat, mat, n*n*sizeof(int), cudaMemcpyHostToDevice);
	dim3 dimBlock(16, 16);
	dim3 dimGrid(64, 64);
	cudaEvent_t start;
	cudaEvent_t stop;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	cudaEventRecord(start, 0);
	for (i = 0; i<it; i++){
		kernel << <dimGrid, dimBlock >> >(d_mat, d_to);
	}
	cudaEventRecord(stop, 0);
	cudaEventSynchronize(start);
	cudaEventSynchronize(stop);
	float time;
	cudaEventElapsedTime(&time, start, stop);
	printf("The total running time is: %f\n", time);
    printf("input any to exit\n");
	scanf("%d", &it);
}
