//
//  main.c
//  logo
//
//  Created by ben on 18/01/2015.
//  Copyright (c) 2015 ben. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

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
    
//    parse(inputString);
    return parse(inputString);
}

/*
 *  Takes argv[1] which should contain a .txt file path
    reads into a malloc'd null-terminated string which is returned.
 */
char * readFile(const char * argv1)
{
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
    return inputString;
}



