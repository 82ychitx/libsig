#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bench.h"
#include "libsig.h"
#include "main.h"

// TODO: Change this to dynamically change when we run out
#define BUF_SIZE 1000

#define FILTER_COLS 5
#define FILTER_ROWS 4

int
main()
{
    FILE* file;

    char buf[256];
    double* input_data;
    double* filter_output_correct;
    double* filter_output;

    size_t num_of_input = 0;
    size_t num_of_filter_correct = 0;

    double filter_coeffs[FILTER_ROWS][FILTER_COLS] = { 0 };

    filter_bench_t filter_benches[] = {
        { "Direct Form I (Naive)", filter_naive_ternary, BENCH_NOT_RAN, 0.0 },
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
    if (filter_output == NULL) {
        return 2;
    }
    filter_input_t filter_input = { filter_coeffs[0], FILTER_COLS,
                                    filter_coeffs[1], FILTER_COLS,
                                    input_data,       num_of_input,
                                    filter_output };

    filter_bench(&filter_input,
                 filter_output_correct,
                 filter_benches,
                 sizeof(filter_benches) / sizeof(filter_bench_t));

    bench_result_t bench = { filter_benches, sizeof(filter_benches) / sizeof(filter_bench_t) };
    bench_print_table(&bench);

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
