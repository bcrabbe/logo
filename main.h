//
//  main.h
//  logo
//
//  Created by ben on 18/01/2015.
//  Copyright (c) 2015 ben. All rights reserved.
//

#ifndef logo_main_h
#define logo_main_h
#include "sput.h"
#include "debug.h"

#define TESTING 1
#define VERBOSE 1
#define PRINT_ERRORS 1 //turn on/off stderr error messages.
#define MAX_ERROR_STRING_SIZE 600
#define NUMBER_OF_DIMENSIONS 2
#define DIM_MAX 1

/******************************************************************************/
//Parser Functions
typedef enum symbol {
    symMAIN, symINSTRCTLST, symINSTRUCTION, symFD, symLT, symRT, symDO, symVAR,
    symVARNUM, symSET, symPOLISH, symOP
} symbol;

typedef struct symNode {
    symbol sym;
    float value;
    struct symNode * next;
} symbolNode;

typedef struct symbolList {
    symbolNode * start;
    symbolNode * end;
    unsigned long length;
} symbolList;

symbolList * parse(char * inputString);



/******************************************************************************/
//Path Making Functions
typedef enum dimension {
    X = 0,
    Y = 1
} dimension;

typedef struct point {
    float r[DIM_MAX+1];//r is postion vector
} point;

typedef struct pointArray {
    point * array;
    int numberOfPoints;
} pointArray;

pointArray * buildPath( symbolList * symList);



/******************************************************************************/
//Drawing Functions
void draw(pointArray * path);



/******************************************************************************/
//Utility Functions
int printError(const char * errorString, const char file[], const char function[], const int line);
char *strdup(const char * s);
int stringsMatch(const char * string1, const char * string2);



/******************************************************************************/
//Module Unit Tests
void unitTests_main();
void unitTests_parser();


#endif
