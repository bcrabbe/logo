//
//  interpreter.c
//  logo
//
//  Created by ben on 18/01/2015.
//  Copyright (c) 2015 ben. All rights reserved.
//

#include "main.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define ANIMATION_SPEED 10
#define SCALE 10


//pathBuilder Functions:
typedef struct turtle {
    float direction;//angle with +ve x axis
    point position;
} turtle;

pointArray * buildPath( symbolList * symList);
turtle * startingPoint();
void sampleTurtle(pointArray * path, turtle * t);
void moveTurtleFD(turtle * t, float ammount);
void rotateTurtle(turtle * t, symbol leftOrRight, float ammount);
float convertDegreesToRadians(float degrees);




pointArray * buildPath( symbolList * symList)
{
    pointArray * path = malloc(sizeof(pointArray));
    path->numberOfPoints=0;
    path->array=NULL;
    turtle * t = startingPoint();
    sampleTurtle(path, t);
    symbolNode * currentInstruction = symList->start;
    while(currentInstruction!=NULL)
    {
        if(currentInstruction->sym==symFD)
        {
            moveTurtleFD( t, currentInstruction->value);
            sampleTurtle(path, t);
        }
        else//RT or LT
        {
            rotateTurtle(t, currentInstruction->sym, currentInstruction->value);
        }
        currentInstruction=currentInstruction->next;
    }
    free(t);
    return path;
}

void moveTurtleFD(turtle * t, float ammount)
{
    t->position.r[X] += ammount*cos(t->direction);
    t->position.r[Y] += ammount*sin(t->direction);
}

void rotateTurtle(turtle * t, symbol leftOrRight, float ammount)
{
    float ammountInRadians = convertDegreesToRadians(ammount);
    if(leftOrRight==symLT) ammountInRadians = -ammountInRadians;
    
    t->direction += ammountInRadians;
    while (t->direction<0)
    {
        t->direction += 2*M_PI;
    }
    while (t->direction>2*M_PI)
    {
        t->direction -= 2*M_PI;
    }
}

float convertDegreesToRadians(float degrees)
{
    return (degrees * M_PI)/180;
}

turtle * startingPoint()
{
    turtle * t = malloc(sizeof(turtle));
    if(t==NULL)
    {
        printError("malloc failed exiting.",__FILE__,__FUNCTION__,__LINE__);
        exit(1);
    }
    t->direction = 0;
    for(dimension dim = X; dim<=DIM_MAX; ++dim)
    {
        t->position.r[dim]=0;
    }
    return t;
}

void sampleTurtle (pointArray * path, turtle * t)
{
    ++path->numberOfPoints;
    point * tmp = realloc(path->array, path->numberOfPoints*sizeof(point));
    if(tmp==NULL)
    {
        printError("realloc failed exiting.",__FILE__,__FUNCTION__,__LINE__);
        exit(1);
    }
    path->array = tmp;
    for(dimension dim = X; dim<=DIM_MAX; ++dim)
    {
        path->array[path->numberOfPoints-1].r[dim] = t->position.r[dim];
    }
}

