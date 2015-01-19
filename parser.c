//
//  parser.c
//  logo
//
//  Created by ben on 18/01/2015.
//  Copyright (c) 2015 ben. All rights reserved.
//
#include "parser.h"
#include "interpreter.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef enum symbol {
    symMAIN, symINSTRCTLST, symINSTRUCTION, symFD, symLT, symRT, symDO, symVAR,
    symVARNUM, symSET, symPOLISH, symOP
} symbol;

typedef struct symNode {
    symbol sym;
    int value;
    struct symNode * next;
} symbolNode;

typedef struct symbolList {
    symbolNode * start;
    symbolNode * end;
    unsigned long length;
} symbolList;

typedef struct parser {
    char ** progArray;
    int numberOfTokens, atToken;
    int * varValues;
    symbolList * symList;
    char ** errorList;
    int numberOfErrors;
} parser;

//symbol parsers
int parseMAIN(parser * p);
int parseINSTRCTLST(parser * p);
int parseINSTRUCTION(parser * p);
int parseFD(parser * p);
int parseRT(parser * p);
int parseLT(parser * p);
int parseVARNUM(parser * p);
char parseVAR(parser * p);


//parserStruct functions
parser * initParser();
parser * getParser(parser * store);
void freeParser();
int incrementAtToken(parser * p);
int getVarValue(char var);
void addSymToList(parser * p, symbol sym, int value);
void printSymList(parser * p);
void printSymNode(symbolNode * node);
//error list functions
int addErrorToList(char * errorString);
void displayErrors(parser * p);
void syntaxError(const char * errorString);
void finishedBeforeWeExpectedError();

//token array functions
char ** tokenise(const char * inputString, int * numberOfTokensPtr, const char * delimiter);
void testTokenArray(char ** tokenArray, int numberOfTokens);
void freeTokenArray(char **tokenArray,int numberOfTokens);




int parse(char * inputString)
{
    unsigned long inputStringLength = strlen(inputString);
    if(!inputStringLength)
    {
        printError("Parser recieved a empty string. Exiting.",__FILE__,__FUNCTION__,__LINE__);
        return 0;
    }
    parser * p = initParser();
   
    p->progArray = tokenise(inputString, &p->numberOfTokens, " \n\r");

    if(VERBOSE)
    {
        testTokenArray(p->progArray, p->numberOfTokens);
    }
    if(parseMAIN(p))
    {
        printf("VALID.\n");
        printSymList(p);
    }
    else
    {
        printSymList(p);
        displayErrors(p);
    }
    freeParser();
    return 1;
}

#pragma mark symbol parsers

int parseMAIN(parser * p)
{
    if(stringsMatch(p->progArray[p->atToken], "{"))
    {
        if(incrementAtToken(p))
        {
            return parseINSTRCTLST(p);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        syntaxError("Expected to begin program with ""{"".");
        return 0;
    }
}

int parseINSTRCTLST(parser * p)
{
    if(stringsMatch(p->progArray[p->atToken], "}"))
    {
        return 1;
    }
    else if(parseINSTRUCTION(p))
    {
        return parseINSTRCTLST(p);
    }
    else
    {
        syntaxError("Expected to read an instruction or a ""}"".");
        return 0;
    }
}

int parseINSTRUCTION(parser * p)
{
    if(parseFD(p))
    {
        return 1;
    }
    else if(parseLT(p))
    {
        return 1;
    }
    else if(parseRT(p))
    {
        return 1;
    }
    else
    {
        //error handled in parseINSTRCTLST()
        return 0;
    }
}

int parseFD(parser * p)
{
    if(stringsMatch(p->progArray[p->atToken], "FD"))
    {
        if(incrementAtToken(p))
        {
            int value = parseVARNUM(p);
            if(!value)
            {
                syntaxError("FD 0 is a redundant instruction.");
                return 0;
            }
            addSymToList(p, symFD, value);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

int parseLT(parser * p)
{
    if(stringsMatch(p->progArray[p->atToken], "LT"))
    {
        if(incrementAtToken(p))
        {
            int value = parseVARNUM(p);
            if(!value)
            {
                syntaxError("LT 0 is a redundant instruction.");
                return 0;
            }
            addSymToList(p, symLT, value);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

int parseRT(parser * p)
{
    if(stringsMatch(p->progArray[p->atToken], "RT"))
    {
        if(incrementAtToken(p))
        {
            int value = parseVARNUM(p);
            if(!value)
            {
                syntaxError("RT 0 is a redundant instruction.");
                return 0;
            }
            addSymToList(p, symRT, value);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

int parseVARNUM(parser * p)
{
    int value;
    if(strlen(p->progArray[p->atToken])<1)
    {
        printError("parseVARNUM recieved a empty string. Exiting.",__FILE__,__FUNCTION__,__LINE__);
        exit(1);
    }
    if(isdigit(p->progArray[p->atToken][0]))//if its a digit...
    {
        value = stringToInt(p->progArray[p->atToken]);
        incrementAtToken(p);
        return value;
    }
    else if(isupper(p->progArray[p->atToken][0]))//if its a upper case letter...
    {
        char var = parseVAR(p);
        if(var)
        {
            value = getVarValue(var);
            incrementAtToken(p);
            return value;
        }
        else
        {
            //error passed from parseVAR()
            return 0;
        }
    }
    else
    {
        syntaxError("Expected to read a number or a <VAR>.");
        return 0;
    }
}

char parseVAR(parser * p)
{
    if(strlen(p->progArray[p->atToken])<1)
    {
        printError("parseVAR recieved a empty string. Exiting.",__FILE__,__FUNCTION__,__LINE__);
        exit(1);
    }
    if(!isupper(p->progArray[p->atToken][0]))
    {
        syntaxError("Expected a variable. Variables should be any uppercase letter A-Z.");
        return '\0';
    }
    else
    {
        return p->progArray[p->atToken][0];
    }
}
       
int stringToInt(const char * string)
{
    int converted=0;
    size_t length = strlen(string);
    if(length<1)
    {
        printError("stringToInt was called with a empty string, this will segfault, so it has just returned 0 \n",
                   __FILE__,__FUNCTION__,__LINE__);
        return 0;
    }
    int sign=1;
    if(string[0]=='-')
    {
        sign = -1;
    }
    for(int i = sign==-1 ? 1 : 0; i<length; ++i)
    {
        if(!isdigit(string[i]))
        {
            syntaxError("Expected to read a number.");
        }
        converted += (unsigned int)(string[i]-'0') * pow( 10, (length-i-1));
    }
    return sign*converted;
}



#pragma mark parser obj functions

parser * initParser()
{
    parser * p = malloc(sizeof(parser));
    if(!p)
    {
        printError("malloc failed exiting.",__FILE__,__FUNCTION__,__LINE__);
        exit(1);
    }
    p->progArray = NULL;
    p->numberOfTokens = 0;
    p->atToken=0;
    p->varValues = calloc('Z'+1, sizeof(int));//over sized array, variables can be indexed by their ascii values
    p->symList = malloc(sizeof(symbolList));
    if(!p->symList)
    {
        printError("realloc failed exiting.",__FILE__,__FUNCTION__,__LINE__);
        exit(1);
    }
    p->symList->length=0;
    p->symList->start=NULL;
    p->symList->end=NULL;
    p->errorList=NULL;
    p->numberOfErrors=0;
    getParser(p);//stores p in static variable
    return p;
}

parser * getParser(parser * store)
{
    static parser * stored = NULL;
    if(store)
    {
        stored = store;
    }
    return stored;
}

void freeParser()
{
    parser * p = getParser(NULL);
    freeTokenArray(p->progArray, p->numberOfTokens);
    //free error list:
    for(int i=0; i<p->numberOfErrors; ++i)
    {
        free(p->errorList[i]);
    }
    free(p->errorList);
    //need a symlistFree aswell
}

int incrementAtToken(parser * p)
{
    if(p->atToken < p->numberOfTokens-1)
    {
        printf("moved to token %d [%s]\n",p->atToken, p->progArray[p->atToken]);
        return ++p->atToken;
    }
    else
    {
        finishedBeforeWeExpectedError();
        displayErrors(p);
        exit(1);
    }
}

int getVarValue(char var)
{
    parser * p = getParser(NULL);
    return p->varValues[(int)var];
}
       
#pragma mark symbol list functions
void addSymToList(parser * p, symbol sym, int value)
{
    symbolNode * firstNode = malloc(sizeof(symbolNode));
    if(firstNode==NULL)
    {
        printError("malloc failed.", __FILE__, __FUNCTION__, __LINE__);
        exit(1);
    }
    firstNode->sym = sym;
    firstNode->value = value;
    firstNode->next = NULL;
    if(p->symList->length==0)
    {//if first node:
        p->symList->start = firstNode;
        p->symList->end = firstNode;
        
    }
    else
    {
        p->symList->end->next = firstNode;
        p->symList->end = firstNode;
    }
    ++p->symList->length;
}

void printSymList(parser * p)
{
    symbolNode * current = p->symList->start;
    printf("{\n");
    while(current)
    {
        printSymNode(current);
        current=current->next;
    }
    printf("}\n");
}

void printSymNode(symbolNode * node)
{
    switch (node->sym)
    {
        case symFD:
        {
            printf("    FD   ");
            break;
        }
        case symRT:
        {
            printf("    RT   ");
            break;
        }
        case symLT:
        {
            printf("    LT   ");
            break;
        }
        case symSET:
        {
            printf("    SET   ");
            break;
        }
        default:
        {
            printError("symList contained a unexpected symbol", __FILE__, __FUNCTION__, __LINE__);
            exit(1);
        }
    }
    printf("%d\n",node->value);
}
#pragma mark error messaging functions

int addErrorToList(char * errorString)
{
    parser * p = getParser(NULL);
    ++p->numberOfErrors;
    char ** tmp = realloc(p->errorList,p->numberOfErrors*sizeof(char*));
    if(!tmp)
    {
        printError("realloc failed exiting.",__FILE__,__FUNCTION__,__LINE__);
        exit(1);
    }
    p->errorList = tmp;
    p->errorList[p->numberOfErrors-1] = strdup(errorString);
    return 1;
}

void displayErrors(parser * p)
{
    for(int i=0; i<p->numberOfErrors; ++i)
    {
        printf("%s \n \n",p->errorList[i]);
    }
}
void finishedBeforeWeExpectedError()
{
    addErrorToList("ERROR: expected program to end with a ""}""\n");
}

void syntaxError(const char * errorString)
{
    parser * p = getParser(NULL);
    char * editableErrorString = strdup(errorString);
    char stringStart[MAX_ERROR_STRING_SIZE];
    if(p->atToken==p->numberOfTokens-1) return;//stop a segfault if last token
    sprintf(stringStart,"ERROR: invalid syntax at token %d [%s] previous token %d [%s]. \n\n",
            p->atToken, p->progArray[p->atToken],
            p->atToken-1,  p->progArray[p->atToken-1]);
    strcat(stringStart,editableErrorString);
    addErrorToList(stringStart);
    free(editableErrorString);
}
void readError(const char * whatWeRead, symbol whatWeExpect)
{
    
}

char * whatDoWeExpectString(symbol context)
{
    char * outputString = malloc(MAX_ERROR_STRING_SIZE);
    switch(context)
    {
        case symMAIN:
        {
            outputString = " <MAIN>        ::= ""{"" <INSTRCTLST>";
            return outputString;
        }
        case symINSTRCTLST:
        {
            outputString = " <INSTRCTLST>  ::= <INSTRUCTION><INSTRCTLST> | \n ""}"" ";
            return outputString;
        }
        case symINSTRUCTION:
        {
            outputString = " <INSTRUCTION> ::= <FD> | \n <LT> | \n <RT> | \n <DO> | \n <SET>";
            return outputString;
        }
        case symFD:
        {
            outputString = " <FD>          ::= ""FD"" <VARNUM>";
            return outputString;
        }
        case symLT:
        {
            outputString = " <LT>          ::= ""LT"" <VARNUM>";
            return outputString;
        }
        case symRT:
        {
            outputString = " <RT>          ::= ""RT"" <VARNUM>";
            return outputString;
        }
        case symDO:
        {
            outputString = " <DO>          ::= ""DO"" <VAR> ""FROM"" <VARNUM> ""TO"" \n <VARNUM> ""{"" <INSTRCTLST>";
            return outputString;
        }
        case symVAR:
        {
            outputString = " <VAR>         ::= [A-Z]";
            return outputString;
        }
        case symVARNUM:
        {
            outputString = " <VARNUM>      ::= number | \n <VAR>";
            return outputString;
        }
        case symSET:
        {
            outputString = " <SET>         ::= ""SET"" <VAR> "":="" <POLISH>";
            return outputString;
        }
        case symPOLISH:
        {
            outputString = " <POLISH>      ::= <OP> <POLISH> | \n <VARNUM> <POLISH> | \n "";"" ";
            return outputString;
        }
        case symOP:
        {
            outputString = " <OP>          ::= ""+"" | \n  ""-"" | \n ""*"" | \n ""/"" ";
            return outputString;
        }
    }
}
#pragma mark tokenArray
/*
 *  Takes the input string and breaks into separate words (where there is a
    space and new string starts) each of these words is stored in the
    commandArray which is an array of strings.
    @returns a string array [0 to numberOfTokens-1]
 */
char ** tokenise(const char * inputString, int * numberOfTokensPtr, const char * delimiter)
{
    static int calls=1;
    char    *stringToken,                       //holds the chunks on the input string as we break it up
            *inputStringDuplicate = strdup(inputString),//duplicate input string for editting
            **tokenArray = NULL;              //this will be an array to hold each of the chunk strings
    int     numberOfTokens=0;
    
    //using http://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
    stringToken = strtok(inputStringDuplicate, delimiter); // gets the first chunk (up to the first space)
    
    // walk through rest of string
    while( stringToken != NULL )
    {
        if(strcmp(stringToken," ")==0 || strcmp(stringToken,",")==0)
        {
            //do nothing
        }
        else
        {
            ++numberOfTokens;
            char ** tmp = (char **)realloc(tokenArray,numberOfTokens*sizeof(char*));//array of strings
            if(!tmp)
            {
                printError("realloc failed, exiting.",__FILE__,__FUNCTION__,__LINE__);
                exit(1);
            }
            tokenArray = tmp;
            tokenArray[numberOfTokens-1]=(char *)malloc((size_t)(strlen(stringToken)*sizeof(char)+1));
            if(!tokenArray[numberOfTokens-1])
            {
                printError("malloc failed, exiting.",__FILE__,__FUNCTION__,__LINE__);
                exit(1);
            }
            strcpy(tokenArray[numberOfTokens-1],stringToken);
        }
        stringToken = strtok(NULL, delimiter);
    }
    free(inputStringDuplicate);//frees the malloc made in strdup()
                               //$(numberOfChunks) strings now stored in the commandArray
    *numberOfTokensPtr=numberOfTokens;
    ++calls;
    return tokenArray;
}

/*
 *  frees the memory allocated to a tokenArray in tokenise func
 */
void freeTokenArray(char **tokenArray,int numberOfTokens)
{
    for(int i=0; i<numberOfTokens; ++i)
    {
        free(tokenArray[i]);
    }
    free(tokenArray);
}



#pragma developement tests
/*
 *  Test function for developement. Prints contents of a tokenArray
 */
void testTokenArray(char ** tokenArray, int numberOfTokens)
{
    printf("testTokenArray:\n");
    for(int i=0; i<numberOfTokens; ++i)
    {
        printf("[%s]   ",tokenArray[i]);
    }
    printf("\n");

}