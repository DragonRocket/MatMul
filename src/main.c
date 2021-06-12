#include "matmul.h"

int main(int argc, char *argv[])
{
    if (argc - 1 >= 2) //Valid is there are atleast two arguments passed
    {
        if (argc - 1 > 2) //if addition arguments are passed
            printf("\nAddition Arguments are Ignored\n");

        //dynamically allocation matrix1 and matrix2 structure
        Mat *mat1 = (Mat *)malloc(sizeof(Mat));
        Mat *mat2 = (Mat *)malloc(sizeof(Mat));

        //populating matrix1 and matrix2
        if (fileToMat(argv[1], mat1) || fileToMat(argv[2], mat2))
            return ERROR;

        if (mat1->rc.col == mat2->rc.row)
        {
            printf("\nCalculating Resultant Matrix\n");

            //dynamically allocating resultant matrix structure
            Mat *res_mat = (Mat *)malloc(sizeof(Mat));
            res_mat->rc.row = mat1->rc.row;
            res_mat->rc.col = mat2->rc.col;

            res_mat->mat = (Matrix)malloc(res_mat->rc.row * sizeof(Matrix));
            for (int i = 0; i < res_mat->rc.row; i++)
                res_mat->mat[i] = (MatrixCol)malloc(res_mat->rc.col * sizeof(Entry));

            //total number of resultant matrix entries
            int resMatEntriesCount = res_mat->rc.row * res_mat->rc.col;

            //geting number of processors in current machine
            int processorCount = get_nprocs();
            int n;

            //optimization
            if (resMatEntriesCount < processorCount)
                n = resMatEntriesCount;
            else
                n = processorCount;

            //thread identifiers for n processors
            pthread_t threadID[n];

            //creating n threads
            for (int i = 0, prevEntCount = 0; i < n; i++)
            {
                //counting entries calculation for each thread
                double distribution = ceil((((float)(resMatEntriesCount - prevEntCount)) / ((float)processorCount - i)));

                //dynamically allocating thread argument
                RowColMulArg *rcmArg = (RowColMulArg *)malloc(sizeof(RowColMulArg));
                rcmArg->prevEntCount = prevEntCount;
                rcmArg->distribution = distribution;
                rcmArg->mat1 = mat1;
                rcmArg->mat2 = mat2;
                rcmArg->res_mat = res_mat;

                prevEntCount = prevEntCount + distribution;

                //creating thread with passed argument
                if (pthread_create(threadID + i, NULL, &rowColMul, rcmArg))
                    return ERROR; //if error
            }

            //joining all threads
            for (int i = 0; i < n; i++)
            {
                if (pthread_join(threadID[i], NULL))
                    return ERROR; //if error
            }

            printf("Resultant Matrix Calculated\n\n");

            int x = 1;
            while (x)
            {
                printf("Select Output: \n");
                printf("0. exit\n");
                printf("1. Screen\n");
                printf("2. File\n");
                printf("3. Screen + File\n");

                printf("\nEnter: ");
                scanf("%d", &x);

                if (x == 1 || x == 3)
                {
                    printf("\n");
                    for (int i = 0; i < res_mat->rc.row; i++)
                    {
                        for (int j = 0; j < res_mat->rc.col; j++)
                            printf("%d\t", res_mat->mat[i][j]);
                        printf("\n");
                    }
                    printf("\n");
                }

                if (x == 2 || x == 3)
                {
                    printf("Enter File Name: ");
                    size_t size = 0;
                    char *filename = NULL;
                    getchar();
                    size = getline(&filename, &size, stdin);
                    filename = (char *)realloc(filename, size);
                    filename[size - 1] = '\0';
                    printf("Matrix written in the file\n\n");

                    FILE *fp = fopen(filename, "w");
                    free(filename), filename = NULL;

                    if (!fp)
                    {
                        printf("File Validation Error\n");
                        return ERROR;
                    }

                    for (int i = 0; i < res_mat->rc.row; i++)
                    {
                        for (int j = 0; j < res_mat->rc.col; j++)
                        {
                            fprintf(fp, "%d", res_mat->mat[i][j]);
                            if (j < res_mat->rc.col - 1)
                                fprintf(fp, "\t");
                        }
                        fprintf(fp, "\n");
                    }

                    if (fclose(fp) == EOF)
                    {
                        printf("File Validation Error\n");
                        return ERROR;
                    }
                }

                if (x < 0 || x > 3)
                    printf("Invalid Option\n\n");
                else
                    x = 0;
            }

            //deallocating matrix1
            for (int i = 0; i < mat1->rc.row; i++)
                free(mat1->mat[i]), mat1->mat[i] = NULL;
            free(mat1->mat), mat1->mat = NULL;

            //deallocating matrix2
            for (int i = 0; i < mat2->rc.row; i++)
                free(mat2->mat[i]), mat2->mat[i] = NULL;
            free(mat2->mat), mat2->mat = NULL;

            //deallocating resultant matrix
            for (int i = 0; i < res_mat->rc.row; i++)
                free(res_mat->mat[i]), res_mat->mat[i] = NULL;
            free(res_mat->mat), res_mat->mat = NULL;

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