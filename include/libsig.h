#ifndef LIBSIG_H_
#define LIBSIG_H_

#include <complex.h>
#include <stddef.h>

typedef enum
{
    LIBSIG_EOK = 0,
    LIBSIG_ERR,
} libsig_error_t;

typedef libsig_error_t (*filter_fn_t)(const double* b,
                                      size_t b_len,
                                      const double* a,
                                      size_t a_len,
                                      const double* x,
                                      size_t x_len,
                                      double* y);

libsig_error_t
filter_naive_ternary(const double* b,
                     size_t b_len,
                     const double* a,
                     size_t a_len,
                     const double* x,
                     size_t x_len,
                     double* y);

libsig_error_t
filter_history_buffer(const double* b,
                      size_t b_len,
                      const double* a,
                      size_t a_len,
                      const double* x,
                      size_t x_len,
                      double* y);

libsig_error_t
conv_naive(const double* u,
 size_t u_len,
 const double* v,
 size_t v_len);

#endif // LIBSIG_H_
