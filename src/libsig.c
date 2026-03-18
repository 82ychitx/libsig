#include "libsig.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

libsig_error_t
filter_naive_ternary(const double* b,
                     size_t b_len,
                     const double* a,
                     size_t a_len,
                     const double* x,
                     size_t x_len,
                     double* y)
{
    libsig_error_t result = LIBSIG_EOK;

    double ff_sum;
    double fb_sum;

    for (size_t n = 0; n < x_len; ++n) {
        ff_sum = 0, fb_sum = 0;

        for (size_t i = 0; i < b_len; ++i) {
            ff_sum += b[i] * (n < i ? 0 : x[n - i]);
        }

        for (size_t j = 1; j < a_len; ++j) {
            fb_sum += a[j] * (n < j ? 0 : y[n - j]);
        }

        y[n] = (1 / a[0]) * (ff_sum - fb_sum);
    }

    return result;
}

libsig_error_t
filter_split_loops(const double* b,
                   size_t b_len,
                   const double* a,
                   size_t a_len,
                   const double* x,
                   size_t x_len,
                   double* y)
{
    libsig_error_t result = LIBSIG_EOK;

    double ff_sum;
    double fb_sum;
    int n;

    int filter_rank = MAX(b_len, a_len);
    // First loop where we branch
    for (n = 0; n < filter_rank; ++n) {
        ff_sum = 0, fb_sum = 0;

        for (int i = 0; i < (int)b_len; ++i) {
            ff_sum += b[i] * ((n - i < 0) ? 0 : x[n - i]);
        }

        for (int j = 1; j < (int)a_len; ++j) {
            fb_sum += a[j] * ((n - j < 0) ? 0 : y[n - j]);
        }

        y[n] = 1 / a[0] * (ff_sum - fb_sum);
    }

    // Second loop we know that we have reached a point where (n - i) or (n -j)
    // will not throw negative indeces
    for (; n < (int)x_len; ++n) {
        ff_sum = 0, fb_sum = 0;

        for (size_t i = 0; i < b_len; ++i) {
            ff_sum += b[i] * x[n - i];
        }

        for (size_t j = 1; j < a_len; ++j) {
            fb_sum += a[j] * y[n - j];
        }

        y[n] = 1 / a[0] * (ff_sum - fb_sum);
    }

    return result;
}

libsig_error_t
filter_history_buffer(const double* b,
                      size_t b_len,
                      const double* a,
                      size_t a_len,
                      const double* x,
                      size_t x_len,
                      double* y)
{
    return LIBSIG_EOK;
}

libsig_error_t
impz(const double* b,
     size_t b_len,
     const double* a,
     size_t a_len,
     double* y,
     size_t y_len)
{
    double ff_sum;
    double fb_sum;
    uint8_t* impulse;

    libsig_error_t status = LIBSIG_EOK;

    impulse = calloc(y_len, sizeof(uint8_t));
    if (!impulse) {
        status = LIBSIG_ERR;
    } else {
        impulse[0] = 1;
        for (size_t n = 0; n < y_len; ++n) {
            ff_sum = 0, fb_sum = 0;

            for (size_t i = 0; i < b_len; ++i) {
                ff_sum += b[i] * ((n < i) ? 0 : impulse[n - i]);
            }

            for (size_t j = 1; j < a_len; ++j) {
                fb_sum += a[j] * ((n < j) ? 0 : y[n - j]);
            }

            y[n] = (1 / a[0]) * (ff_sum - fb_sum);
        }
    }

    free(impulse);

    return status;
}

libsig_error_t
conv_naive(const double* u,
           size_t u_len,
           const double* v,
           size_t v_len,
           double* y,
           size_t y_len)
{
    libsig_error_t result = LIBSIG_EOK;
    double sum;
    size_t conv_len = u_len + v_len - 1;
    if (y_len != conv_len) {
        result = LIBSIG_EINVALID_INPUT;
    } else {
        for (size_t n = 0; n < conv_len; ++n) {
            sum = 0;
            for (size_t k = 0; k < u_len; ++k) {
                if (n >= k && (n - k) < v_len) {
                    sum += u[k] * v[n - k];
                }
            }

            y[n] = sum;
        }
    }

    return result;
}

libsig_error_t
conv_bounded(const double* u,
             size_t u_len,
             const double* v,
             size_t v_len,
             double* y,
             size_t y_len)
{
    size_t lower_bound;
    size_t upper_bound;
    double sum;

    libsig_error_t result = LIBSIG_EOK;
    size_t conv_len = u_len + v_len - 1;

    if (y_len != conv_len) {
        result = LIBSIG_EINVALID_INPUT;
    } else {
        for (size_t n = 0; n < conv_len; ++n) {
            sum = 0;
            lower_bound = (n >= v_len) ? (n - v_len + 1) : 0;
            upper_bound = (n < u_len) ? n : (u_len - 1);
            for (size_t k = lower_bound; k <= upper_bound; ++k) {
                sum += u[k] * v[n - k];
            }
            y[n] = sum;
        }
    }

    return result;
}
