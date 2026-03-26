#include "file_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

file_io_err_t
file_io_read_double_matrix(const char* filename,
                           double** out_buf,
                           size_t* rows,
                           size_t* cols)
{
    file_io_err_t result = FILE_IO_EOK;
    FILE* file;
    char buf[1024];
    double* tmp_buf;
    size_t capacity = FILE_IO_INIT_BUF_CAPACITY;

    if ((file = fopen(filename, "r")) == NULL) {
        result = FILE_IO_EINVALID;
    } else {
        if ((tmp_buf = malloc(sizeof(double) * capacity)) == NULL) {
            result = FILE_IO_EALLOC;
        } else {
            size_t row_count = 0;
            size_t col_count = 0;
            size_t total_elements = 0;

            while (fgets(buf, sizeof(buf), file) != NULL &&
                   result == FILE_IO_EOK) {
                size_t curr_col = 0;
                char* token = strtok(buf, ",");

                while (token != NULL) {
                    if (total_elements >= capacity) {
                        capacity *= 2;
                        double* tmp =
                          realloc(tmp_buf, capacity * sizeof(double));
                        if (tmp == NULL) {
                            result = FILE_IO_EREALLOC;
                            break;
                        } else {
                            tmp_buf = tmp;
                        }
                    }

                    tmp_buf[total_elements++] = atof(token);
                    token = strtok(NULL, ",");
                    ++curr_col;
                }

                if (result == FILE_IO_EOK) {
                    if (row_count == 0) {
                        // The number of columns on the first row defines the
                        // matrix's number of columns
                        col_count = curr_col;
                    } else if (curr_col != col_count) {
                        result = FILE_IO_EMALFORMED_CSV;
                    }

                    ++row_count;
                }
            }

            if (result == FILE_IO_EOK && total_elements > 0) {
                double* final_buf =
                  realloc(tmp_buf, total_elements * sizeof(double));
                if (final_buf == NULL) {
                    // If the reallocate fails we simply return the previously
                    // allocated buffer.
                    *out_buf = tmp_buf;
                } else {
                    *out_buf = final_buf;
                }

                *rows = row_count;
                *cols = col_count;
            } else {
                free(tmp_buf);
                *out_buf = NULL;
                *rows = 0;
                *cols = 0;
            }
        }

        fclose(file);
    }

    return result;
}

file_io_err_t
file_io_write_double_matrix(const char* filename,
                            const double* buf,
                            size_t rows,
                            size_t cols)
{
    file_io_err_t result = FILE_IO_EOK;
    FILE* file;

    if ((file = fopen(filename, "w")) == NULL) {
        result = FILE_IO_EINVALID;
    } else {
        for (size_t r = 0; r < rows; ++r) {
            for (size_t c = 0; c < cols; ++c) {
                size_t idx = (r * cols) + c;
                fprintf(file, "%lf", buf[idx]);
                if (c < cols - 1) {
                    fprintf(file, ",");
                }
            }

            fprintf(file, "\n");
        }

        fclose(file);
    }

    return result;
}

file_io_err_t
file_io_read_complex_matrix(const char* filename,
                            double complex** out_buf,
                            size_t* rows,
                            size_t* cols)
{
    file_io_err_t result = FILE_IO_EOK;
    FILE* file;
    char buf[1024];
    double complex* tmp_buf;
    size_t capacity = FILE_IO_INIT_BUF_CAPACITY;

    if ((file = fopen(filename, "r")) == NULL) {
        result = FILE_IO_EINVALID;
    } else {
        if ((tmp_buf = malloc(sizeof(double complex) * capacity)) == NULL) {
            result = FILE_IO_EALLOC;
        } else {
            size_t row_count = 0;
            size_t col_count = 0;
            size_t total_elements = 0;

            while (fgets(buf, sizeof(buf), file) != NULL &&
                   result == FILE_IO_EOK) {
                size_t curr_col = 0;
                char* token = strtok(buf, ",");

                while (token != NULL) {
                    double real = 0.0, imag = 0.0;

                    if (total_elements >= capacity) {
                        capacity *= 2;
                        double complex* tmp =
                          realloc(tmp_buf, capacity * sizeof(double complex));
                        if (tmp == NULL) {
                            result = FILE_IO_EREALLOC;
                            break;
                        } else {
                            tmp_buf = tmp;
                        }
                    }

                    // Assuming the complex number to be in format Re+Im*i;
                    if (sscanf(token, "%lf%lfi", &real, &imag) < 1) {
                        result = FILE_IO_EMALFORMED_CSV;
                        break;
                    } else {
                        tmp_buf[total_elements++] = CMPLX(real, imag);
                        token = strtok(NULL, ",");
                        ++curr_col;
                    }
                }

                if (result == FILE_IO_EOK) {
                    if (row_count == 0) {
                        // The number of columns on the first row defines the
                        // matrix's number of columns
                        col_count = curr_col;
                    } else if (curr_col != col_count) {
                        result = FILE_IO_EMALFORMED_CSV;
                    }

                    ++row_count;
                }
            }

            if (result == FILE_IO_EOK && total_elements > 0) {
                double complex* final_buf =
                  realloc(tmp_buf, total_elements * sizeof(double complex));
                if (final_buf == NULL) {
                    // If the reallocate fails we simply return the previously
                    // allocated buffer.
                    *out_buf = tmp_buf;
                } else {
                    *out_buf = final_buf;
                }

                *rows = row_count;
                *cols = col_count;
            } else {
                free(tmp_buf);
                *out_buf = NULL;
                *rows = 0;
                *cols = 0;
            }
        }

        fclose(file);
    }

    return result;
}

file_io_err_t
file_io_write_complex_matrix(const char* filename,
                             const double complex* buf,
                             size_t rows,
                             size_t cols)
{
    file_io_err_t result = FILE_IO_EOK;
    FILE* file;

    if ((file = fopen(filename, "w")) == NULL) {
        result = FILE_IO_EINVALID;
    } else {
        for (size_t r = 0; r < rows; ++r) {
            for (size_t c = 0; c < cols; ++c) {
                size_t idx = (r * cols) + c;
                double complex val = buf[idx];

                fprintf(file, "%lf%+lfi", creal(val), cimag(val));
                if (c < cols - 1) {
                    fprintf(file, ",");
                }
            }

            fprintf(file, "\n");
        }

        fclose(file);
    }

    return result;
}
