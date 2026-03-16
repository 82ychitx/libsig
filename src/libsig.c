#include "libsig.h"
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
filter_history_buffer(const double* b,
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

    // If we are accessing negative indeces, than a branching will occur.
    // This will slow down the performance and not make use of DSP HW
    // instructions. To do this we split the loops into two where one takes care
    // of the negative indeces and the second loops will make use of the HW
    // instructions.
    for (size_t n = 0; n < x_len; ++n) {
        ff_sum = 0, fb_sum = 0;

        for (size_t i = 0; i < b_len; ++i) {
            ff_sum += b[i] * (n - i < 0 ? 0 : x[n - i]);
        }

        for (size_t j = 1; j < a_len; ++j) {
            fb_sum += a[j] * (n - j < 0 ? 0 : y[n - j]);
        }

        y[n] = 1 / a[0] * (ff_sum - fb_sum);
    }

    return result;
}
