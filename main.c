//
//  main.c
//  logo
//
//  Created by ben on 18/01/2015.
//  Copyright (c) 2015 ben. All rights reserved.
//
//#define sprint(s) printf(#s " = ""%s""\n",s);
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "main.h"

char * readFile(const char * argv1);

//unit test functions
void unitTests();
void testReadFile();
void testStrDup();
void testStringsMatch();
void testFloatCompare();

#pragma mark Main
int main(int argc, const char * argv[])
{
    if(TESTING)
    {
        unitTests();
        return 1;
    }
    if(argc!=2)
    {
        fprintf(stderr, "ERROR: expected a .txt file path as 1st argument.\nExiting.\n");
        exit(0);
    }
    char * inputString = readFile(argv[1]);
    if(inputString==NULL) exit(1);
    
    symbolList * symList = parse(inputString);
    if(symList==NULL) return 0;
    
    pointArray * path = buildPath(symList);
    if(path==NULL) return 0;
    draw(path);
    return 1;
}


#pragma mark Input Functions
/*
 *  Takes argv[1] which should contain a .txt file path
    reads into a malloc'd null-terminated string which is returned.
 */
char * readFile(const char * argv1)
{
    if(VERBOSE) printf("readFile opening %s.\n",argv1);
    FILE * fp = NULL;
    fp = fopen(argv1, "r");
    if(!fp)
    {
        printError("could not open file.", __FILE__, __FUNCTION__, __LINE__);
        return NULL;
    }
    char * inputString = NULL;
    char fileChar = getc(fp);
    int length=0;
    while(fileChar!=EOF)
    {
        ++length;
        char * tmp = realloc( inputString, length);
        if(!tmp)
        {
            printError("realloc in readFile() failed.", __FILE__, __FUNCTION__, __LINE__);
            return NULL;
        }
        inputString = tmp;
        inputString[length-1]=fileChar;
        fileChar = getc(fp);
    }
    char * tmp = realloc( inputString, length);
    if(!tmp)
    {
        printError("realloc in readFile() failed.", __FILE__, __FUNCTION__, __LINE__);
        return NULL;
    }
    inputString = tmp;
    inputString[length]='\0';
    return inputString;
}





#pragma mark Utility Functions
/*
 Wrapper for printing errors to call just copy and paste :
 
printError("put your error string here",__FILE__, __FUNCTION__, __LINE__);
 
 */
int printError(const char * errorString, const char file[], const char function[], const int line)
{
    if(PRINT_ERRORS)
    {
        char * editableErrorString = strdup(errorString);
        char stringStart[MAX_ERROR_STRING_SIZE];
        sprintf(stringStart,"\n******Error in %s %s line %d.\n\n", file, function, line);
        strcat(stringStart,editableErrorString);
        printf("%s\n\n",stringStart);
        free(editableErrorString);
    }
    return 1;
}

/*
 *  Duplicates a string, mallocs the space.
 */
char *strdup(const char * source)
{
    size_t len = 1+strlen(source);//gets the size of s
    char * dest = malloc(len);//allocates a block big enough to hold s
    if(dest==NULL)
    {
        printError("malloc in strdup() failed.", __FILE__, __FUNCTION__, __LINE__);
        exit(0);
    }
    return memcpy(dest, source, len);//returns ptr to start of string
}

int stringsMatch(const char * string1, const char * string2)
{
    if(strcmp(string1,string2)==0) return 1;
    else return 0;
}

int floatCompare(float a, float b)
{
    float epsilon = 0.001;
    return fabs(a-b)<epsilon ? 1 : 0;
}

/* frees a symbolList allocated by the parser module
 */
void freeSymList(symbolList * symList)
{
    symbolNode * current = symList->start;
    while(current!=NULL)
    {
        symbolNode * toBeFreed = current;
        current = current->next;
        free(toBeFreed);
    }
    free(symList);
}

/* frees a path allocated by the path module
 */
void freePath(pointArray * path )
{
    free(path->array);
    free(path);
}
#pragma mark Unit Tests

void unitTests()
{
    printf("********************************************************************\n");
    printf("\n*                       UNIT TESTS                                 *\n\n");
    printf("********************************************************************\n\n");
    
    printf("\n\n\n********************************************************************\n");
    printf("\n*                       Testing main.c                             *\n\n");
    printf("********************************************************************\n\n");
    unitTests_main();
    
    printf("\n\n\n********************************************************************\n");
    printf("\n*                       Testing parser.c                           *\n\n");
    printf("********************************************************************\n\n");
    unitTests_parser();
    
    printf("\n\n\n********************************************************************\n");
    printf("\n*                       Testing path.c                             *\n\n");
    printf("********************************************************************\n\n");
    unitTests_path();
    
    printf("\n\n\n********************************************************************\n");
    printf("\n*                       Testing draw.c                             *\n\n");
    printf("********************************************************************\n\n");
    unitTests_draw();
    

}

void unitTests_main()
{
    sput_start_testing();
    sput_set_output_stream(NULL);

    sput_enter_suite("testReadFile()");
    sput_run_test(testReadFile);
    sput_leave_suite();
    
    sput_enter_suite("testStrDup()");
    sput_run_test(testStrDup);
    sput_leave_suite();
    
    sput_enter_suite("testStringsMatch()");
    sput_run_test(testStringsMatch);
    sput_leave_suite();
    
    sput_enter_suite("testFloatCompare()");
    sput_run_test(testFloatCompare);
    sput_leave_suite();
    
    
    sput_finish_testing();

}

void testReadFile()
{
    sput_fail_unless(readFile("thisIsNotAFile.txt")==NULL, "trying to open a file that doesnt exist should print error and return NULL");
    char * returnedString = readFile("readFileTest.txt");
    FILE * fp = NULL;
    fp = fopen("readFileTest.txt", "r");
    if(!fp)
    {
        printError("could not open test file.", __FILE__, __FUNCTION__, __LINE__);
    }
    int charFromTheReturnedStringPostion=0;
    int returnedStringLength = (int)strlen(returnedString);
    char charFromFile = getc(fp);
    char charFromTheReturnedString = returnedString[charFromTheReturnedStringPostion];
    while(charFromFile!=EOF && charFromTheReturnedStringPostion<=returnedStringLength)
    {
        char str[500];
        sprintf(str, "Checks each char of the file with that of the returned string. Now checking %c == %c.(file == returned)",charFromFile,charFromTheReturnedString);
        sput_fail_unless(charFromFile==charFromTheReturnedString,str);
        charFromFile = getc(fp);
        charFromTheReturnedString = returnedString[++charFromTheReturnedStringPostion];
    }
}

void testStrDup()
{
    char testString[15] = "testString";
    char * duplicatedString = strdup(testString);
    char str[500];
    sprintf(str, "Checks that a duplicated string matches the original. Checking %s==%s",testString,duplicatedString);
    sput_fail_unless(strcmp(testString,duplicatedString)==0, str);
}

void testStringsMatch()
{
    char testString[15] = "testString";
    char testString2[15] = "testString";
    char notTestString[15] = "notTestString";
    char str[500];
    sprintf(str, "Checks that stringsMatch returns 1 for a match. Checking with %s==%s.",testString,testString2);
    sput_fail_unless(stringsMatch(testString, testString2)==1, str);
    sprintf(str, "Checks that stringsMatch returns 0 for no match. Checking with %s==%s.",testString,notTestString);
    sput_fail_unless(stringsMatch(testString, notTestString)==0, str);
}

void testFloatCompare()
{
    sput_fail_unless(floatCompare(74.23, 74.23)==1, "Check with equal floats, should return 1.");
    sput_fail_unless(floatCompare(74.23, 2)==0, "Check with nonequal floats, should return 0.");

}

























