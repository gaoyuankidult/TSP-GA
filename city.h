#ifndef CITY_H
#define CITY_H



typedef struct city_struct{
    double x;
    double y;
    double (*getx)(struct city_struct*);
    double (*gety)(struct city_struct*);
    void (*setx)(struct city_struct*, double);
    void (*sety)(struct city_struct*, double);
    void (*CityInfo)(struct city_struct*);

} City;

void setx(City* self, double x_)
{
    self->x = x_;
}

void sety(City *self, double y_)
{
    self->y = y_;
}

double getx(City* self)
{
    return self->x;
}

double gety(City* self)
{
    return self->y;
}

void CityInfo(City* self)
{
    printf("%lf\t%lf\n",self->x,self->y);
}


const City CITY_DEFAULT = {0.0,0.0,getx,gety,setx,sety,CityInfo};

#endif
