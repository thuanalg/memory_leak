
// C program for Pascal’s Triangle
// A O(n^2) time and O(1) extra space
// function for Pascal's Triangle
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
void printPascal(int n)
{

    int C = 1; // used to represent C(line, i)
    //printf("line: %d\n", line);
    for (int i = 1; i <= n; i++)
    {
        printf("%d ", C); // The first value in a line is always 1
        C = C * (n - i) / i;
        //1 = 1 * (9-1)/1;
        //9 = 9 * (9-2)/2
    }
    printf("\n");
    
}
// Driver code
int main(int argc, char *argv[])
{
    int n = 6;
    sscanf(argv[1], "%d", &n);
	printPascal(n);
	return 0;
}
