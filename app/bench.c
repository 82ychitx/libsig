#include "bench.h"

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static bool
_is_buffers_equal(const double* a, const double* b, size_t len)
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

void
bench_print_table(bench_result_t* result)
{
    printf("==========================================================\n");
    printf("Function\t\t| Duration [s]\t| Duration [ms]\t| Success?\t\n");
    printf("==========================================================\n");

    // Print the filter benches
    printf("------------------------- filter -------------------------\n");
    for (size_t i = 0; i < result->filter_len; ++i) {
        printf("%s         | %lf | %lf | %d \n",
               result->filter_benches[i].algo_name,
               result->filter_benches[i].duration,
               result->filter_benches[i].duration * 1000,
               result->filter_benches[i].state);
    }
    printf("-------------------------- conv --------------------------\n");
    /* printf("conv         | %13.6f | %14.3f \n", time_conv, time_conv * 1000);
     */
    /* printf("----------------------------------------------------\n"); */
    /* printf("freqz        | %13.8f | %14.5f \n", time_freqz, time_freqz *
     * 1000); */
    /* printf( */
    /*   "series       | %13.8f | %14.5f \n", time_series, time_series * 1000);
     */
    /* printf( */
    /*   "parallel     | %13.8f | %14.5f \n", time_parallel, time_parallel *
     * 1000); */
    /* printf( */
    /*   "feedback     | %13.8f | %14.5f \n", time_feedback, time_feedback *
     * 1000); */
    printf("==========================================================\n");
}

bench_error_t
filter_bench(const filter_input_t* input,
             const double* output_correct,
             filter_bench_t* benches,
             size_t benches_len)
{
    bool result = BENCH_EOK;

    clock_t tic;
    for (size_t i = 0; i < benches_len; ++i) {
        bool algo_res;

        tic = clock();
        benches[i].func(input->a,
                        input->a_len,
                        input->b,
                        input->b_len,
                        input->x,
                        input->x_len,
                        input->y);
        tic = clock() - tic;

        algo_res = _is_buffers_equal(input->y, output_correct, input->x_len);

        benches[i].state = algo_res ? BENCH_SUCCESS : BENCH_FAILED;
        benches[i].duration = (double)tic / CLOCKS_PER_SEC;
    }

    return result;
}
