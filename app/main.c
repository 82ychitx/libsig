#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "libsig.h"

// TODO: Change this to dynamically change when we run out
#define BUF_SIZE 1000

int
main(int argc, char* argv[])
{
    FILE* data_file;
    clock_t tic;
    char buf[32];
    double line_val;
    double* p_data;
    size_t num_of_data = 0;
    
    data_file = fopen("./data/input/signal_small.csv", "r");
    if (!data_file)
        return 1;

    // Allocate buffer for now
    p_data = malloc(sizeof(double) * BUF_SIZE);
    if (!p_data)
        return 2;

    tic = clock();
    while(fgets(buf, sizeof(buf), data_file) != NULL) {
        line_val = atof(buf);
        p_data[num_of_data] = line_val;
        ++num_of_data;
    }
    tic = clock() - tic;

    for (size_t i = 0; i < num_of_data; ++i) {
        printf("value: %zu = %lf\n", i, p_data[i]);
    }

    printf("\nTime it took reading the file: %lf", ((double)tic / CLOCKS_PER_SEC));

    free(p_data);
    fclose(data_file);
    return 0;
}
