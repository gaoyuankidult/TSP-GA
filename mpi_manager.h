#ifndef MPI_MANAGER_H
#define MPI_MANAGER_H

#include <mpi.h>

void InitializeMPI(int argc, char** argv, int* rank, int* size)
{
    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, rank);
    MPI_Comm_size (MPI_COMM_WORLD, size);
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
