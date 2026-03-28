#include "bench.h"
#include "file_io.h"
#include "libsig.h"

#include <complex.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// -----------------------------------------------------------------------------
// Private Helper Functions
// -----------------------------------------------------------------------------

static bool
_is_double_buffs_equal(const double* a,
                       size_t a_len,
                       const double* b,
                       size_t b_len)
{
    bool result = true;

    if (a_len != b_len) {
        result = false;
    } else {
        double epsilon = 1e-9;
        for (size_t i = 0; i < a_len; ++i) {
            if (fabs(a[i] - b[i]) > epsilon) {
                result = false;
                break;
            }
        }
    }

    return result;
}

static bool
_is_complex_buffs_equal(const double complex* a,
                        size_t a_len,
                        const double complex* b,
                        size_t b_len)
{
    bool result = true;

    if (a_len != b_len) {
        result = false;
    } else {
        double epsilon = 1e-9;
        for (size_t i = 0; i < a_len; ++i) {
            if (cabs(a[i] - b[i]) > epsilon) {
                result = false;
                break;
            }
        }
    }

    return result;
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
// Algorithm-Specific Runners & Checkers
// -----------------------------------------------------------------------------

// ========================== FILTER ==========================
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

    return _is_double_buffs_equal(
      input->y, input->x_len, expected->data, expected->len);
}

// ========================== IMPZ ==========================

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

    return _is_double_buffs_equal(
      input->y, input->y_len, expected->data, expected->len);
}

// ========================== CONV ==========================

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

    return _is_double_buffs_equal(
      input->y, input->y_len, expected->data, expected->len);
}

// ========================== SERIES ==========================

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
        result = _is_double_buffs_equal(input->sys_out_n,
                                        input->sys_out_n_len,
                                        expected->n,
                                        expected->n_len) &&
                 _is_double_buffs_equal(input->sys_out_d,
                                        input->sys_out_d_len,
                                        expected->d,
                                        expected->d_len);
    }

    return result;
}

// ========================== PARALLEL ==========================

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
        result = _is_double_buffs_equal(input->sys_out_n,
                                        input->sys_out_n_len,
                                        expected->n,
                                        expected->n_len) &&
                 _is_double_buffs_equal(input->sys_out_d,
                                        input->sys_out_d_len,
                                        expected->d,
                                        expected->d_len);
    }

    return result;
}

// ========================== FEEDBACK ==========================

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
        result = _is_double_buffs_equal(input->sys_out_n,
                                        input->sys_out_n_len,
                                        expected->n,
                                        expected->n_len) &&
                 _is_double_buffs_equal(input->sys_out_d,
                                        input->sys_out_d_len,
                                        expected->d,
                                        expected->d_len);
    }

    return result;
}

// ========================== FREQZ ==========================
static void
_freqz_runner(void* ctx, generic_fn_t func)
{
    const freqz_input_t* input = (const freqz_input_t*)ctx;
    ((freqz_fn_t)func)(input->b,
                       input->b_len,
                       input->a,
                       input->a_len,
                       input->w,
                       input->w_len,
                       input->h);
}

static bool
_freqz_checker(const void* ctx, const void* expected_data)
{
    bool result = true;
    const freqz_input_t* input = (const freqz_input_t*)ctx;
    const expected_1d_complex_t* expected =
      (const expected_1d_complex_t*)expected_data;

    if (input->w_len != expected->len) {
        result = false;
    } else {
        result = _is_complex_buffs_equal(
          input->h, input->w_len, expected->data, expected->len);
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
    _print_bench_section(
      "feedback", result->feedback_benches, result->feedback_len);
    _print_bench_section("freqz", result->freqz_benches, result->freqz_len);
}

algo_bench_t
algo_bench_init(const char* algo_name, const generic_fn_t fn)
{
    algo_bench_t bench = { algo_name, fn, BENCH_NOT_RAN, 0.0 };
    return bench;
}

// ========================== FILTER ==========================
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
filter_bench_suite(const double* coeffs,
                   size_t coeffs_rows,
                   size_t coeffs_cols,
                   const double* input_data,
                   size_t input_len,
                   algo_bench_t* benches,
                   size_t benches_len)
{
    bench_error_t status = BENCH_EOK;
    size_t correct_rows, correct_cols;
    double* correct_output;
    double* output;

    if (coeffs_rows != 2) {
        status = BENCH_EINVALID_INPUT;
    } else if (file_io_read_double_matrix("./data/output/filter_result.csv",
                                          &correct_output,
                                          &correct_rows,
                                          &correct_cols) != FILE_IO_EOK) {
        status = BENCH_EFILE;
    } else {
        if ((output = malloc(sizeof(double) * input_len)) == NULL) {
            status = BENCH_EALLOC;
        } else {
            filter_input_t filter_input = {
                &coeffs[0],  coeffs_cols, &coeffs[coeffs_cols],
                coeffs_cols, input_data,  input_len,
                output
            };
            expected_1d_t expected_out = { correct_output, correct_rows };

            filter_bench(&filter_input, &expected_out, benches, benches_len);

            free(output);
        }

        free(correct_output);
    }

    return status;
}

// ========================== IMPZ ==========================
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
impz_bench_suite(const double* coeffs,
                 size_t coeffs_rows,
                 size_t coeffs_cols,
                 algo_bench_t* benches,
                 size_t benches_len)
{
    bench_error_t status = BENCH_EOK;
    size_t correct_rows, correct_cols;
    double* output_correct;
    double* output;

    if (coeffs_rows != 2) {
        status = BENCH_EINVALID_INPUT;
    } else if (file_io_read_double_matrix("./data/output/impz_result.csv",
                                          &output_correct,
                                          &correct_rows,
                                          &correct_cols) != FILE_IO_EOK) {
        status = BENCH_EFILE;
    } else {
        if ((output = malloc(correct_rows * sizeof(double))) == NULL) {
            status = BENCH_EALLOC;
        } else {
            impz_input_t impz_input = {
                &coeffs[0],  coeffs_cols, &coeffs[coeffs_cols],
                coeffs_cols, output,      correct_rows,
            };
            expected_1d_t expected_out = { output_correct, correct_rows };

            impz_bench(&impz_input, &expected_out, benches, benches_len);

            free(output);
        }
        free(output_correct);
    }

    return status;
}

// ========================== CONV ==========================

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
conv_bench_suite(const double* input_data,
                 size_t input_len,
                 algo_bench_t* benches,
                 size_t len)
{
    bench_error_t status = BENCH_EOK;
    size_t correct_rows, correct_cols, impz_rows, impz_cols;
    double* output_correct;
    double* output;
    double* impz_input;

    if ((file_io_read_double_matrix("./data/output/conv_result.csv",
                                    &output_correct,
                                    &correct_rows,
                                    &correct_cols) != FILE_IO_EOK) ||
        (file_io_read_double_matrix("./data/output/impz_result.csv",
                                    &impz_input,
                                    &impz_rows,
                                    &impz_cols) != FILE_IO_EOK)) {
        status = BENCH_EFILE;
    } else {
        if (correct_rows != CONV_FULL_LEN(input_len, impz_rows)) {
            status = BENCH_EINVALID_INPUT;
        } else if ((output = malloc(CONV_FULL_LEN(input_len, impz_rows) *
                                    sizeof(double))) == NULL) {
            status = BENCH_EALLOC;
        } else {
            conv_input_t conv_input = {
                input_data, input_len, impz_input,
                impz_rows,  output,    CONV_FULL_LEN(input_len, impz_rows)
            };
            expected_1d_t expected_out = { output_correct, correct_rows };

            conv_bench(&conv_input, &expected_out, benches, len);

            free(output);
        }

        free(output_correct);
        free(impz_input);
    }

    return status;
}

// ========================== SERIES ==========================

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
series_bench_suite(const double* input_coeffs,
                   size_t coeffs_rows,
                   size_t coeffs_cols,
                   algo_bench_t* benches,
                   size_t len)
{
    bench_error_t status = BENCH_EOK;
    size_t correct_rows_n, correct_cols_n, correct_rows_d, correct_cols_d;
    double *output_correct_n, *output_correct_d;
    double *output_n, *output_d;
    size_t expected_out_len = CONV_FULL_LEN(coeffs_cols, coeffs_cols);

    if (coeffs_rows != 4) {
        status = BENCH_EINVALID_INPUT;
    } else if ((file_io_read_double_matrix("./data/output/series_result_n.csv",
                                           &output_correct_n,
                                           &correct_rows_n,
                                           &correct_cols_n) != FILE_IO_EOK) ||
               (file_io_read_double_matrix("./data/output/series_result_d.csv",
                                           &output_correct_d,
                                           &correct_rows_d,
                                           &correct_cols_d) != FILE_IO_EOK)) {
        status = BENCH_EFILE;
    } else {
        if (correct_cols_n != expected_out_len ||
            correct_cols_d != expected_out_len) {
            status = BENCH_EINVALID_INPUT;
        } else if (((output_n = malloc(expected_out_len * sizeof(double))) ==
                    NULL) ||
                   ((output_d = malloc(expected_out_len * sizeof(double))) ==
                    NULL)) {
            status = BENCH_EALLOC;
        } else {
            series_input_t series_input = { &input_coeffs[0 * coeffs_cols],
                                            coeffs_cols,
                                            &input_coeffs[1 * coeffs_cols],
                                            coeffs_cols,
                                            &input_coeffs[2 * coeffs_cols],
                                            coeffs_cols,
                                            &input_coeffs[3 * coeffs_cols],
                                            coeffs_cols,
                                            output_n,
                                            expected_out_len,
                                            output_d,
                                            expected_out_len };

            expected_tf_t expected = { output_correct_n,
                                       correct_cols_n,
                                       output_correct_d,
                                       correct_cols_d };

            series_bench(&series_input, &expected, benches, len);

            free(output_n);
            free(output_d);
        }

        free(output_correct_n);
        free(output_correct_d);
    }

    return status;
}

// ========================== PARALLEL ==========================

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
parallel_bench_suite(const double* input_coeffs,
                     size_t coeffs_rows,
                     size_t coeffs_cols,
                     algo_bench_t* benches,
                     size_t len)
{
    bench_error_t status = BENCH_EOK;
    size_t correct_rows_n, correct_cols_n, correct_rows_d, correct_cols_d;
    double *output_correct_n, *output_correct_d;
    double *output_n, *output_d;
    size_t expected_out_len = CONV_FULL_LEN(coeffs_cols, coeffs_cols);

    if (coeffs_rows != 4) {
        status = BENCH_EINVALID_INPUT;
    } else if ((file_io_read_double_matrix(
                  "./data/output/parallel_result_n.csv",
                  &output_correct_n,
                  &correct_rows_n,
                  &correct_cols_n) != FILE_IO_EOK) ||
               (file_io_read_double_matrix(
                  "./data/output/parallel_result_d.csv",
                  &output_correct_d,
                  &correct_rows_d,
                  &correct_cols_d) != FILE_IO_EOK)) {
        status = BENCH_EFILE;
    } else {
        if (correct_cols_n != expected_out_len ||
            correct_cols_d != expected_out_len) {
            status = BENCH_EINVALID_INPUT;
        } else if (((output_n = malloc(expected_out_len * sizeof(double))) ==
                    NULL) ||
                   ((output_d = malloc(expected_out_len * sizeof(double))) ==
                    NULL)) {
            status = BENCH_EALLOC;
        } else {
            parallel_input_t parallel_input = { &input_coeffs[0 * coeffs_cols],
                                                coeffs_cols,
                                                &input_coeffs[1 * coeffs_cols],
                                                coeffs_cols,
                                                &input_coeffs[2 * coeffs_cols],
                                                coeffs_cols,
                                                &input_coeffs[3 * coeffs_cols],
                                                coeffs_cols,
                                                output_n,
                                                expected_out_len,
                                                output_d,
                                                expected_out_len };

            expected_tf_t expected_output = { output_correct_n,
                                              correct_cols_n,
                                              output_correct_d,
                                              correct_cols_d };

            parallel_bench(&parallel_input, &expected_output, benches, len);

            free(output_n);
            free(output_d);
        }

        free(output_correct_n);
        free(output_correct_d);
    }

    return status;
}

// ========================== FEEDBACK ==========================

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
feedback_bench_suite(const double* input_coeffs,
                     size_t coeffs_rows,
                     size_t coeffs_cols,
                     algo_bench_t* benches,
                     size_t len)
{
    bench_error_t status = BENCH_EOK;
    size_t correct_rows_n, correct_cols_n, correct_rows_d, correct_cols_d;
    double *output_correct_n, *output_correct_d;
    double *output_n, *output_d;
    size_t expected_out_len = CONV_FULL_LEN(coeffs_cols, coeffs_cols);

    if (coeffs_rows != 4) {
        status = BENCH_EINVALID_INPUT;
    } else if ((file_io_read_double_matrix(
                  "./data/output/feedback_result_n.csv",
                  &output_correct_n,
                  &correct_rows_n,
                  &correct_cols_n) != FILE_IO_EOK) ||
               (file_io_read_double_matrix(
                  "./data/output/feedback_result_d.csv",
                  &output_correct_d,
                  &correct_rows_d,
                  &correct_cols_d) != FILE_IO_EOK)) {
        status = BENCH_EFILE;
    } else {
        if (correct_cols_n != expected_out_len ||
            correct_cols_d != expected_out_len) {
            status = BENCH_EINVALID_INPUT;
        } else if (((output_n = malloc(expected_out_len * sizeof(double))) ==
                    NULL) ||
                   ((output_d = malloc(expected_out_len * sizeof(double))) ==
                    NULL)) {
            status = BENCH_EALLOC;
        } else {
            feedback_input_t feedback_input = { &input_coeffs[0 * coeffs_cols],
                                                coeffs_cols,
                                                &input_coeffs[1 * coeffs_cols],
                                                coeffs_cols,
                                                &input_coeffs[2 * coeffs_cols],
                                                coeffs_cols,
                                                &input_coeffs[3 * coeffs_cols],
                                                coeffs_cols,
                                                output_n,
                                                expected_out_len,
                                                output_d,
                                                expected_out_len };

            expected_tf_t expected_output = { output_correct_n,
                                              correct_cols_n,
                                              output_correct_d,
                                              correct_cols_d };

            feedback_bench(&feedback_input, &expected_output, benches, len);

            free(output_n);
            free(output_d);
        }

        free(output_correct_n);
        free(output_correct_d);
    }

    return status;
}

// ========================== FREQZ ==========================
bench_error_t
freqz_bench(const freqz_input_t* input,
            const expected_1d_complex_t* output_correct,
            algo_bench_t* benches,
            size_t len)
{
    _run_generic_bench((void*)input,
                       (const void*)output_correct,
                       benches,
                       len,
                       _freqz_runner,
                       _freqz_checker);
    return BENCH_EOK;
}

bench_error_t
freqz_bench_suite(const double* input_coeffs,
                  size_t coeffs_rows,
                  size_t coeffs_cols,
                  algo_bench_t* benches,
                  size_t len)
{
    bench_error_t status = BENCH_EOK;
    size_t correct_rows, correct_cols, input_rows, input_cols;
    double complex *output_correct, *output;
    double* input_freqs;

    if (coeffs_rows != 2) {
        status = BENCH_EINVALID_INPUT;
    } else if ((file_io_read_double_matrix("./data/input/normalized_freqs.csv",
                                           &input_freqs,
                                           &input_rows,
                                           &input_cols) != FILE_IO_EOK) ||
               (file_io_read_complex_matrix("./data/output/freqz_result.csv",
                                            &output_correct,
                                            &correct_rows,
                                            &correct_cols) != FILE_IO_EOK)) {
        status = BENCH_EFILE;
    } else {
        if ((output = malloc(input_rows * sizeof(double complex))) == NULL) {
            status = BENCH_EALLOC;
        } else {
            freqz_input_t freqz_input = { .b = &input_coeffs[0],
                                          .b_len = coeffs_cols,
                                          .a = &input_coeffs[coeffs_cols],
                                          .a_len = coeffs_cols,
                                          .w = input_freqs,
                                          .w_len = input_rows,
                                          .h = output };

            expected_1d_complex_t expected_output = { .data = output_correct,
                                                      .len = correct_rows };

            freqz_bench(&freqz_input, &expected_output, benches, len);

            free(output);
        }

        free(output_correct);
        free(input_freqs);
    }

    return status;
}
