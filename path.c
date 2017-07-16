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
  float direction;//angle with +ve x axis (in radians)
  point position;//r=(x,y)
} turtle;

pointArray ** buildPath(symbolList * symList, int numberOfTurtles);
turtle ** initTurtles(int numberOfTurtles);
void sampleTurtle(pointArray * path, turtle * t);
void moveTurtleFD(turtle * t, float ammount);
void rotateTurtle(turtle * t, symbol leftOrRight, float ammount);
float convertDegreesToRadians(float degrees);
pointArray ** initPath(int numberOfTurtles);

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
pointArray ** buildPath( symbolList * symList, int numberOfTurtles)
{
  if(symList->length==0) return NULL;
  pointArray ** paths = initPath(numberOfTurtles);
  turtle ** turtles = initTurtles(numberOfTurtles);
  for(int turtle = 0; turtle<numberOfTurtles; ++turtle) {
    //    turtle * t = turtles[turtle];
    pointArray * path = paths[turtle];
    sampleTurtle(path, turtles[turtle]);
    symbolNode * currentInstruction = symList->start;
    while(currentInstruction!=NULL) {
      if(currentInstruction->sym==symFD) {
	moveTurtleFD(turtles[turtle], currentInstruction->value);
	sampleTurtle(path, turtles[turtle]);
      } else if(currentInstruction->sym==symRT || currentInstruction->sym==symLT) {
	rotateTurtle(turtles[turtle], currentInstruction->sym,
		     currentInstruction->value);
      } else {
	printError("Unexpected sym in symList.",
		   __FILE__, __FUNCTION__, __LINE__);
	return NULL;
      }
      currentInstruction=currentInstruction->next;
    }
    free(turtles[turtle]);
  }
  free(turtles);
  freeSymList(symList);
  return paths;
}

/**
   Moves turtle in the direction t->direction by ammount.
*/
void moveTurtleFD(turtle * t, float ammount)
{
  t->position.r[X] += ammount*cos(t->direction);
  t->position.r[Y] += ammount*sin(t->direction);
}

/**
   if leftOrRight= symRT then increase t->direction by ammount
   if leftOrRight= symLT then decrease t->direction by ammount
   Values wrapped so 0 <=  t->direction <= 2pi
*/
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

/**
   Converts an angle in degrees into radians
*/
float convertDegreesToRadians(float degrees)
{
  return (degrees * M_PI)/180;
}

/**
   Builds and returns a fresh list of turtle structs. 
   Starting from the origin and point in different directions symetrically.
*/
turtle ** initTurtles(int numberOfTurtles)
{
  printf("numberOfTurtles = %d ", numberOfTurtles);
  turtle ** turtles = malloc(numberOfTurtles*sizeof(turtle *));
  if(turtles==NULL) {
    printError("malloc failed exiting.",__FILE__,__FUNCTION__,__LINE__);
    exit(1);
  }
  for(int t=0; t<numberOfTurtles; ++t) {
    printf("2 numberOfTurtles = %d ", numberOfTurtles);
    turtles[t] = malloc(sizeof(turtle));
    if(turtles[t]==NULL) {
      printError("malloc failed exiting.",__FILE__,__FUNCTION__,__LINE__);
      exit(1);
    }
    turtles[t]->direction = (t/(float)numberOfTurtles)*2*M_PI;
    printf("turtle %d -> dir= %f, was set to %f numberOfTurtles = %f t/numberOfTurtles ",t, turtles[t]->direction, (t/(float)numberOfTurtles)*2*M_PI, (float)numberOfTurtles, t/(float)numberOfTurtles);
    for(dimension dim = X; dim<=DIM_MAX; ++dim){
      turtles[t]->position.r[dim]=0;
    }
  }
  return turtles;  
}

/**
   Builds and returns a * to a fresh pointArray struct
*/
pointArray ** initPath(int numberOfTurtles)
{
  pointArray ** paths = malloc(numberOfTurtles*sizeof(pointArray *));
  if(paths==NULL) {
    printError("malloc failed exiting.",__FILE__,__FUNCTION__,__LINE__);
    exit(1);
  }
  for(int i=0; i<numberOfTurtles; ++i) {
    paths[i] = malloc(sizeof(pointArray));
    if(paths[i]==NULL) {
      printError("malloc failed exiting.",__FILE__,__FUNCTION__,__LINE__);
      exit(1);
    }
    paths[i]->numberOfPoints=0;
    paths[i]->array=NULL;
  }
  return paths;
}

/**
   Takes turtles position and adds it to the path.
*/
void sampleTurtle (pointArray * path, turtle * t)
{
  ++path->numberOfPoints;
  point * tmp = realloc(path->array, path->numberOfPoints*sizeof(point));
  if(tmp==NULL) {
    printError("realloc failed exiting.",__FILE__,__FUNCTION__,__LINE__);
    exit(1);
  }
  path->array = tmp;
  for(dimension dim = X; dim<=DIM_MAX; ++dim) {
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

void testRotateTurtle()
{
  turtle * t = initTurtles(1)[0];
    
  rotateTurtle(t,symLT, 90);
  if(VERBOSE) printf("dirc = %f/pi\n",t->direction/M_PI);
  sput_fail_unless(floatCompare(t->direction,3*M_PI_2)==1 ,
		   "rotating left pi/2 should set t->direction to 3*pi/2.");
  t->direction=0;
  rotateTurtle(t,symRT, 90);
  if(VERBOSE) printf("dirc = %f/pi\n",t->direction/M_PI);
  sput_fail_unless(floatCompare(t->direction, M_PI_2)==1 ,
		   "rotating right pi/2 should set t->direction to pi/2.");
  t->direction=0;
  rotateTurtle(t,symRT, 720+45);
  if(VERBOSE) printf("dirc = %f/pi\n",t->direction/M_PI);
  sput_fail_unless(floatCompare(t->direction, M_PI_4)==1 ,
		   "rotating right 4pi +pi/4 should set t->direction to pi/4.");
  t->direction=0;
  rotateTurtle(t,symLT, 720+45);
  if(VERBOSE) printf("dirc = %f/pi\n",t->direction/M_PI);
  sput_fail_unless(floatCompare(t->direction, 2*M_PI - M_PI_4)==1 ,
		   "rotating left 4pi +pi/4 should set t->direction to pi/4.");
  free(t);
}

void testMoveTurtleFD()
{
  turtle * t = initTurtles(1)[0];
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
  t = initTurtles(1)[0];
  rotateTurtle(t,symRT, 45);
  moveTurtleFD(t, ammount);
  sput_fail_unless(floatCompare(t->position.r[X],ammount/sqrt(2)) &&
		   floatCompare(t->position.r[Y],ammount/sqrt(2)) ,
		   "Checking that non right angle directions work");
  free(t);
  t = initTurtles(1)[0];
  rotateTurtle(t,symLT, 45);
  moveTurtleFD(t, ammount);
  sput_fail_unless(floatCompare(t->position.r[X],ammount/sqrt(2)) &&
		   floatCompare(t->position.r[Y],-ammount/sqrt(2)) ,
		   "Checking that non right angle directions work");
  free(t);
  t = initTurtles(1)[0];
  rotateTurtle(t,symRT, 135);
  moveTurtleFD(t, ammount);
  sput_fail_unless(floatCompare(t->position.r[X],-ammount/sqrt(2)) &&
		   floatCompare(t->position.r[Y],ammount/sqrt(2)) ,
		   "Checking that non right angle directions work");
  free(t);
  t = initTurtles(1)[0];
  rotateTurtle(t, symLT,135 );
  moveTurtleFD(t, ammount);
  sput_fail_unless(floatCompare(t->position.r[X],-ammount/sqrt(2)) &&
		   floatCompare(t->position.r[Y],-ammount/sqrt(2)) ,
		   "Checking that non right angle directions work");
  free(t);
  t = initTurtles(1)[0];
  rotateTurtle(t, symRT, 1.9);
  moveTurtleFD(t, ammount);
  sput_fail_unless(floatCompare(t->position.r[X],cos(convertDegreesToRadians(1.9))*ammount) &&
		   floatCompare(t->position.r[Y],sin(convertDegreesToRadians(1.9))*ammount) ,
		   "Checking that non right angle directions work");
}

void testSampleTurtle()
{
  turtle * t = initTurtles(1)[0];
  pointArray * path = initPath(1)[0];
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
  pointArray * path = buildPath(mockSymListForPathUnitTests(),1)[0];
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


pointArray ** mockPathsForDrawUnitTests()
{
  pointArray ** paths = buildPath(mockSymListForPathUnitTests(),1);
  return paths;
}

pointArray * mockPathForDrawUnitTests()
{
  pointArray * path = buildPath(mockSymListForPathUnitTests(),1)[0];
  return path;
}


