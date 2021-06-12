#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <math.h>
#include <sys/sysinfo.h>
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
    int **mat;
    RowCol rc;
} Mat;

typedef RowCol RowColMulData;

typedef struct rowColMulInfo
{
    RowColMulData *rcmd;
    int n;
} RowColMulInfo;

typedef struct rowColMulArg
{
    RowColMulInfo *rcmi;
    Mat *mat1;
    Mat *mat2;
    Mat *res_mat;
} RowColMulArg;

Status fileToMat(char *filename, Mat *mat)
{
    FILE *fp = fopen(filename, "r");

    if (!fp)
    {
        printf("File Validation Error\n");
        return ERROR;
    }

    fscanf(fp, "%d %d", &mat->rc.row, &mat->rc.col);
    mat->mat = (Matrix)malloc(mat->rc.row * sizeof(MatrixCol));

    for (int i = 0; i < mat->rc.row; i++)
    {
        mat->mat[i] = (MatrixCol)malloc(mat->rc.col * sizeof(Entry));
        for (int j = 0; j < mat->rc.col; j++)
            fscanf(fp, "%d", &mat->mat[i][j]);
    }

    if (fclose(fp) == EOF)
    {
        printf("File Validation Error\n");
        return ERROR;
    }

    return SUCCESS;
}

pthread_mutex_t lock;

void *rowColMul(void *arg)
{
    RowColMulArg *rcmArg = (RowColMulArg *)arg;

    for (int i = 0; i < rcmArg->rcmi->n; i++)
    {
        int totalValue = 0;
        for (int z = 0; z < rcmArg->mat1->rc.col; z++)
        {
            totalValue += rcmArg->mat1->mat[rcmArg->rcmi->rcmd[i].row][z] * rcmArg->mat2->mat[z][rcmArg->rcmi->rcmd[i].col];
        }
        rcmArg->res_mat->mat[rcmArg->rcmi->rcmd[i].row][rcmArg->rcmi->rcmd[i].col] = totalValue;
    }

    free(rcmArg->rcmi->rcmd), rcmArg->rcmi->rcmd = NULL;
    free(rcmArg->rcmi), rcmArg->rcmi = NULL;
    free(rcmArg), rcmArg = NULL;
}

int main(int argc, char *argv[])
{
    if (argc - 1 == 2)
    {
        Mat *mat1 = (Mat *)malloc(sizeof(Mat));
        Mat *mat2 = (Mat *)malloc(sizeof(Mat));

        if (fileToMat(argv[1], mat1) || fileToMat(argv[2], mat2))
            return ERROR;

        if (mat1->rc.col == mat2->rc.row)
        {
            Mat *res_mat = (Mat *)malloc(sizeof(Mat));
            res_mat->rc.row = mat1->rc.row;
            res_mat->rc.col = mat2->rc.col;

            res_mat->mat = (Matrix)malloc(res_mat->rc.row * sizeof(Matrix));
            res_mat->mat[0] = (MatrixCol)malloc(res_mat->rc.col * sizeof(Entry));

            //total number of resultant matrix entries
            int resMatEntriesCount = res_mat->rc.row * res_mat->rc.col;

            //geting number of processors in current machine
            int processorCount = get_nprocs();

            //thread identifiers for n processors
            // pthread_t threadID[processorCount];

            struct timeval start, end;
            gettimeofday(&start, NULL);

            for (int i = 0, x = 0, y = 0, remainResMatEntriesCount = resMatEntriesCount; i < processorCount; i++)
            {
                double distribution = ceil((((float)remainResMatEntriesCount) / ((float)processorCount - i)));

                remainResMatEntriesCount -= distribution;

                RowColMulData *rcmd = (RowColMulData *)malloc(distribution * sizeof(RowColMulData));

                for (int z = 0, distribCount = distribution; distribCount;)
                {
                    for (; distribCount && y < res_mat->rc.col; distribCount--, y++)
                    {
                        rcmd[z].row = x;
                        rcmd[z].col = y;
                        z++;
                    }
                    if (y >= res_mat->rc.col)
                    {
                        res_mat->mat[++x] = (int *)malloc(res_mat->rc.col * sizeof(int));
                        y = 0;
                    }
                }

                RowColMulArg *rcmArg = (RowColMulArg *)malloc(sizeof(RowColMulArg));
                rcmArg->rcmi = (RowColMulData *)malloc(sizeof(RowColMulData));
                rcmArg->rcmi->n = distribution;
                rcmArg->rcmi->rcmd = rcmd;
                rcmArg->mat1 = mat1;
                rcmArg->mat2 = mat2;
                rcmArg->res_mat = res_mat;

                rowColMul(rcmArg);

                // if (pthread_create(threadID + i, NULL, &rowColMul, rcmArg))
                //     return ERROR; //if error
            }

            //joining all threads
            // for (int i = 0; i < processorCount; i++)
            // {
            //     if (pthread_join(threadID[i], NULL))
            //         return -1; //if error
            // }

            for (int i = 0; i < res_mat->rc.row; i++)
            {
                for (int j = 0; j < res_mat->rc.col; j++)
                    printf("%d\t", res_mat->mat[i][j]);
                printf("\n");
            }

            gettimeofday(&end, NULL);
            long seconds = (end.tv_sec - start.tv_sec);
            long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
            printf("The elapsed time is %d seconds and %d micros\n", seconds, micros);

            return 0;
        }
        else
        {
            printf("Matrices not Mutliplicable");
            return ERROR;
        }
    }
    else
    {
        printf("Invalid Number of Arguments\n");
        return ERROR;
    }
}
