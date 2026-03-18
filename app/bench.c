#include "bench.h"
#include "libsig.h"
#include "main.h"

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// -----------------------------------------------------------------------------
// Private Helper Functions
// -----------------------------------------------------------------------------

static bool
_is_buffers_equal(const double* a, const double* b, size_t len)
{
    double epsilon = 1e-9;
    for (size_t i = 0; i < len; ++i) {
        if (fabs(a[i] - b[i]) > epsilon) {
            return false;
        }
    }
    return true;
}

static void
_print_bench_section(const char* section_name,
                     const algo_bench_t* benches,
                     size_t len)
{
    printf("----------------------------- %-6s -----------------------------\n",
           section_name);
    for (size_t i = 0; i < len; ++i) {
        printf("%-20s | %13.6lf | %14.3lf | %-10s\n",
               benches[i].algo_name,
               benches[i].duration,
               benches[i].duration * 1000.0,
               bench_state_to_str(benches[i].state));
    }
}

// -----------------------------------------------------------------------------
// Generic Benchmarking Engine
// -----------------------------------------------------------------------------

// A function pointer type that unpacks a void* context and runs the generic
// function
typedef void (*runner_wrapper_t)(void* ctx, generic_fn_t func);

/*
 * The universal benchmarking loop.
 * Handles timing, state recording, and equality checks for ANY algorithm.
 */
static void
_run_generic_bench(void* ctx,
                   double* out_buffer,
                   size_t out_len,
                   const double* output_correct,
                   algo_bench_t* benches,
                   size_t benches_len,
                   runner_wrapper_t runner)
{
    for (size_t i = 0; i < benches_len; ++i) {
        clock_t tic = clock();

        // Call the wrapper, which will cast the context and run the specific
        // algorithm
        runner(ctx, benches[i].func);

        clock_t toc = clock();

        bool algo_res = _is_buffers_equal(out_buffer, output_correct, out_len);

        benches[i].state = algo_res ? BENCH_SUCCESS : BENCH_FAILED;
        benches[i].duration = (double)(toc - tic) / CLOCKS_PER_SEC;
    }
}

// -----------------------------------------------------------------------------
// Algorithm-Specific Runners
// -----------------------------------------------------------------------------

static void
_filter_runner(void* ctx, generic_fn_t func)
{
    const filter_input_t* input = (const filter_input_t*)ctx;
    ((filter_fn_t)func)(input->b,
                        input->b_len,
                        input->a,
                        input->a_len,
                        input->x,
                        input->x_len,
                        input->y);
}

static void
_impz_runner(void* ctx, generic_fn_t func)
{
    const impz_input_t* input = (const impz_input_t*)ctx;
    ((impz_fn_t)func)(
      input->b, input->b_len, input->a, input->a_len, input->y, input->y_len);
}

static void
_conv_runner(void* ctx, generic_fn_t func)
{
    const conv_input_t* input = (const conv_input_t*)ctx;
    ((conv_fn_t)func)(
      input->u, input->u_len, input->v, input->v_len, input->y, input->y_len);
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

const char*
bench_state_to_str(const bench_state_t state)
{
    switch (state) {
        case BENCH_NOT_RAN:
            return "Not Ran";
        case BENCH_SUCCESS:
            return "Success";
        case BENCH_FAILED:
            return "Failed";
        default:
            return "Unknown";
    }
}

void
bench_print_table(const bench_result_t* result)
{
    printf(
      "==================================================================\n");
    printf("%-20s | %-13s | %-14s | %-10s\n",
           "Function",
           "Duration [s]",
           "Duration [ms]",
           "State");
    printf(
      "==================================================================\n");

    _print_bench_section("filter", result->filter_benches, result->filter_len);
    _print_bench_section("impz", result->impz_benches, result->impz_len);
    _print_bench_section("conv", result->conv_benches, result->conv_len);
}

algo_bench_t
init_algo_bench(const char* algo_name, const generic_fn_t fn)
{
    algo_bench_t bench = { algo_name, fn, BENCH_NOT_RAN, 0.0 };
    return bench;
}

bench_error_t
filter_bench(const filter_input_t* input,
             const double* output_correct,
             algo_bench_t* benches,
             size_t benches_len)
{
    _run_generic_bench((void*)input,
                       input->y,
                       input->x_len,
                       output_correct,
                       benches,
                       benches_len,
                       _filter_runner);
    return BENCH_EOK;
}

bench_error_t
impz_bench(const impz_input_t* input,
           const double* output_correct,
           algo_bench_t* benches,
           size_t benches_len)
{
    _run_generic_bench((void*)input,
                       input->y,
                       input->y_len,
                       output_correct,
                       benches,
                       benches_len,
                       _impz_runner);
    return BENCH_EOK;
}

bench_error_t
conv_bench(const conv_input_t* input,
           const double* output_correct,
           algo_bench_t* benches,
           size_t benches_len)
{
    _run_generic_bench((void*)input,
                       input->y,
                       input->y_len,
                       output_correct,
                       benches,
                       benches_len,
                       _conv_runner);
    return BENCH_EOK;
}

bench_error_t
filter_bench_suite(const double coeffs[FILTER_ROWS][FILTER_COLS],
                   const double* input_data,
                   size_t input_len,
                   algo_bench_t* benches,
                   size_t benches_len)
{
    bench_error_t status = BENCH_EOK;
    size_t num_correct = 0;
    double* output_correct = malloc(sizeof(double) * BUF_SIZE);
    double* output = malloc(sizeof(double) * input_len);

    if (!output_correct || !output) {
        status = BENCH_EALLOC;
    } else if (read_file_to_buffer("./data/output/filter_result.csv",
                                   output_correct,
                                   &num_correct) != 0) {
        status = BENCH_EFILE;
    } else {
        filter_input_t filter_input = {
            coeffs[0],           FILTER_COLS, coeffs[1], FILTER_COLS,
            (double*)input_data, input_len,   output
        };

        filter_bench(&filter_input, output_correct, benches, benches_len);
    }

    free(output_correct);
    free(output);

    return status;
}

bench_error_t
impz_bench_suite(const double coeffs[FILTER_ROWS][FILTER_COLS],
                 algo_bench_t* benches,
                 size_t benches_len)
{
    bench_error_t status = BENCH_EOK;
    size_t num_correct = 0;
    double* output_correct = malloc(sizeof(double) * IMPZ_OUT_LEN);
    double* output = malloc(sizeof(double) * IMPZ_OUT_LEN);

    if (!output_correct || !output) {
        status = BENCH_EALLOC;
    } else if (read_file_to_buffer("./data/output/impz_result.csv",
                                   output_correct,
                                   &num_correct) != 0) {
        status = BENCH_EFILE;
    } else {
        impz_input_t impz_input = { coeffs[0],   FILTER_COLS, coeffs[1],
                                    FILTER_COLS, output,      IMPZ_OUT_LEN };

        impz_bench(&impz_input, output_correct, benches, benches_len);
    }

    free(output_correct);
    free(output);

    return status;
}

bench_error_t
conv_bench_suite(const double* input_data,
                 size_t input_len,
                 algo_bench_t* benches,
                 size_t len)
{
    bench_error_t status = BENCH_EOK;
    size_t num_correct = 0;
    size_t impz_in_len = 0;
    double* output_correct =
      malloc(sizeof(double) * CONV_FULL_LEN(input_len, IMPZ_OUT_LEN));
    double* output =
      malloc(sizeof(double) * CONV_FULL_LEN(input_len, IMPZ_OUT_LEN));
    double* impz_input = malloc(sizeof(double) * IMPZ_OUT_LEN);

    if (!output_correct || !output || !impz_input) {
        status = BENCH_EALLOC;
    } else if (read_file_to_buffer("./data/output/conv_result.csv",
                                   output_correct,
                                   &num_correct) != 0) {
        status = BENCH_EFILE;
    } else if (num_correct != CONV_FULL_LEN(input_len, IMPZ_OUT_LEN)) {
        status = BENCH_ERR;
    } else if (read_file_to_buffer("./data/output/impz_result.csv",
                                   impz_input,
                                   &impz_in_len) != 0) {
        status = BENCH_EFILE;
    } else if (IMPZ_OUT_LEN != impz_in_len) {
        status = BENCH_ERR;
    } else {
        conv_input_t conv_input = {
            input_data,  input_len, impz_input,
            impz_in_len, output,    CONV_FULL_LEN(input_len, impz_in_len)
        };

        conv_bench(&conv_input, output_correct, benches, len);
    }

    free(output_correct);
    free(output);
    free(impz_input);

    return status;
}
