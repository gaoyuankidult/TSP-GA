#ifndef GENETIC_ALGORITHM
#define GENETIC_ALGORITHM


#include <stdlib.h>
#include <math.h>

#include "circular_buffer.h"
#include "city.h"
#include "travel_manager.h"
#ifdef USE_MPI
#include "mpi_manager.h"
#endif

typedef struct genetic_struct{
    unsigned int population_size;
    unsigned int population_number;
    double** fitness;
    circular_buffer** population;
    TravelManager* travel_manager;
    void (*InitPopulation)(struct genetic_struct*,const int,const int, TravelManager*);
    double (*CalcFitness)(struct genetic_struct*,circular_buffer*);
    circular_buffer* (*FindRelativeBest)(struct genetic_struct*,int);
    void (*PrintGene)(circular_buffer* cb);
    void (*Evolve)(void*);
    void (*Distribute)();
    void (*Mutate)(circular_buffer* gene,int);
    circular_buffer** (*Breed)(struct genetic_struct*,circular_buffer* father, circular_buffer* mother);
    void (*Collapse)(struct genetic_struct*);
    void (*ExchangeWith)(struct genetic_struct* , int[]);
} GeneticAlgorithm;

void InitPopulation(GeneticAlgorithm* ga,const unsigned int population_size_,const unsigned int population_number_,TravelManager* travel_manager_)
{
    unsigned int i = 0;
    unsigned int j = 0;

    ga->travel_manager = travel_manager_;

    ga->population_size = population_size_;
    ga->population_number = population_number_;

    ga->population = malloc(population_number_ * sizeof(circular_buffer*));
    ga->fitness = malloc(population_number_ * sizeof(double*));

    for(i = 0;i < population_number_;i++)
    {
        (ga->fitness)[i] = malloc(population_size_*sizeof(double));
        (ga->population)[i] = malloc(population_size_*sizeof(circular_buffer));
    }

    for(j = 0; j < population_number_;j++)
    {
        for(i = 0; i < population_size_;i++ )
        {
            cb_init((ga->population)[j] +i,CITY_NUMBER,sizeof(City*));
            travel_manager_->RandomInitPath(travel_manager_,(ga->population)[j]+i);
        }
    }
}

double CalcFitness(GeneticAlgorithm* ga,circular_buffer* cb)
{
    unsigned int i = 0;
    double fitness = 0.0;
    //cb->count
    if(NULL == cb)
    {
        exit(0);
    }
    for(i = 0;i < cb->count;i++)
    {
        fitness += ga->travel_manager->GetDistance((*((City**)(cb->buffer)+i)),(*((City**)(cb->buffer)+(i+1)%cb->count)));
    }
    return fitness;
}
circular_buffer* FindRelativeBest(struct genetic_struct* ga,int population_rank_)
{
    unsigned int i = 0;
    unsigned int j = 0;
    double best1[BEST_NUMBER];

    for(i = 0;i < BEST_NUMBER; i++)
    {
#ifdef INFINITY
        best1[i] = INFINITY;
#else
        best1[i] = 0x7FFFFFFFFFFFFFFF;
#endif
    }
    circular_buffer** elite_individual = NULL;
    circular_buffer* temp_elite = NULL;
    elite_individual = malloc(sizeof(circular_buffer*)*BEST_NUMBER);

    for(i = 0;i < ga->population_size;i++)
    {
        *((ga->fitness)[population_rank_]+i) = ga->CalcFitness(ga,(ga->population)[population_rank_]+i);
        //                printf("%d",best1[BEST_NUMBER-1]);
        if(best1[BEST_NUMBER-1] >= (ga->fitness)[population_rank_][i])
        {

            best1[BEST_NUMBER-1] = *((ga->fitness)[population_rank_]+i);
            elite_individual[BEST_NUMBER-1]  = (ga->population)[population_rank_]+i;


            for(j= 1;j < BEST_NUMBER ;j++)
            {
                if(best1[BEST_NUMBER-j] < best1[BEST_NUMBER-j-1])
                {
                    SWAP(best1[BEST_NUMBER-j],best1[BEST_NUMBER-j-1]);
                    SWAP(elite_individual[BEST_NUMBER-j],elite_individual[BEST_NUMBER-j-1]);
                }
            }
        }

    }

#ifdef PRINT_FITNESS
    for(i = 0;i <ga->population_size;i++)
    {
        printf("fitness:%lf\n",*((ga->fitness)[population_rank_]+i));
    }
    printf("best1 fitness are :");
    for(i = 0;i < BEST_NUMBER;i++)
    {
        printf("%lf,",best1[i]);
    }
    printf("\n");
#endif
    temp_elite = elite_individual[rand()%BEST_NUMBER];
    free(elite_individual);

    return temp_elite;
}
void PrintGene(circular_buffer* cb)
{

    if(NULL == cb)
    {
        printf("Can not print gene, the path is NULL.\n");
    }
#ifndef DEBUG
#ifdef PRINT_GENE
    unsigned int i = 0;
    printf("The path is through visiting :\n");
    for(i = 0;i < cb->count;i++)
    {
        (*((City**)(cb->buffer)+i))->CityInfo((*((City**)(cb->buffer)+i)));
    }
#endif
#endif
    return;
}
void Distribute()
{

    return;
}
void Evolve(void*  ga_)
{
    struct genetic_struct* ga = (struct genetic_struct*)ga_;
    unsigned int i = 0;
    circular_buffer** elite_individual  = NULL;
    elite_individual = malloc(sizeof(circular_buffer*) * ga->population_number);
    for(i = 0; i < ga->population_number;i++)
    {
        elite_individual[i] = ga->FindRelativeBest(ga,i);
//        //ga->PrintGene(elite_individual[i]);
    }

    ga->Breed(ga,elite_individual[0],elite_individual[1]);
    free(elite_individual);
    //printf("The fitness of gene algo is : %lf\n",ga->CalcFitness(ga,ga->FindRelativeBest(ga,0)));
#ifdef USE_PTHREAD
//    pthread_exit(NULL);
#endif
    return;
}
circular_buffer** Breed(struct genetic_struct* ga,circular_buffer* father, circular_buffer* mother)
{

    unsigned int i = 0;
    unsigned int parents_number = 2;
    circular_buffer ** child = NULL;
    int** locus = NULL;
    int** parents_ranks = NULL;


    //#ifdef PROCESS_CHECK
    //    printf("(%d)Initialize Breed Variables \n",p_r_i++);
    //#endif


    parents_ranks = malloc(sizeof(int*)*parents_number);
    for(i = 0; i< parents_number;i++)
    {
        parents_ranks[i] = malloc(father->count * sizeof(int));
    }


    for(i = 0;i < father->count;i++)
    {
        // fathers' rank
        parents_ranks[0][i] = (father->ranks)[i];
        // mothers' rank
        parents_ranks[1][i] = (mother->ranks)[i];
    }

#ifdef BREED_PARENTS_RANK
    // mothers' rank
    for(i = 0;i < father->count;i++)
    {
        printf("Rank of father:%d\n",parents_ranks[0][i]);
    }
    printf("\n");
    // mothers' rank
    for(i = 0;i < father->count;i++)
    {
        printf("Rank of mother:%d\n",parents_ranks[1][i]);
    }
    printf("done\n");
#endif


    locus = malloc(sizeof(int*)*father->count);

    for(i = 0; i< father->count;i++)
    {
        locus[i] = malloc(parents_number * sizeof(int));
    }
    unsigned int all_rank_same = 0;
    for(i = 0;i < father->count;i++)
    {
        // father's next loci
        locus[parents_ranks[0][i]][0] = parents_ranks[0][(i+1)%father->count];
        // mother's next loci
        locus[parents_ranks[1][i]][1] = parents_ranks[1][(i+1)%mother->count];

    }
    for(i = 0;i < father->count;i++)
    {
        if(locus[i][0]==locus[i][1])
        {
            all_rank_same ++;
        }
    }

    if(father->count == all_rank_same )
    {
#ifdef RANDOM_MOTHER
        RandomInitPath(ga->travel_manager,mother,ga->travel_manager->cities);
        for(i = 0;i < father->count;i++)
        {
            // mothers' rank
            parents_ranks[1][i] = (mother->ranks)[i];    int* child_rank;
            child_rank = malloc(father->count * sizeof(int));
        }
        for(i = 0;i < father->count;i++)
        {
            // mother's next loci
            locus[parents_ranks[1][i]][1] = parents_ranks[1][(i+1)%mother->count];
        }
#endif
#ifdef MUTATE_MOTHER
        ga->Mutate(mother,MUTATE_MOTHER_GENE_NUM);
        for(i = 0;i < father->count;i++)
        {
            // mothers' rank
            parents_ranks[1][i] = (mother->ranks)[i];
        }
        for(i = 0;i < father->count;i++)
        {
            // mother's next loci
            locus[parents_ranks[1][i]][1] = parents_ranks[1][(i+1)%mother->count];
        }
#endif

#ifdef RANDOM_FATHER
        RandomInitPath(ga->travel_manager,father,ga->travel_manager->cities);
        for(i = 0;i < father->count;i++)
        {
            // mothers' rank
            parents_ranks[0][i] = (father->ranks)[i];
        }
        for(i = 0;i < father->count;i++)
        {
            // mother's next loci
            locus[parents_ranks[0][i]][0] = parents_ranks[0][(i+1)%father->count];
        }
#endif
#ifdef MUTATE_FATHER
        ga->Mutate(father,MUTATE_FATHER_GENE_NUM);
        for(i = 0;i < father->count;i++)
        {
            // mothers' rank
            parents_ranks[0][i] = (father->ranks)[i];
        }
        for(i = 0;i < father->count;i++)
        {
            // mother's next loci
            locus[parents_ranks[0][i]][0] = parents_ranks[0][(i+1)%father->count];
        }
#endif
    }
#endif
    // check whether all ranks are the same.

#ifdef BREED_PARENTS_LOCUS
    // mothers' rank
    for(i = 0;i < father->count;i++)
    {
        printf("The two candidates that follows loci %d are: %d and %d\n",i,locus[i][0],locus[i][1]);
    }
    printf("\n");
#endif



    unsigned int current_loci = 0;
    int selection = 0;
    unsigned int j = 0;
    unsigned int k = 0;
    int child_rank[father->count];
    unsigned int gene_count = father->count;
    unsigned int population_size = ga->population_size;


    child = ga->population;
#ifdef USE_OPENMP
#pragma omp parallel for firstprivate(child_rank,current_loci) private(selection,i,j)
    for(k = 0; k < ga->population_number ; k++)
#endif
    for(k = 0; k < ga->population_number ; k++)
    {
#ifdef USE_OPENMP
        srand((time(NULL)) ^ omp_get_thread_num());
#endif
//#pragma omp master
//            printf("The thread number is: %d\n",omp_get_num_threads());
#ifdef FIRST_AS_PARENTS
        child_rank[0] = parents_ranks[rand()%2][0];
#endif
#ifdef FIRST_AS_RANDOM
        child_rank[0] = rand()%CITY_NUMBER;
#endif
        for(j = 0; j < population_size ; j++)
        {
            for(i = 0; i < gene_count-1; i++)
            {
#ifdef NATURE_SELECT
                selection = rand()%2;
#endif
                current_loci = locus[child_rank[i]][selection];

                if(Any(current_loci,child_rank,i+1))
                {
                    current_loci = locus[child_rank[i]][1-selection];
                }
                // if that also exist, randomly choose one
                // Here is i+1 as it needs to go through from 0 to i

                    while(Any(current_loci,child_rank,i+1))
                    {
                        current_loci = rand()%gene_count;
                    }
              child_rank[(i+1)%gene_count] =  current_loci;
            }

#ifdef CROSS_BIRTH

#endif


#ifdef BREED_CHILD_RANK
            // mothers' rank
            for(i = 0;i < father->count;i++)
            {
                printf("child loci is %d \n",child_rank[i]);
            }
            printf("\n");
#endif

            ga->travel_manager->SetPath(ga->travel_manager,(child[k])+j,child_rank);
            if(rand()%100 < MUTATE_PERCENT)
            {
                ga->Mutate((child[k])+j,MUTATE_NORMAL_GENE_NUM);
            }

        }
    }

    for(i = 0; i< parents_number;i++)
    {
        free(parents_ranks[i]);
    }
    free(parents_ranks);
    parents_ranks = NULL;

#ifdef NORMAL_BIRTH
    for(i = 0; i< father->count;i++)
    {
        free(locus[i]);
    }
    free(locus);
    locus = NULL;
#endif


    return child;
}
void Mutate(circular_buffer* gene,int mu_num)
{
    int i = 0;
    //Need Ranks to Decide What to Flip
#ifdef UNIFORM_MUTATE
    int ranks[2];
#endif

#ifdef BIASED_MUTATE
    int ranks[CITY_NUMBER];
    for(i = 0; i< CITY_NUMBER;i++)
    {
        ranks[i] = i;
    }
#endif
    for(i = 0;i<mu_num;i++)
    {
        //
#ifdef UNIFORM_MUTATE
        ranks[0]=rand()%CITY_NUMBER;
        ranks[1]=rand()%CITY_NUMBER;
#endif
        SWAP(*(((City**)gene->buffer)+ranks[0]),*(((City**)gene->buffer)+ranks[1]));
        SWAP(gene->ranks[ranks[0]],gene->ranks[ranks[1]]);
    }

    return;
}
void Collapse(struct genetic_struct* ga)
{

    unsigned int i = 0;
    for(i = 0; i < ga->population_number;i++)
    {
        free((ga->population)[i]);
        free((ga->fitness)[i]);
    }
    free(ga->population);
    free(ga->fitness);

    return;
}

void ExchangeWith(GeneticAlgorithm* ga, int rank[])
{
    circular_buffer ** slice_pointer = NULL;
    slice_pointer = ga->population;
    ga->travel_manager->SetPath(ga->travel_manager,*slice_pointer,rank);
    cb_store_ranks(*slice_pointer,rank);
    return;
}
const GeneticAlgorithm GA_DEFAULT={0,0,NULL,NULL,NULL,InitPopulation,CalcFitness,FindRelativeBest,PrintGene,Evolve,Distribute,Mutate,Breed,Collapse,ExchangeWith};
