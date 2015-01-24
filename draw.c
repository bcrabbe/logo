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

typedef struct scaler {
    float scale[NUMBER_OF_DIMENSIONS];//pixcels per unit distance
    float offset[NUMBER_OF_DIMENSIONS];
} scaler;

void startSDL(display *d);
void checkSDLwinClosed(display *d);
void quitSDL(display * d);
scaler getScaler(display * d, pointArray * path);


pointArray * scale(pointArray * path, scaler s)
{
    pointArray * scaledPath = malloc(sizeof(pointArray));
    scaledPath->numberOfPoints = path->numberOfPoints;
    scaledPath->array = malloc(path->numberOfPoints*sizeof(point));
    for(int point = 0; point<path->numberOfPoints; ++point)
    {
        for(dimension dim = X; dim<=DIM_MAX; ++dim)
        {
            scaledPath->array[point].r[dim] = path->array[point].r[dim]*s.scale[dim] + s.offset[dim];
        }
        // printf("scale (%f,%f)->(%f,%f)\n",path->array[point].r[X],path->array[point].r[Y],
        //  scaledPath->array[point].r[X],scaledPath->array[point].r[Y]);

    }
    return scaledPath;
}

void draw(pointArray * path)
{
    display * d = malloc(sizeof(display));
    startSDL(d);
    scaler s = getScaler(d, path);
    //printf("sX = %f, sY = %f px/unit offset x = %f y = %f\n",s.scale[X],s.scale[Y],s.offset[X],s.offset[Y]);
    pointArray * scaledPath = scale(path, s);
    SDL_SetRenderDrawColor( d->renderer, 0x00, 0x00, 0x00, 0xFF );
    SDL_RenderClear(d->renderer);
    SDL_SetRenderDrawColor( d->renderer, 0xFF, 0xFF, 0xFF, 0xFF );
    for(int point = 0; point<path->numberOfPoints-1; ++point)
    {
        //SDL_SetRenderDrawColor( d->renderer, rand()%255, 0, rand()%255, 0xFF );
        //  printf("(%f,%f)->(%f,%f)\n",scaledPath->array[point].r[X],scaledPath->array[point].r[Y],
        //     scaledPath->array[point+1].r[X],scaledPath->array[point+1].r[Y]);
        SDL_RenderDrawLine(d->renderer,
                           scaledPath->array[point].r[X],scaledPath->array[point].r[Y],
                           scaledPath->array[point+1].r[X],scaledPath->array[point+1].r[Y]);

    }
    SDL_RenderPresent(d->renderer);
    while(!d->finished)
    {
        checkSDLwinClosed(d);
    }
    quitSDL(d);
}

    
scaler getScaler(display * d, pointArray * path)
{
    scaler s;
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
    //  printf("min(%f,%f) max(%f,%f)\n",rMin[X],rMin[Y],rMax[X],rMax[Y]);
    for(dimension dim = X; dim<=DIM_MAX; ++dim)
    {//want the total spans to take up 90% of the window:
        s.scale[dim] = (0.9*(float)d->winSize[dim])/(rMax[dim]-rMin[dim]);
        s.offset[dim] = d->winSize[dim]/2-((rMax[dim]+rMin[dim])/2);
    }
    return s;
}

                           
                           
void startSDL(display *d)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "\nUnable to initialize SDL:  %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
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
    if(d->win == NULL) {
        fprintf(stderr, "\nUnable to initialize SDL Window:  %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    
    d->renderer = SDL_CreateRenderer(d->win, -1, 0);
    if(d->renderer == NULL) {
        fprintf(stderr, "\nUnable to initialize SDL Renderer:  %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    
    d->event = malloc(sizeof(SDL_Event));
    
    SDL_SetRenderDrawColor(d->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    if(SDL_RenderClear(d->renderer)<0)
    {
        printf( "Could not renderClear in startSDL Error: %s\n", SDL_GetError() );
        exit(6);
    }
    SDL_RenderPresent(d->renderer);
}

/*  Call frequently to see if the wind has been closed
 then check d->finished to see if this is the case
 */
void checkSDLwinClosed(display *d)
{
    if(SDL_PollEvent(d->event))
    {
        if( d->event->type == SDL_QUIT ) {
            d->finished = 1;
        }
    }
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



