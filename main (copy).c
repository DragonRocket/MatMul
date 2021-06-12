#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <math.h>
#include <sys/sysinfo.h>
#include <sys/time.h>

typedef struct rowCol
{
    int row;
    int col;
} RowCol;

typedef int **Mat;

typedef RowCol RowColMulDataType;

typedef struct rowColMulArg
{
    int n;
    RowColMulDataType *rc;
    Mat mat1;
    Mat mat2;
    int mat1Cols;
    int mat2Rows;
    Mat res_mat;
} RowColMulArg;

int **fileToMat(char *filename, int *row, int *col)
{
    FILE *fp = fopen(filename, "r");

    if (!fp)
    {
        printf("File Validation Error\n");
        return NULL;
    }

    fscanf(fp, "%d %d", row, col);

    Mat mat = (Mat)malloc(*row * sizeof(int *));
    for (int i = 0; i < *row; i++)
    {
        mat[i] = (int *)malloc(*col * sizeof(int));
        for (int j = 0; j < *col; j++)
            fscanf(fp, "%d", &mat[i][j]);
    }

    if (fclose(fp) == EOF)
    {
        printf("File Validation Error\n");
        return NULL;
    }

    return mat;
}

void *rowColMul(void *arg)
{
    RowColMulArg *rcmArg = (RowColMulArg *)arg;

    for (int i = 0; i < rcmArg->n; i++)
    {
        int totalValue = 0;
        for (int z = 0; z < rcmArg->mat1Cols; z++)
        {
            totalValue += rcmArg->mat1[rcmArg->rc[i].row][z] * rcmArg->mat2[z][rcmArg->rc[i].col];
        }
        rcmArg->res_mat[rcmArg->rc[i].row][rcmArg->rc[i].col] = totalValue;
        printf("%d\t", totalValue);
    }

    //free(rcmArg->rc), rcmArg->rc = NULL;
}

int main(int argc, char *argv[])
{
    if (argc - 1 == 2)
    {
        RowCol rc1;
        RowCol rc2;

        Mat mat1 = fileToMat(argv[1], &rc1.row, &rc1.col);
        Mat mat2 = fileToMat(argv[2], &rc2.row, &rc2.col);

        if (!mat1 || !mat2)
            return -1;

        if (rc1.col == rc2.row)
        {
            RowCol res_mat_rc;
            res_mat_rc.row = rc1.row;
            res_mat_rc.col = rc2.col;

            Mat res_mat = (Mat)malloc(res_mat_rc.row * sizeof(int *));
            res_mat[0] = (int *)malloc(res_mat_rc.col * sizeof(int));

            int total_res_entries = res_mat_rc.row * res_mat_rc.col;

            //geting number of processors in current machine
            int processorCount = get_nprocs();

            //thread identifiers for n processors
            pthread_t threadID[processorCount];

            struct timeval start, end;

            gettimeofday(&start, NULL);

            for (int i = 0, x = 0, y = 0, remain_total_res_entries = total_res_entries; i < processorCount; i++)
            {
                double distribution = ceil((((float)remain_total_res_entries) / ((float)processorCount - i)));

                remain_total_res_entries -= distribution;

                RowColMulDataType *rc = (RowColMulDataType *)malloc(distribution * sizeof(RowColMulDataType));
                int distribCount = distribution;
                int z = 0;
                while (distribCount)
                {
                    for (; distribCount && y < res_mat_rc.col; distribCount--, y++)
                    {
                        rc[z].row = x;
                        rc[z].col = y;
                        z++;
                    }
                    if (y >= res_mat_rc.col)
                    {
                        res_mat[++x] = (int *)malloc(res_mat_rc.col * sizeof(int));
                        y = 0;
                    }
                }

                RowColMulArg *rcmArg = (RowColMulArg *)malloc(sizeof(RowColMulArg));
                rcmArg->n=distribution;
                rcmArg->rc=rc;
                rcmArg->mat1=mat1;
                rcmArg->mat2=mat2;
                rcmArg->mat1Cols=rc1.col;
                rcmArg->mat2Rows=rc2.col;
                rcmArg->res_mat=res_mat;

                // rowColMul(rcmArg);

                if (pthread_create(threadID + i, NULL, &rowColMul, rcmArg))
                    return -1; //if error
            }

            //joining all threads
            for (int i = 0; i < processorCount; i++)
            {
                if (pthread_join(threadID[i], NULL))
                    return -1; //if error
            }

            printf("\n");
            for (int i = 0; i < res_mat_rc.row; i++)
            {
                for (int j = 0; j < res_mat_rc.col; j++)
                    printf("%d\t", res_mat[i][j]);
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
            return -1;
        }
    }
    else
    {
        printf("Invalid Number of Arguments\n");
        return -1;
    }
}