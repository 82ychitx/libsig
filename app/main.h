#ifndef MAIN_H_
#define MAIN_H_

#include "libsig.h"

typedef struct
{
    char* algo_name;
    filter_fn_t func;
} filter_bench_t;

int
read_file_to_buffer(const char* filename, double* buffer, size_t* read_len);

bool
is_buffers_equal(const double* a, const double* b, size_t len);

#endif // MAIN_H_
