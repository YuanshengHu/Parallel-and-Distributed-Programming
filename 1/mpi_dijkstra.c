#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include "mpi.h"

const int INFINITY = 1000000;

void Print_dists(int dist[], int n);
void Print_paths(int pred[], int n);
int  Find_min_dist(int dist[], int known[], int n);
void Dijkstra(int loc_mat[], int loc_dist[], int loc_pred[], int loc_n, int my_rank, MPI_Comm comm, int n);
int Read_n(int my_rank, MPI_Comm comm);
MPI_Datatype Build_blk_col_type(int n, int loc_n);
void Read_matrix(int loc_mat[], int n, int loc_n, 
      MPI_Datatype blk_col_mpi_t, int my_rank, MPI_Comm comm);

int main(int argc, char* argv[]) {
   int  n;
   int  *loc_mat;
   int  loc_n,p,my_rank;
   MPI_Comm comm;
   MPI_Datatype blk_col_mpi_t;
   MPI_Init(&argc, &argv);
   comm = MPI_COMM_WORLD;
   MPI_Comm_size(comm, &p);
   MPI_Comm_rank(comm, &my_rank);
   n = Read_n(my_rank, comm);
   loc_n = n/p;
   loc_mat = malloc(n*loc_n*sizeof(int));
   blk_col_mpi_t = Build_blk_col_type(n, loc_n);
   Read_matrix(loc_mat, n, loc_n, blk_col_mpi_t, my_rank, comm);
   int *loc_dist, *loc_pred;
   loc_dist = (int*)malloc(loc_n*sizeof(int));
   loc_pred = (int*)malloc(loc_n*sizeof(int));  
   Dijkstra(loc_mat, loc_dist, loc_pred, loc_n, my_rank,comm,n);
   free(loc_mat);
   free(loc_dist);
   free(loc_pred);
   MPI_Type_free(&blk_col_mpi_t);
   MPI_Finalize();
   
   return 0;
}  /* main */
void Print_matrix(int mat[], int n) {
   int i, j;

   for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++)
         if (mat[i*n+j] == INFINITY)
            printf("i ");
         else
            printf("%d ", mat[i*n+j]);
      printf("\n");
   }
}  /* Print_matrix */
void Dijkstra(int loc_mat[], int loc_dist[], int loc_pred[], int loc_n, int my_rank, MPI_Comm comm, int n) {
   int i, u, loc_u, v, *loc_known, new_dist, *dist, *pred;
   dist = (int*)malloc(n*sizeof(int));
   pred = (int*)malloc(n*sizeof(int));
   loc_known = (int*)malloc(loc_n*sizeof(int));
   if (my_rank == 0) {
      loc_known[0] = 1;
      loc_pred[0] = 0;
      loc_dist[0] = 0;
      for (v = 1; v < loc_n; v++) {
         loc_dist[v] = loc_mat[0*loc_n + v];
         loc_pred[v] = 0;
         loc_known[v] = 0;
      }
   } else {
      for (v = 0; v < loc_n; v++) {
         loc_dist[v] = loc_mat[0*loc_n + v];
         loc_pred[v] = 0;
         loc_known[v] = 0;
      }
   }
   /* On each pass find an additional vertex */
   /* whose distance to 0 is known           */
   struct timeval t1,t2;
   gettimeofday(&t1,NULL);
   for (i = 1; i < n; i++) {
      loc_u = Find_min_dist(loc_dist, loc_known, loc_n);
      int my_min[2], glbl_min[2];
      my_min[0] = loc_dist[loc_u];
      my_min[1] = loc_u + my_rank*loc_n;
      if(loc_u == -1) my_min[0] = INFINITY;
      MPI_Allreduce(my_min,glbl_min,1,MPI_2INT,MPI_MINLOC,comm);
      u = glbl_min[1];
      int dist_u = glbl_min[0];
      if (my_rank == (u/loc_n)) {
         loc_known[u%loc_n] = 1;
      }
      for (v = 0; v < loc_n; v++) 
         if (!loc_known[v]) {
            new_dist = dist_u + loc_mat[u*loc_n + v];
            if (new_dist < loc_dist[v]) {
               loc_dist[v] = new_dist;
               loc_pred[v] = u;
            }
         }
   } /* for i */
   MPI_Gather(loc_dist, loc_n, MPI_INT, dist, loc_n, MPI_INT, 0 ,comm);
   MPI_Gather(loc_pred, loc_n, MPI_INT, pred, loc_n, MPI_INT, 0 ,comm);
   gettimeofday(&t2,NULL);
   if (my_rank == 0){
      printf("The distance from 0 to each vertex is:\n");
      Print_dists(dist, n);
      printf("The shortest path from 0 to each vertex is:\n");
      Print_paths(pred, n);
      printf("the running time is %ld \n",(t2.tv_usec-t1.tv_usec));
   }
}  /* Dijkstra */

int Read_n(int my_rank, MPI_Comm comm) {
   int n;
   
   if (my_rank == 0){
      printf("Please input the numbers of vertex:\n");
      scanf("%d", &n);
   }
   MPI_Bcast(&n, 1, MPI_INT, 0, comm);
   return n;
}  /* Read_n */

MPI_Datatype Build_blk_col_type(int n, int loc_n) {
   MPI_Aint lb, extent;
   MPI_Datatype block_mpi_t;
   MPI_Datatype first_bc_mpi_t;
   MPI_Datatype blk_col_mpi_t;

   MPI_Type_contiguous(loc_n, MPI_INT, &block_mpi_t);
   MPI_Type_get_extent(block_mpi_t, &lb, &extent);

   MPI_Type_vector(n, loc_n, n, MPI_INT, &first_bc_mpi_t);
   MPI_Type_create_resized(first_bc_mpi_t, lb, extent,
         &blk_col_mpi_t);
   MPI_Type_commit(&blk_col_mpi_t);

   MPI_Type_free(&block_mpi_t);
   MPI_Type_free(&first_bc_mpi_t);

   return blk_col_mpi_t;
}  /* Build_blk_col_type */

void Read_matrix(int loc_mat[], int n, int loc_n, MPI_Datatype blk_col_mpi_t, int my_rank, MPI_Comm comm) {
   int* mat = NULL, i, j;

   if (my_rank == 0) {
      printf("Please input the whole edges:\n");
      mat = malloc(n*n*sizeof(int));
      for (i = 0; i < n; i++)
         for (j = 0; j < n; j++)
            scanf("%d", &mat[i*n + j]);
   }
   MPI_Scatter(mat, 1, blk_col_mpi_t,
           loc_mat, n*loc_n, MPI_INT, 0, comm);

   if (my_rank == 0) free(mat);
}  /* Read_matrix */

int Find_min_dist(int dist[], int loc_known[], int loc_n) {
   int loc_v;
   int loc_u = -1;
   int loc_min_dist = INFINITY;
   for (loc_v = 0; loc_v < loc_n; loc_v++)
      if (!loc_known[loc_v])
         if (dist[loc_v] < loc_min_dist) {
            loc_u = loc_v;
            loc_min_dist = dist[loc_v];
         }
   return loc_u;
}  /* Find_min_dist */
void Print_dists(int dist[], int n) {
   int v;

   printf("  v    dist 0->v\n");
   printf("----   ---------\n");
                  
   for (v = 1; v < n; v++)
      printf("%3d       %4d\n", v, dist[v]);
   printf("\n");
} /* Print_dists */  
void Print_paths(int pred[], int n) {
   int v, w, *path, count, i;

   path =  malloc(n*sizeof(int));

   printf("  v     Path 0->v\n");
   printf("----    ---------\n");
   for (v = 1; v < n; v++) {
      printf("%3d:    ", v);
      count = 0;
      w = v;
      while (w != 0) {
         path[count] = w;
         count++;
         w = pred[w];
      }
      printf("0 ");
      for (i = count-1; i >= 0; i--)
         printf("%d ", path[i]);
      printf("\n");
   }

   free(path);
}  /* Print_paths */
