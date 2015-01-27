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

#pragma mark prototypes
//pathBuilder Functions:
typedef struct turtle {
    //these angles describe te direction the turtle faces:
    float theta;//angle with +ve x axis (in radians)
    float phi;//angle 0->pi with the +ve z axis (rad)
    float tilt;//angle 0->2pi with +ve z axis (rad)
    float forwardDirection[3];// this is a unit vector in the forward direction
    float UpDownRotationAxis[3];//the unit vector perp to forward direction which U/DT rotate around.
    float LeftRightRotationAxis[3];//the unit vector perp to forward direction which U/DT rotate around.
    point position;//r=(x,y,z)
} turtle;

pointArray * buildPath( symbolList * symList);
turtle * startingPoint();
void sampleTurtle(pointArray * path, turtle * t);
void moveTurtleFD(turtle * t, float ammount);
void rotateTurtle(turtle * t, symbol leftOrRight, float ammount);
float convertDegreesToRadians(float degrees);
pointArray * initPath();

#pragma mark Unit Test Prototypes
void testDegreesToRadians();
void testStartingPoint();
void testRotateTurtle();
void testMoveTurtleFD();
void testSampleTurtle();
void testBuildPath();

#pragma mark Path Builder Functions
/**
 Module interface,
 Call with a populated symList, turns these instructions into a path (a struct containing array of
 points) which is then returned.
 */
pointArray * buildPath( symbolList * symList)
{
    if(symList->length==0) return NULL;
    pointArray * path = initPath();
    turtle * t = startingPoint();
    sampleTurtle(path, t);
    symbolNode * currentInstruction = symList->start;
    while(currentInstruction!=NULL)//move through linked list
    {
        if(currentInstruction->sym==symFD)
        {
            moveTurtleFD( t, currentInstruction->value);
            sampleTurtle(path, t);
        }
        else if(currentInstruction->sym==symRT || currentInstruction->sym==symLT ||
                currentInstruction->sym==symUT || currentInstruction->sym==symDT )
        {
            rotateTurtle(t, currentInstruction->sym, currentInstruction->value);
        }
        else
        {
            printError("Unexpected sym in symList.", __FILE__, __FUNCTION__, __LINE__);
            return NULL;
        }
        currentInstruction=currentInstruction->next;
    }
    free(t);
    freeSymList(symList);
    return path;
}

/**
 Moves turtle in the direction t->direction by ammount.
 */
void moveTurtleFD(turtle * t, float ammount)
{
    for(dimension dim = X; dim<=Z; ++dim)
    {
        t->position.r[dim] += ammount*t->forwardDirection[dim];
    }
}

/**
 Rotates a turtle by the type and ammount specified.
 */
void rotateTurtle(turtle * t, symbol sym, float ammount)
{
    float ammountInRadians = convertDegreesToRadians(ammount);
    if(sym==symDT || sym==symLT) ammountInRadians = -ammountInRadians;
    
    if( sym==symLT || sym==symRT )
    {
        t->theta += ammountInRadians;
        while (t->theta<0)
        {
            t->theta += 2*M_PI;
        }
        while (t->theta>2*M_PI)
        {
            t->theta -= 2*M_PI;
        }
    }
    if( sym==symUT || sym==symDT )
    {
        t->phi += ammountInRadians;
        while (t->phi<0)
        {
            t->phi += 2*M_PI;
        }
        while (t->phi>2*M_PI)
        {
            t->phi -= 2*M_PI;
        }
    }
    if(VERBOSE) printf("turtle angles: theta = %f, phi = %f)\n", t->theta, t->phi);

    t->forwardDirection[X] = sin(t->phi)*cos(t->theta);
    t->forwardDirection[Y] = sin(t->phi)*sin(t->theta);
    t->forwardDirection[Z] = cos(t->phi);
}

/**
 Converts an angle in degrees into radians
 */
float convertDegreesToRadians(float degrees)
{
    return (degrees * M_PI)/180;
}

/**
 Builds and returns a * to a fresh turtle struct.
 Turtle starts facing along +ve x axis and we, looking down the +ve z axis
 see the top of its shell.
 */
turtle * startingPoint()
{
    turtle * t = malloc(sizeof(turtle));
    if(t==NULL)
    {
        printError("malloc failed exiting.",__FILE__,__FUNCTION__,__LINE__);
        exit(1);
    }
    t->theta = 0;
    t->phi = M_PI_2;
    t->forwardDirection[X]=sin(t->phi)*cos(t->theta);
    t->forwardDirection[Y]=sin(t->phi)*sin(t->theta);
    t->forwardDirection[Z]=cos(t->phi);
    for(dimension dim = X; dim<=DIM_MAX; ++dim)
    {
        t->position.r[dim]=0;
    }
    return t;
}

/**
 Builds and returns a * to a fresh pointArray struct
 */
pointArray * initPath()
{
    pointArray * path = malloc(sizeof(pointArray));
    path->numberOfPoints=0;
    path->array=NULL;
    return path;
}

/**
 Takes turtles position and adds it to the path.
 */
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

#pragma mark Unit Test Functions
void unitTests_path()
{
    sput_start_testing();
    sput_set_output_stream(NULL);

    sput_enter_suite("testDegreesToRadians()");
    sput_run_test(testDegreesToRadians);
    sput_leave_suite();
    
    sput_enter_suite("testStartingPoint()");
    sput_run_test(testStartingPoint);
    sput_leave_suite();
    
    sput_enter_suite("testRotateTurtle()");
    sput_run_test(testRotateTurtle);
    sput_leave_suite();
    
    sput_enter_suite("testMoveTurtleFD()");
    sput_run_test(testMoveTurtleFD);
    sput_leave_suite();
    
    sput_enter_suite("testSampleTurtle()");
    sput_run_test(testSampleTurtle);
    sput_leave_suite();
   
    sput_enter_suite("testBuildPath()");
    sput_run_test(testBuildPath);
    sput_leave_suite();
    
    sput_finish_testing();
}

void testDegreesToRadians()
{
    sput_fail_unless(floatCompare(convertDegreesToRadians(90),M_PI_2), "check convertDegreesToRadians.");
    sput_fail_unless(floatCompare(convertDegreesToRadians(45),M_PI_4), "check convertDegreesToRadians.");
    sput_fail_unless(floatCompare(convertDegreesToRadians(180),M_PI), "check convertDegreesToRadians.");
}

void testStartingPoint()
{
    turtle * t = startingPoint();
    
    sput_fail_unless(t->theta==0,
                     "Check all elements of the returned stuct are accessible and set correctly.");
    for(dimension dim = X; dim<=DIM_MAX; ++dim)
    {
        sput_fail_unless(t->position.r[dim]==0,
                         "Check all elements of the returned stuct are accessible and set correctly.");
        sput_fail_unless(t->position.r[dim]==0,
                         "Check all elements of the returned stuct are accessible and set correctly.");
    }
    
    free(t);

}

void testRotateTurtle()
{
    turtle * t = startingPoint();
    
    rotateTurtle(t,symLT, 90);
    if(VERBOSE) printf("dirc = %f/pi\n",t->theta/M_PI);
    sput_fail_unless(floatCompare(t->theta,3*M_PI_2)==1 ,
                     "rotating left pi/2 should set t->direction to 3*pi/2.");
    t->theta=0;
    rotateTurtle(t,symRT, 90);
    if(VERBOSE) printf("dirc = %f/pi\n",t->theta/M_PI);
    sput_fail_unless(floatCompare(t->theta, M_PI_2)==1 ,
                     "rotating right pi/2 should set t->direction to pi/2.");
    t->theta=0;
    rotateTurtle(t,symRT, 720+45);
    if(VERBOSE) printf("dirc = %f/pi\n",t->theta/M_PI);
    sput_fail_unless(floatCompare(t->theta, M_PI_4)==1 ,
                     "rotating right 4pi +pi/4 should set t->direction to pi/4.");
    t->theta=0;
    rotateTurtle(t,symLT, 720+45);
    if(VERBOSE) printf("dirc = %f/pi\n",t->theta/M_PI);
    sput_fail_unless(floatCompare(t->theta, 2*M_PI - M_PI_4)==1 ,
                     "rotating left 4pi +pi/4 should set t->direction to pi/4.");
    free(t);
}

void testMoveTurtleFD()
{
    turtle * t = startingPoint();
    float ammount = 0.5;
    moveTurtleFD(t, ammount);
    sput_fail_unless(floatCompare(t->position.r[X],ammount) &&
                     floatCompare(t->position.r[Y],0) ,
                     "moving FD 0.5 with direction = 0 should set r[X] to 0.5.");
    
    rotateTurtle(t,symRT, 90);
    moveTurtleFD(t, ammount);
    sput_fail_unless(floatCompare(t->position.r[X],ammount) &&
                     floatCompare(t->position.r[Y],ammount) ,
                     "moving FD 0.5 with direction = pi/2 should set r[Y] to 0.5.");
    
    rotateTurtle(t,symRT, 90);
    moveTurtleFD(t, ammount);
    sput_fail_unless(floatCompare(t->position.r[X],0) &&
                     floatCompare(t->position.r[Y],ammount) ,
                     "moving FD 0.5 with direction = pi should set r[X] back to 0.");
    rotateTurtle(t,symRT, 90);
    moveTurtleFD(t, ammount);
    sput_fail_unless(floatCompare(t->position.r[X],0) &&
                     floatCompare(t->position.r[Y],0) ,
                     "moving FD 0.5 with direction = pi should set r[Y] back to 0.");
    free(t);
    t = startingPoint();
    rotateTurtle(t,symRT, 45);
    moveTurtleFD(t, ammount);
    sput_fail_unless(floatCompare(t->position.r[X],ammount/sqrt(2)) &&
                     floatCompare(t->position.r[Y],ammount/sqrt(2)) ,
                     "Checking that non right angle directions work");
    free(t);
    t = startingPoint();
    rotateTurtle(t,symLT, 45);
    moveTurtleFD(t, ammount);
    sput_fail_unless(floatCompare(t->position.r[X],ammount/sqrt(2)) &&
                     floatCompare(t->position.r[Y],-ammount/sqrt(2)) ,
                     "Checking that non right angle directions work");
    free(t);
    t = startingPoint();
    rotateTurtle(t,symRT, 135);
    moveTurtleFD(t, ammount);
    sput_fail_unless(floatCompare(t->position.r[X],-ammount/sqrt(2)) &&
                     floatCompare(t->position.r[Y],ammount/sqrt(2)) ,
                     "Checking that non right angle directions work");
    free(t);
    t = startingPoint();
    rotateTurtle(t, symLT,135 );
    moveTurtleFD(t, ammount);
    sput_fail_unless(floatCompare(t->position.r[X],-ammount/sqrt(2)) &&
                     floatCompare(t->position.r[Y],-ammount/sqrt(2)) ,
                     "Checking that non right angle directions work");
    free(t);
    t = startingPoint();
    rotateTurtle(t, symRT, 1.9);
    moveTurtleFD(t, ammount);
    sput_fail_unless(floatCompare(t->position.r[X],cos(convertDegreesToRadians(1.9))*ammount) &&
                     floatCompare(t->position.r[Y],sin(convertDegreesToRadians(1.9))*ammount) ,
                     "Checking that non right angle directions work");
}

void testSampleTurtle()
{
    turtle * t = startingPoint();
    pointArray * path = initPath();
    sampleTurtle(path, t);
    sput_fail_unless(floatCompare(path->array[0].r[X],0) && floatCompare(path->array[0].r[Y],0),
                    "Sampling the initial turtle should add a point at the origin to path. ");
    
    float ammount = 0.5;
    moveTurtleFD(t, ammount);
    sampleTurtle(path, t);
    sput_fail_unless(floatCompare(path->array[1].r[X],ammount) && floatCompare(path->array[1].r[Y],0),
                     "Moving turtle in a square sampling at each corner checking it gets added the pointArray");
    
    rotateTurtle(t,symRT, 90);
    moveTurtleFD(t, ammount);
    sampleTurtle(path, t);
    sput_fail_unless(floatCompare(path->array[2].r[X],ammount) && floatCompare(path->array[2].r[Y],ammount),
                     "Moving turtle in a square sampling at each corner checking it gets added the pointArray");
    rotateTurtle(t,symRT, 90);
    moveTurtleFD(t, ammount);
    sampleTurtle(path, t);
    sput_fail_unless(floatCompare(path->array[3].r[X],0) && floatCompare(path->array[3].r[Y],ammount),
                     "Moving turtle in a square sampling at each corner checking it gets added the pointArray");
    rotateTurtle(t,symRT, 90);
    moveTurtleFD(t, ammount);
    sampleTurtle(path, t);
    sput_fail_unless(floatCompare(path->array[4].r[X],0) && floatCompare(path->array[4].r[Y],0),
                     "Moving turtle in a square sampling at each corner checking it gets added the pointArray");

    freePath(path);
    free(t);
}

void testBuildPath()
{
    pointArray * path = buildPath(mockSymListForPathUnitTests());
    sput_fail_unless(floatCompare(path->array[0].r[X],0) &&
                     floatCompare(path->array[0].r[Y],0),
                     "Testing buildPath with mockSymList.");
    sput_fail_unless(floatCompare(path->array[1].r[X],20) &&
                     floatCompare(path->array[1].r[Y],0),
                     "Testing buildPath with mockSymList.");
    sput_fail_unless(floatCompare(path->array[2].r[X],20) &&
                     floatCompare(path->array[2].r[Y],20),
                     "Testing buildPath with mockSymList.");
    sput_fail_unless(floatCompare(path->array[3].r[X],40) &&
                     floatCompare(path->array[3].r[Y],20),
                     "Testing buildPath with mockSymList.");
    sput_fail_unless(floatCompare(path->array[4].r[X],0) &&
                     floatCompare(path->array[4].r[Y],20),
                     "Testing buildPath with mockSymList.");
    sput_fail_unless(floatCompare(path->array[5].r[X],0) &&
                     floatCompare(path->array[5].r[Y],0),
                     "Testing buildPath with mockSymList.");
    freePath(path);
}


/**Copied from parser.c for reference
 Builds a symbolList for use in path.c unit tests
 
symbolList * mockSymListForPathUnitTests()
{
    parser * p = initParser();
    addSymToList(p, symFD, 20);
    addSymToList(p, symRT, 90);
    addSymToList(p, symFD, 20);
    addSymToList(p, symLT, 90);
    addSymToList(p, symFD, 20);
    addSymToList(p, symRT, 180);
    addSymToList(p, symFD, 40);
    addSymToList(p, symRT, 90);
    addSymToList(p, symFD, 20);
    symbolList * symList = p->symList;
    freeParser(p);//note that this function never frees p->symList.
    return symList;
}

*/


pointArray * mockPathForDrawUnitTests()
{
    pointArray * path = buildPath(mockSymListForPathUnitTests());
    return path;
}


