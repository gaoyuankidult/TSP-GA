#ifndef TRAVEL_MANAGER_H
#define TRAVEL_MANAGER_H

#include <time.h>
#include "circular_buffer.h"
#include "city.h"
#include "algorithm.h"

typedef struct travel_manager{
    int city_number;
    City* cities;
    void (* InitCities)(struct travel_manager*,int, City[]);
    void (* SetCities)(double[], double[], City[], int);
    double (* GetDistance)(City*, City*);

    void (* RandomInitPath)(struct travel_manager*, circular_buffer*,City[]);
    void (* SetPath)(struct travel_manager*, circular_buffer* ,int[] );
} TravelManager;

void InitCities(struct travel_manager* tm,int num_city, City cities_[])
{
    int i = 0;
    tm->city_number = num_city;
    tm->cities = cities_;
    for(i = 0; i< tm->city_number; i++){
        cities_[i] = CITY_DEFAULT;
    }
    return;
}

void SetCities(double x[], double y[] ,City cities[], int num_city)
{
    int i = 0;
    for(i = 0; i< num_city; i++)
    {
        cities[i].setx(&cities[i],x[i]);
        cities[i].sety(&cities[i],y[i]);
    }
    return;
}

double GetDistance(City* city1, City* city2)
{
    return sqrt(pow(city1->getx(city1)-city2->getx(city2),2) + pow(city1->gety(city1)-city2->gety(city2),2));
}


void RandomInitPath(struct travel_manager* tm, circular_buffer* buffer,City cities[])
{
    int i = 0;
    int ranking[tm->city_number];

    for (i = 0; i < tm->city_number; i++) ranking[i] = i;


    shuffle_int(ranking,tm->city_number);
    tm->SetPath(tm,buffer,ranking);

    return;
}


void SetPath(struct travel_manager*tm, circular_buffer* buffer,int ranking[])
{
    int i = 0;
    void *pointer;
    cb_clear(buffer);
#ifdef PRINT_PATH
    printf("after:");
#endif
    for (i = 0; i < tm->city_number; i++)
    {
#ifdef PRINT_PATH
        printf("%d",ranking[i]);
#endif
        pointer = &tm->cities[ranking[i]];
        cb_push_back(buffer,&pointer);
    }
    cb_store_ranks(buffer,ranking);
#ifdef PRINT_PATH
    printf("\n");
#endif

    return;
}
#endif
