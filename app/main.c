#include <math.h>
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
    FILE* file;
    clock_t tic;

    char buf[256];
    double* input_data;
    double* filter_output_correct;
    double* filter_output;

    size_t num_of_input = 0;
    size_t num_of_filter_correct = 0;

    double filter_coeffs[FILTER_ROWS][FILTER_COLS] = { 0 };

    filter_bench_t filter_fns[] = {
        { "Direct Form I (Naive)", filter_naive_ternary },
    };

    file = fopen("./data/input/filter_coeffs.csv", "r");
    if (!file) {
        return 1;
    }

    uint8_t row = 0;
    // Will be 4x5 since we are loading the same file over and over again
    while (fgets(buf, sizeof(buf), file) != NULL && row < FILTER_ROWS) {
        uint8_t col = 0;
        char* token = strtok(buf, ",");

        while (token != NULL && col < FILTER_COLS) {
            filter_coeffs[row][col] = atof(token);

            token = strtok(NULL, ",");
            ++col;
        }

        ++row;
    }
    fclose(file);

    // Allocate buffer for now
    input_data = malloc(sizeof(double) * BUF_SIZE);
    if (!input_data) {
        return 2;
    }

    if (read_file_to_buffer(
          "./data/input/signal_small.csv", input_data, &num_of_input) != 0) {
        return 1;
    }

    filter_output_correct = malloc(sizeof(double) * BUF_SIZE);
    if (!filter_output_correct) {
        return 2;
    }

    if (read_file_to_buffer("./data/output/filter_result.csv",
                            filter_output_correct,
                            &num_of_filter_correct) != 0) {
        return 1;
    }

    filter_output = malloc(sizeof(double) * num_of_input);
    if (!filter_output) {
        return 2;
    }

    for (size_t i = 0; i < sizeof(filter_fns) / sizeof(filter_bench_t); ++i) {
        bool result;
        
        tic = clock();
        filter_fns[i].func(filter_coeffs[0],
                           FILTER_COLS,
                           filter_coeffs[1],
                           FILTER_COLS,
                           input_data,
                           num_of_input,
                           filter_output);
        tic = clock() - tic;

        result = is_buffers_equal(filter_output, filter_output_correct, num_of_filter_correct);
        printf("Filter algorithm: %s, took %lf s to complete and the results are equal: %b\n",
               filter_fns[i].algo_name,
               (double)tic / CLOCKS_PER_SEC,
               result);
    }

    free(input_data);
    free(filter_output_correct);
    free(filter_output);

    return 0;
}

int
read_file_to_buffer(const char* filename, double* buffer, size_t* read_len)
{
    FILE* file;
    char buf[256];
    double line_val;

    file = fopen(filename, "r");
    if (!file) {
        printf("Could not open file: %s", filename);
        return 1;
    }

    *read_len = 0;
    while (fgets(buf, sizeof(buf), file) != NULL) {
        line_val = atof(buf);
        buffer[*read_len] = line_val;
        ++*read_len;
    }
    fclose(file);

    return 0;
}

bool
is_buffers_equal(const double* a, const double* b, size_t len)
{
    bool result = true;
    double epsilon = 1e-9;
    for (size_t i = 0; i < len; ++i) {
        if (fabs(a[i] - b[i]) > epsilon) {
            result = false;
            break;
        }
    }

    return result;
}
