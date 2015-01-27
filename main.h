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
#include <math.h>

/******************************************************************************/
//Options:

#define TESTING 0//runs the test if set
#define VERBOSE 0//prints info to terminal disable for speed.
#define PRINT_ERRORS 1 //turn on/off stderr error messages.
#define MAX_ERROR_STRING_SIZE 600

//graphics opts:
#define FPS 50
#define SDL_WINDOW_WIDTH 900
#define SDL_WINDOW_HEIGHT 660
#define STRETCH_TO_FIT_WINDOW 0
#define VIEWING_DISTANCE 100 // as a fraction of largest image dimension
#define ROTATION_SENSITIVITY M_PI/170 //how many rads to rotate per tick
#define ADJUST_ZOOM_TO_FIT_ROTATED_OBJECT 1

#define ZOOM_SENSITIVITY 0.7 //zooming will increase scale by a factor of ZOOM_SENSITIVITY*100 %
#define SCALE_AT_START 0.7 //of screen width

/******************************************************************************/

#ifndef M_PI
#define M_PI 3.14159265359
#endif
#ifndef M_PI_2
#define M_PI_2 3.14159265359/2
#endif
#ifndef M_PI_4
#define M_PI_4 3.14159265359/4
#endif

/******************************************************************************/
//Parser Module
typedef enum symbol {
    symMAIN, symINSTRCTLST, symINSTRUCTION, symFD, symLT, symRT, symUT, symDT, symDO, symVAR,
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
//Path Making Module

#define NUMBER_OF_DIMENSIONS 3
#define DIM_MAX NUMBER_OF_DIMENSIONS - 1

typedef enum dimension {
    X = 0,
    Y = 1,
    Z = 2
} dimension;

typedef struct point {
    float r[NUMBER_OF_DIMENSIONS];//r is postion vector
} point;

typedef struct pointArray {
    point * array;
    int numberOfPoints;
} pointArray;

pointArray * buildPath( symbolList * symList);



/******************************************************************************/
//Drawing Module

void draw(pointArray * path);



/******************************************************************************/
//Utility Functions
int printError(const char * errorString, const char file[], const char function[], const int line);
char *strdup(const char * source);
int stringsMatch(const char * string1, const char * string2);
int floatCompare(float a, float b);
void freeSymList(symbolList * symList);
void freePath(pointArray * path );


/******************************************************************************/
//Module Unit Tests
void unitTests_main();
void unitTests_parser();
void unitTests_path();
void unitTests_draw();


//structure mocking functions for tests
symbolList * mockSymListForPathUnitTests();
pointArray * mockPathForDrawUnitTests();

#endif
