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

#define CITY_NUMBER 70
#define RADIUS 100
#define POPULATION 50000
#define GENERATIONS 4
#define PARENTS_NUMBER 2
#define BEST_NUMBER 1
#define MUTATE_PERCENT 85

#define EXCHANGE_INTERVAL GENERATIONS/2

#define MUTATE_NORMAL_GENE_NUM 1

#define MUTATE_MOTHER_GENE_NUM 4

#define MUTATE_FATHER_GENE_NUM 0

#define THREAD_NUMBER 2

#define UNIFIED_MUTATION_RATE
#define HIGH_MUTATION_RATE_IN_SECOND_POPULATION

#define FIRST_AS_PARENTS
//#define FIRST_AS_RANDOM

#define UNIFORM_MUTATE

#define NORMAL_BIRTH
//#define CROSS_BIRTH

//#define RANDOM_MOTHER
#define MUTATE_MOTHER

//#define RANDOM_FATHER
#define MUTATE_FATHER

#define NATURE_SELECT
//#define HUMAN_SELECT


//#define USE_MPI
#define USE_OPENMP





#include "city.h"
#include "travel_manager.h"
#ifdef USE_OPENMP
#include <omp.h>
#endif
#ifdef USE_MPI
#include "mpi_manager.h"
#endif

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
void CircleGenerator(double x[],double y[])
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
    IGNORE(argc);
    IGNORE(argv);
    int rank = 0;
    int i = 0;
    srand(time(NULL));
    double x[CITY_NUMBER];
    double y[CITY_NUMBER];

    clock_t start = clock();
#ifdef USE_OPENMP
    omp_set_num_threads(THREAD_NUMBER);
#pragma omp parallel
    {
#pragma omp master
            printf("The thread number is: %d\n",omp_get_num_threads());
    }
#endif


    //LinearGenerator(x,y);
    CircleGenerator(x,y);
#ifdef USE_MPI
    int size = 0;
    MPIManager mpi_manager;
    mpi_manager = (MPIManager){InitializeMPI,FinalizeMPI,MPI_Scatter};
#endif

#ifdef PROCESS_CHECK
    printf("(%d)Initialize Travel Manager \n",p_r_i++);
#endif

    TravelManager travel_controller1;
    travel_controller1 = (TravelManager){0,NULL,InitCities,SetCities,GetDistance,RandomInitPath,SetPath};


#ifdef PROCESS_CHECK
    printf("(%d)Initialize Cities \n",p_r_i++);
#endif

    City cities[CITY_NUMBER];
    travel_controller1.InitCities(&travel_controller1,CITY_NUMBER,cities);
    travel_controller1.SetCities(x,y,cities,CITY_NUMBER);

    GeneticAlgorithm ga1;
    ga1 = GA_DEFAULT;
    ga1.InitPopulation(&ga1,POPULATION,PARENTS_NUMBER,&travel_controller1);
#ifdef PROCESS_CHECK
    printf("(%d)Initialize Genetic Algorithm Population \n",p_r_i++);
#endif


    TravelManager travel_controller2;
    travel_controller2 = (TravelManager){0,NULL,InitCities,SetCities,GetDistance,RandomInitPath,SetPath};
    travel_controller2.InitCities(&travel_controller2,CITY_NUMBER,cities);
    travel_controller2.SetCities(x,y,cities,CITY_NUMBER);
    GeneticAlgorithm ga2;
    ga2 = GA_DEFAULT;
    ga2.InitPopulation(&ga2,POPULATION,PARENTS_NUMBER,&travel_controller2);

    circular_buffer* best1;
    circular_buffer* best2;

    GeneticAlgorithm **g;
    g = malloc(sizeof(GeneticAlgorithm*)*2);
    g[0] = &ga1;
    g[1] = &ga2;

#pragma omp parallel for
for(i = 0;i<2;i++)
{
        g[i]->Evolve(g[i]);
        //ga1.Evolve(&ga1);
//        best1 = ga1.FindRelativeBest(&ga1,0);
//        printf("The fitness of gene algo 1 is : %lf\n",ga1.CalcFitness(&ga1,best1));


        //ga2.Evolve(&ga2);
//        best2 = ga2.FindRelativeBest(&ga2,0);
//        printf("The fitness of gene algo 2 is : %lf\n",ga2.CalcFitness(&ga2,best2));
}

#ifdef PROCESS_CHECK
    printf("(%d)Evolving \n",p_r_i++);
#endif

    //cb_print_cities(best1);
//    cb_store_cities(best1,"/home/gao/Desktop/temp.txt");

    ga1.Collapse(&ga1);
    ga2.Collapse(&ga2);

    if( 0 == rank)
    {
        //printf( "Total process number is %d\n",size);
    }

#ifdef PROCESS_CHECK
    printf("(%d)Finalize MPI \n",p_r_i++);
#endif
    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    printf ("Elasped time is %.2f seconds.\n", seconds );
    return 0;
}
