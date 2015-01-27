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
    NONE = 0
} sdlKey;

typedef struct scaler {
    float scale[NUMBER_OF_DIMENSIONS];//pixcels per unit distance
    float offset[NUMBER_OF_DIMENSIONS];
    float centreOfPath[NUMBER_OF_DIMENSIONS];
    float centreOfWindow[NUMBER_OF_DIMENSIONS];
    float spanOfPath[NUMBER_OF_DIMENSIONS];
} scaler;

#pragma mark prototypes
scaler * getScaler(display * d, pointArray * path);
pointArray * scale(pointArray * path, scaler * s);
void renderPath(display * d, pointArray * path);
sdlKey getSdlKeyPresses(display * d);
void zoom(scaler * s, int zoomIn);

display * startSDL();
int checkSDLwinClosed(display *d);
void quitSDL(display * d);

void printPath(pointArray * path, char * name);

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
    if(VERBOSE) printPath(path, "orininal path:");

    display * d = startSDL();
    
    scaler * s = getScaler(d, path);
    pointArray * scaledPath = scale(path, s);
    if(VERBOSE) printPath(scaledPath, "orininal path:");
    printf("Press up and down arrows to zoom in/out.\n");
    while(!d->finished)
    {
        renderPath(d, scaledPath);
        sdlKey key = getSdlKeyPresses(d);
        if(key==UP) zoom(s,1);//zoomin
        if(key==DOWN) zoom(s,0);//zoomout
        freePath(scaledPath);
        scaledPath = scale(path, s);
        if(VERBOSE) printPath(scaledPath, "orininal path:");
        checkSDLwinClosed(d);
        SDL_Delay(1e3/FPS);
    }
    free(s);//free scaler
    freePath(path);//free unscaled path
    freePath(scaledPath);
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
        for(dimension dim = X; dim<=DIM_MAX; ++dim)
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
    for(dimension dim = X; dim<=DIM_MAX; ++dim)
    {
        s->spanOfPath[dim] = rMax[dim]-rMin[dim];//FD units
        s->centreOfPath[dim] = (rMax[dim]-rMin[dim])/2;//FD units
        s->centreOfWindow[dim] = (float)d->winSize[dim]/2;//px
        s->scale[dim] = 0.3*( (float)d->winSize[dim]  / s->spanOfPath[dim]);//px per fd unit
        s->offset[dim] = -s->scale[dim]*s->centreOfPath[dim] + s->centreOfWindow[dim] + s->scale[dim]*s->spanOfPath[dim]/2;
    }
    if(!STRETCH_TO_FIT_WINDOW)
    {//if we dont want to alter ratio then use the smallest scale for both.
        if( s->scale[X] < s->scale[Y] ) s->scale[Y] = s->scale[X];
        if( s->scale[X] > s->scale[Y] ) s->scale[X] = s->scale[Y];
    }
    if(VERBOSE)
    {
        printf("scale: %f,%f\n", s->scale[X], s->scale[Y]);
        printf("offset: %f,%f\n", s->offset[X], s->offset[Y]);
    }
    return s;
}
/**
 Takes path and transforms each point on to the display coordinates.
 returns a new, malloc'd, path. This and the old one should be free'd.
 */
pointArray * scale(pointArray * path, scaler * s)
{
    pointArray * scaledPath = malloc(sizeof(pointArray));
    if(scaledPath==NULL)
    {
        printError("pointArray * scaledPath = malloc(sizeof(pointArray)) failed exiting.",
                   __FILE__,__FUNCTION__,__LINE__);
        return NULL;
    }
    scaledPath->numberOfPoints = path->numberOfPoints;
    scaledPath->array = malloc(path->numberOfPoints*sizeof(point));
    if(scaledPath->array==NULL)
    {
        printError("scaledPath->array = malloc(path->numberOfPoints*sizeof(point)) failed exiting.",
                   __FILE__,__FUNCTION__,__LINE__);
        return NULL;
    }
    for(int point = 0; point<path->numberOfPoints; ++point)
    {
        for(dimension dim = X; dim<=DIM_MAX; ++dim)
        {
            scaledPath->array[point].r[dim] = path->array[point].r[dim]*s->scale[dim] + s->offset[dim];
        }
    }
    return scaledPath;
}
/* if zoomin > 1 increases the scale by ZOOM_SENSITIVITY of the current scale
    else it is decreased by same
 */
void zoom(scaler * s, int zoomIn)
{
    for(dimension dim = X; dim<=DIM_MAX; ++dim)
    {
        if(zoomIn) s->scale[dim] +=  s->scale[dim]*ZOOM_SENSITIVITY;
        else  s->scale[dim] -=  s->scale[dim]*ZOOM_SENSITIVITY;
    }
}

#pragma mark SDL functions
/* conect the points in the path with lines in SDL window
 */
void renderPath(display * d, pointArray * path)
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

void printPath(pointArray * path, char * name)
{
    char nameString[MAX_ERROR_STRING_SIZE] = "\n\nPrinting ";
    sprintf(nameString, "\n\nShowing %s .\n\n",name);
    for(int p=0; p<path->numberOfPoints; ++p)
    {
        printf("( %f, %f)\n",path->array[p].r[X], path->array[p].r[Y]);
    }
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
        }
        checkSDLwinClosed(d);
        SDL_Delay(1e3/FPS);
        if(d->finished) return NONE;
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
    for(dimension dim = X; dim<=Y; ++dim)
    {
        sput_fail_unless(floatCompare(s->scale[dim], 0.8*( (float)d->winSize[dim]  / s->spanOfPath[dim])),
                         "the x scaler should be set to this value.");
        sput_fail_unless(floatCompare(s->offset[dim],-s->scale[dim]*s->centreOfPath[dim] + s->centreOfWindow[dim] +
                                      s->scale[dim]*s->spanOfPath[dim]/2),
                         "the x offset should be set to this value.");
    }

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






