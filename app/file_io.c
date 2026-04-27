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

    double* tmp_buf = NULL;
    char* line_buf = NULL;

    size_t capacity = FILE_IO_INIT_BUF_CAPACITY;
    size_t line_capacity = 4096; // Initial line buffer size

    if ((file = fopen(filename, "r")) == NULL) {
        result = FILE_IO_EALLOC;
    } else {
        if ((tmp_buf = malloc(sizeof(double) * capacity)) == NULL) {
            result = FILE_IO_EALLOC;
        } else if ((line_buf = malloc(line_capacity)) == NULL) {
            result = FILE_IO_EALLOC;
            free(tmp_buf);
        } else {
            size_t row_count = 0;
            size_t col_count = 0;
            size_t total_elements = 0;

            // Loop until end of file or error
            while (result == FILE_IO_EOK) {
                size_t line_len = 0;
                line_buf[0] = '\0';

                // --- Dynamic Line Reading Loop ---
                while (fgets(line_buf + line_len,
                             line_capacity - line_len,
                             file) != NULL) {
                    line_len += strlen(line_buf + line_len);

                    // Check if we found the newline character (end of row)
                    if (line_len > 0 && line_buf[line_len - 1] == '\n') {
                        break;
                    }

                    // If EOF is reached without a newline, we are done reading
                    // this final row
                    if (feof(file)) {
                        break;
                    }

                    // We ran out of space in line_buf, double its capacity
                    line_capacity *= 2;
                    char* new_line_buf = realloc(line_buf, line_capacity);

                    if (new_line_buf == NULL) {
                        result = FILE_IO_EREALLOC;
                        break;
                    }
                    line_buf = new_line_buf;
                }

                // If nothing was read (EOF reached at the start of the loop),
                // break out
                if (line_len == 0) {
                    break;
                }

                // If a reallocation error occurred during line reading, exit
                // the outer loop
                if (result != FILE_IO_EOK) {
                    break;
                }

                // --- Parsing the Line ---
                size_t curr_col = 0;
                // Include \r and \n in delimiters to safely strip trailing
                // linebreaks
                char* token = strtok(line_buf, ",\r\n");

                while (token != NULL) {
                    if (total_elements >= capacity) {
                        capacity *= 2;
                        double* tmp =
                          realloc(tmp_buf, capacity * sizeof(double));
                        if (tmp == NULL) {
                            result = FILE_IO_EREALLOC;
                            break;
                        }
                        tmp_buf = tmp;
                    }

                    tmp_buf[total_elements++] = atof(token);
                    token = strtok(NULL, ",\r\n");
                    ++curr_col;
                }

                if (result == FILE_IO_EOK) {
                    // Ignore completely empty lines
                    if (curr_col > 0) {
                        if (row_count == 0) {
                            // The number of columns on the first row defines
                            // the matrix's number of columns
                            col_count = curr_col;
                        } else if (curr_col != col_count) {
                            result = FILE_IO_EMALFORMED_CSV;
                        }
                        ++row_count;
                    }
                }
            }

            // --- Cleanup and Finalizing Outputs ---
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

            free(line_buf);
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

    double complex* tmp_buf = NULL;
    char* line_buf = NULL;

    size_t capacity = FILE_IO_INIT_BUF_CAPACITY;
    size_t line_capacity = 4096; // Initial line buffer size

    if ((file = fopen(filename, "r")) == NULL) {
        result = FILE_IO_EALLOC;
    } else {
        if ((tmp_buf = malloc(sizeof(double complex) * capacity)) == NULL) {
            result = FILE_IO_EALLOC;
        } else if ((line_buf = malloc(line_capacity)) == NULL) {
            result = FILE_IO_EALLOC;
            free(tmp_buf);
        } else {
            size_t row_count = 0;
            size_t col_count = 0;
            size_t total_elements = 0;

            // Loop until end of file or error
            while (result == FILE_IO_EOK) {
                size_t line_len = 0;
                line_buf[0] = '\0';

                // --- Dynamic Line Reading Loop ---
                while (fgets(line_buf + line_len,
                             line_capacity - line_len,
                             file) != NULL) {
                    line_len += strlen(line_buf + line_len);

                    // Check if we found the newline character (end of row)
                    if (line_len > 0 && line_buf[line_len - 1] == '\n') {
                        break;
                    }

                    // If EOF is reached without a newline, we are done reading
                    // this final row
                    if (feof(file)) {
                        break;
                    }

                    // We ran out of space in line_buf, double its capacity
                    line_capacity *= 2;
                    char* new_line_buf = realloc(line_buf, line_capacity);

                    if (new_line_buf == NULL) {
                        result = FILE_IO_EREALLOC;
                        break;
                    }
                    line_buf = new_line_buf;
                }

                // If nothing was read (EOF reached at the start of the loop),
                // break out
                if (line_len == 0) {
                    break;
                }

                // If a reallocation error occurred during line reading, exit
                // the outer loop
                if (result != FILE_IO_EOK) {
                    break;
                }

                // --- Parsing the Line ---
                size_t curr_col = 0;
                // Include \r and \n in delimiters to safely strip trailing
                // linebreaks
                char* token = strtok(line_buf, ",\r\n");

                while (token != NULL) {
                    if (total_elements >= capacity) {
                        capacity *= 2;
                        double complex* tmp =
                          realloc(tmp_buf, capacity * sizeof(double complex));
                        if (tmp == NULL) {
                            result = FILE_IO_EREALLOC;
                            break;
                        }
                        tmp_buf = tmp;
                    }

                    double real = 0.0, imag = 0.0;
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
                    // Ignore completely empty lines
                    if (curr_col > 0) {
                        if (row_count == 0) {
                            // The number of columns on the first row
                            // defines the matrix's number of columns
                            col_count = curr_col;
                        } else if (curr_col != col_count) {
                            result = FILE_IO_EMALFORMED_CSV;
                        }
                        ++row_count;
                    }
                }
            }

            // --- Cleanup and Finalizing Outputs ---
            if (result == FILE_IO_EOK && total_elements > 0) {
                double complex* final_buf =
                  realloc(tmp_buf, total_elements * sizeof(double complex));
                if (final_buf == NULL) {
                    // If the reallocate fails we simply return the
                    // previously allocated buffer.
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

            free(line_buf);
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
