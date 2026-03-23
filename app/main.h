#ifndef MAIN_H_
#define MAIN_H_

#include <stddef.h>

#define BUF_SIZE 100'000

#define FILTER_COLS 5
#define FILTER_ROWS 4


int
read_file_to_buffer(const char* filename, double* buffer, size_t* read_len);

int
dump_buffer_to_csv(const char* filename, const double* buffer, size_t len);

int
load_filter_coeffs(const char* filepath,
                   double coeffs[FILTER_ROWS][FILTER_COLS]);

#endif // MAIN_H_
