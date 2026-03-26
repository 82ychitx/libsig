#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bench.h"
#include "file_io.h"
#include "libsig.h"

int
main()
{
    file_io_err_t file_res;
    bench_error_t status = BENCH_EOK;
    double* input_data = NULL;
    double* filter_coeffs = NULL;
    size_t filter_cols, filter_rows, input_rows, input_cols;

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

    algo_bench_t feedback_benches[] = {
        algo_bench_init("Naive", (generic_fn_t)feedback_naive),
    };

    algo_bench_t freqz_benches[] = {
        algo_bench_init("Naive", (generic_fn_t)freqz_naive),
    };

    if ((file_res = file_io_read_double_matrix("./data/input/filter_coeffs.csv",
                                               &filter_coeffs,
                                               &filter_rows,
                                               &filter_cols)) != FILE_IO_EOK) {
        printf("Failed to load filter coefficients. Error: %d\n", file_res);
        status = BENCH_EFILE;
    } else if ((file_res =
                  file_io_read_double_matrix("./data/input/signal_small.csv",
                                             &input_data,
                                             &input_rows,
                                             &input_cols)) != FILE_IO_EOK) {
        printf("Failed to load input data. Error: %d\n", file_res);
        status = BENCH_EALLOC;
    } else if (input_cols != 1) {
        status = BENCH_EINVALID_INPUT;
    } else if ((status = filter_bench_suite(filter_coeffs,
                                            input_data,
                                            input_rows,
                                            filter_benches,
                                            ALGO_BENCH_LEN(filter_benches))) !=
               BENCH_EOK) {
        printf("Error running filter benches.\n");
    } else if ((status = impz_bench_suite(filter_coeffs,
                                          filter_rows,
                                          filter_cols,
                                          impz_benches,
                                          ALGO_BENCH_LEN(impz_benches))) !=
               BENCH_EOK) {
        printf("Error running impz benches.\n");
    } else if ((status = conv_bench_suite(input_data,
                                          input_rows,
                                          conv_benches,
                                          ALGO_BENCH_LEN(conv_benches))) !=
               BENCH_EOK) {
        printf("Error running conv benches.");
    } else if ((status = series_bench_suite(filter_coeffs,
                                            filter_rows,
                                            filter_cols,
                                            series_benches,
                                            ALGO_BENCH_LEN(series_benches))) !=
               BENCH_EOK) {
        printf("Error running series benches.\n");
    } else if ((status = parallel_bench_suite(
                  filter_coeffs,
                  filter_rows,
                  filter_cols,
                  parallel_benches,
                  ALGO_BENCH_LEN(parallel_benches))) != BENCH_EOK) {
        printf("Error running parallel benches.\n");
    } else if ((status = feedback_bench_suite(
                  filter_coeffs,
                  filter_rows,
                  filter_cols,
                  feedback_benches,
                  ALGO_BENCH_LEN(feedback_benches))) != BENCH_EOK) {
        printf("Error running feedback benches.\n");
    } else if ((status = freqz_bench_suite(filter_coeffs,
                                           filter_rows,
                                           filter_cols,
                                           freqz_benches,
                                           ALGO_BENCH_LEN(freqz_benches))) !=
               BENCH_EOK) {
        printf("Error running freqz benches.\n");
    } else {
        bench_result_t bench = {
            filter_benches,   ALGO_BENCH_LEN(filter_benches),
            impz_benches,     ALGO_BENCH_LEN(impz_benches),
            conv_benches,     ALGO_BENCH_LEN(conv_benches),
            series_benches,   ALGO_BENCH_LEN(series_benches),
            parallel_benches, ALGO_BENCH_LEN(parallel_benches),
            feedback_benches, ALGO_BENCH_LEN(feedback_benches),
            freqz_benches,    ALGO_BENCH_LEN(freqz_benches),
        };

        bench_print_table(&bench);
    }

    free(input_data);
    free(filter_coeffs);

    return status;
}
