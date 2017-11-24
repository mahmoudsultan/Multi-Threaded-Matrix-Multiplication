#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


/**
 * Dynamically Allocates matrix with rows and cols count as in inputs
*/
double** allocate_matrix(int rows, int cols);

/**
 * Reads matrix from a file into memory
 */
void read_matrix(double**, FILE*, int, int);
void print_matrix(double** mat_ptr, FILE* f, int rows, int cols);

/** Matrix multiplications that don't use threads
 * @param double** mat1_ptr: pointer to the first matrix 2D dobule Array
 * @param double** mat2_ptr: pointer to the second matrix 2D dobule Array
 * @param double** mat2_ptr: pointer to the matrix 2D dobule Array where the result would be stored
 * @param int mat1_rows: no of rows in the first matrix
 * @param int mat1_cols: no of cols in the first matrix which should also be the no of rows
 *  in the second matrix
 * @param int mat2_cols: no of cols in the second matrix
*/
void non_threaded_mat_mult(double**, double**, double**, int, int, int);
void calc_row(double**, double**, double**, int, int, int);
void calc_element(double*, double**, double**, int, int, int);

/** 
 * Struct to pass multiple arguments to the Calculate element/row functions
*/
typedef struct CalcElementArgs {
    double* mat1_row_ptr;
    double** mat2_ptr;
    double** out_mat_ptr; 
    int current_mat1_row; 
    int mat1_cols; 
    int current_mat2_col;
} CalcElementArgs;

typedef struct CalcRowArgs {
    double** mat1_ptr;
    double** mat2_ptr;
    double** out_mat_ptr;
    int current_mat1_row;
    int mat1_cols;
    int mat2_cols;
} CalcRowArgs;

/** wrapper functions for calc_row and calc_element that takes a sturcture as arguments */
void calc_row_threaded(CalcRowArgs* args);
void calc_element_threaded(CalcElementArgs* args);

void threaded_mat_mult_per_row(double**, double**, double**, int, int, int);
void threaded_mat_mult_per_element(double**, double**, double**, int, int, int);


int main() {
    int mat1_rows, mat1_cols, mat2_cols, type;
    printf("Enter 2 Matrix Dimentions: \n");
    scanf("%d %d %d %d", &mat1_rows, &mat1_cols, &mat2_cols, &type);

    // initialize matrices including the space where the result matrix will be stored
    double** mat1 = allocate_matrix(mat1_rows, mat1_cols);
    double** mat2 = allocate_matrix(mat1_cols, mat2_cols);
    double** result_mat = allocate_matrix(mat1_rows, mat2_cols);

    // read the input matrices
    read_matrix(mat1, fopen("../tests/inputs/a.txt", "r"), mat1_rows, mat1_cols);
    read_matrix(mat2, fopen("../tests/inputs/b.txt", "r"), mat1_cols, mat2_cols);

    /** multiply **/
    // non_threaded_mat_mult(mat1, mat2, result_mat, mat1_rows, mat1_cols, mat2_cols);
    // threaded_mat_mult_per_row(mat1, mat2, result_mat, mat1_rows, mat1_cols, mat2_cols);
    // threaded_mat_mult_per_element(mat1, mat2, result_mat, mat1_rows, mat1_cols, mat2_cols);

    switch(type) {
        case 0:
            non_threaded_mat_mult(mat1, mat2, result_mat, mat1_rows, mat1_cols, mat2_cols);
            break;
        case 1:
            threaded_mat_mult_per_row(mat1, mat2, result_mat, mat1_rows, mat1_cols, mat2_cols);
            break;
        case 2:
            threaded_mat_mult_per_element(mat1, mat2, result_mat, mat1_rows, mat1_cols, mat2_cols);
            break;
        default:
            non_threaded_mat_mult(mat1, mat2, result_mat, mat1_rows, mat1_cols, mat2_cols);
    }

    print_matrix(result_mat, fopen("../tests/inputs/o.txt", "w"), mat1_rows, mat2_cols);
    return 0;
}

double** allocate_matrix(int rows, int cols) {
    double** mat_pointer = (double**) malloc(rows * sizeof(double*));
    for (int j = 0; j < rows; j++) {
        *(mat_pointer + j) = (double*) malloc(cols * sizeof(double));
    }
    return mat_pointer;
}

void read_matrix(double** mat_ptr, FILE* f, int rows, int cols) {
    if (!f) {
        perror("Error Reading The Matrix Input File");
    }

    // reading matrix row
    double* row_ptr;
    for (int i = 0; i < rows; i++) {
        row_ptr = *(mat_ptr + i);
        // reading col values
        for (int j = 0; j < cols; j++) {
            fscanf(f, "%lf ", (row_ptr + j));
        }
//        fscanf(f, "\n");
    }
}

void print_matrix(double** mat_ptr, FILE* f, int rows, int cols) {
    if (!f) {
        perror("Error Opening The Matrix Output File");
    }

    // reading matrix row
    double* row_ptr;
    for (int i = 0; i < rows; i++) {
        row_ptr = *(mat_ptr + i);
        // reading col values
        for (int j = 0; j < cols; j++) {
            fprintf(f, "%lf ", *(row_ptr + j));
        }
        fprintf(f, "\n");
    }
}

void non_threaded_mat_mult(double** mat1_ptr, double** mat2_ptr, double** out_mat_ptr, int mat1_rows, int mat1_cols, int mat2_cols) {
    double* mat1_row_ptr;
    double cell_value;

    // iterate over each row of the first matrix
    for (int i = 0; i < mat1_rows; i++) {
        // mat1_row_ptr = *(mat1_ptr + i);
        // // iterate over each col in the second matrix
        // for (int j = 0; j < mat2_cols; j++) {
        //     cell_value = 0;
        //     // iterate over each col in the first matrix and row in the second matrix
        //     for (int row_col = 0; row_col < mat1_cols; row_col++) {
        //         cell_value += (*(mat1_row_ptr + row_col) * *(*(mat2_ptr + row_col) + j));
        //     }
        //     // place the cell value in the appropriate location in out matrix
        //     *(*(out_mat_ptr + i) + j) = cell_value;
        // }
        calc_row(mat1_ptr, mat2_ptr, out_mat_ptr, i, mat1_cols, mat2_cols);
    }
}

/**
 * Calculates an row in the output matrix
 */ 
void calc_row(double** mat1_ptr, double** mat2_ptr, double** out_mat_ptr, int current_mat1_row, int mat1_cols, int mat2_cols) {
    double* mat1_row_ptr = *(mat1_ptr + current_mat1_row);
    // double cell_value;
    // iterate over each col in the second matrix
    for (int j = 0; j < mat2_cols; j++) {
        // cell_value = 0;
        // // iterate over each col in the first matrix and row in the second matrix
        // for (int row_col = 0; row_col < mat1_cols; row_col++) {
        //     cell_value += (*(mat1_row_ptr + row_col) * *(*(mat2_ptr + row_col) + j));
        // }
        // // place the cell value in the appropriate location in out matrix
        // *(*(out_mat_ptr + current_mat1_row) + j) = cell_value;
        calc_element(mat1_row_ptr, mat2_ptr, out_mat_ptr, current_mat1_row, mat1_cols, j);
    }
}

/**
 * Calculates one element in the output matrix 
 */
void calc_element(double* mat1_row_ptr, double** mat2_ptr, double** out_mat_ptr, int current_mat1_row, int mat1_cols, int current_mat2_col) {
    int cell_value = 0;
    // iterate over each col in the first matrix and row in the second matrix
    for (int row_col = 0; row_col < mat1_cols; row_col++) {
        cell_value += (*(mat1_row_ptr + row_col) * *(*(mat2_ptr + row_col) + current_mat2_col));
    }
    // place the cell value in the appropriate location in out matrix
    *(*(out_mat_ptr + current_mat1_row) + current_mat2_col) = cell_value;
}

void threaded_mat_mult_per_element(double** mat1_ptr, double** mat2_ptr, double** out_mat_ptr, int mat1_rows, int mat1_cols, int mat2_cols) {
    double* mat1_row_ptr;
    // double cell_value;

    // initialize threads pointers array
    pthread_t** col_threads = malloc(sizeof(pthread_t*) * (mat1_rows * mat2_cols + 1));
    CalcElementArgs** col_threads_args = malloc(sizeof(CalcElementArgs*) * (mat1_rows * mat2_cols + 1));
    *(col_threads + (mat1_rows * mat2_cols)) = NULL;
    *(col_threads_args + (mat1_rows * mat2_cols)) = NULL;

    int counter = 0, status;
    CalcElementArgs* current_args;

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // iterate over each row of the first matrix
    for (int i = 0; i < mat1_rows; i++) {
        mat1_row_ptr = *(mat1_ptr + i);
        // iterate over each col in the second matrix
        for (int j = 0; j < mat2_cols; j++) {
            // cell_value = 0;

            // initialize col thread arguments structure
            col_threads_args[counter] = malloc(sizeof(CalcElementArgs));
            current_args = col_threads_args[counter];
            current_args->mat1_row_ptr = mat1_row_ptr;
            current_args->mat2_ptr = mat2_ptr;
            current_args->out_mat_ptr = out_mat_ptr;
            current_args->current_mat1_row = i;
            current_args->mat1_cols = mat1_cols;
            current_args->current_mat2_col = j;

            // create a thread to calculate this element
            col_threads[counter] = malloc(sizeof(pthread_t));
            status = pthread_create(col_threads[counter], &attr, calc_element_threaded, col_threads_args[counter]);
            if (status) {
                fprintf(stderr,"Error - pthread_create() return code: %d\n", status);
                exit(EXIT_FAILURE);
            }
            counter++;
            
            // calc_element(mat1_row_ptr, mat2_ptr, out_mat_ptr, i, mat1_cols, j);
        }
    }

    for (counter = 0; counter < (mat1_rows * mat2_cols); counter++) {
        pthread_join(*col_threads[counter], NULL);
    }

    // TODO: Free the args structure
}

/** 
 * Calculates Matrix multiplications but creates a thread to calculate each row of the output matrix
*/
void threaded_mat_mult_per_row(double** mat1_ptr, double** mat2_ptr, double** out_mat_ptr, int mat1_rows, int mat1_cols, int mat2_cols) {
    double* mat1_row_ptr;
    double cell_value;

    // create an array to store pointers to threads created
    pthread_t** rows_threads = malloc(sizeof(pthread_t*) * mat1_rows);

    // create an array to store pointers to threads arguments for each thread
    // TODO: possible refactor: all threads argument are the same except the current row count
    CalcRowArgs** row_threads_params = malloc(sizeof(CalcRowArgs*) * mat1_rows);
    CalcRowArgs* current_args;

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    int status = 0;

    // iterate over each row of the first matrix and create a thread for each
    for (int i = 0; i < mat1_rows; i++) {
        // initialize the threads arguments and datastructure
        rows_threads[i] = malloc(sizeof(pthread_t));
        row_threads_params[i] = malloc(sizeof(CalcRowArgs));
        current_args = row_threads_params[i];
        current_args->mat1_ptr = mat1_ptr;
        current_args->mat2_ptr = mat2_ptr;
        current_args->out_mat_ptr = out_mat_ptr;
        current_args->current_mat1_row = i;
        current_args->mat1_cols = mat1_cols;
        current_args->mat2_cols = mat2_cols;
        
        status = pthread_create(rows_threads[i], &attr, calc_row_threaded, row_threads_params[i]);
        if (status) {
            fprintf(stderr,"Error - pthread_create() return code: %d\n", status);
            exit(EXIT_FAILURE);
        }
    }

    // wait for threads to finish
    for (int i = 0; i < mat1_rows; i++) {
        pthread_join(*rows_threads[i], NULL);
    }

    // TODO: Free the args structure
}

// wrapper function for the calc_row
void calc_row_threaded(CalcRowArgs* args) {
    printf("%d in thread %d \n", args->current_mat1_row, pthread_self());
    calc_row(args->mat1_ptr, args->mat2_ptr, args->out_mat_ptr, args->current_mat1_row, args->mat1_cols, args->mat2_cols);
}

// wrapper function for the calc_element
void calc_element_threaded(CalcElementArgs* args) {
    printf("(%d, %d) in thread %d \n", args->current_mat1_row, args->current_mat2_col, pthread_self());
    calc_element(args->mat1_row_ptr, args->mat2_ptr, args->out_mat_ptr, args->current_mat1_row, args->mat1_cols, args->current_mat2_col);
}