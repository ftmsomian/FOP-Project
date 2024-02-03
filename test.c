/**
 * C program to find file permission, size, creation and last modification date of 
 * a given file.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <string.h>


int main()
{
    char filepath[100] = "git config useremail : user.";
    char* answer = strstr( filepath , "email");
    printf("%s" , answer);
    return 0;
}


