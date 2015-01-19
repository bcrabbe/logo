//
//  main.h
//  logo
//
//  Created by ben on 18/01/2015.
//  Copyright (c) 2015 ben. All rights reserved.
//

#ifndef logo_main_h
#define logo_main_h

#define VERBOSE 1
#define PRINT_ERRORS 1 //turn on/off stderr error messages.
#define MAX_ERROR_STRING_SIZE 400

//generic functions
int printError(const char * errorString, const char file[], const char function[], const int line);
char *strdup(const char * s);
int stringsMatch(const char * string1, const char * string2);
int stringToInt(const char * string);

#endif
