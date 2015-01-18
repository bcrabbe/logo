//
//  parser.c
//  logo
//
//  Created by ben on 18/01/2015.
//  Copyright (c) 2015 ben. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

#define GRAMMAR_ERROR 2
#define SYNTAX_ERROR 3
#define SYSTEM_ERROR 4

#define PRINT_ERRORS 1 //turn on/off stderr error messages.
#define MAX_ERROR_STRING_SIZE 400

inline int parserError(char * errorString, const char function[], int line);

int parse(char * inputString)
{
    unsigned long inputStringLength = strlen(inputString);
    if(!inputStringLength)
    {
        parserError("Parser recieved a empty string. Exiting.",__FUNCTION__,__LINE__);
    }
    return 1;
}


inline int parserError(char * errorString, const char function[], int line)
{
    if(PRINT_ERRORS)
    {
        char stringStart[MAX_ERROR_STRING_SIZE];
        if(sprintf(stringStart,"Parser Error: function %s line %d.\n",function,line)!=2)
        {
            fprintf(stderr, "ERROR: sprintf in parserError.\n");
            return 0;
        }
        strcat(stringStart,errorString);
        printf("%s\n",stringStart);
    }
    return 1;
}
