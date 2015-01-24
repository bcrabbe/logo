//
//  parser.c
//  logo
//
//  Created by ben on 18/01/2015.
//  Copyright (c) 2015 ben. All rights reserved.
//
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef enum operator {
    opPlus = '+',
    opMinus = '-',
    opMultiply = '*',
    opDivide = '/',
} operator;


typedef struct stack {
    int itemsInStack;
    float * array;
} stack;

typedef struct parser {
    char ** progArray;
    int numberOfTokens, atToken;
    float * varValues;
    symbolList * symList;
    stack * polishCalcStack;
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
int parseVARNUM(parser * p, float * result);
char parseVAR(parser * p);
int parseDO(parser * p);
int parseSET(parser * p);
int parseOP(parser * p, operator * op);
int parsePOLISH(parser * p, float * result);


//parserStruct functions

parser * initParser();
parser * getParser(parser * store);
void freeParser(parser * p);
int incrementAtToken(parser * p);

//parserStruct -> varValues funcs
float getVarValue(parser *p, char var);
int setVarValue(parser *p, char var, float newValue);

//parserStruct -> symbol list funcs
void addSymToList(parser * p, symbol sym, float value);
void printSymList(parser * p);
void printSymNode(symbolNode * node);

//parserStruct -> polish calc functions
void pushValue(parser *p, float value);
int popToOperator(parser * p, operator op);
void clearCalculatorStack(parser * p);

//parserStruct -> error list functions
int addErrorToList(parser * p,char * errorString);
void displayErrors(parser * p);
void syntaxError(parser * p,const char * errorString);
void * whatDoWeExpectString(parser * p,symbol context);
void finishedBeforeWeExpectedError(parser * p);

//parserStruct -> token array functions
char ** tokenise(const char * inputString, int * numberOfTokensPtr, const char * delimiter);
void testTokenArray(char ** tokenArray, int numberOfTokens);
void freeTokenArray(char **tokenArray,int numberOfTokens);




symbolList * parse(char * inputString)
{
    unsigned long inputStringLength = strlen(inputString);
    if(!inputStringLength)
    {
        printError("Parser recieved a empty string. Exiting.",__FILE__,__FUNCTION__,__LINE__);
        return 0;
    }
    parser * p = initParser();
   
    p->progArray = tokenise(inputString, &p->numberOfTokens, " \n\r\t");

    if(VERBOSE)
    {
        testTokenArray(p->progArray, p->numberOfTokens);
    }
    if(parseMAIN(p))
    {
        printf("\n\nVALID.\n");
        printSymList(p);
    }
    else
    {
        printSymList(p);
        displayErrors(p);
    }
    freeParser(p);
    return p->symList;
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
        syntaxError(p,"Expected to begin program with ""{"".");
        return 0;
    }
}

int parseINSTRCTLST(parser * p)
{
    if(stringsMatch(p->progArray[p->atToken], "}"))
    {
        // if(!incrementAtToken(p)) return ;
        return 1;
    }
    else if(parseINSTRUCTION(p))
    {
        return parseINSTRCTLST(p);
    }
    else
    {
        syntaxError(p,"Expected to read an instruction or a ""}"".");
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
    else if(parseDO(p))
    {
        return 1;
    }
    else if(parseSET(p))
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
            float value;
            parseVARNUM(p,&value);
            if(value==0)
            {
                syntaxError(p,"FD 0 is a redundant instruction.");
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
            float value;
            parseVARNUM(p,&value);
            if(value==0)
            {
                syntaxError(p,"LT 0 is a redundant instruction.");
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
    if(!stringsMatch(p->progArray[p->atToken], "RT")) return 0;
    else
    {
        if(incrementAtToken(p)==0) return 0;
        else
        {
            float value;
            parseVARNUM(p,&value);
            if(value==0)
            {
                syntaxError(p,"RT 0 is a redundant instruction.");
                return 0;
            }
            addSymToList(p, symRT, value);
            return 1;
        }
    }
}

int parseVARNUM(parser * p, float * result)
{
    float value;
    if(strlen(p->progArray[p->atToken])<1)
    {
        printError("parseVARNUM recieved a empty string. Exiting.",__FILE__,__FUNCTION__,__LINE__);
        exit(1);
    }
    if(isdigit(p->progArray[p->atToken][0]))//if its a digit...
    {
       /* if( !stringToInt(p->progArray[p->atToken],&value) )
        {
            syntaxError(p,"parseVARNUM expected to read a number.");
            return 0;//if 0 means we read a non digit in the string so fails.
        }*/
        value = atof(p->progArray[p->atToken]);
        if(incrementAtToken(p)==0) return 0;
        *result = value;
        return 1;
    }
    else if(isupper(p->progArray[p->atToken][0]))//if its a upper case letter...
    {
        char var = parseVAR(p);
        if(var=='\0') return 0; //could not read a valid VAR. error sent in func
        value = getVarValue(p,var);//retrieves the value of the specified variable
        *result = value;
        return 1;
    }
    else
    {
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
        //syntaxError(p,"Expected a variable. Variables should be any uppercase letter A-Z.");
        return '\0';
    }
    else
    {//increment token and if successful return the char
        return incrementAtToken(p) ? p->progArray[p->atToken-1][0] : 0;
    }
}


/*
 * <DO> ::=  "DO" <VAR> "FROM" <VARNUM> "TO"  <VARNUM> "{" <INSTRCTLST>
 *
 */
int parseDO(parser * p)
{
    //"DO"
    if(!stringsMatch(p->progArray[p->atToken], "DO")) return 0;
    else
    {
        if(incrementAtToken(p)==0) return 0;
        
        //<VAR>
        char var = parseVAR(p);
        if(var=='\0')
        {
            whatDoWeExpectString(p,symDO);
            syntaxError(p,"Could not read ""VAR"".");
            return 0;
        }
        
        // "FROM"
        if(!stringsMatch(p->progArray[p->atToken], "FROM"))
        {
            whatDoWeExpectString(p,symDO);
            syntaxError(p,"Could not read ""FROM"".");
            return 0;
        }
        if(incrementAtToken(p)==0) return 0;
 
        // <VARNUM> start
        float fromVarNum;
        if(!parseVARNUM(p,&fromVarNum))
        {
            whatDoWeExpectString(p,symDO);
            syntaxError(p,"Could not read 1st <VARNUM>.");
            return 0;
        }
        
        // "TO"
        if(!stringsMatch(p->progArray[p->atToken], "TO"))
        {
            whatDoWeExpectString(p,symDO);
            syntaxError(p,"Could not read ""TO"".");
            return 0;
        }
        if(incrementAtToken(p)==0) return 0;
        
        // <VARNUM> end
        float toVarNum;
        if(!parseVARNUM(p,&toVarNum))
        {
            whatDoWeExpectString(p,symDO);
            syntaxError(p,"Could not read 2nd <VARNUM>.");
            return 0;
        }
        
        // get "{"
        if(!stringsMatch(p->progArray[p->atToken], "{"))
        {
            whatDoWeExpectString(p,symDO);
            syntaxError(p,"Could not read ""{"".");
            return 0;
        }
        if(incrementAtToken(p)==0) return 0;
        //now loop:
        int loopStartToken = p->atToken;
        for(int iter = fromVarNum; iter<=toVarNum; ++iter)
        {
            p->varValues[(int)var]=iter;
            if(parseINSTRCTLST(p)==0) return 0;
            p->atToken = loopStartToken;
        }
        return 1;
    }
}

/* 
 * <SET> ::= "SET" <VAR> ":=" <POLISH>
 */
int parseSET(parser * p)
{
    // "SET"
    if(!stringsMatch(p->progArray[p->atToken], "SET")) return 0;
    else
    {
        if(incrementAtToken(p)==0) return 0;

        char var = parseVAR(p);
        if(var=='\0')
        {
            whatDoWeExpectString(p,symDO);
            syntaxError(p,"Could not read ""VAR"".");
            return 0;
        }
        
        // ":="
        if(!stringsMatch(p->progArray[p->atToken], ":="))
        {
            whatDoWeExpectString(p,symDO);
            syntaxError(p,"Could not read "":="".");
            return 0;
        }
        if(incrementAtToken(p)==0) return 0;
        
        clearCalculatorStack(p);
        float setToValue;
        if(parsePOLISH(p, &setToValue)==0) return 0;
        else setVarValue(p, var, setToValue);
        
        return 1;
    }
    
}


/*
 * <POLISH> ::= <OP> <POLISH> | <VARNUM> <POLISH> | ";"
 */
int parsePOLISH(parser * p, float * result)
{
    if(stringsMatch(p->progArray[p->atToken], ";"))
    {
        if(p->polishCalcStack->itemsInStack!=1)
        {
            syntaxError(p,"polish expression incorrectly formatted.");
            return 0;
        }
        if(!incrementAtToken(p)) return 0;
        *result = p->polishCalcStack->array[0];
        return 1;
    }
    // <VARNUM> end
    float varnumValue;
    operator op;
    if(parseVARNUM(p,&varnumValue))
    {
        pushValue(p, varnumValue);
        return parsePOLISH(p, result);
    }
    if(parseOP(p,&op))
    {
        popToOperator(p, op);
        return parsePOLISH(p, result);
    }
    whatDoWeExpectString(p, symPOLISH);
    syntaxError(p,"could not read VARNUM or OP.");
    return 0;
}



int parseOP(parser * p, operator * op)
{
    if(stringsMatch(p->progArray[p->atToken], "+"))
    {
        *op = opPlus;
    }
    else if(stringsMatch(p->progArray[p->atToken], "-"))
    {
        *op = opMinus;
    }
    else if(stringsMatch(p->progArray[p->atToken], "*"))
    {
        *op = opMultiply;
    }
    else if(stringsMatch(p->progArray[p->atToken], "/"))
    {
        *op = opDivide;
    }
    if(incrementAtToken(p)==0) return 0;
    else return 1;
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
    
    p->polishCalcStack = malloc(sizeof(stack));
    p->polishCalcStack->array = NULL;
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
    free(p->polishCalcStack->array);
    free(p->polishCalcStack);
    //do not want to free symlist as this is returned.
}

int incrementAtToken(parser * p)
{
    if(p->atToken < p->numberOfTokens-1)
    {
        ++p->atToken;
        if(VERBOSE) printf("moved to token %d [%s]\n",p->atToken, p->progArray[p->atToken]);
        return 1;
    }
    else
    {
        finishedBeforeWeExpectedError(p);
        displayErrors(p);
        exit(1);
    }
}

int setVarValue(parser *p, char var, float newValue)
{
    p->varValues[(int)var]=newValue;
    return p->varValues[(int)var];
}

float getVarValue(parser *p, char var)
{
    return p->varValues[(int)var];
}
       
#pragma mark symbol list functions
void addSymToList(parser * p, symbol sym, float value)
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
    printf("%f\n",node->value);
}

#pragma mark polish calculator functions

void pushValue(parser *p, float value)
{
    ++p->polishCalcStack->itemsInStack;
    
    float * tmp = realloc(p->polishCalcStack->array, p->polishCalcStack->itemsInStack*sizeof(int));
    if(!tmp)
    {
        printError("realloc failed. Exiting.", __FILE__, __FUNCTION__, __LINE__);
        exit(1);
    }
    p->polishCalcStack->array = tmp;
    p->polishCalcStack->array[p->polishCalcStack->itemsInStack-1]=value;
}

int popToOperator(parser * p, operator op)
{
    if(p->polishCalcStack->itemsInStack<2)
    {
        printError("popToOperator called when there is less than two items on stack. returning 0.", __FILE__, __FUNCTION__, __LINE__);
        return 0;
    }
    float   rhs = p->polishCalcStack->array[p->polishCalcStack->itemsInStack-1],
            lhs = p->polishCalcStack->array[p->polishCalcStack->itemsInStack-2],
            result;
    switch (op)
    {
        case opPlus:
        {
            result = lhs + rhs;
            break;
        }
        case opMinus:
        {
            result = lhs - rhs;
            break;
        }
        case opMultiply:
        {
            result = lhs * rhs;
            break;
        }
        case opDivide:
        {
            result = lhs / rhs;
            break;
        }
    }
    p->polishCalcStack->itemsInStack -= 2;
    
    float * tmp = realloc(p->polishCalcStack->array, p->polishCalcStack->itemsInStack*sizeof(int));
    if(!tmp)
    {
        printError("realloc failed. Exiting.", __FILE__, __FUNCTION__, __LINE__);
        exit(1);
    }
    p->polishCalcStack->array = tmp;
    pushValue(p, result);
    return 1;
}

void clearCalculatorStack(parser * p)
{
    if(p->polishCalcStack->array) free(p->polishCalcStack->array);
    p->polishCalcStack->array=NULL;
    p->polishCalcStack->itemsInStack=0;
}


#pragma mark error messaging functions
int addErrorToList(parser * p, char * errorString)
{
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
        printf("\n%s \n",p->errorList[i]);
    }
}

void finishedBeforeWeExpectedError(parser * p)
{
    addErrorToList(p,"ERROR: expected program to end with a ""}""\n");
}

void syntaxError(parser * p, const char * errorString)
{
    char * editableErrorString = strdup(errorString);
    char stringStart[MAX_ERROR_STRING_SIZE];
    if(p->atToken==p->numberOfTokens-1) return;//stop a segfault if last token
    sprintf(stringStart,"ERROR: invalid syntax at token %d ""%s"" previous token %d ""%s"". \n\n",
            p->atToken, p->progArray[p->atToken],
            p->atToken-1,  p->progArray[p->atToken-1]);
    strcat(stringStart,editableErrorString);
    addErrorToList(p,stringStart);
    free(editableErrorString);
}

void readError(const char * whatWeRead, symbol whatWeExpect)
{
    
}

void * whatDoWeExpectString(parser * p, symbol context)
{
    char * outputString = malloc(MAX_ERROR_STRING_SIZE);
    switch(context)
    {
        case symMAIN:
        {
            outputString = " Expected:\n <MAIN>        ::= ""{"" <INSTRCTLST>";
            return outputString;
        }
        case symINSTRCTLST:
        {
            outputString = " Expected:\n <INSTRCTLST>  ::= <INSTRUCTION><INSTRCTLST> | \n ""}"" ";
            return outputString;
        }
        case symINSTRUCTION:
        {
            outputString = " Expected:\n <INSTRUCTION> ::= <FD> | \n <LT> | \n <RT> | \n <DO> | \n <SET>";
            return outputString;
        }
        case symFD:
        {
            outputString = " Expected:\n <FD>          ::= ""FD"" <VARNUM>";
            return outputString;
        }
        case symLT:
        {
            outputString = " Expected:\n <LT>          ::= ""LT"" <VARNUM>";
            return outputString;
        }
        case symRT:
        {
            outputString = " Expected:\n <RT>          ::= ""RT"" <VARNUM>";
            return outputString;
        }
        case symDO:
        {
            outputString = " Expected:\n <DO>          ::= ""DO"" <VAR> ""FROM"" <VARNUM> ""TO"" \n <VARNUM> ""{"" <INSTRCTLST>";
            return outputString;
        }
        case symVAR:
        {
            outputString = " Expected:\n <VAR>         ::= [A-Z]";
            return outputString;
        }
        case symVARNUM:
        {
            outputString = " Expected:\n <VARNUM>      ::= number | \n <VAR>";
            return outputString;
        }
        case symSET:
        {
            outputString = " Expected:\n <SET>         ::= ""SET"" <VAR> "":="" <POLISH>";
            return outputString;
        }
        case symPOLISH:
        {
            outputString = " Expected:\n <POLISH>      ::= <OP> <POLISH> | \n <VARNUM> <POLISH> | \n "";"" ";
            return outputString;
        }
        case symOP:
        {
            outputString = " Expected:\n <OP>          ::= ""+"" | \n  ""-"" | \n ""*"" | \n ""/"" ";
            return outputString;
        }
    }
    addErrorToList(p,outputString);
    free(outputString);
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
        printf("%d:[%s]   ",i,tokenArray[i]);
    }
    printf("\n\n\n");

}

