#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cuda.h>
clock_t sum_time = 0;
const int TILE = 32;
const int SIDE = 8;
__global__ void kernel(int *or_mat, int * to){
	__shared__ int mat[TILE][TILE + 1];
	int x = blockIdx.x * TILE + threadIdx.x;
	int y = blockIdx.y * TILE + threadIdx.y;
#pragma unroll
	for (int k = 0; k < TILE; k += SIDE) {
		if (x < 2048 && y + k < 2048)mat[threadIdx.y + k][threadIdx.x] = or_mat[((y + k) * 2048) + x];
	}
	__syncthreads();
	x = blockIdx.y * TILE + threadIdx.x;
	y = blockIdx.x * TILE + threadIdx.y;
#pragma unroll
	for (int k = 0; k < TILE; k += SIDE){
		if (x < 2048 && y + k < 2048)to[(y + k) * 2048 + x] = mat[threadIdx.x][threadIdx.y + k];
	}
}
int main(){
	int n, it, i, j;
	int * mat, *to, *d_mat, *d_to;
	int *d_mat2, *d_to2;
	int *d_mat3, *d_to3;
	int *d_mat4, *d_to4;
	n = 2048;
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
	dim3 dimBlock(32, 8);
	dim3 dimGrid(64, 64);
	cudaEvent_t start;
	cudaEvent_t stop;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	cudaEventRecord(start, 0);
	for (i = 0; i<it; i++){
		kernel << <dimGrid, dimBlock>> >(d_mat, d_to);
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
