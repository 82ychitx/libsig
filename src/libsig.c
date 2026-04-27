#include "libsig.h"
#include <complex.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

libsig_error_t
filter_naive_ternary(const double* restrict b,
                     size_t b_len,
                     const double* restrict a,
                     size_t a_len,
                     const double* restrict x,
                     size_t x_len,
                     double* y)
{
    libsig_error_t result = LIBSIG_EOK;

    double ff_sum;
    double fb_sum;

    for (size_t n = 0; n < x_len; ++n) {
        ff_sum = 0.0, fb_sum = 0.0;

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
filter_split_loops(const double* restrict b,
                   size_t b_len,
                   const double* restrict a,
                   size_t a_len,
                   const double* restrict x,
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
        ff_sum = 0.0, fb_sum = 0.0;

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
        ff_sum = 0.0, fb_sum = 0.0;

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
filter_history_buffer(const double* restrict b,
                      size_t b_len,
                      const double* restrict a,
                      size_t a_len,
                      const double* restrict x,
                      size_t x_len,
                      double* restrict y)
{
    libsig_error_t result = LIBSIG_EOK;

    size_t filter_rank = MAX(b_len, a_len) - 1;
    double* state_buf = calloc(filter_rank, sizeof(double));
    double w_current;

    // Right now we assume that "a" coefficients are normalized - starting
    // from 1.

    for (size_t n = 0; n < x_len; ++n) {
        w_current = x[n];

        for (size_t k = 1; k < a_len; ++k) {
            w_current -= a[k] * state_buf[k - 1];
        }

        y[n] = b[0] * w_current;

        for (size_t k = 1; k < b_len; ++k) {
            y[n] += b[k] * state_buf[k - 1];
        }

        for (size_t k = filter_rank - 1; k > 0; --k) {
            state_buf[k] = state_buf[k - 1];
        }

        state_buf[0] = w_current;
    }

    free(state_buf);
    return result;
}

libsig_error_t
impz_alloc_dirac(const double* restrict b,
                 size_t b_len,
                 const double* restrict a,
                 size_t a_len,
                 double* restrict y,
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
            ff_sum = 0.0, fb_sum = 0.0;

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
impz_optimized(const double* restrict b,
               size_t b_len,
               const double* restrict a,
               size_t a_len,
               double* restrict y,
               size_t y_len)
{
    libsig_error_t status = LIBSIG_EOK;
    size_t filter_rank = MAX(b_len, a_len);
    size_t boundary = MIN(filter_rank, y_len);
    double ff_sum, fb_sum;

    double a0_inv = 1.0 / a[0];

    if (y_len == 0 || a_len == 0 || b_len == 0) {
        status = LIBSIG_EINVALID_INPUT;
    } else {
        for (size_t n = 0; n < boundary; ++n) {
            ff_sum = (n < b_len) ? b[n] : 0.0;
            fb_sum = 0.0;

            for (size_t j = 1; j <= n && j < a_len; ++j) {
                fb_sum += a[j] * y[n - j];
            }

            y[n] = (ff_sum - fb_sum) * a0_inv;
        }

        for (size_t n = boundary; n < y_len; ++n) {
            fb_sum = 0.0;

            for (size_t j = 1; j < a_len; ++j) {
                fb_sum += a[j] * y[n - j];
            }

            y[n] = (-fb_sum) * a0_inv;
        }
    }

    return status;
}

libsig_error_t
conv_naive(const double* restrict u,
           size_t u_len,
           const double* restrict v,
           size_t v_len,
           double* restrict y,
           size_t y_len)
{
    libsig_error_t result = LIBSIG_EOK;
    double sum;
    size_t conv_len = u_len + v_len - 1;
    if (y_len != conv_len) {
        result = LIBSIG_EINVALID_INPUT;
    } else {
        for (size_t n = 0; n < conv_len; ++n) {
            sum = 0.0;
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
conv_bounded(const double* restrict u,
             size_t u_len,
             const double* restrict v,
             size_t v_len,
             double* restrict y,
             size_t y_len)
{
    libsig_error_t result = LIBSIG_EOK;

    size_t lower_bound;
    size_t upper_bound;
    double sum;

    size_t conv_len = u_len + v_len - 1;

    if (y_len != conv_len) {
        result = LIBSIG_EINVALID_INPUT;
    } else {
        for (size_t n = 0; n < conv_len; ++n) {
            sum = 0.0;
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

static size_t
_next_power_of_2(size_t n)
{
    size_t result = 1;

    if (n != 0) {
        while (result < n) {
            result <<= 1;
        }
    }

    return result;
}

libsig_error_t
conv_fft_single_thread(const double* restrict u,
                       size_t u_len,
                       const double* restrict v,
                       size_t v_len,
                       double* restrict y,
                       size_t y_len)
{
    libsig_error_t result = LIBSIG_EOK;

    size_t conv_len = CONV_FULL_LEN(u_len, v_len);
    size_t fft_len = _next_power_of_2(conv_len);
    double complex* p1;
    double complex* p2;
    double complex* y_fft;

    if (y_len != conv_len) {
        result = LIBSIG_EINVALID_INPUT;
    } else if ((p1 = malloc(fft_len * sizeof(double complex))) == NULL) {
        result = LIBSIG_ERR;
    } else if ((p2 = malloc(fft_len * sizeof(double complex))) == NULL) {
        free(p1);
        result = LIBSIG_ERR;
    } else if ((y_fft = malloc(fft_len * sizeof(double complex))) == NULL) {
        free(p1);
        free(p2);
        result = LIBSIG_ERR;
    } else {
        if (fft(u, u_len, p1, fft_len) != LIBSIG_EOK) {
            result = LIBSIG_ERR;
        } else if (fft(v, v_len, p2, fft_len) != LIBSIG_EOK) {
            result = LIBSIG_ERR;
        } else {
            for (size_t i = 0; i < fft_len; ++i) {
                y_fft[i] = p1[i] * p2[i];
            }

            // IFFT and reuse the p1 array
            if (ifft(y_fft, fft_len, p1) != LIBSIG_EOK) {
                result = LIBSIG_ERR;
            } else {
                // Extract the real part into the output
                for (size_t i = 0; i < y_len; ++i) {
                    y[i] = creal(p1[i]);
                }
            }
        }

        free(p1);
        free(p2);
        free(y_fft);
    }

    return result;
}

libsig_error_t
conv_fft_parallel(const double* u,
                  size_t u_len,
                  const double* v,
                  size_t v_len,
                  double* y,
                  size_t y_len)
{
    libsig_error_t result = LIBSIG_EOK;

    size_t conv_len = CONV_FULL_LEN(u_len, v_len);
    size_t fft_len = _next_power_of_2(conv_len);
    double complex* p1;
    double complex* p2;
    double complex* y_fft;

    if (y_len != conv_len) {
        result = LIBSIG_EINVALID_INPUT;
    } else if ((p1 = malloc(fft_len * sizeof(double complex))) == NULL) {
        result = LIBSIG_ERR;
    } else if ((p2 = malloc(fft_len * sizeof(double complex))) == NULL) {
        free(p1);
        result = LIBSIG_ERR;
    } else if ((y_fft = malloc(fft_len * sizeof(double complex))) == NULL) {
        free(p1);
        free(p2);
        result = LIBSIG_ERR;
    } else {
        libsig_error_t err_u = LIBSIG_EOK;
        libsig_error_t err_v = LIBSIG_EOK;

// Run the two forward FFTs simultaneously
#pragma omp parallel sections
        {
#pragma omp section
            {
                err_u = fft(u, u_len, p1, fft_len);
            }
#pragma omp section
            {
                err_v = fft(v, v_len, p2, fft_len);
            }
        }

        // Check if either thread failed
        if (err_u != LIBSIG_EOK || err_v != LIBSIG_EOK) {
            result = LIBSIG_ERR;
        } else {
            for (size_t i = 0; i < fft_len; ++i) {
                y_fft[i] = p1[i] * p2[i];
            }

            // IFFT and reuse the p1 array
            if (ifft(y_fft, fft_len, p1) != LIBSIG_EOK) {
                result = LIBSIG_ERR;
            } else {
                // Extract the real part into the output
                for (size_t i = 0; i < y_len; ++i) {
                    y[i] = creal(p1[i]);
                }
            }
        }

        free(p1);
        free(p2);
        free(y_fft);
    }

    return result;
}

libsig_error_t
series_naive(const double* sys1_n,
             size_t sys1_n_len,
             const double* sys1_d,
             size_t sys1_d_len,
             const double* sys2_n,
             size_t sys2_n_len,
             const double* sys2_d,
             size_t sys2_d_len,
             double* sys_out_n,
             size_t sys_out_n_len,
             double* sys_out_d,
             size_t sys_out_d_len)
{
    libsig_error_t result = LIBSIG_EOK;
    if (sys_out_n_len != CONV_FULL_LEN(sys1_n_len, sys2_n_len) ||
        sys_out_d_len != CONV_FULL_LEN(sys1_d_len, sys2_d_len)) {
        result = LIBSIG_EINVALID_INPUT;
    } else if (conv_naive(sys1_n,
                          sys1_n_len,
                          sys2_n,
                          sys2_n_len,
                          sys_out_n,
                          sys_out_n_len) != LIBSIG_EOK) {
        result = LIBSIG_ERR;
    } else if (conv_naive(sys1_d,
                          sys1_d_len,
                          sys2_d,
                          sys2_d_len,
                          sys_out_d,
                          sys_out_d_len) != LIBSIG_EOK) {
        result = LIBSIG_ERR;
    }

    return result;
}

libsig_error_t
series_bounded(const double* sys1_n,
               size_t sys1_n_len,
               const double* sys1_d,
               size_t sys1_d_len,
               const double* sys2_n,
               size_t sys2_n_len,
               const double* sys2_d,
               size_t sys2_d_len,
               double* sys_out_n,
               size_t sys_out_n_len,
               double* sys_out_d,
               size_t sys_out_d_len)
{
    libsig_error_t result = LIBSIG_EOK;
    if (sys_out_n_len != CONV_FULL_LEN(sys1_n_len, sys2_n_len) ||
        sys_out_d_len != CONV_FULL_LEN(sys1_d_len, sys2_d_len)) {
        result = LIBSIG_EINVALID_INPUT;
    } else if (conv_bounded(sys1_n,
                            sys1_n_len,
                            sys2_n,
                            sys2_n_len,
                            sys_out_n,
                            sys_out_n_len) != LIBSIG_EOK) {
        result = LIBSIG_ERR;
    } else if (conv_bounded(sys1_d,
                            sys1_d_len,
                            sys2_d,
                            sys2_d_len,
                            sys_out_d,
                            sys_out_d_len) != LIBSIG_EOK) {
        result = LIBSIG_ERR;
    }

    return result;
}

libsig_error_t
parallel_naive(const double* sys1_n,
               size_t sys1_n_len,
               const double* sys1_d,
               size_t sys1_d_len,
               const double* sys2_n,
               size_t sys2_n_len,
               const double* sys2_d,
               size_t sys2_d_len,
               double* sys_out_n,
               size_t sys_out_n_len,
               double* sys_out_d,
               size_t sys_out_d_len)
{
    libsig_error_t result = LIBSIG_EOK;
    double *p1, *p2;
    size_t p1_len = CONV_FULL_LEN(sys1_n_len, sys2_d_len);
    size_t p2_len = CONV_FULL_LEN(sys2_n_len, sys1_d_len);
    size_t out_num_len = MAX(p1_len, p2_len);

    if (sys_out_n_len != out_num_len ||
        sys_out_d_len != CONV_FULL_LEN(sys1_d_len, sys2_d_len)) {
        result = LIBSIG_EINVALID_INPUT;
    } else if ((p1 = calloc(out_num_len, sizeof(double))) == NULL) {
        result = LIBSIG_ERR;
    } else if ((p2 = calloc(out_num_len, sizeof(double))) == NULL) {
        free(p1);
        result = LIBSIG_ERR;
    } else {
        if (conv_naive(sys1_d,
                       sys1_d_len,
                       sys2_d,
                       sys2_d_len,
                       sys_out_d,
                       sys_out_d_len) != LIBSIG_EOK) {
            result = LIBSIG_ERR;
        } else if (conv_naive(
                     sys1_n, sys1_n_len, sys2_d, sys2_d_len, p1, p1_len) !=
                   LIBSIG_EOK) {
            result = LIBSIG_ERR;
        } else if (conv_naive(
                     sys2_n, sys2_n_len, sys1_d, sys1_d_len, p2, p2_len) !=
                   LIBSIG_EOK) {
            result = LIBSIG_ERR;
        } else {
            for (size_t i = 0; i < out_num_len; ++i) {
                // We can use modulo operations to allign the smaller array to
                // start with the zeros and then wrap around to the actual
                // values.
                sys_out_n[i] = p1[(p1_len + i) % out_num_len] +
                               p2[(p2_len + i) % out_num_len];
            }
        }

        free(p1);
        free(p2);
    }

    return result;
}

libsig_error_t
parallel_bounded(const double* sys1_n,
                 size_t sys1_n_len,
                 const double* sys1_d,
                 size_t sys1_d_len,
                 const double* sys2_n,
                 size_t sys2_n_len,
                 const double* sys2_d,
                 size_t sys2_d_len,
                 double* sys_out_n,
                 size_t sys_out_n_len,
                 double* sys_out_d,
                 size_t sys_out_d_len)
{
    libsig_error_t result = LIBSIG_EOK;
    double *p1, *p2;
    size_t p1_len = CONV_FULL_LEN(sys1_n_len, sys2_d_len);
    size_t p2_len = CONV_FULL_LEN(sys2_n_len, sys1_d_len);
    size_t out_num_len = MAX(p1_len, p2_len);

    if (sys_out_n_len != out_num_len ||
        sys_out_d_len != CONV_FULL_LEN(sys1_d_len, sys2_d_len)) {
        result = LIBSIG_EINVALID_INPUT;
    } else if ((p1 = calloc(out_num_len, sizeof(double))) == NULL) {
        result = LIBSIG_ERR;
    } else if ((p2 = calloc(out_num_len, sizeof(double))) == NULL) {
        free(p1);
        result = LIBSIG_ERR;
    } else {
        if (conv_bounded(sys1_d,
                         sys1_d_len,
                         sys2_d,
                         sys2_d_len,
                         sys_out_d,
                         sys_out_d_len) != LIBSIG_EOK) {
            result = LIBSIG_ERR;
        } else if (conv_bounded(
                     sys1_n, sys1_n_len, sys2_d, sys2_d_len, p1, p1_len) !=
                   LIBSIG_EOK) {
            result = LIBSIG_ERR;
        } else if (conv_bounded(
                     sys2_n, sys2_n_len, sys1_d, sys1_d_len, p2, p2_len) !=
                   LIBSIG_EOK) {
            result = LIBSIG_ERR;
        } else {
            for (size_t i = 0; i < out_num_len; ++i) {
                // We can use modulo operations to allign the smaller array to
                // start with the zeros and then wrap around to the actual
                // values.
                sys_out_n[i] = p1[(p1_len + i) % out_num_len] +
                               p2[(p2_len + i) % out_num_len];
            }
        }

        free(p1);
        free(p2);
    }

    return result;
}

libsig_error_t
feedback_naive(const double* sys1_n,
               size_t sys1_n_len,
               const double* sys1_d,
               size_t sys1_d_len,
               const double* sys2_n,
               size_t sys2_n_len,
               const double* sys2_d,
               size_t sys2_d_len,
               double* sys_out_n,
               size_t sys_out_n_len,
               double* sys_out_d,
               size_t sys_out_d_len)
{
    libsig_error_t result = LIBSIG_EOK;
    double *p1, *p2;
    size_t p1_len = CONV_FULL_LEN(sys1_n_len, sys2_n_len);
    size_t p2_len = CONV_FULL_LEN(sys2_d_len, sys1_d_len);
    size_t out_num_len = MAX(p1_len, p2_len);

    if (sys_out_n_len != CONV_FULL_LEN(sys1_n_len, sys2_n_len) ||
        sys_out_d_len != out_num_len) {
        result = LIBSIG_EINVALID_INPUT;
    } else if ((p1 = calloc(out_num_len, sizeof(double))) == NULL) {
        result = LIBSIG_ERR;
    } else if ((p2 = calloc(out_num_len, sizeof(double))) == NULL) {
        free(p1);
        result = LIBSIG_ERR;
    } else {
        if (conv_naive(sys1_n,
                       sys1_n_len,
                       sys2_d,
                       sys2_d_len,
                       sys_out_n,
                       sys_out_n_len) != LIBSIG_EOK) {
            result = LIBSIG_ERR;
        } else if (conv_naive(
                     sys1_n, sys1_n_len, sys2_n, sys2_n_len, p1, p1_len) !=
                   LIBSIG_EOK) {
            result = LIBSIG_ERR;
        } else if (conv_naive(
                     sys1_d, sys1_d_len, sys2_d, sys2_d_len, p2, p2_len) !=
                   LIBSIG_EOK) {
            result = LIBSIG_ERR;
        } else {
            for (size_t i = 0; i < out_num_len; ++i) {
                sys_out_d[i] = p1[(p1_len + i) % out_num_len] +
                               p2[(p2_len + i) % out_num_len];
            }
        }

        free(p1);
        free(p2);
    }

    return result;
}

libsig_error_t
feedback_bounded(const double* sys1_n,
                 size_t sys1_n_len,
                 const double* sys1_d,
                 size_t sys1_d_len,
                 const double* sys2_n,
                 size_t sys2_n_len,
                 const double* sys2_d,
                 size_t sys2_d_len,
                 double* sys_out_n,
                 size_t sys_out_n_len,
                 double* sys_out_d,
                 size_t sys_out_d_len)
{
    libsig_error_t result = LIBSIG_EOK;
    double *p1, *p2;
    size_t p1_len = CONV_FULL_LEN(sys1_n_len, sys2_n_len);
    size_t p2_len = CONV_FULL_LEN(sys2_d_len, sys1_d_len);
    size_t out_num_len = MAX(p1_len, p2_len);

    if (sys_out_n_len != CONV_FULL_LEN(sys1_n_len, sys2_n_len) ||
        sys_out_d_len != out_num_len) {
        result = LIBSIG_EINVALID_INPUT;
    } else if ((p1 = calloc(out_num_len, sizeof(double))) == NULL) {
        result = LIBSIG_ERR;
    } else if ((p2 = calloc(out_num_len, sizeof(double))) == NULL) {
        free(p1);
        result = LIBSIG_ERR;
    } else {
        if (conv_naive(sys1_n,
                       sys1_n_len,
                       sys2_d,
                       sys2_d_len,
                       sys_out_n,
                       sys_out_n_len) != LIBSIG_EOK) {
            result = LIBSIG_ERR;
        } else if (conv_naive(
                     sys1_n, sys1_n_len, sys2_n, sys2_n_len, p1, p1_len) !=
                   LIBSIG_EOK) {
            result = LIBSIG_ERR;
        } else if (conv_naive(
                     sys1_d, sys1_d_len, sys2_d, sys2_d_len, p2, p2_len) !=
                   LIBSIG_EOK) {
            result = LIBSIG_ERR;
        } else {
            for (size_t i = 0; i < out_num_len; ++i) {
                sys_out_d[i] = p1[(p1_len + i) % out_num_len] +
                               p2[(p2_len + i) % out_num_len];
            }
        }

        free(p1);
        free(p2);
    }

    return result;
}

libsig_error_t
freqz_naive(const double* b,
            size_t b_len,
            const double* a,
            size_t a_len,
            const double* w,
            size_t w_len,
            double complex* h)
{
    libsig_error_t result = LIBSIG_EOK;
    double complex num_sum;
    double complex den_sum;

    for (size_t i = 0; i < w_len; ++i) {
        num_sum = 0.0 + 0.0 * I;
        den_sum = 0.0 + 0.0 * I;

        for (size_t k = 0; k < b_len; ++k) {
            num_sum += b[k] * cexp(-I * w[i] * k);
        }

        for (size_t k = 0; k < a_len; ++k) {
            den_sum += a[k] * cexp(-I * w[i] * k);
        }

        h[i] = num_sum / den_sum;
    }

    return result;
}

// Calculate the number of bits to represent a number, ex. 8, log2(8) = 3 bits.
static size_t
_numof_bits(size_t to_represent)
{
    size_t bits_num = 0;
    size_t temp = to_represent;
    while (temp > 1) {
        ++bits_num;
        temp >>= 1;
    }

    return bits_num;
}

// Reverse the bits of 'to_reverse'
static size_t
_reverse_bit_order(size_t to_reverse, size_t bits_num)
{
    size_t reversed = 0;
    for (size_t b = 0; b < bits_num; ++b) {
        // If the b-th bit of 'to_reverse' is 1, set the corresponding mirrored
        // bit in reversed
        if ((to_reverse >> b) & 1) {
            reversed |= (1 << (bits_num - 1 - b));
        }
    }

    return reversed;
}

static double complex*
_calculate_twiddle_factors(size_t fft_len, double complex exponent)
{
    double complex* twiddles = malloc(fft_len / 2 * sizeof(double complex));
    for (size_t i = 0; i < fft_len / 2; ++i) {
        twiddles[i] = cexp(exponent * i);
    }
    return twiddles;
}

libsig_error_t
fft(const double* restrict x,
    size_t x_len,
    double complex* restrict y,
    size_t fft_len)
{
    libsig_error_t result = LIBSIG_EOK;

    // Check if the output fft_len is larger than zero and a power of two
    // (required for Radix-2 algorithm)
    if (fft_len == 0 || (fft_len & (fft_len - 1)) != 0) {
        result = LIBSIG_EINVALID_INPUT;
    } else {
        size_t bits_num = _numof_bits(fft_len);

        for (size_t i = 0; i < fft_len; ++i) {
            size_t reversed_i = _reverse_bit_order(i, bits_num);

            // Copy or Pad into the reversed index
            if (i < x_len) {
                y[reversed_i] = x[i] + I * 0.0;
            } else {
                y[reversed_i] = 0.0 + I * 0.0;
            }
        }

        double complex* twiddles = _calculate_twiddle_factors(fft_len, (-I * 2.0 * M_PI) / fft_len);
        size_t step = 1;
        size_t half_step;
        // We start at the "bottom" of recursion and move our way "back up".
        // The number of "steps" up the recursion is defined by the bits_num
        for (size_t i = 0; i < bits_num; ++i) {
            // For each recursion step we have practically divided the signals
            // into bins. The number of bins is defined by the "level" of the
            // recursion where each "step" up reduces the number of bins by a
            // power of 2.
            half_step = step;
            step <<= 1;
            size_t twiddle_step = fft_len / step;
            for (size_t offset = 0; offset < fft_len; offset += step) {
                for (size_t k = 0; k < half_step; ++k) {
                    double complex twiddle = twiddles[k * twiddle_step];
                    size_t top_idx = offset + k;
                    size_t bot_idx = offset + k + half_step;
                    double complex p = y[top_idx];
                    double complex q = twiddle * y[bot_idx];

                    y[top_idx] = p + q;
                    y[bot_idx] = p - q;
                }
            }
        }
    }

    return result;
}

libsig_error_t
ifft(const double complex* restrict fft,
     size_t fft_len,
     double complex* restrict y)
{
    libsig_error_t result = LIBSIG_EOK;
    if (fft_len == 0 || (fft_len & (fft_len - 1)) != 0) {
        result = LIBSIG_EINVALID_INPUT;
    } else {
        size_t bits_num = _numof_bits(fft_len);

        for (size_t i = 0; i < fft_len; ++i) {
            size_t reversed_i = _reverse_bit_order(i, bits_num);

            if (i < fft_len) {
                y[reversed_i] = fft[i];
            } else {
                y[reversed_i] = 0.0 + I * 0.0;
            }
        }

        double complex* twiddles = _calculate_twiddle_factors(fft_len,  (I * 2.0 * M_PI) / fft_len);
        size_t step = 1;
        size_t half_step;
        for (size_t i = 0; i < bits_num; ++i) {
            half_step = step;
            step <<= 1;
            size_t twiddle_step = fft_len / step;
            for (size_t offset = 0; offset < fft_len; offset += step) {
                for (size_t k = 0; k < half_step; ++k) {
                    double complex twiddle = twiddles[k * twiddle_step];
                    size_t top_idx = offset + k;
                    size_t bot_idx = offset + k + half_step;
                    double complex p = y[top_idx];
                    double complex q = twiddle * y[bot_idx];

                    y[top_idx] = p + q;
                    y[bot_idx] = p - q;
                }
            }
        }

        for (size_t i = 0; i < fft_len; ++i) {
            y[i] = y[i] / fft_len;
        }
    }

    return result;
}
