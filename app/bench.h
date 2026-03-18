#ifndef BENCH_H_
#define BENCH_H_

#include "libsig.h"
#include "main.h"

#define ALGO_BENCH_LEN(p_benches) (sizeof((p_benches)) / sizeof(algo_bench_t))
#define IMPZ_OUT_LEN 100

typedef enum
{
    BENCH_EOK = 0,
    BENCH_EINVALID_INPUT,
    BENCH_EFILE,
    BENCH_EALLOC,
    BENCH_ERR,
} bench_error_t;

typedef enum
{
    BENCH_NOT_RAN = 0,
    BENCH_SUCCESS,
    BENCH_FAILED,
} bench_state_t;

// Generic function pointer which will make it possible to work with multiple
// functions for the algo_bench_t
typedef void (*generic_fn_t)(void);

typedef struct
{
    const char* algo_name;
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
    const double* u;
    size_t u_len;
    const double* v;
    size_t v_len;
    double* y;
    size_t y_len;
} conv_input_t;

typedef struct
{
    algo_bench_t* filter_benches;
    size_t filter_len;
    algo_bench_t* impz_benches;
    size_t impz_len;
    algo_bench_t* conv_benches;
    size_t conv_len;
} bench_result_t;

const char*
bench_state_to_str(const bench_state_t state);

void
bench_print_table(const bench_result_t* result);

algo_bench_t
init_algo_bench(const char* algo_name, const generic_fn_t fn);

bench_error_t
filter_bench(const filter_input_t* input,
             const double* output_correct,
             algo_bench_t* benches,
             size_t len);

bench_error_t
filter_bench_suite(const double coeffs[FILTER_ROWS][FILTER_COLS],
                   const double* input_data,
                   size_t input_len,
                   algo_bench_t* benches,
                   size_t benches_len);

bench_error_t
impz_bench(const impz_input_t* input,
           const double* output_correct,
           algo_bench_t* benches,
           size_t len);

bench_error_t
impz_bench_suite(const double coeffs[FILTER_ROWS][FILTER_COLS],
                 algo_bench_t* benches,
                 size_t benches_len);

bench_error_t
conv_bench(const conv_input_t* input,
           const double* output_correct,
           algo_bench_t* benches,
           size_t len);

bench_error_t
conv_bench_suite(const double* input_data,
                 size_t input_len,
                 algo_bench_t* benches,
                 size_t len);

#endif // BENCH_H_
