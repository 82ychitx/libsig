#ifndef LIBSIG_H_
#define LIBSIG_H_


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

// ========================== SERIES ==========================
typedef libsig_error_t (*series_fn_t)(const double* sys1_n,
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
                                      size_t sys_out_d_len);
                                      
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
             size_t sys_out_d_len);

// ========================== PARALLEL ==========================
typedef libsig_error_t (*parallel_fn_t)(const double* sys1_n,
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
                                        size_t sys_out_d_len);

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
               size_t sys_out_d_len);

#endif // LIBSIG_H_
