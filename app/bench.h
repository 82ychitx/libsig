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

// Generic function pointer which will make it possible to work with multiple functions for the algo_bench_t
typedef void (*generic_fn_t)(void);

typedef struct
{
    char* algo_name;
    generic_fn_t func;
    bench_state_t state;
    double duration;
} algo_bench_t;

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
    const double* b;
    size_t b_len;
    const double* a;
    size_t a_len;
    double* y;
    size_t y_len;
} impz_input_t;

typedef struct
{
    algo_bench_t* filter_benches;
    size_t filter_len;
    algo_bench_t* impz_benches;
    size_t impz_len;
} bench_result_t;

void
bench_print_table(bench_result_t* result);

bench_error_t
filter_bench(const filter_input_t* input,
             const double* output_correct,
             algo_bench_t* benches,
             size_t benches_len);

bench_error_t
impz_bench(const impz_input_t* input,
           const double* output_correct,
           algo_bench_t* benches,
           size_t benches_len);

#endif // BENCH_H_
