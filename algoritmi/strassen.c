#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utility.h"

static int cols = 10;
static int rows = 10;

void init_matrix(int matrix[rows][cols], int lower, int upper) 
{
    srand(time(NULL));

    for (int row = 0; row < rows; row++) 
    {
        for (int col = 0; col < cols; col++) 
        {
            matrix[row][col] = rnd_number(lower, upper);
        }
    }
}

void print_matrix(int matrix[rows][cols]) 
{
    for (int row = 0; row < rows; row++) 
    {
        printf("[");
        for (int col = 0; col < cols - 1; col++) 
        {
            printf("%4d,", matrix[row][col]);
        }
        printf("%4d  ]\n", matrix[row][cols - 1]);
    }
}

int main( void ) 
{
    int matrix[rows][cols];
    init_matrix(matrix, 0, 100);
    print_matrix(matrix);

    return EXIT_SUCCESS;
}
