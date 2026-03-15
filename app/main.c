#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libsig.h"
#include "main.h"

// TODO: Change this to dynamically change when we run out
#define BUF_SIZE 1000

#define FILTER_COLS 5
#define FILTER_ROWS 4

int
main(int argc, char* argv[])
{
    FILE* input_data_file;
    FILE* filter_coeffs_file;
    clock_t tic;
    char buf[256];
    double line_val;
    double* input_data;
    double* output_data;
    size_t num_of_data = 0;

    double filter_coeffs[FILTER_ROWS][FILTER_COLS] = { 0 };

    filter_bench_t filter_fns[] = {
        { "Direct Form I (Naive)", filter_naive_ternary },
    };

    filter_coeffs_file = fopen("./data/input/filter_coeffs.csv", "r");
    if (!filter_coeffs_file)
        return 1;

    uint8_t row = 0;
    // Will be 4x5 since we are loading the same file over and over again
    while (fgets(buf, sizeof(buf), filter_coeffs_file) != NULL &&
           row < FILTER_ROWS) {
        uint8_t col = 0;
        char* token = strtok(buf, ",");

        while (token != NULL && col < FILTER_COLS) {
            filter_coeffs[row][col] = atof(token);

            token = strtok(NULL, ",");
            ++col;
        }

        ++row;
    }
    fclose(filter_coeffs_file);

    input_data_file = fopen("./data/input/signal_small.csv", "r");
    if (!input_data_file)
        return 1;

    // Allocate buffer for now
    input_data = malloc(sizeof(double) * BUF_SIZE);
    if (!input_data)
        return 2;

    while (fgets(buf, sizeof(buf), input_data_file) != NULL) {
        line_val = atof(buf);
        input_data[num_of_data] = line_val;
        ++num_of_data;
    }
    fclose(input_data_file);

    output_data = malloc(sizeof(double) * num_of_data);
    if (!output_data)
        return 2;

    for (size_t i = 0; i < sizeof(filter_fns) / sizeof(filter_bench_t); ++i) {
        tic = clock();
        filter_fns[i].func(filter_coeffs[0],
                           FILTER_COLS,
                           filter_coeffs[1],
                           FILTER_COLS,
                           input_data,
                           num_of_data,
                           output_data);
        tic = clock() - tic;

        printf("Filter algorithm: %s, took %lf s to complete.\n",
               filter_fns[i].algo_name,
               (double)tic / CLOCKS_PER_SEC);
    }

    free(input_data);
    free(output_data);

    return 0;
}
