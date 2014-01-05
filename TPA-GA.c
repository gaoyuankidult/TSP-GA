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

#define CITY_NUMBER 60
#define RADIUS 50
#define POPULATION 3000
#define GENERATIONS 1000
#define PARENTS_NUMBER 2
#define BEST_NUMBER 1
#define EXCHANGE_INTERVAL GENERATIONS/2

#define MUTATE_PERCENT 85
#define MUTATE_NORMAL_GENE_NUM 1
#define MUTATE_MOTHER_GENE_NUM 4
#define MUTATE_FATHER_GENE_NUM 0

#define THREAD_NUMBER 1

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


#define USE_MPI
//#define USE_OPENMP
//#define USE_PTHREAD




#include "city.h"
#include "travel_manager.h"
#ifdef USE_OPENMP
#include <omp.h>
#endif
#ifdef USE_MPI
#include "mpi_manager.h"
#endif
#ifdef USE_PTHREAD
#include <pthread.h>
#endif
#include "genetic_algorithm.h"
#include "math.h"
#include "time.h"


void SinGenerator(double x[],double y[])
{
    int i = 0;
    for (i = 0; i< CITY_NUMBER;i++)
    {
        x[i]=i;
        y[i]=sin(i);
    }
    return;
}
void ParabolaGenerator(double x[],double y[])
{
    int i = 0;
    for (i = 0; i< CITY_NUMBER;i++)
    {
        x[i]=i;
        y[i]=2*M_PI*i*i;
    }
    return;
}
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
    int i = 0;
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

#ifdef USE_PTHREAD
    pthread_t threads[THREAD_NUMBER];
    int rc = 0;
#endif


    //LinearGenerator(x,y);
    CircleGenerator(x,y);
    //SinGenerator(x,y);
#ifdef USE_MPI
    int size = 0;
    int rank = 0;
    MPIManager mpi_manager;
    mpi_manager = (MPIManager){InitializeMPI,FinalizeMPI};
    mpi_manager.InitializeMPI(argc,argv,&rank,&size);
#endif

#ifdef PROCESS_CHECK
    if(0 == rank)
    {
        printf("(%d)Initialize Travel Manager \n",p_r_i++);
    }
#endif

    TravelManager travel_controller;
    travel_controller = (TravelManager){0,NULL,InitCities,SetCities,GetDistance,RandomInitPath,SetPath};


#ifdef PROCESS_CHECK
    if(0 == rank)
    {
        printf("(%d)Initialize Cities \n",p_r_i++);
    }
#endif

    City cities[CITY_NUMBER];
    travel_controller.InitCities(&travel_controller,CITY_NUMBER,cities);
    travel_controller.SetCities(x,y,cities,CITY_NUMBER);
    GeneticAlgorithm ga;
    ga = (GeneticAlgorithm){0,0,NULL,NULL,NULL,InitPopulation,CalcFitness,FindRelativeBest,PrintGene,Evolve,Distribute,Mutate,Breed,Collapse,ExchangeWith};
    ga.InitPopulation(&ga,POPULATION,PARENTS_NUMBER,&travel_controller);

#ifdef PROCESS_CHECK
    if(0 == rank)
    {
        printf("(%d)Initialize Genetic Algorithm Population \n",p_r_i++);
    }
#endif

#ifdef USE_OPENMP
#pragma omp parallel for
    for(i = 0;i<2;i++)
    {
        g[i]->Evolve(g[i]);
    }
#endif

#ifdef USE_PTHREAD
    for(i = 0;i<THREAD_NUMBER;i++)
    {
        printf("In main: creating thread %ld\n", i);
        rc = pthread_create(&threads[i], NULL, g[i]->Evolve, (void *)g[i]);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
#endif

#ifdef USE_MPI
    srand(time(NULL)^rank);
    circular_buffer* temp_city;
    char* temp_buffer = NULL;
    int city_ranks[CITY_NUMBER];
    double fitness = 0.0;
    for(i = 0;i < GENERATIONS;i++)
    {
        ga.Evolve(&ga);
        MPI_Status stat;
        temp_city = (ga.FindRelativeBest(&ga,0));
        cb_output_ranks(temp_city,city_ranks);

        if(size > 1)
        {
            MPI_Send(city_ranks, CITY_NUMBER, MPI_INT, (rank+1)%size, 0, MPI_COMM_WORLD);
            MPI_Recv(city_ranks, CITY_NUMBER, MPI_INT, rank==0?size-1:rank-1, 0, MPI_COMM_WORLD, &stat);
        }
        fitness = ga.CalcFitness(&ga,ga.FindRelativeBest(&ga,0));
        if(0 == rank)
        {
            printf("The fitness of gene algo is : %lf\n",fitness);
        }

        ga.ExchangeWith(&ga,city_ranks);
        MPI_Barrier(MPI_COMM_WORLD);

        if(fitness < 315)
        {
            printf("The current iteration is %d\n  ",i);
            if(size > 1)
            {
                MPI_Abort(MPI_COMM_WORLD,911);
            }
            break;
        }
    }
    free(temp_buffer);

#endif
#ifdef PROCESS_CHECK
    if(0 == rank)
    {
        printf("(%d)Evolving \n",p_r_i++);
    }
#endif
    //    cb_store_cities(best1,"/home/gao/Desktop/temp.txt");
    ga.Collapse(&ga);
#ifdef USE_MPI
#ifdef PROCESS_CHECK
    if(0 == rank)
    {
        printf("(%d)Finalize MPI \n",p_r_i++);
    }
    mpi_manager.FinalizeMPI();
#endif
#endif
#ifdef USE_PTHREAD
    (void) pthread_join(&threads[0], NULL);
    (void) pthread_join(&threads[1], NULL);
#endif
    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    printf ("Elasped time is %.2f seconds.\n", seconds );
    return 0;
}
