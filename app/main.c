#include <stdio.h>
#include <stdlib.h>

#include "libsig.h"

// TODO: Change this to dynamically change when we run out
#define BUF_SIZE 1000

int
main(int argc, char* argv[])
{
    FILE* data_file = fopen("./data/input/signal_small.csv", "r");
    if (!data_file)
        return 1;

    // Allocate buffer for now
    double* p_data = malloc(sizeof(double) * BUF_SIZE);
    if (!p_data)
        return 2;

    char buf[32];
    double line_val;
    size_t data_num = 0;
    
    while(fgets(buf, sizeof(buf), data_file) != NULL) {
        line_val = atof(buf);
        p_data[data_num] = line_val;
        ++data_num;
    }

    for (size_t i = 0; i < data_num; ++i) {
        printf("value: %zu = %lf\n", i, p_data[i]);
    }

    free(p_data);
    fclose(data_file);
    return 0;
}
