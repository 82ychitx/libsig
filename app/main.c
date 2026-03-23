#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bench.h"
#include "libsig.h"
#include "main.h"

int
main()
{
    bench_error_t status = BENCH_EOK;
    double* input_data = NULL;
    size_t num_of_input = 0;
    double filter_coeffs[FILTER_ROWS][FILTER_COLS] = { 0 };

    algo_bench_t filter_benches[] = {
        algo_bench_init("Form 1", (generic_fn_t)filter_naive_ternary),
        algo_bench_init("Split loops", (generic_fn_t)filter_split_loops),
    };

    algo_bench_t impz_benches[] = {
        algo_bench_init("Alloc buf", (generic_fn_t)impz),
    };

    algo_bench_t conv_benches[] = {
        algo_bench_init("Naive", (generic_fn_t)conv_naive),
        algo_bench_init("Bounded", (generic_fn_t)conv_bounded),
    };

    algo_bench_t series_benches[] = {
        algo_bench_init("Naive", (generic_fn_t)series_naive),
    };

    algo_bench_t parallel_benches[] = {
        algo_bench_init("Naive", (generic_fn_t)parallel_naive),
    };

    if (load_filter_coeffs("./data/input/filter_coeffs.csv", filter_coeffs) !=
        0) {
        printf("Failed to load filter coefficients.");
        status = BENCH_EFILE;
    } else if ((input_data = malloc(sizeof(double) * BUF_SIZE)) == NULL) {
        printf("Error allocating input data buffer.");
        status = BENCH_EALLOC;
    } else if (read_file_to_buffer("./data/input/signal_small.csv",
                                   input_data,
                                   &num_of_input) != 0) {
        printf("Failed to load input data.");
        return BENCH_EFILE;
    } else if ((status = filter_bench_suite(filter_coeffs,
                                            input_data,
                                            num_of_input,
                                            filter_benches,
                                            ALGO_BENCH_LEN(filter_benches))) !=
               BENCH_EOK) {
        printf("Error running filter benches.");
    } else if ((status = impz_bench_suite(
                  filter_coeffs, impz_benches, ALGO_BENCH_LEN(impz_benches))) !=
               BENCH_EOK) {
        printf("Error running impz benches.");
    } else if ((status = conv_bench_suite(input_data,
                                          num_of_input,
                                          conv_benches,
                                          ALGO_BENCH_LEN(conv_benches))) !=
               BENCH_EOK) {
        printf("Error running conv benches.");
    } else if ((status = series_bench_suite(filter_coeffs, series_benches, ALGO_BENCH_LEN(series_benches))) != BENCH_EOK) {
        printf("Error running series benches.");
    } else if ((status = parallel_bench_suite(filter_coeffs, parallel_benches, ALGO_BENCH_LEN(parallel_benches))) != BENCH_EOK) {
        printf("Error running parallel benches.");
    } else {

        bench_result_t bench = {
            filter_benches,   ALGO_BENCH_LEN(filter_benches),
            impz_benches,     ALGO_BENCH_LEN(impz_benches),
            conv_benches,     ALGO_BENCH_LEN(conv_benches),
            series_benches,   ALGO_BENCH_LEN(series_benches),
            parallel_benches, ALGO_BENCH_LEN(parallel_benches)
        };

        bench_print_table(&bench);
    }

    free(input_data);

    return status;
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

int
dump_buffer_to_csv(const char* filename, const double* buffer, size_t len)
{
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        return 1;
    }

    fprintf(file, "Index,Value\n");

    for (size_t i = 0; i < len; ++i) {
        fprintf(file, "%zu,%lf\n", i, buffer[i]);
    }

    fclose(file);
    return 0;
}

int
load_filter_coeffs(const char* filepath,
                   double coeffs[FILTER_ROWS][FILTER_COLS])
{
    FILE* file = fopen(filepath, "r");
    if (!file)
        return 1;

    char buf[256];
    uint8_t row = 0;

    while (fgets(buf, sizeof(buf), file) != NULL && row < FILTER_ROWS) {
        uint8_t col = 0;
        char* token = strtok(buf, ",");
        while (token != NULL && col < FILTER_COLS) {
            coeffs[row][col] = atof(token);
            token = strtok(NULL, ",");
            ++col;
        }
        ++row;
    }
    fclose(file);
    return 0;
}
