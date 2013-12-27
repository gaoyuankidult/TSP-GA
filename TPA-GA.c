#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include "circular_buffer.h"



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


#define CITY_NUMBER 6

int main (int argc, char* argv[])
{
    int rank = 0;
    int size = 0;

    MPIManager mpi_manager;
    mpi_manager = (MPIManager){InitializeMPI,FinalizeMPI};
    TravelMnager cities_manager;
    cities_manager = (TravelMnager){InitCities,SetCities,GetDistance};
    City cities[CITY_NUMBER];
    cities_manager.InitCities(CITY_NUMBER,cities);
    double x[CITY_NUMBER] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    double y[CITY_NUMBER] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    cities_manager.SetCities(x,y,cities,CITY_NUMBER);
    //  printf("%lf\n",cities_manager.GetDistance(&cities[1],&cities[5]));
    mpi_manager.InitializeMPI(argc, argv, &rank, &size);
    circular_buffer path;
    cb_init(path,CITY_NUMBER,sizeof(City*));




    printf( "Hello world from process %d of %d\n", rank, size );
    mpi_manager.FinalizeMPI();
    return 0;
}
