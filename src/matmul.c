#include "matmul.h"

Status fileToMat(char *filename, Mat *mat)
{
    //calculating row and columns of matrix from file

    FILE *fp = fopen(filename, "r");

    if (!fp)
    {
        printf("File Validation Error\n");
        return ERROR;
    }

    mat->rc.row = 0;
    mat->rc.col = 0;

    Entry e;
    char ch;
    bool calculated = false;

    while (fscanf(fp, "%d%c", &e, &ch) != EOF)
    {
        if (ch == '\n')
        {
            mat->rc.row++;

            if (!calculated)
            {
                mat->rc.col++;
                calculated = true;
            }
        }

        if (!calculated && ch == '\t')
        {
            mat->rc.col++;
        }
    }

    if (fclose(fp) == EOF)
    {
        printf("File Validation Error\n");
        return ERROR;
    }


    //populating the matrix from file

    fp = fopen(filename, "r");

    if (!fp)
    {
        printf("File Validation Error\n");
        return ERROR;
    }

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

void *rowColMul(void *arg)
{
    RowColMulArg *rcmArg = (RowColMulArg *)arg;

    int prevX = (rcmArg->prevEntCount - 1) / rcmArg->res_mat->rc.col;
    int prevY = (rcmArg->prevEntCount - 1) % rcmArg->res_mat->rc.col;

    for (int i = prevX, j = prevY + 1; rcmArg->distribution;)
    {
        for (; rcmArg->distribution && j < rcmArg->res_mat->rc.col; rcmArg->distribution--, j++)
        {
            Entry totalEntryValue = 0;
            for (int k = 0; k < rcmArg->mat1->rc.col; k++) //calculating the each resultant value from matrix1 and matrix2
                totalEntryValue += rcmArg->mat1->mat[i][k] * rcmArg->mat2->mat[k][j];

            rcmArg->res_mat->mat[i][j] = totalEntryValue;
        }

        if (j >= rcmArg->res_mat->rc.col) //when next row is encountered
        {
            i++;
            j = 0;
        }
    }

    //deallocating passed argument
    free(rcmArg), rcmArg = NULL;
}