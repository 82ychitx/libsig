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
typedef void (*runner_fn_t)(void* ctx, generic_fn_t func);
typedef bool (*checker_fn_t)(const void* ctx, const void* expected_data);

/*
 * The universal benchmarking loop.
 * Handles timing, state recording, and equality checks for ANY algorithm.
 */
static void
_run_generic_bench(void* ctx,
                   const void* output_correct,
                   algo_bench_t* benches,
                   size_t benches_len,
                   runner_fn_t runner,
                   checker_fn_t checker)
{
    for (size_t i = 0; i < benches_len; ++i) {
        clock_t tic = clock();

        // Call the wrapper, which will cast the context and run the specific
        // algorithm
        runner(ctx, benches[i].func);

        clock_t toc = clock();

        bool algo_res = checker(ctx, output_correct);

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

static bool
_filter_checker(const void* ctx, const void* output_correct)
{
    const filter_input_t* input = (const filter_input_t*)ctx;
    const expected_1d_t* expected = (const expected_1d_t*)output_correct;

    return _is_buffers_equal(input->y, expected->data, expected->len);
}

static void
_impz_runner(void* ctx, generic_fn_t func)
{
    const impz_input_t* input = (const impz_input_t*)ctx;
    ((impz_fn_t)func)(
      input->b, input->b_len, input->a, input->a_len, input->y, input->y_len);
}

static bool
_impz_checker(const void* ctx, const void* output_correct)
{
    const impz_input_t* input = (const impz_input_t*)ctx;
    const expected_1d_t* expected = (const expected_1d_t*)output_correct;

    return _is_buffers_equal(input->y, expected->data, expected->len);
}

static void
_conv_runner(void* ctx, generic_fn_t func)
{
    const conv_input_t* input = (const conv_input_t*)ctx;
    ((conv_fn_t)func)(
      input->u, input->u_len, input->v, input->v_len, input->y, input->y_len);
}

static bool
_conv_checker(const void* ctx, const void* output_correct)
{
    const conv_input_t* input = (const conv_input_t*)ctx;
    const expected_1d_t* expected = (const expected_1d_t*)output_correct;

    return _is_buffers_equal(input->y, expected->data, expected->len);
}

static void
_series_runner(void* ctx, generic_fn_t func)
{
    const series_input_t* input = (const series_input_t*)ctx;
    ((series_fn_t)func)(input->sys1_n,
                        input->sys1_n_len,
                        input->sys1_d,
                        input->sys1_d_len,
                        input->sys2_n,
                        input->sys2_n_len,
                        input->sys2_d,
                        input->sys2_d_len,
                        input->sys_out_n,
                        input->sys_out_n_len,
                        input->sys_out_d,
                        input->sys_out_d_len);
}

static bool
_series_checker(const void* ctx, const void* expected_data)
{
    bool result = true;
    const series_input_t* input = (const series_input_t*)ctx;
    const expected_tf_t* expected = (const expected_tf_t*)expected_data;

    if (input->sys_out_n_len != expected->n_len ||
        input->sys_out_d_len != expected->d_len) {
        result = false;
    } else {
        result =
          _is_buffers_equal(input->sys_out_n, expected->n, expected->n_len) &&
          _is_buffers_equal(input->sys_out_d, expected->d, expected->d_len);
    }

    return result;
}

static void
_parallel_runner(void* ctx, generic_fn_t func)
{
    const parallel_input_t* input = (const parallel_input_t*)ctx;
    ((parallel_fn_t)func)(input->sys1_n,
                          input->sys1_n_len,
                          input->sys1_d,
                          input->sys1_d_len,
                          input->sys2_n,
                          input->sys2_n_len,
                          input->sys2_d,
                          input->sys2_d_len,
                          input->sys_out_n,
                          input->sys_out_n_len,
                          input->sys_out_d,
                          input->sys_out_d_len);
}

static bool
_parallel_checker(const void* ctx, const void* expected_data)
{
    bool result = true;
    const parallel_input_t* input = (const parallel_input_t*)ctx;
    const expected_tf_t* expected = (const expected_tf_t*)expected_data;

    if (input->sys_out_n_len != expected->n_len ||
        input->sys_out_d_len != expected->d_len) {
        result = false;
    } else {
        result =
          _is_buffers_equal(input->sys_out_n, expected->n, expected->n_len) &&
          _is_buffers_equal(input->sys_out_d, expected->d, expected->d_len);
    }

    return result;
}

static void
_feedback_runner(void* ctx, generic_fn_t func)
{
    const feedback_input_t* input = (const feedback_input_t*)ctx;
    ((feedback_fn_t)func)(input->sys1_n,
                          input->sys1_n_len,
                          input->sys1_d,
                          input->sys1_d_len,
                          input->sys2_n,
                          input->sys2_n_len,
                          input->sys2_d,
                          input->sys2_d_len,
                          input->sys_out_n,
                          input->sys_out_n_len,
                          input->sys_out_d,
                          input->sys_out_d_len);
}

static bool
_feedback_checker(const void* ctx, const void* expected_data)
{
    bool result = true;
    const feedback_input_t* input = (const feedback_input_t*)ctx;
    const expected_tf_t* expected = (const expected_tf_t*)expected_data;

    if (input->sys_out_n_len != expected->n_len ||
        input->sys_out_d_len != expected->d_len) {
        result = false;
    } else {
        result =
          _is_buffers_equal(input->sys_out_n, expected->n, expected->n_len) &&
          _is_buffers_equal(input->sys_out_d, expected->d, expected->d_len);
    }

    return result;
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
    _print_bench_section("series", result->series_benches, result->series_len);
    _print_bench_section(
      "parallel", result->parallel_benches, result->parallel_len);
}

algo_bench_t
algo_bench_init(const char* algo_name, const generic_fn_t fn)
{
    algo_bench_t bench = { algo_name, fn, BENCH_NOT_RAN, 0.0 };
    return bench;
}

bench_error_t
filter_bench(const filter_input_t* input,
             const expected_1d_t* output_correct,
             algo_bench_t* benches,
             size_t len)
{
    _run_generic_bench((void*)input,
                       (const void*)output_correct,
                       benches,
                       len,
                       _filter_runner,
                       _filter_checker);
    return BENCH_EOK;
}

bench_error_t
impz_bench(const impz_input_t* input,
           const expected_1d_t* output_correct,
           algo_bench_t* benches,
           size_t len)
{
    _run_generic_bench((void*)input,
                       (const void*)output_correct,
                       benches,
                       len,
                       _impz_runner,
                       _impz_checker);
    return BENCH_EOK;
}

bench_error_t
conv_bench(const conv_input_t* input,
           const expected_1d_t* output_correct,
           algo_bench_t* benches,
           size_t len)
{
    _run_generic_bench((void*)input,
                       (const void*)output_correct,
                       benches,
                       len,
                       _conv_runner,
                       _conv_checker);
    return BENCH_EOK;
}

bench_error_t
series_bench(const series_input_t* input,
             const expected_tf_t* output_correct,
             algo_bench_t* benches,
             size_t len)
{
    _run_generic_bench((void*)input,
                       (const void*)output_correct,
                       benches,
                       len,
                       _series_runner,
                       _series_checker);
    return BENCH_EOK;
}

bench_error_t
parallel_bench(const parallel_input_t* input,
               const expected_tf_t* output_correct,
               algo_bench_t* benches,
               size_t len)
{
    _run_generic_bench((void*)input,
                       (const void*)output_correct,
                       benches,
                       len,
                       _parallel_runner,
                       _parallel_checker);
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
        expected_1d_t expected_out = { output_correct, num_correct };

        filter_bench(&filter_input, &expected_out, benches, benches_len);
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
    size_t out_len = sizeof(double) * IMPZ_OUT_LEN;
    double* output_correct = malloc(out_len);
    double* output = malloc(out_len);

    if (!output_correct || !output) {
        status = BENCH_EALLOC;
    } else if (read_file_to_buffer("./data/output/impz_result.csv",
                                   output_correct,
                                   &num_correct) != 0) {
        status = BENCH_EFILE;
    } else {
        impz_input_t impz_input = { coeffs[0],   FILTER_COLS, coeffs[1],
                                    FILTER_COLS, output,      IMPZ_OUT_LEN };
        expected_1d_t expected_out = { output_correct, num_correct };

        impz_bench(&impz_input, &expected_out, benches, benches_len);
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
        expected_1d_t expected_out = { output_correct, num_correct };

        conv_bench(&conv_input, &expected_out, benches, len);
    }

    free(output_correct);
    free(output);
    free(impz_input);

    return status;
}

bench_error_t
series_bench_suite(const double input_coeffs[FILTER_ROWS][FILTER_COLS],
                   algo_bench_t* benches,
                   size_t len)
{
    bench_error_t status = BENCH_EOK;
    size_t num_correct_n = 0;
    size_t num_correct_d = 0;
    size_t expected_out_len = CONV_FULL_LEN(FILTER_COLS, FILTER_COLS);

    double* output_correct_n = malloc(sizeof(double) * expected_out_len);
    double* output_correct_d = malloc(sizeof(double) * expected_out_len);
    double* output_n = malloc(sizeof(double) * expected_out_len);
    double* output_d = malloc(sizeof(double) * expected_out_len);

    if (!output_correct_n || !output_correct_d || !output_n || !output_d) {
        status = BENCH_EALLOC;
    } else if (read_file_to_buffer("./data/output/series_result_n.csv",
                                   output_correct_n,
                                   &num_correct_n) != 0) {
        status = BENCH_EFILE;
    } else if (read_file_to_buffer("./data/output/series_result_d.csv",
                                   output_correct_d,
                                   &num_correct_d) != 0) {
        status = BENCH_EFILE;
    } else {
        series_input_t series_input = {
            input_coeffs[0], FILTER_COLS,   input_coeffs[1], FILTER_COLS,
            input_coeffs[2], FILTER_COLS,   input_coeffs[3], FILTER_COLS,
            output_n,        expected_out_len, output_d,        expected_out_len
        };

        expected_tf_t expected = {
            output_correct_n, expected_out_len, output_correct_d, expected_out_len
        };

        series_bench(&series_input, &expected, benches, len);
    }

    free(output_correct_n);
    free(output_correct_d);
    free(output_n);
    free(output_d);

    return status;
}

bench_error_t
parallel_bench_suite(const double input_coeffs[FILTER_ROWS][FILTER_COLS],
                     algo_bench_t* benches,
                     size_t len)
{
    bench_error_t status = BENCH_EOK;
    size_t num_correct_n = 0;
    size_t num_correct_d = 0;

    size_t expected_output_len = CONV_FULL_LEN(FILTER_COLS, FILTER_COLS);

    double* output_correct_n = malloc(sizeof(double) * expected_output_len);
    double* output_correct_d = malloc(sizeof(double) * expected_output_len);
    double* output_n = malloc(sizeof(double) * expected_output_len);
    double* output_d = malloc(sizeof(double) * expected_output_len);

    if (!output_correct_n || !output_correct_d || !output_n || !output_d) {
        status = BENCH_EALLOC;
    } else if (read_file_to_buffer("./data/output/parallel_result_n.csv",
                                   output_correct_n,
                                   &num_correct_n) != 0) {
        status = BENCH_EFILE;
    } else if (read_file_to_buffer("./data/output/parallel_result_d.csv",
                                   output_correct_d,
                                   &num_correct_d) != 0) {
        status = BENCH_EFILE;
    } else {
        parallel_input_t parallel_input = {
            input_coeffs[0], FILTER_COLS,   input_coeffs[1], FILTER_COLS,
            input_coeffs[2], FILTER_COLS,   input_coeffs[3], FILTER_COLS,
            output_n,        expected_output_len, output_d,        expected_output_len
        };

        expected_tf_t expected_output = {
            output_correct_n, expected_output_len, output_correct_d, expected_output_len
        };

        parallel_bench(&parallel_input, &expected_output, benches, len);
    }

    free(output_correct_n);
    free(output_correct_d);
    free(output_n);
    free(output_d);

    return status;
}

bench_error_t
feedback_bench(const feedback_input_t* input,
               const expected_tf_t* output_correct,
               algo_bench_t* benches,
               size_t len)
{
    _run_generic_bench((void*)input,
                       (const void*)output_correct,
                       benches,
                       len,
                       _feedback_runner,
                       _feedback_checker);
    return BENCH_EOK;
}

bench_error_t
feedback_bench_suite(const double input_coeffs[FILTER_ROWS][FILTER_COLS],
                     algo_bench_t* benches,
                     size_t len)
{
    bench_error_t status = BENCH_EOK;
    size_t num_correct_n = 0;
    size_t num_correct_d = 0;

    size_t expected_output_len = CONV_FULL_LEN(FILTER_COLS, FILTER_COLS);

    double* output_correct_n = malloc(sizeof(double) * expected_output_len);
    double* output_correct_d = malloc(sizeof(double) * expected_output_len);
    double* output_n = malloc(sizeof(double) * expected_output_len);
    double* output_d = malloc(sizeof(double) * expected_output_len);

    if (!output_correct_n || !output_correct_d || !output_n || !output_d) {
        status = BENCH_EALLOC;
    } else if (read_file_to_buffer("./data/output/feedback_result_n.csv",
                                   output_correct_n,
                                   &num_correct_n) != 0) {
        status = BENCH_EFILE;
    } else if (read_file_to_buffer("./data/output/feedback_result_d.csv",
                                   output_correct_d,
                                   &num_correct_d) != 0) {
        status = BENCH_EFILE;
    } else {
        feedback_input_t feedback_input = {
            input_coeffs[0], FILTER_COLS,   input_coeffs[1], FILTER_COLS,
            input_coeffs[2], FILTER_COLS,   input_coeffs[3], FILTER_COLS,
            output_n,        expected_output_len, output_d,        expected_output_len
        };

        expected_tf_t expected_output = {
            output_correct_n, expected_output_len, output_correct_d, expected_output_len
        };

        feedback_bench(&feedback_input, &expected_output, benches, len);
    }

    free(output_correct_n);
    free(output_correct_d);
    free(output_n);
    free(output_d);

    return status;

}
