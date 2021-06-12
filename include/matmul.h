#ifndef H_MATMUL
#define H_MATMUL

#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <math.h>
#include <sys/sysinfo.h>
#include <stdbool.h>
#include <sys/time.h>

#define ERROR -1
#define SUCCESS 0

typedef int Status;

typedef int **Matrix;
typedef int *MatrixCol;
typedef int Entry;

typedef struct rowCol
{
    int row;
    int col;
} RowCol;

typedef struct mat
{
    Matrix mat;
    RowCol rc;
} Mat;

typedef struct rowColMulArg
{
    int prevEntCount;
    int distribution;
    Mat *mat1;
    Mat *mat2;
    Mat *res_mat;
} RowColMulArg;

Status fileToMat(char *, Mat *);
void *rowColMul(void *);

#endif