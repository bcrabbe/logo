//
//  draw.c
//  logo
//
//  Created by ben on 24/01/2015.
//  Copyright (c) 2015 ben. All rights reserved.
//
#include "main.h"
#include <stdio.h>
#include <SDL2/SDL.h>

typedef struct display
{
    SDL_bool finished;
    SDL_bool skip;
    SDL_Window *win;
    int winSize[2];
    SDL_Renderer *renderer;
    SDL_Event *event;
    
} display;

typedef enum sdlKey {
    UP = 1,
    DOWN = -1,
    LEFT = -2,
    RIGHT = +2,
    NONE = 0
} sdlKey;

typedef struct scaler {
    float scale[2];//pixcels per unit distance
    float offset[2];//px
    float imageDimension[3];
    float minZ;//the minumum Z value
    float centreOfPath[NUMBER_OF_DIMENSIONS];
    float centreOfWindow[NUMBER_OF_DIMENSIONS];
    float spanOfPath[NUMBER_OF_DIMENSIONS];
} scaler;

typedef enum rotate {
    rotateAroundX,
    rotateAroundY,
} rotation;
#pragma mark prototypes
scaler * getScaler(display * d, pointArray * path);
pointArray * renderPath(pointArray * path, scaler * s);
pointArray * scalePath(pointArray * path2d, scaler * s);
void drawPath(display * d, pointArray * path);
sdlKey getSdlKeyPresses(display * d);
void zoom(scaler * s, int zoomIn);
void rotatePath(pointArray * path, rotation , float angle);
void transformPoint( point * vector, float matrix[3][3] );

display * startSDL();
int checkSDLwinClosed(display *d);
void quitSDL(display * d);

void printPath(pointArray * path, dimension maxDim, char * name);

#pragma mark Unit Test Prototypes
void testStartSDL();
void testGetScaler();
void testScalePath();

#pragma mark draw functions
/**
 Draws lines between each of the points in path to an sdl window
 */
void draw(pointArray * path)
{
    if(VERBOSE) printPath(path,Z,"original path :");
    display * d = startSDL();
    scaler * s = getScaler(d, path);//size information on path, needed for rendering
    pointArray * path2d = renderPath(path, s);//makes 2d path from view
    if(VERBOSE) printPath(path2d,Y,"Rendered Path :");
    free(s);//free old scaler
    s = getScaler(d, path2d);//2nd path has new dimensions
    pointArray * scaled2dPath = scalePath(path2d, s);//scale it to window size
    if(VERBOSE) printPath(scaled2dPath,Y,"Scaled2dPath :");
    freePath(path2d);//dont need unscaled 2d path anymore
    
    printf("Press arrows to rotate.\n");
    sdlKey key = NONE;
    while(!d->finished)
    {
        drawPath(d, scaled2dPath);
        checkSDLwinClosed(d);
        key = getSdlKeyPresses(d);
        if(key==NONE ) break;
        //rotates original path:
        if(key==UP) rotatePath(path, rotateAroundY, ROTATION_SENSITIVITY );
        if(key==DOWN) rotatePath(path, rotateAroundY, -ROTATION_SENSITIVITY);
        if(key==LEFT) rotatePath(path, rotateAroundX, -ROTATION_SENSITIVITY );
        if(key==RIGHT) rotatePath(path, rotateAroundX, ROTATION_SENSITIVITY);
        if(ADJUST_ZOOM_TO_FIT_ROTATED_OBJECT)
        {
            free(s);//free old scaler
            s = getScaler(d, path);
        }
        if(VERBOSE) printPath(path,Z,"Rotated 3dPath :");
        path2d = renderPath(path, s);
        if(VERBOSE) printPath(path2d,Y,"Rotated 2dPath");
        free(s);//free old scaler
        s = getScaler(d, path2d);
        scaled2dPath = scalePath(path2d, s);
        if(VERBOSE) printPath(scaled2dPath, Y, "Rotated and scaled 2dPath :");

    }
    free(s);//free scaler
    freePath(path);//free unscaled path
    freePath(scaled2dPath);
    quitSDL(d);
}




#pragma Scaling functions

/*
 Takes path and works out what the px/unit (FD) distance should be to fit the
 entire path on to 90% of the screen. It also works out the translation vector required to
 move the path into the centre of the window.
 It returns a malloc'd scaler *
 */
scaler * getScaler(display * d, pointArray * path)
{
    scaler * s = malloc(sizeof(scaler));
    if(s==NULL)
    {
        printError("scaler * s = malloc(sizeof(scaler)) failed exiting.",__FILE__,__FUNCTION__,__LINE__);
        return NULL;
    }
    float rMin[NUMBER_OF_DIMENSIONS]={0};
    float rMax[NUMBER_OF_DIMENSIONS]={0};
    for(int point = 0; point<path->numberOfPoints; ++point)
    {
        for(dimension dim = X; dim<=Z; ++dim)
        {
            if(path->array[point].r[dim] > rMax[dim])
            {
                rMax[dim] = path->array[point].r[dim];
            }
            if(path->array[point].r[dim] < rMin[dim])
            {
                rMin[dim] = path->array[point].r[dim];
            }
        }
    }
    for(dimension dim = X; dim<=Y; ++dim)
    {
        s->spanOfPath[dim] = rMax[dim]-rMin[dim];
        s->centreOfPath[dim] = (rMax[dim]-rMin[dim])/2;
        s->centreOfWindow[dim] = (float)d->winSize[dim]/2;
        s->scale[dim] = 0.8*( (float)d->winSize[dim]  / s->spanOfPath[dim]);
        s->offset[dim] =  -s->scale[dim]*s->centreOfPath[dim] + s->centreOfWindow[dim] + s->scale[dim]*s->spanOfPath[dim]/2;
        s->imageDimension[dim] = s->spanOfPath[dim];
    }
    s->minZ = rMin[Z];


    if(!STRETCH_TO_FIT_WINDOW)
    {//if we dont want to alter ratio then use the smallest scale for both.
        if( s->scale[X] < s->scale[Y] ) s->scale[Y] = s->scale[X];
        if( s->scale[X] > s->scale[Y] ) s->scale[X] = s->scale[Y];
    }
    if(VERBOSE)
    {
        printf("scale: %f,%f\n", s->scale[X], s->scale[Y]);
        // printf("offset: %f,%f\n", s->offset[X], s->offset[Y]);
    }
    return s;
}
/**
 Takes path and transforms each point: path->array[point].r[dim]*s->scale[dim] + s->offset[dim]
 This maps the points on to the display coordinates.
 returns a new, malloc'd, path. This and the old one should be free'd.
 */
pointArray * scalePath(pointArray * path2d, scaler * s)
{
    pointArray * scaled2DPath = malloc(sizeof(pointArray));
    if(scaled2DPath==NULL)
    {
        printError("pointArray * scaledPath = malloc(sizeof(pointArray)) failed exiting.",
                   __FILE__,__FUNCTION__,__LINE__);
        return NULL;
    }
    scaled2DPath->numberOfPoints = path2d->numberOfPoints;
    scaled2DPath->array = malloc(path2d->numberOfPoints*sizeof(point));
    if(scaled2DPath->array==NULL)
    {
        printError("scaledPath->array = malloc(path->numberOfPoints*sizeof(point)) failed exiting.",
                   __FILE__,__FUNCTION__,__LINE__);
        return NULL;
    }
    for(int point = 0; point < path2d->numberOfPoints; ++point)
    {
        for(dimension dim = X; dim<=Y; ++dim)
        {
            scaled2DPath->array[point].r[dim] = path2d->array[point].r[dim]*s->scale[dim] + s->offset[dim];
        }
        scaled2DPath->array[point].r[Z] = 0;
    }
    return scaled2DPath;
}

pointArray * renderPath(pointArray * path, scaler * s)
{
    pointArray * path2d = malloc(sizeof(pointArray));
    if(path2d==NULL)
    {
        printError("pointArray * scaledPath = malloc(sizeof(pointArray)) failed exiting.",
                   __FILE__,__FUNCTION__,__LINE__);
        return NULL;
    }
    path2d->numberOfPoints = path->numberOfPoints;
    path2d->array = malloc(path->numberOfPoints*sizeof(point));
    if(path2d->array==NULL)
    {
        printError("scaledPath->array = malloc(path->numberOfPoints*sizeof(point)) failed exiting.",
                   __FILE__,__FUNCTION__,__LINE__);
        return NULL;
    }
    for(int point = 0; point<path2d->numberOfPoints; ++point)
    {
        path2d->array[point].r[X] = path->array[point].r[X] /
                                    (path->array[point].r[Z] + VIEWING_DISTANCE*s->imageDimension[X]+s->minZ);
        path2d->array[point].r[Y] = path->array[point].r[Y] /
                                    (path->array[point].r[Z] + VIEWING_DISTANCE*s->imageDimension[Y]+s->minZ);
        path2d->array[point].r[Z] = 0;
    }
    return path2d;
}

void rotatePath(pointArray * path, rotation rot, float angle)//angle in radians
{
    float rotationMatrix[3][3]={{0}};
    if(rot==rotateAroundX)
    {
        rotationMatrix[0][0] = cos(angle);
        rotationMatrix[0][2] = sin(angle);
        rotationMatrix[2][0] = -sin(angle);
        rotationMatrix[2][2] = cos(angle);
        rotationMatrix[1][1] = 1;
    }
    if(rot==rotateAroundY)
    {
        rotationMatrix[0][0] = 1;
        rotationMatrix[1][1] = cos(angle);
        rotationMatrix[1][2] = sin(angle);
        rotationMatrix[2][1] = -sin(angle);
        rotationMatrix[2][2] = cos(angle);
    }

    for(int point = 0; point<path->numberOfPoints; ++point)
    {
        transformPoint(&path->array[point],rotationMatrix);
    }
}

void transformPoint( point * vector, float matrix[3][3] )
{
    for(dimension dim = X; dim<=Z; ++dim)
    {
        vector->r[dim] = matrix[dim][0]*vector->r[X] +
                        matrix[dim][1]*vector->r[Y] +
                        matrix[dim][2]*vector->r[Z];
    }
}

void printPath(pointArray * path, dimension maxDim, char * name)
{
    char nameString[MAX_ERROR_STRING_SIZE] = "\n\nPrinting ";
    sprintf(nameString, "\n\nShowing %s .\n\n",name);
    for(int p=0; p<path->numberOfPoints; ++p)
    {
        if(maxDim==Y) printf("( %f, %f)\n",path->array[p].r[X], path->array[p].r[Y]);
        else if(maxDim==Z) printf("( %f, %f, %f)\n",path->array[p].r[X], path->array[p].r[Y], path->array[p].r[Z]);
    }
}

#pragma mark SDL functions

/* conect the points in the path with lines in SDL window
 */
void drawPath(display * d, pointArray * path)
{
    SDL_SetRenderDrawColor( d->renderer, 0x00, 0x00, 0x00, 0xFF );
    SDL_RenderClear(d->renderer);
    SDL_SetRenderDrawColor( d->renderer, 0xFF, 0xFF, 0xFF, 0xFF );
    for(int point = 0; point<path->numberOfPoints-1; ++point)
    {
        SDL_RenderDrawLine(d->renderer,
                           path->array[point].r[X],path->array[point].r[Y],
                           path->array[point+1].r[X],path->array[point+1].r[Y]);
    }
    SDL_RenderPresent(d->renderer);
}

/*  waits for an arrow or for the window to be closed
 */
sdlKey getSdlKeyPresses(display * d)
{
    while(1)
    {
        SDL_PollEvent(d->event);
        if( d->event->type == SDL_KEYDOWN )
        {
            int sym = d->event->key.keysym.sym;
            if (sym == SDLK_UP )         return UP;
            else if (sym == SDLK_DOWN )  return DOWN;
            else if (sym == SDLK_LEFT )  return LEFT;
            else if (sym == SDLK_RIGHT )  return RIGHT;
        }
        if(checkSDLwinClosed(d)) return NONE;
        SDL_Delay(1e3/FPS);
    }
}
/*  Call frequently to see if the wind has been closed
 then check d->finished to see if this is the case
 */
int checkSDLwinClosed(display *d)
{
    if(SDL_PollEvent(d->event))
    {
        if( d->event->type == SDL_QUIT )
        {
            d->finished = 1;
            return 1;
        }
    }
    return 0;
}
display * startSDL()
{
    display * d = malloc(sizeof(display));
    if(d==NULL)
    {
        printError("display * d = malloc(sizeof(display)) failed exiting.",__FILE__,__FUNCTION__,__LINE__);
        return NULL;
    }
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        char errStr[MAX_ERROR_STRING_SIZE];
        sprintf(errStr, "Unable to initialize SDL:  %s", SDL_GetError());
        printError(errStr, __FILE__, __FUNCTION__, __LINE__);
        SDL_Quit();
        free(d);
        return NULL;
    }
    d->finished = 0;
    d->skip = 0;
    d->winSize[X] = 900;
    d->winSize[Y] = 660;
    d->win= SDL_CreateWindow("SDL Window",
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             d->winSize[X], d->winSize[Y],
                             SDL_WINDOW_SHOWN);
    if(d->win == NULL)
    {
        char errStr[MAX_ERROR_STRING_SIZE];
        sprintf(errStr, "Unable to initialize SDL Window:  %s", SDL_GetError());
        printError(errStr, __FILE__, __FUNCTION__, __LINE__);
        SDL_Quit();
        free(d);
        return NULL;
    }
    d->renderer = SDL_CreateRenderer(d->win, -1, 0);
    if(d->renderer == NULL)
    {
        char errStr[MAX_ERROR_STRING_SIZE];
        sprintf(errStr, "Unable to initialize SDL renderer:  %s", SDL_GetError());
        printError(errStr, __FILE__, __FUNCTION__, __LINE__);
        SDL_Quit();
        free(d);
        return NULL;
    }
    
    d->event = malloc(sizeof(SDL_Event));
    if(d->event==NULL)
    {
        printError("d->event = malloc(sizeof(SDL_Event)) failed exiting.",__FILE__,__FUNCTION__,__LINE__);
        return NULL;
    }
    SDL_SetRenderDrawColor(d->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    if(SDL_RenderClear(d->renderer)<0)
    {
        char errStr[MAX_ERROR_STRING_SIZE];
        sprintf(errStr, "Could not renderClear in startSDL Error: %s", SDL_GetError());
        printError(errStr, __FILE__, __FUNCTION__, __LINE__);
        SDL_Quit();
        free(d);
        return NULL;
    }
    SDL_RenderPresent(d->renderer);
    return d;
}



/* call if window is closed
 */
void quitSDL(display * d)
{
    free(d->event);
    SDL_DestroyRenderer( d->renderer);
    SDL_DestroyWindow( d->win );
    d->renderer = NULL;
    d->win = NULL;
    SDL_Quit();
}
#if 0
#pragma mark Unit Test Functions
void unitTests_draw()
{
    sput_start_testing();
    sput_set_output_stream(NULL);
    
    sput_enter_suite("testStartSDL()");
    sput_run_test(testStartSDL);
    sput_leave_suite();

    sput_enter_suite("testGetScaler()");
    sput_run_test(testGetScaler);
    sput_leave_suite();

    sput_enter_suite("testScalePath()");
    sput_run_test(testScalePath);
    sput_leave_suite();
    
    sput_finish_testing();
}


void testStartSDL()
{
    display * d = startSDL();
    
    sput_fail_unless(d->finished==0, "Checking all elements of d are accessible and correctly set.");
    sput_fail_unless(d->skip==0, "Checking all elements of d are accessible and correctly set.");
    sput_fail_unless(d->winSize[X]==SDL_WINDOW_WIDTH, "Checking all elements of d are accessible and correctly set.");
    sput_fail_unless(d->winSize[Y]==SDL_WINDOW_HEIGHT, "Checking all elements of d are accessible and correctly set.");
    sput_fail_unless(d->win!=NULL, "Checking all elements of d are accessible and correctly set.");
    sput_fail_unless(d->renderer!=NULL, "Checking all elements of d are accessible and correctly set.");
    sput_fail_unless(d->event!=NULL, "Checking all elements of d are accessible and correctly set.");
}

void testGetScaler()
{
    display * d = startSDL();
    pointArray * path = mockPathForDrawUnitTests();
    scaler * s = getScaler(d, path);
    /*  this is the mock path:
        0,0
        20,0
        20,20
        40,20
        0,20
        0,0
(0,0)----------(20,0)
    |          |
    |          |
    |          |
    |          |(20,20)
    |          |___________
(0,20)---------------------(40,20)
     */
    sput_fail_unless(floatCompare(s->scale[X],(0.9*(float)d->winSize[X])/(40)),
                     "the x scaler should be set to this value.");
    sput_fail_unless(floatCompare(s->scale[Y],(0.9*(float)d->winSize[Y])/(20)),
                     "the y scaler should be set to this value.");
    sput_fail_unless(floatCompare(s->offset[X],(float)d->winSize[X]/2-s->scale[X]*20),
                     "the x offset should be set to this value.");
    sput_fail_unless(floatCompare(s->offset[Y],(float)d->winSize[Y]/2-s->scale[Y]*10),
                     "the y offset should be set to this value.");
    free(s);
    freePath(path);
    quitSDL(d);
}

void testScalePath()
{
    display * d = startSDL();
    pointArray * path = mockPathForDrawUnitTests();
    scaler * s = getScaler(d, path);
    pointArray * scaledPath = scale(path, s);
    for(int point = 0; point<scaledPath->numberOfPoints; ++point)
    {
        for(dimension dim=X;dim<=DIM_MAX;++dim)
        {
            sput_fail_unless( scaledPath->array[point].r[dim] <= d->winSize[dim] &&
                              scaledPath->array[point].r[dim] >= 0,
            "Each coordinate of the scaled path should be within the window dimensions");
        }
    }
    renderPath(d,scaledPath);
    while(!d->finished)
    {
        checkSDLwinClosed(d);
    }
    free(s);
    freePath(path);
    quitSDL(d);
}


#endif


