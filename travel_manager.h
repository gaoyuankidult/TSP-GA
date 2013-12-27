

typedef struct travel_manager{
    void (* InitCities)(int, City[]);
    void (* SetCities)(double[], double[], City[], int);
    double (* GetDistance)(City*, City*);

    void (* InitPath)(int, circular_buffer*);
    void (* RandomInitPath)(int, circular_buffer*);
    void (* SetPath)(circular_buffer*);

} TravelMnager;

void InitCities(int num_city, City cities[])
{
    int i = 0;
    for(i = 0; i< num_city; i++){
        cities[i] = CITY_DEFAULT;
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
