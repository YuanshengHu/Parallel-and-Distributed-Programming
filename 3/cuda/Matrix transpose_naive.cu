#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <cuda.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
clock_t sum_time = 0;
__global__ void kernel(int *mat, int * to){
	int i = blockIdx.y * blockDim.y + threadIdx.y;
	int j = blockIdx.x * blockDim.x + threadIdx.x;
	int index_in = i * 1024 + j;
	int index_out = j * 1024 + i;
	to[index_out] = mat[index_in];
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
	cudaEventRecord(start,0);
	for (i = 0; i<it; i++){
		kernel <<<dimGrid, dimBlock>>>(d_mat, d_to);
	}
	cudaEventRecord(stop,0);
	cudaEventSynchronize(start);
	cudaEventSynchronize(stop);
	float time;
	cudaEventElapsedTime(&time, start, stop);
	printf("The total running time is: %f\n", time);
	scanf("%d", &it);
}