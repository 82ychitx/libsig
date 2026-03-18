#ifndef LIBSIG_H_
#define LIBSIG_H_

#include <complex.h>
#include <stddef.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define CONV_FULL_LEN(a, b) ((a) + (b) - 1)

typedef enum
{
    LIBSIG_EOK = 0,
    LIBSIG_ERR,
    LIBSIG_EINVALID_INPUT
} libsig_error_t;

// ========================== FILTER ==========================
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
filter_split_loops(const double* b,
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

// ========================== IMPZ ==========================
typedef libsig_error_t (*impz_fn_t)(const double* b,
                                    size_t b_len,
                                    const double* a,
                                    size_t a_len,
                                    double* y,
                                    size_t y_len);

libsig_error_t
impz(const double* b,
     size_t b_len,
     const double* a,
     size_t a_len,
     double* y,
     size_t y_len);

// ========================== CONV ==========================
typedef libsig_error_t (*conv_fn_t)(const double* u,
                                    size_t u_len,
                                    const double* v,
                                    size_t v_len,
                                    double* y,
                                    size_t y_len);
libsig_error_t
conv_naive(const double* u,
           size_t u_len,
           const double* v,
           size_t v_len,
           double* y,
           size_t y_len);

libsig_error_t
conv_bounded(const double* u,
             size_t u_len,
             const double* v,
             size_t v_len,
             double* y,
             size_t y_len);

#endif // LIBSIG_H_
