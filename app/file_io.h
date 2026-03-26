#ifndef FILE_IO_H_
#define FILE_IO_H_

#include <complex.h>
#include <stddef.h>

#define FILE_IO_INIT_BUF_CAPACITY 1024

typedef enum
{
    FILE_IO_EOK = 0,
    FILE_IO_EINVALID,
    FILE_IO_EALLOC,
    FILE_IO_EREALLOC,
    FILE_IO_EMALFORMED_CSV,
    FILE_IO_ERR,
} file_io_err_t;

file_io_err_t
file_io_read_double_matrix(const char* filename,
                           double** out_buf,
                           size_t* rows,
                           size_t* cols);

file_io_err_t
file_io_write_double_matrix(const char* filename,
                            const double* buf,
                            size_t rows,
                            size_t cols);

file_io_err_t
file_io_read_complex_matrix(const char* filename,
                            double complex** out_buf,
                            size_t* rows,
                            size_t* cols);

file_io_err_t
file_io_write_complex_matrix(const char* filename,
                             const double complex* buf,
                             size_t rows,
                             size_t cols);

#endif // FILE_IO_H_
