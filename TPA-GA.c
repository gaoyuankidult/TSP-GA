#include <stdio.h>

#include <math.h>

#include "circular_buffer.h"
#define DEBUG
//#undef DEBUG

#ifdef DEBUG

#define PRINT_FITNESS
#undef PRINT_FITNESS

#define PRINT_GENE
#undef PRINT_GENE

#define BREED_PARENTS_RANK
#undef BREED_PARENTS_RANK

#define BREED_PARENTS_LOCUS
#undef BREED_PARENTS_LOCUS

#define PRINT_PATH
#undef PRINT_PATH

#define BREED_CHILD_RANK
#undef BREED_CHILD_RANK

#define PROCESS_CHECK
//process_rank_indicator
int p_r_i = 0;
//#undef PROCESS_CHECK

#endif

#define CITY_NUMBER 10
#define RADIUS 10
#define POPULATION 5000
#define GENERATIONS 200
#define PARENTS_NUMBER 2
#define BEST_NUMBER 2

#include "city.h"
#include "travel_manager.h"
#include "mpi_manager.h"
#include "genetic_algorithm.h"
#include "math.h"
#include "time.h"

void LinearGenerator(double x[],double y[])
{
    int i = 0;
    for (i = 0; i< CITY_NUMBER;i++)
    {
        x[i]=i;
        y[i]=i;
    }
    return;
}
void CircleGenerator(double x[],double y[],double radius)
{
    int i = 0;
    for (i = 0; i< CITY_NUMBER;i++)
    {
        x[i]=sin(2*M_PI/CITY_NUMBER*i)*RADIUS;
        y[i]=cos(2*M_PI/CITY_NUMBER*i)*RADIUS;
    }
    return;
}

int main (int argc, char* argv[])
{
    int rank = 0;
    int size = 0;
    int i = 0;
    srand(time(NULL));
    double x[CITY_NUMBER];
    double y[CITY_NUMBER];
    //LinearGenerator(x,y);
    CircleGenerator(x,y,10);
    MPIManager mpi_manager;
    mpi_manager = (MPIManager){InitializeMPI,FinalizeMPI};

    TravelManager travel_controller;
    travel_controller = (TravelManager){0,NULL,InitCities,SetCities,GetDistance,RandomInitPath,SetPath};
#ifdef PROCESS_CHECK
    printf("(%d)Initialize Travel Manager \n",p_r_i++);
#endif
    City cities[CITY_NUMBER];
    travel_controller.InitCities(&travel_controller,CITY_NUMBER,cities);
    travel_controller.SetCities(x,y,cities,CITY_NUMBER);
#ifdef PROCESS_CHECK
    printf("(%d)\Initialize Cities \n",p_r_i++);
#endif

    mpi_manager.InitializeMPI(argc, argv, &rank, &size);
#ifdef PROCESS_CHECK
    printf("(%d)Initialize MPI \n",p_r_i++);
#endif
    GeneticAlgorithm ga;
    ga = GA_DEFAULT;
    ga.InitPopulation(&ga,POPULATION,PARENTS_NUMBER,&travel_controller,cities);
#ifdef PROCESS_CHECK
    printf("(%d)Initialize Genetic Algorithm Population \n",p_r_i++);
#endif
    for(i = 0; i < GENERATIONS; i++)
    {
        ga.Evolve(&ga,&mpi_manager);
    }
#ifdef PROCESS_CHECK
    printf("(%d)Evolving \n",p_r_i++);
#endif

    circular_buffer* best;
    best = ga.FindRelativeBest(&ga,0);
    //cb_print_cities(best);
    cb_store_cities(best,"/home/gao/Desktop/temp.txt");
    printf("The fitness of gene is : %lf\n",ga.CalcFitness(&ga,best));
    ga.Collapse(&ga);


    if( 0 == rank)
    {
        //printf( "Total process number is %d\n",size);
    }

    mpi_manager.FinalizeMPI();
#ifdef PROCESS_CHECK
    printf("(%d)Finalize MPI \n",p_r_i++);
#endif
    return 0;
}
