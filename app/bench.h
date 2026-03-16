#ifndef BENCH_H_
#define BENCH_H_

#include "libsig.h"

typedef enum
{
    BENCH_EOK = 0,
    BENCH_EINVALID_INPUT,
    BENCH_EFILE,
    BENCH_ERR
} bench_error_t;

typedef enum
{
    BENCH_NOT_RAN = 0,
    BENCH_SUCCESS,
    BENCH_FAILED,
} bench_state_t;

typedef struct
{
    char* algo_name;
    filter_fn_t func;
    bench_state_t state;
    double duration;
} filter_bench_t;

typedef struct
{
    const double* b;
    size_t b_len;
    const double* a;
    size_t a_len;
    const double* x;
    size_t x_len;
    double* y;
} filter_input_t;

typedef struct
{
    filter_bench_t* filter_benches;
    size_t filter_len;
} bench_result_t;

void
bench_print_table(bench_result_t* result);

bench_error_t
filter_bench(const filter_input_t* input,
             const double* output_correct,
             filter_bench_t* benches,
             size_t benches_len);

#endif // BENCH_H_
