#ifndef CB_EXTENSION
#define CB_EXTENSION

#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include "city.h"

void cb_print_cities(circular_buffer *cb)
{
    unsigned int i = 0;
    if(cb->buffer == NULL)
    {
        printf("NULL pointer!");
        return;
    }
    printf("X\tY\t\n");
    for(i = 0;i < cb->count;i++)
    {
        (*((City**)(cb->buffer)+i))->CityInfo((*((City**)(cb->buffer)+i)));
    }
    return;
}
void cb_store_cities(circular_buffer *cb, const char* filename)
{
    unsigned int i = 0;
    if(cb->buffer == NULL)
    {
        printf("NULL pointer!");
        return;
    }
    FILE *ofp;
    char *mode = "w";


    ofp = fopen(filename, mode);

    if (ofp == NULL) {
      fprintf(stderr, "Can't open output file %s!\n",
              filename);
      exit(1);
    }
    fprintf(ofp,"X\tY\t\n");
    for(i = 0;i < cb->count;i++)
    {
        fprintf(ofp,"%lf\t%lf\t\n",(*((City**)(cb->buffer)+i))->x,(*((City**)(cb->buffer)+i))->y);
    }
    close(ofp);
    return;
}

void cb_store_ranks(circular_buffer *cb,int *ranks)
{
    int unsigned i = 0;
    if(NULL == cb->ranks)
        cb->ranks = malloc(cb->count * sizeof(int));
    for(i = 0; i < cb-> count ; i++)
    {
        (cb->ranks)[i] = ranks[i];
    }
    return;
}

void cb_output_ranks(circular_buffer *cb,int ranks[])
{
    int unsigned i = 0;
    for(i = 0; i < cb-> count ; i++)
    {
        ranks[i] = (cb->ranks)[i];
    }
    return;
}

#endif
