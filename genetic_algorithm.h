#ifndef GENETIC_ALGORITHM
#define GENETIC_ALGORITHM


#include <stdlib.h>
#include <math.h>

#include "circular_buffer.h"
#include "city.h"
#include "travel_manager.h"
#include "mpi_manager.h"

typedef struct genetic_struct{
    int population_size;
    int population_number;
    double** fitness;
    circular_buffer** population;
    TravelManager* travel_manager;
    void (*InitPopulation)(struct genetic_struct*,const int,const int, TravelManager*,City*);
    double (*CalcFitness)(struct genetic_struct*,circular_buffer*);
    circular_buffer* (*FindRelativeBest)(struct genetic_struct*,int);
    void (*PrintGene)(circular_buffer* cb);
    void (*Evolve)(struct genetic_struct*,MPIManager* mpi_manager_);
    void (*Distribute)();
    void (*Mutate)();
    circular_buffer** (*Breed)(struct genetic_struct*,circular_buffer* father, circular_buffer* mother);
    void (*Collapse)(struct genetic_struct*);
} GeneticAlgorithm;

void InitPopulation(GeneticAlgorithm* ga,const unsigned int population_size_,const unsigned int population_number_,TravelManager* travel_manager_,City* cities)
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
            travel_manager_->RandomInitPath(travel_manager_,(ga->population)[j]+i,cities);
        }
    }
}

double CalcFitness(GeneticAlgorithm* ga,circular_buffer* cb)
{
    unsigned int i = 0;
    double fitness = 0.0;
    //cb->count
    for(i = 0;i < cb->count;i++)
    {
        fitness += ga->travel_manager->GetDistance((*((City**)(cb->buffer)+i)),(*((City**)(cb->buffer)+(i+1)%cb->count)));
    }
    return fitness;
}
circular_buffer* FindRelativeBest(GeneticAlgorithm* ga,int population_rank_)
{
    unsigned int i = 0;
    unsigned int j = 0;
    double best[BEST_NUMBER];

    for(i = 0;i < BEST_NUMBER; i++)
    {
#ifdef INFINITY
        best[i] = INFINITY;
#else
        best[i] = 0x7FFFFFFFFFFFFFFF;

#endif
    }
    circular_buffer** elite_individual = NULL;
    circular_buffer* temp_elite = NULL;
    elite_individual = malloc(sizeof(circular_buffer*)*BEST_NUMBER);
    //    printf("%d",ga->population_number);
    //ga->CalcFitness(ga,ga->population);
    //      printf("1count:%d\n",(ga->population)[population_rank_][0].count);

    for(i = 0;i < ga->population_size;i++)
    {
        *((ga->fitness)[population_rank_]+i) = ga->CalcFitness(ga,(ga->population)[population_rank_]+i);

        if(best[BEST_NUMBER-1] > (ga->fitness)[population_rank_][i])
        {

            best[BEST_NUMBER-1] = *((ga->fitness)[population_rank_]+i);
            elite_individual[BEST_NUMBER-1]  = (ga->population)[population_rank_]+i;
            j = 1;
            while(best[BEST_NUMBER-j] < best[BEST_NUMBER-j-1])
            {
                swap(best[BEST_NUMBER-j],best[BEST_NUMBER-j-1]);
                swap(elite_individual[BEST_NUMBER-j],elite_individual[BEST_NUMBER-j-1]);
            }
        }

    }

#ifdef PRINT_FITNESS
    for(i = 0;i <= ga->population_size;i++)
    {
        printf("fitness:%lf\n",*((ga->fitness)[population_rank_]+i));
    }
    printf("best fitness is :%lf\n",best);
#endif
    temp_elite = elite_individual[rand()%BEST_NUMBER];
    free(elite_individual);

    return temp_elite;
}
void PrintGene(circular_buffer* cb)
{
    unsigned int i = 0;
    if(NULL == cb)
    {
        printf("Can not print gene, the path is NULL.\n");
    }
#ifndef DEBUG
#ifdef PRINT_GENE
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
void Evolve(struct genetic_struct* ga,MPIManager* mpi_manager_)
{
    unsigned int i = 0;
    circular_buffer** elite_individual  = NULL;
    elite_individual = malloc(sizeof(circular_buffer*) * ga->population_number);
    for(i = 0; i < ga->population_number;i++)
    {
        elite_individual[i] = FindRelativeBest(ga,i);

        //ga->PrintGene(elite_individual[i]);
    }

    ga->Breed(ga,elite_individual[0],elite_individual[1]);
    free(elite_individual);
    return;
}
circular_buffer** Breed(struct genetic_struct* ga,circular_buffer* father, circular_buffer* mother)
{

    unsigned int i = 0;
    unsigned int parents_number = 2;
    circular_buffer ** child = NULL;
    int** locus = NULL;
    int** parents_ranks = NULL;
    int* child_rank;

    //#ifdef PROCESS_CHECK
    //    printf("(%d)Initialize Breed Variables \n",p_r_i++);
    //#endif
    child_rank = malloc(father->count * sizeof(int));
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

    for(i = 0;i < father->count;i++)
    {
        // father's next loci
        locus[parents_ranks[0][i]][0] = parents_ranks[0][(i+1)%father->count];
        // mother's next loci
        locus[parents_ranks[1][i]][1] = parents_ranks[1][(i+1)%mother->count];
    }

#ifdef BREED_PARENTS_LOCUS
    // mothers' rank
    for(i = 0;i < father->count;i++)
    {
        printf("The two candidates that follows loci %d are: %d and %d\n",i,locus[i][0],locus[i][1]);
    }
    printf("\n");
#endif

    child_rank[0] = parents_ranks[rand()%2][0];

    unsigned int current_loci = 0;
    int selection = 0;
    int j = 0;
    int k = 0;
    for(k = 0; k < ga->population_number ; k++)
    {

        for(j = 0; j < ga->population_size ; j++)
        {
            for(i = 0; i < father->count-1; i++)
            {

                selection = rand()%2;

                current_loci = locus[child_rank[i]][selection];
                //    printf("child loci is 2\n");

                if(Any(current_loci,child_rank,i+1))
                {
                    current_loci = locus[child_rank[i]][1-selection];
                }
                // if that also exist, randomly choose one
                // Here is i+1 as it needs to go through from 0 to i

                while(Any(current_loci,child_rank,i+1))
                {
                    current_loci = rand()%father->count;
                }
                // printf("child loci is \n");
                child_rank[(i+1)%father->count] = current_loci;

            }

#ifdef BREED_CHILD_RANK
            // mothers' rank
            for(i = 0;i < father->count;i++)
            {
                printf("child loci is %d \n",child_rank[i]);
            }
            printf("\n");
#endif

            child = ga->population;

            ga->travel_manager->SetPath(ga->travel_manager,(child[k])+j,child_rank);


        }
    }

    //#ifdef PROCESS_CHECK
    //    printf("(%d)Breed Children \n",p_r_i++);
    //#endif


    free(child_rank);

    for(i = 0; i< parents_number;i++)
    {
        free(parents_ranks[i]);
    }
    free(parents_ranks);
    for(i = 0; i< father->count;i++)
    {
        free(locus[i]);
    }
    free(locus);
    parents_ranks = NULL;
    locus = NULL;
    return child;
}
void Mutate()
{
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

const GeneticAlgorithm GA_DEFAULT={0,0,NULL,NULL,NULL,InitPopulation,CalcFitness,FindRelativeBest,PrintGene,Evolve,Distribute,Mutate,Breed,Collapse};
#endif
