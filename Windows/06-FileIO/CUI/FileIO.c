//headers
#include <stdio.h>

//main()
int main(void)
{
    //variable declaration
    FILE *pFile = NULL;

    //code
    if(fopen_s(&pFile, "Log.txt", "w") != 0)
    {
        printf("Cannot open desired file\n");
        exit(0);
    }

    fprintf(pFile, "India is my Country\n");
    
    fclose(pFile);
    pFile = NULL;

    return (0);
}