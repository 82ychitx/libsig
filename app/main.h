#ifndef MAIN_H_
#define MAIN_H_

#include "libsig.h"

typedef struct {
    char* algo_name;
    filter_fn_t func;
} filter_bench_t;

#endif // MAIN_H_
