#ifndef MPI_MANAGER_H
#define MPI_MANAGER_H

#include <mpi.h>

void InitializeMPI(int argc, char** argv, int* rank, int* size)
{
    int rc = 0;
    rc = MPI_Init (&argc, &argv);
    if (rc != MPI_SUCCESS) {
      printf("MPI initialization failed\n");
      exit(1);
    }
    rc = MPI_Comm_rank (MPI_COMM_WORLD, rank);
    if (rc != MPI_SUCCESS) {
      printf("MPI get rank failed\n");
      exit(1);
    }
    rc = MPI_Comm_size (MPI_COMM_WORLD, size);
    if (rc != MPI_SUCCESS) {
      printf("MPI get size failed\n");
      exit(1);
    }
    return ;
}
void FinalizeMPI()
{
    MPI_Finalize();
    return ;
}



typedef struct MPIManager{
    void (*InitializeMPI)(int, char**, int*, int*);
    void (*FinalizeMPI)();
}MPIManager;

#endif
