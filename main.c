//
//  main.c
//  logo
//
//  Created by ben on 18/01/2015.
//  Copyright (c) 2015 ben. All rights reserved.
//
#define sprint(s) printf(#s " = ""%s""\n",s);
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "main.h"
#include "parser.h"

char * readFile(const char * argv1);


int main(int argc, const char * argv[])
{
    if(argc!=2)
    {
        fprintf(stderr, "ERROR: expected a .txt file path as 1st argument.\nExiting.\n");
        exit(0);
    }
    char * inputString = readFile(argv[1]);
    
//  parse(inputString);
    return parse(inputString);
}

/*
 *  Takes argv[1] which should contain a .txt file path
    reads into a malloc'd null-terminated string which is returned.
 */
char * readFile(const char * argv1)
{
    printf("opening %s.\n",argv1);
    FILE * fp = NULL;
    fp = fopen(argv1, "r");
    if(!fp)
    {
        fprintf(stderr, "ERROR: could not open file. Exiting.\n");
        exit(0);
    }
    char * inputString = NULL;
    char fileChar = getc(fp);
    int length=0;
    while(fileChar!=EOF)
    {
        ++length;
        char * tmp = realloc( inputString, length );
        if(!tmp)
        {
            fprintf(stderr, "ERROR: realloc in readFile() main.c failed. Exiting.\n");
            exit(0);
        }
        inputString = tmp;
        inputString[length-1]=fileChar;
        fileChar = getc(fp);
    }
    char * tmp = realloc( inputString, length );
    if(!tmp)
    {
        fprintf(stderr, "ERROR: realloc in readFile() main.c failed. Exiting.\n");
        exit(0);
    }
    inputString = tmp;
    inputString[length]='\0';
    sprint(inputString);
    return inputString;
}

#pragma mark genericFunctions

int printError(const char * errorString, const char file[], const char function[], const int line)
{
    if(PRINT_ERRORS)
    {
        char * editableErrorString = strdup(errorString);
        char stringStart[MAX_ERROR_STRING_SIZE];
        sprintf(stringStart,"Error in %s module in function %s at line %d.\n\n", file, function, line);
        strcat(stringStart,editableErrorString);
        printf("%s\n\n",stringStart);
        free(editableErrorString);
    }
    return 1;
}

/*
 *  Duplicates a string, mallocs the space.
 */
char *strdup(const char * s)
{
    size_t len = 1+strlen(s);//gets the size of s
    char *p = malloc(len);//allocates a block big enough to hold s
    
    return p ? memcpy(p, s, len) : NULL;//if p is non 0 ie malloc worked, then copy everything in s into p and return p. if p is NULL malloc didnt work so return NULL
}

int stringsMatch(const char * string1, const char * string2)
{
    if(strcmp(string1,string2)==0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


