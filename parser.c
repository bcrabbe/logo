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
  opExpo = '^'
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
int addSymToList(parser * p, symbol sym, float value);
int printSymList(parser * p);
int printSymNode(symbolNode * node);

//parserStruct -> polish calc functions
int pushValue(parser *p, float value);
int popToOperator(parser * p, operator op);
void clearStack(stack * s);

//parserStruct -> error list functions
int addErrorToList(parser * p,char * errorString);
int displayErrors(parser * p);
int syntaxError(parser * p, const char * errorString);
int addWhatDoWeExpectStringToErrorList(parser * p, symbol context);

//parserStruct -> token array functions
char ** tokenise(const char * inputString, int * numberOfTokensPtr, const char * delimiter);
void testTokenArray(char ** tokenArray, int numberOfTokens);
void freeTokenArray(char **tokenArray,int numberOfTokens);
int isStringWhiteSpace(char * string);


//Unit tests
#pragma mark Unit Test Prototypes

void testTokenise();
void testInitParser();
void testIncrementToken();
void testParserErrors();
void testVarValueFunctions();
void testSymListFunctions();
void testPolishCalcFunctions();

void testParseVar();
void testParseVarnum();
void testParseOp();
void testParsePolish();
void testParseSet();
void testParseFd();
void testParseRt();
void testParseLt();
void testParseInstruction();
void testParseDo();
void testParseInstrctlst();
void testParseMain();

symbolList * parse(char * inputString)
{
  unsigned long inputStringLength = strlen(inputString);
  if(!inputStringLength)
    {
      printError("Parser recieved a empty string. Exiting.",__FILE__,__FUNCTION__,__LINE__);
      return NULL;
    }
  parser * p = initParser();
   
  p->progArray = tokenise(inputString, &p->numberOfTokens, " \n\r\t\v\f");

  if(VERBOSE)
    {
      testTokenArray(p->progArray, p->numberOfTokens);
    }
  if(parseMAIN(p))
    {
      if(VERBOSE)
        {
	  printf("\n\nProgram was validated successfully.\n");
	  printSymList(p);
        }
    }
  else
    {
      printSymList(p);
      displayErrors(p);
      freeParser(p);
      return NULL;
    }
  symbolList * symList = p->symList;
  freeParser(p);
  return symList;
}
/**
 *<MAIN>        ::= ""{"" <INSTRCTLST>
 */
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
/**
 * <INSTRCTLST>  ::= <INSTRUCTION><INSTRCTLST> | ""}"" 
 */
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
      syntaxError(p,"Expected to read an instruction or a ""}"".");
      return 0;
    }
}
/**
   <INSTRUCTION> ::= <FD> | <LT> | <RT> | <DO> | <SET>
*/
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
/**
 *<FD>          ::= ""FD"" <VARNUM>
 */
int parseFD(parser * p)
{
  if(stringsMatch(p->progArray[p->atToken], "FD"))
    {
      if(incrementAtToken(p)==0) return 0;
      else
        {
	  float value;
	  if(parseVARNUM(p,&value)==0)
            {
	      addWhatDoWeExpectStringToErrorList(p, symFD);
	      syntaxError(p,"FD could not read VARNUM.");
	      return 0;
            }
	  if(value==0)
            {
	      syntaxError(p,"FD 0 is a redundant instruction.");
	      return 1;
            }
	  return addSymToList(p, symFD, value);
        }
    }
  return 0;
}
/**
 *<LT>          ::= ""LT"" <VARNUM>
 */
int parseLT(parser * p)
{
  if(stringsMatch(p->progArray[p->atToken], "LT"))
    {
      if(incrementAtToken(p)==0) return 0;
      else
        {
	  float value;
	  if(parseVARNUM(p,&value)==0)
            {
	      addWhatDoWeExpectStringToErrorList(p, symLT);
	      syntaxError(p,"LT could not read VARNUM.");
	      return 0;
            }
	  if(value==0)
            {
	      syntaxError(p,"LT 0 is a redundant instruction.");
	      return 1;
            }
	  return addSymToList(p, symLT, value);
        }
   
    }
  else return 0;
}
/**
 *<RT>          ::= ""RT"" <VARNUM>
 */
int parseRT(parser * p)
{
  if(!stringsMatch(p->progArray[p->atToken], "RT")) return 0;
  else
    {
      if(incrementAtToken(p)==0) return 0;
      else
        {
	  float value;
	  if(parseVARNUM(p,&value)==0)
            {
	      addWhatDoWeExpectStringToErrorList(p, symRT);
	      syntaxError(p,"RT could not read VARNUM.");
	      return 0;
            }
	  if(value==0)
            {
	      syntaxError(p,"RT 0 is a redundant instruction.");
	      return 1;
            }
	  return addSymToList(p, symRT, value);
        }
    }
}
/*
 * <VARNUM>      ::= number | <VAR>
 */
int parseVARNUM(parser * p, float * result)
{
  float value;
  if(strlen(p->progArray[p->atToken])<1)
    {
      printError("parseVARNUM recieved a empty string.",__FILE__,__FUNCTION__,__LINE__);
      return 0;
    }
  if(isdigit(p->progArray[p->atToken][0]) ||
     ( p->progArray[p->atToken][0]=='-' &&
       strlen(p->progArray[p->atToken])>1 &&
       isdigit(p->progArray[p->atToken][1]))) 
    {
      value = atof(p->progArray[p->atToken]);
      if(incrementAtToken(p)==0) return 0;
      *result = value;
      return 1;
    }
  else if(isupper(p->progArray[p->atToken][0]))
    {
      char var = parseVAR(p);
      if(var=='\0') return 0; //could not read a valid VAR. error sent in func
      value = getVarValue(p,var);//retrieves the value of the specified variable
      *result = value;
      return 1;
    }
  else if(p->progArray[p->atToken][0]=='-' &&
	  strlen(p->progArray[p->atToken])>1 &&
	  isupper(p->progArray[p->atToken][1])) {
      char var = parseVAR(p);
      if(var=='\0') return 0; //could not read a valid VAR. error sent in func
      value = getVarValue(p,var);//retrieves the value of the specified variable
      *result = -value;
      return 1;
    } else {
      return 0;
    }
}

/**
   Reads the token p->progArray[p->atToken], if it is a valid VAR i.e. a char 'A'-'Z' 
   then that charecter is return, other wise 0 is.
 
   <VAR>         ::= [A-Z]
*/
char parseVAR(parser * p)
{
  if(strlen(p->progArray[p->atToken])<1)
    {
      printError("parseVAR recieved a empty string. Exiting.",__FILE__,__FUNCTION__,__LINE__);
      return '\0';
    }
  if(isupper(p->progArray[p->atToken][0]))
    {
      return incrementAtToken(p) ? p->progArray[p->atToken-1][0] : '\0';
    }
  else if(p->progArray[p->atToken][0]=='-' &&
       strlen(p->progArray[p->atToken])>1 &&
       isupper(p->progArray[p->atToken][1])) 
    {
      return incrementAtToken(p) ? p->progArray[p->atToken-1][1] : '\0';     
    } else {
    return '\0';
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
	  addWhatDoWeExpectStringToErrorList(p,symDO);
	  syntaxError(p,"Could not read ""VAR"".");
	  return 0;
        }
        
      // "FROM"
      if(!stringsMatch(p->progArray[p->atToken], "FROM"))
        {
	  addWhatDoWeExpectStringToErrorList(p,symDO);
	  syntaxError(p,"Could not read ""FROM"".");
	  return 0;
        }
      if(incrementAtToken(p)==0) return 0;
 
      // <VARNUM> start
      float fromVarNum;
      if(!parseVARNUM(p,&fromVarNum))
        {
	  addWhatDoWeExpectStringToErrorList(p,symDO);
	  syntaxError(p,"Could not read 1st <VARNUM>.");
	  return 0;
        }
        
      // "TO"
      if(!stringsMatch(p->progArray[p->atToken], "TO"))
        {
	  addWhatDoWeExpectStringToErrorList(p,symDO);
	  syntaxError(p,"Could not read ""TO"".");
	  return 0;
        }
      if(incrementAtToken(p)==0) return 0;
        
      // <VARNUM> end
      float toVarNum;
      if(!parseVARNUM(p,&toVarNum))
        {
	  addWhatDoWeExpectStringToErrorList(p,symDO);
	  syntaxError(p,"Could not read 2nd <VARNUM>.");
	  return 0;
        }
        
      // get "{"
      if(!stringsMatch(p->progArray[p->atToken], "{"))
        {
	  addWhatDoWeExpectStringToErrorList(p,symDO);
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
	  addWhatDoWeExpectStringToErrorList(p,symDO);
	  syntaxError(p,"Could not read ""VAR"".");
	  return 0;
        }
        
      // ":="
      if(!stringsMatch(p->progArray[p->atToken], ":="))
        {
	  addWhatDoWeExpectStringToErrorList(p,symDO);
	  syntaxError(p,"Could not read "":="".");
	  return 0;
        }
      if(incrementAtToken(p)==0) return 0;
        
      clearStack(p->polishCalcStack);
      float setToValue;
      if(parsePOLISH(p, &setToValue)==0) return 0;
      if(setVarValue(p, var, setToValue)==0) return 0;
        
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
      if(incrementAtToken(p)==0) return 0;
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
      if(popToOperator(p, op)==0)
        {
	  syntaxError(p,"polish expression incorrectly formatted.");
	  return 0;
        }
      return parsePOLISH(p, result);
    }
  addWhatDoWeExpectStringToErrorList(p, symPOLISH);
  syntaxError(p,"could not read VARNUM or OP or ;.");
  return 0;
}

/*
  <OP>          ::= ""+"" | ""-"" | ""*"" | ""/"" | ""^""
*/
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
  else if(stringsMatch(p->progArray[p->atToken], "^"))
    {
      *op = opExpo;
    }
  else return 0;
    
  return incrementAtToken(p) ? 1 : 0;
}

#pragma mark parser obj functions
/**
   builds and returns a * to parser struct.
*/
parser * initParser()
{
  parser * p = malloc(sizeof(parser));
  if(p==NULL)
    {
      printError("parser * p = malloc(sizeof(parser)) failed exiting.",__FILE__,__FUNCTION__,__LINE__);
      exit(1);
    }
  p->progArray = NULL;
  p->numberOfTokens = 0;
  p->atToken=0;
  p->varValues = calloc('Z'+1, sizeof(int));//over sized array, variables can be indexed by their ascii values
  p->symList = malloc(sizeof(symbolList));
  if(p->symList==NULL)
    {
      printError(" p->symList = malloc(sizeof(symbolList)) failed exiting.",__FILE__,__FUNCTION__,__LINE__);
      exit(1);
    }
  p->symList->length=0;
  p->symList->start=NULL;
  p->symList->end=NULL;
    
  p->polishCalcStack = malloc(sizeof(stack));
  p->polishCalcStack->array = NULL;
  p->polishCalcStack->itemsInStack=0;
  p->errorList=NULL;
  p->numberOfErrors=0;
  return p;
}

/**
   free's p except for p->symList since this is returned by parse()
*/
void freeParser(parser * p)
{
  freeTokenArray(p->progArray, p->numberOfTokens);
  //free error list:
  for(int i=0; i<p->numberOfErrors; ++i)
    {
      free(p->errorList[i]);
    }
  free(p->errorList);
  free(p->polishCalcStack->array);
  free(p->polishCalcStack);
  free(p);
  //do not want to free symlist as this is returned.
}

/**
   Increments p->atToken if possible and returns 1. 
   if there is not another token it returns 0.
*/
int incrementAtToken(parser * p)
{
  if(p->atToken < p->numberOfTokens-1)
    {
      ++p->atToken;
      //if(VERBOSE) printf("moved to token %d [%s]\n",p->atToken, p->progArray[p->atToken]);
      return 1;
    }
  else
    {
      addErrorToList(p,"ERROR: expected program to end with a ""}""\n");
      return 0;
    }
}

#pragma mark VAR value functions
/**
   sets the value of var (should be 'A' to 'Z', if its not it sends an erro and returns 0) to newValue 
   and returns that value.
*/
int setVarValue(parser *p, char var, float newValue)
{
  if(var>'Z' || var<'A')
    {
      char errStr[MAX_ERROR_STRING_SIZE];
      sprintf(errStr, "setVarValue was passed the invalid variable charecter %c vars should be 'A' to 'Z' only.",var);
      printError(errStr, __FILE__, __FUNCTION__, __LINE__);
      return 0;
    }
  p->varValues[(int)var]=newValue;
  return 1;
}

/**
   Gets the value of var (should be 'A' to 'Z', if its not it sends an erro and returns 0)
   and returns that value.
*/
float getVarValue(parser *p, char var)
{
  if(var>'Z' || var<'A')
    {
      char errStr[MAX_ERROR_STRING_SIZE];
      sprintf(errStr, "getVarValue was passed the invalid variable charecter %c vars should be 'A' to 'Z' only.",var);
      printError(errStr, __FILE__, __FUNCTION__, __LINE__);
      return 0;
    }
  return p->varValues[(int)var];
}
       
#pragma mark symbol list functions
/**
   Creates a symbolNode for the input values and ands it to p->symList linked list
   symList only needs to contain FD LT and RT instructions, all others can be expanded to these.
   if this function is called with a sym other than these, it prints an error and returns 0.
   otherwise it returns the new length of the list
*/
int addSymToList(parser * p, symbol sym, float value)
{
  symbolNode * newNode = malloc(sizeof(symbolNode));
  if(newNode==NULL)
    {
      printError("symbolNode * newNode = malloc(sizeof(symbolNode)) failed.", __FILE__, __FUNCTION__, __LINE__);
      exit(1);
    }
  newNode->sym = sym;
  newNode->value = value;
  newNode->next = NULL;
  if(sym!=symFD && sym!=symLT && sym!=symRT)
    {
      printError("addSymToList called a sym type that doesnt need to be placed on symList.", __FILE__, __FUNCTION__, __LINE__);
      return 0;
    }
  if(p->symList->length==0)
    {//if first node:
      p->symList->start = newNode;
      p->symList->end = newNode;
    }
  else
    {
      p->symList->end->next = newNode;
      p->symList->end = newNode;
    }
  return (int)++p->symList->length;
}

/**
   Prints each node of p->symList to stdout.
   Returns 1 if completed succesfully, 0 if there is unexpected symbols in the list.
*/
int printSymList(parser * p)
{
  symbolNode * current = p->symList->start;
  printf("{\n");
  while(current)
    {
      if(printSymNode(current)==0) return 0;
      current=current->next;
    }
  printf("}\n");
  return 1;
}

/**
   Prints a line detailing the contents of node to stdout.
   Returns 1 if completed succesfully, 0 if there is an unexpected symbols.
*/
int printSymNode(symbolNode * node)
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
    default:
      {
	printError("symList contained a unexpected symbol. Only FD, RT, and LT instructions are neccessary to be added to symList, others can be expanded to just these.", __FILE__, __FUNCTION__, __LINE__);
	return 0;
      }
    }
  printf("%f\n",node->value);
  return 1;
}

#pragma mark polish calculator functions
/**
   accesses p->polishCalcStack. pushes value onto the top of the stack.
   returns the number of items now in the stack.
*/
int pushValue(parser *p, float value)
{
  ++p->polishCalcStack->itemsInStack;
    
  float * tmp = realloc(p->polishCalcStack->array, p->polishCalcStack->itemsInStack*sizeof(int));
  if(tmp==NULL)
    {
      printError(" float * tmp = realloc(p->polishCalcStack->array, p->polishCalcStack->itemsInStack*sizeof(int)) failed. Exiting.", __FILE__, __FUNCTION__, __LINE__);
      exit(1);
    }
  p->polishCalcStack->array = tmp;
  p->polishCalcStack->array[p->polishCalcStack->itemsInStack-1]=value;
  return p->polishCalcStack->itemsInStack;
}

/**
   accesses p->polishCalcStack poping the top two values off and combining them with
   operator op. the top item goes to the rhs the 2nd top goes lhs. the result is pushed 
   back on to the stack.
   returns 1 if successful 0 if not.
*/
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
    case opExpo:
      {
	result = pow(lhs, rhs);
	if(VERBOSE) {
	  lfprint(lhs);
	  lfprint(rhs);
	  lfprint(result);	  
	}
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
  return pushValue(p, result) ? 1 : 0;
}

/**
   resets stack struct.
*/
void clearStack(stack * s)
{
  if(s->array) free(s->array);
  s->array=NULL;
  s->itemsInStack=0;
}


#pragma mark error messaging functions
/**
   Adds error string to the string array p->errorList.
   @returns 1 if sucessful
*/
int addErrorToList(parser * p, char * errorString)
{
  ++p->numberOfErrors;
  char ** tmp = realloc(p->errorList,p->numberOfErrors*sizeof(char*));
  if(tmp==NULL)
    {
      printError("realloc failed exiting.",__FILE__,__FUNCTION__,__LINE__);
      exit(1);
    }
  p->errorList = tmp;
  p->errorList[p->numberOfErrors-1] = strdup(errorString);
  return 1;
}

/**
   Prints all of the strings int p->errorList array.
   @returns 1
*/
int displayErrors(parser * p)
{
  if(p->numberOfErrors==0) return 0;
  printf("\n\nParsing Failed. There were %d errors:",p->numberOfErrors);
  for(int i=0; i<p->numberOfErrors; ++i)
    {
      printf("\n%s \n",p->errorList[i]);
    }
  printf("\n");
  return 1;
}

/**
   Adds a syntax error to p->errorList array.
   @returns 1 if successful. returns 0 and a Error message if unsuccessful.
*/
int syntaxError(parser * p, const char * errorString)
{
  if(strlen(errorString)<1)
    {
      printError("Function was called with empty string.", __FILE__, __FUNCTION__, __LINE__);
      return 0;
    }
  if(p->numberOfTokens<1)
    {
      printError("Syntax error called but there are no tokens yet.", __FILE__, __FUNCTION__, __LINE__);
      return 0;
    }
  char * editableErrorString = strdup(errorString);
  char stringStart[MAX_ERROR_STRING_SIZE]={'\0'};
    
  if(p->atToken>=1)
    {
      sprintf(stringStart,"ERROR: invalid syntax at token %d ""%s"" previous token %d ""%s"". \n",
	      p->atToken, p->progArray[p->atToken],
	      p->atToken-1,  p->progArray[p->atToken-1]);
    }
  strcat(stringStart,editableErrorString);
  free(editableErrorString);
    
  return addErrorToList(p,stringStart);
}

/**
   Adds a line explaing the expected syntax of a symbol context to p->errorList array.
   @returns 1 if successful. returns 0 and a Error message if unsuccessful.
*/
int addWhatDoWeExpectStringToErrorList(parser * p, symbol context)
{
  char outputString[MAX_ERROR_STRING_SIZE];

  switch(context)
    {
    case symMAIN:
      sprintf(outputString,"Expected: <MAIN>        ::= ""{"" <INSTRCTLST>");
      break;
    case symINSTRCTLST:
      sprintf(outputString,"Expected: <INSTRCTLST>  ::= <INSTRUCTION><INSTRCTLST> | ""}"" ");
      break;
    case symINSTRUCTION:
      sprintf(outputString,"Expected: <INSTRUCTION> ::= <FD> | <LT> | <RT> | <DO> | <SET>");
      break;
    case symFD:
      sprintf(outputString, "Expected: <FD>          ::= ""FD"" <VARNUM>");
      break;
    case symLT:
      sprintf(outputString, "Expected: <LT>          ::= ""LT"" <VARNUM>");
      break;
    case symRT:
      sprintf(outputString, "Expected: <RT>          ::= ""RT"" <VARNUM>");
      break;
    case symDO:
      sprintf(outputString, "Expected: <DO>          ::= ""DO"" <VAR> ""FROM"" <VARNUM> ""TO"" <VARNUM> ""{"" <INSTRCTLST>");
      break;
    case symVAR:
      sprintf(outputString, "Expected: <VAR>         ::= [A-Z]");
      break;
    case symVARNUM:
      sprintf(outputString, "Expected: <VARNUM>      ::= number | <VAR>");
      break;
    case symSET:
      sprintf(outputString,"Expected: <SET>         ::= ""SET"" <VAR> "":="" <POLISH>");
      break;
    case symPOLISH:
      sprintf(outputString,"Expected: <POLISH>      ::= <OP> <POLISH> | <VARNUM> <POLISH> |  "";"" ");
      break;
    case symOP:
      sprintf(outputString, "Expected: <OP>          ::= ""+"" | ""-"" | ""*"" | ""/"" ");
      break;
    default:
      printError("read an invalid symbol context.", __FILE__, __FUNCTION__, __LINE__);
      return 0;
    }
  int successfullyAdded = addErrorToList(p,outputString);
  return successfullyAdded ? 1 : 0;
}

#pragma mark tokenArray
/*
 *  Takes the input string and breaks into separate words where ever it finds a charecter contained in the delimeter string each of these words is stored in the returned array which is an array of strings. the number of strings is stored in numberOfTokensPtr.
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
      if(isStringWhiteSpace(stringToken))
        {
	  //discard this token, do nothing
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

/**
   Returns 1 if string is entirly whitespace ( \t\r\v\f\n)
*/
int isStringWhiteSpace(char * string)
{
  for(int i=0;string[i];++i)
    {
      if(!isspace(string[i])) return 0;
    }
  return 1;
}

/*
 *  frees the memory allocated to a tokenArray in tokenise func
 */
void freeTokenArray(char **tokenArray,int numberOfTokens)
{
  for(int i=0; i<numberOfTokens; ++i)
    {
      free(tokenArray[i]);
      printf("freed row");
    }
  free(tokenArray);
  printf("freed col");
    
}

#pragma mark developement tests
/*
 *  Test function for developement. Prints contents of a tokenArray
 */
void testTokenArray(char ** tokenArray, int numberOfTokens)
{
  printf("\ntestTokenArray:\n");
  for(int i=0; i<numberOfTokens; ++i)
    {
      printf("%d:[%s]   ",i,tokenArray[i]);
    }
  printf("\n\n");
}

/******************************************************************************/
//Unit Tests

#pragma mark Parser Unit Test Functions

void unitTests_parser()
{
  sput_start_testing();
  sput_set_output_stream(NULL);

  sput_enter_suite("testTokenise()");
  sput_run_test(testTokenise);
  sput_leave_suite();
    
  sput_enter_suite("testInitParser()");
  sput_run_test(testInitParser);
  sput_leave_suite();
    
  sput_enter_suite("testIncrementToken()");
  sput_run_test(testIncrementToken);
  sput_leave_suite();

  sput_enter_suite("testParserErrors()");
  sput_run_test(testParserErrors);
  sput_leave_suite();
    
  sput_enter_suite("testVarValueFunctions()");
  sput_run_test(testVarValueFunctions);
  sput_leave_suite();
    
  sput_enter_suite("testSymListFunctions()");
  sput_run_test(testSymListFunctions);
  sput_leave_suite();
    
  sput_enter_suite("testPolishCalcFunctions()");
  sput_run_test(testPolishCalcFunctions);
  sput_leave_suite();

  sput_enter_suite("testParseVar()");
  sput_run_test(testParseVar);
  sput_leave_suite();

  sput_enter_suite("testParseVarnum()");
  sput_run_test(testParseVarnum);
  sput_leave_suite();
    
  sput_enter_suite("testParseOp()");
  sput_run_test(testParseOp);
  sput_leave_suite();
    
  sput_enter_suite("testParsePolish()");
  sput_run_test(testParsePolish);
  sput_leave_suite();
    
  sput_enter_suite("testParseSet()");
  sput_run_test(testParseSet);
  sput_leave_suite();

  sput_enter_suite("testParseFd()");
  sput_run_test(testParseFd);
  sput_leave_suite();
    
  sput_enter_suite("testParseLt()");
  sput_run_test(testParseLt);
  sput_leave_suite();

  sput_enter_suite("testParseRt()");
  sput_run_test(testParseRt);
  sput_leave_suite();

  sput_enter_suite("testParseDo()");
  sput_run_test(testParseDo);
  sput_leave_suite();
    
  sput_enter_suite("testParseInstruction()");
  sput_run_test(testParseInstruction);
  sput_leave_suite();
    
  sput_enter_suite("testParseInstrctlst()");
  sput_run_test(testParseInstrctlst);
  sput_leave_suite();

  sput_enter_suite("testParseMain()");
  sput_run_test(testParseMain);
  sput_leave_suite();


  sput_finish_testing();

}

/**
   Parse unit test suite.
   tests the tokenise and freeTokenArray functions
*/
void testTokenise()
{
  int numberOfTokensTest1=0;
  char ** test1 = tokenise("break this string up",&numberOfTokensTest1," ");
  if(VERBOSE) testTokenArray(test1,numberOfTokensTest1);
  sput_fail_unless(!strcmp(test1[0],"break")  &&
		   !strcmp(test1[1],"this")   &&
		   !strcmp(test1[2],"string") &&
		   !strcmp(test1[3],"up")     &&
		   numberOfTokensTest1==4 ,
		   "Checks basic functionality. Tested with ""break this string up"" and space & comma delimeters, should return each seperate word.");
  freeTokenArray(test1,numberOfTokensTest1);
    
  test1 = tokenise("\tbreak\nthis string\tup\r\n",&numberOfTokensTest1," \t\r\n");
  if(VERBOSE) testTokenArray(test1,numberOfTokensTest1);
  sput_fail_unless(!strcmp(test1[0],"break")  &&
		   !strcmp(test1[1],"this")   &&
		   !strcmp(test1[2],"string") &&
		   !strcmp(test1[3],"up")     &&
		   numberOfTokensTest1==4,
		   "Checks that \r \t \n are handled correctly. Tested with ""\\tbreak\\nthis string\\tup\\r\\n"" and "" \\t\\r\\n"" delim, should return each seperate word.");
  freeTokenArray(test1,numberOfTokensTest1);
    
  test1 = tokenise("  break\t   \r   this\n    \t  \r  string \r\n\t up\t    ",&numberOfTokensTest1," \t\r\n");
  if(VERBOSE) testTokenArray(test1,numberOfTokensTest1);
  sput_fail_unless(!strcmp(test1[0],"break")  &&
		   !strcmp(test1[1],"this")   &&
		   !strcmp(test1[2],"string") &&
		   !strcmp(test1[3],"up")     &&
		   numberOfTokensTest1==4,
		   "Checks that tokens that contain only spaces are discarded even when it is not a delimeter. Tested with ""  break\\t   \\r   this\\n    \\t  \\r  string \\r\\n\\t up\\t    "" and ""\\t\\r\\n"" delim it should return each seperate word.");
  freeTokenArray(test1,numberOfTokensTest1);
}

/**
   Parse unit test suite.
   tests the initParser and freeParser functions
*/
void testInitParser()
{
  parser * p = initParser();
  sput_fail_unless(p->progArray == NULL,"Checking that all structure elements are accessible and set correctly.");
  sput_fail_unless(p->numberOfTokens == 0,"Checking that all structure elements are accessible and set correctly.");
  sput_fail_unless(p->atToken==0,"Checking that all structure elements are accessible and set correctly.");
  sput_fail_unless(p->varValues[0]==0,"Checking that all structure elements are accessible and set correctly.");
  sput_fail_unless(p->varValues['Z']==0,"Checking that all structure elements are accessible and set correctly.");
  sput_fail_unless(p->symList->length==0,"Checking that all structure elements are accessible and set correctly.");
  sput_fail_unless(p->symList->start==NULL,"Checking that all structure elements are accessible and set correctly.");
  sput_fail_unless(p->symList->end==NULL,"Checking that all structure elements are accessible and set correctly.");
  sput_fail_unless(p->polishCalcStack->array==NULL,"Checking that all structure elements are accessible and set correctly.");
  sput_fail_unless(p->polishCalcStack->itemsInStack==0,"Checking that all structure elements are accessible and set correctly.");
  sput_fail_unless(p->errorList==NULL,"Checking that all structure elements are accessible and set correctly.");
  sput_fail_unless(p->numberOfErrors==0,"Checking that all structure elements are accessible and set correctly.");
  freeParser(p);
    
}

/**
   Parse unit test suite.
   tests the incrementToken functions
*/
void testIncrementToken()
{
  parser * p = initParser();
  p->progArray = tokenise("0 1 2 3 4 5 6 7 8 9", &p->numberOfTokens, " \n\r\t\v\f");
  for(int i = 0; i<9; ++i)
    {
      sput_fail_unless(incrementAtToken(p)==1, "We have made 10 tokens in progArray, check that we can then incrementAtToken 10 times successfully.");
    }
  sput_fail_unless(incrementAtToken(p)==0, "and on the 11th it should fail.");
  freeParser(p);
}

/**
   Parse unit test suite.
   tests the addErrorToList, syntaxError, addWhatDoWeExpectStringToErrorList and displayErrors functions.
*/
void testParserErrors()
{
  parser * p = initParser();
  //test addErrorToList()
  addErrorToList(p, "test string");
  sput_fail_unless(p->numberOfErrors==1 && strcmp(p->errorList[p->numberOfErrors-1],"test string")==0, "Checks addErrorToList.");
    
  //test invalid syntaxError()
  sput_fail_unless(syntaxError(p, "testing syntaxError")==0, "calling syntaxError before we have a program loaded should print error and return 0.");
  sput_fail_unless(syntaxError(p, "")==0, "calling syntaxError with an empty string should print error and return 0.");
    
  //test valid syntaxError()
  p->progArray = tokenise("0 1 2 3 4 5 6 7 8 9", &p->numberOfTokens, " \n\r\t\v\f");
  sput_fail_unless(syntaxError(p, "testing syntaxError")==1 && p->numberOfErrors==2, "Checks valid call of syntaxError with 0->atToken=0. ");
  sput_fail_unless(strcmp(p->errorList[p->numberOfErrors-1], "testing syntaxError")==0, "Since p->atToken = 0 at last call we should see no tokens printed in message.");
    
  p->atToken=9;
  sput_fail_unless(syntaxError(p, "testing syntaxError")==1 && p->numberOfErrors==3, "Checks syntaxError when at last token.");
  sput_fail_unless(strcmp(p->errorList[p->numberOfErrors-1], "testing syntaxError")!=0, "Since p->atToken != 0 at last call we should see tokens printed in message.");
    
  for(symbol sym = symMAIN; sym<=symOP ; ++sym)
    {
      sput_fail_unless(addWhatDoWeExpectStringToErrorList(p, sym)==1, "Check that addWhatDoWeExpectStringToErrorList can add to the errorList");
    }
  sput_fail_unless(addWhatDoWeExpectStringToErrorList(p, symOP+1)==0, "Check that addWhatDoWeExpectStringToErrorList correctly handles a invalde symbol context");

  printf("Calling Display Errors:");
  sput_fail_unless(displayErrors(p)==1, "Checks displayErrors returns 1");
  freeParser(p);
}

/**
   Parse unit test suite.
   tests the setVarValue and getVarValue functions.
*/
void testVarValueFunctions()
{
  parser * p = initParser();
  float setTo = 15.2;
  for(char var = 'A'; var<='Z'; ++var)
    {
      sput_fail_unless( setVarValue(p, var, setTo), "call setVarValue for every possibile variable A-Z");
      sput_fail_unless(getVarValue(p, var)==setTo, "call getVarValue for every possibile variable A-Z. check it returns what we just set it to.");
      sput_fail_unless(getVarValue(p, var)==p->varValues[(int)var], "call getVarValue for every possibile variable A-Z check it returns what is in the p->varValues array.");

    }
  sput_fail_unless(setVarValue(p, 'A'-1, setTo)==0, "call setVarValue'A'-1 should print an error and return 0.");
  sput_fail_unless(setVarValue(p, 'Z'+1, setTo)==0, "call setVarValue'Z'+1 should print an error and return 0.");

  sput_fail_unless(getVarValue(p, 'A'-1)==0, "call getVarValue with 'A'-1 should print an error and return 0.");
  sput_fail_unless(getVarValue(p, 'Z'+1)==0, "call getVarValue with 'Z'+1 should print an error and return 0.");

  freeParser(p);
}

/**
   Parse unit test suite.
   tests the addSymToList function.
*/
void testSymListFunctions()
{
  parser * p = initParser();
  float value=40.23;

  sput_fail_unless(addSymToList(p, symFD, value)==1, "should be able to add symFD to list");
  sput_fail_unless(addSymToList(p, symRT, value)==2, "should be able to add symRT to list");
  sput_fail_unless(addSymToList(p, symFD, value)==3, "should be able to add symFD to list");
  for(symbol sym = symMAIN; sym<=symOP ; ++sym)
    {
      if(sym==symFD || sym==symLT || sym==symRT)
        {
	  //doNothing
        }
      else
        {
	  sput_fail_unless(addSymToList(p, sym, value)==0, "should  not be able to add other syms to list.");
        }
    }
  freeParser(p);
}

/**
   Parse unit test suite.
   tests the PolishCalc functions.
*/
void testPolishCalcFunctions()
{
  parser * p = initParser();
  //test pushValue()
  float value = 10.2;
  sput_fail_unless(pushValue(p, value)==1 &&
		   p->polishCalcStack->array[p->polishCalcStack->itemsInStack-1]==value,
		   "Check that pushValue is successful and that the pushed value is top of the stack after");
  //test clearStack()
  clearStack(p->polishCalcStack);
  sput_fail_unless(p->polishCalcStack->itemsInStack==0 &&
		   p->polishCalcStack->array==NULL,
		   "Checks that itemsInStack=0 && array==NULL after clearStack.");
  float epsilon = 0.0002;
  //test popToOperator
  pushValue(p, value);
  pushValue(p, value);
  sput_fail_unless(popToOperator(p, opPlus)==1 &&
		   fabs(p->polishCalcStack->array[p->polishCalcStack->itemsInStack-1]-(value+value))<epsilon,
		   "check that popToOperator with opPlus works and returns the correct result.");
  pushValue(p, value);
  pushValue(p, value);
  sput_fail_unless(popToOperator(p, opMinus)==1 &&
		   fabs(p->polishCalcStack->array[p->polishCalcStack->itemsInStack-1]-(value-value))<epsilon,
		   "check that popToOperator with opMinus works and returns the correct result.");
  pushValue(p, value);
  pushValue(p, value);
  sput_fail_unless(popToOperator(p, opMultiply)==1 &&
		   fabs(p->polishCalcStack->array[p->polishCalcStack->itemsInStack-1]-(value*value))<epsilon,
		   "check that popToOperator with opMultiply works and returns the correct result.");
  pushValue(p, value);
  pushValue(p, value);
  sput_fail_unless(popToOperator(p, opDivide)==1 &&
		   fabs(p->polishCalcStack->array[p->polishCalcStack->itemsInStack-1]-(value/value))<epsilon,
		   "check that popToOperator with opMultiply works and returns the correct result.");
    
  freeParser(p);
}

#pragma mark Sym Parser Unit Tests
void testParseVar()
{
  parser * p = initParser();
    
  p->progArray = tokenise("s }", &p->numberOfTokens, " ");
  sput_fail_unless(parseVAR(p)=='\0', "s is not a valid variable so should return '\0'");
  freeTokenArray(p->progArray, p->numberOfTokens);
  p->atToken=0;

  p->progArray = tokenise("A", &p->numberOfTokens, " ");
  sput_fail_unless(parseVAR(p)==0, "can not increment token since there is only one, should return 0");
  freeTokenArray(p->progArray, p->numberOfTokens);
  p->atToken=0;

  p->progArray = tokenise("S }", &p->numberOfTokens, " ");
  sput_fail_unless(parseVAR(p)=='S', "is valid, should return 'S'");
  freeParser(p);
}

void testParseVarnum()
{
  parser * p = initParser();
  float result=0, epsilon=0.002;
    
  p->progArray = tokenise("1992.232", &p->numberOfTokens, " ");
  sput_fail_unless(parseVARNUM(p,&result)==0, "Should read the value, but since there is only 1 token, incrementing should fail and return 0.");
  freeTokenArray(p->progArray, p->numberOfTokens);
  p->atToken=0;

  p->progArray = tokenise("1992.232 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseVARNUM(p,&result)==1 && fabs(result-1992.232)<epsilon, "Valid. Should read value 1992.232.");
  freeTokenArray(p->progArray, p->numberOfTokens);
  p->atToken=0;

  p->progArray = tokenise("a", &p->numberOfTokens, " ");
  sput_fail_unless(parseVARNUM(p,&result)==0, "can not increment token since there is only one, should return 0");
  freeTokenArray(p->progArray, p->numberOfTokens);
  p->atToken=0;

  setVarValue(p, 'S', 33.3);
  p->progArray = tokenise("S }", &p->numberOfTokens, " ");
  sput_fail_unless(parseVARNUM(p,&result)==1 && fabs(result-33.3)<epsilon, "is valid, should return value of 'S'");
  freeTokenArray(p->progArray, p->numberOfTokens);
  p->atToken=0;

  setVarValue(p, 'S', 33.3);
  p->progArray = tokenise("-S }", &p->numberOfTokens, " ");
  testTokenArray(p->progArray, p->numberOfTokens);
  sput_fail_unless(parseVARNUM(p,&result)==1 && fabs(result+33.3)<epsilon, "is valid, should return value of '-S'");
  freeTokenArray(p->progArray, p->numberOfTokens);
  p->atToken=0;
  
  //read -ve val
  p->progArray = tokenise("-120 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseVARNUM(p,&result)==1 &&
		   fabs(result+120)<epsilon, "Valid. Should read value -120.");
  p->atToken=0;

  freeParser(p);//will free last token array in call 
    
}

void testParseOp()
{
  parser * p = initParser();
  operator op;
    
  p->progArray = tokenise("+", &p->numberOfTokens, " ");
  sput_fail_unless(parseOP(p,&op)==0, "should read + but then only one token so cannot increment should return 0.");
  freeTokenArray(p->progArray, p->numberOfTokens);
  p->atToken=0;

  p->progArray = tokenise("+ }", &p->numberOfTokens, " ");
  sput_fail_unless(parseOP(p,&op)==1 && op==opPlus, "Valid, should read +.");
  freeTokenArray(p->progArray, p->numberOfTokens);
  p->atToken=0;

  p->progArray = tokenise("- }", &p->numberOfTokens, " ");
  sput_fail_unless(parseOP(p,&op)==1 && op==opMinus, "Valid, should read -.");
  freeTokenArray(p->progArray, p->numberOfTokens);
  p->atToken=0;

  p->progArray = tokenise("* }", &p->numberOfTokens, " ");
  sput_fail_unless(parseOP(p,&op)==1 && op==opMultiply, "Valid, should read *.");
  freeTokenArray(p->progArray, p->numberOfTokens);
  p->atToken=0;

  p->progArray = tokenise("/ }", &p->numberOfTokens, " ");
  sput_fail_unless(parseOP(p,&op)==1 && op==opDivide, "Valid, should read /.");

  freeParser(p);
}

void testParsePolish()
{
  parser * p = initParser();
  float result, epsilon=0.02;
    
  p->progArray = tokenise(";", &p->numberOfTokens, " ");
  sput_fail_unless(parsePOLISH(p,&result)==0, " just ; is invalid syntax, should add error and return 0.");
  displayErrors(p);

  freeParser(p);
  p=initParser();

  p->progArray = tokenise("20.2 ; }", &p->numberOfTokens, " ");
  sput_fail_unless(parsePOLISH(p,&result)==1 && fabs(result-20.2)<epsilon, " just  20.2 ; } is valid syntax, should result is 20.2 and return 1.");
  displayErrors(p);

  freeParser(p);
  p=initParser();

  setVarValue(p, 'A', 20.2);
  p->progArray = tokenise("A ; }", &p->numberOfTokens, " ");
  sput_fail_unless(parsePOLISH(p,&result)==1 && fabs(result-20.2)<epsilon, " just  A ; } is valid syntax, result is 20.2 and return 1.");
  displayErrors(p);

  freeParser(p);
  p=initParser();
  setVarValue(p, 'A', 20.2);

  p->progArray = tokenise("A A ; }", &p->numberOfTokens, " ");
  sput_fail_unless(parsePOLISH(p,&result)==0, "A A ; } is invalid syntax, should push error and return 0.");
  displayErrors(p);

  freeParser(p);
  p=initParser();
  setVarValue(p, 'A', 20.2);

  p->progArray = tokenise("A A * ; }", &p->numberOfTokens, " ");
  sput_fail_unless(parsePOLISH(p,&result)==1 && fabs(result-(20.2*20.2))<epsilon, "A A *; } is valid syntax, result should be 20.2^2 and return 1.");
  displayErrors(p);

  freeParser(p);
  p=initParser();
  setVarValue(p, 'A', 20.2);

  p->progArray = tokenise("A A * - ; }", &p->numberOfTokens, " ");
  sput_fail_unless(parsePOLISH(p,&result)==0, "A A * - ; } is invalid syntax, should push error and return 0.");
  displayErrors(p);

  freeParser(p);
  p=initParser();
  setVarValue(p, 'A', 20.2);

  p->progArray = tokenise("A A * A - ; }", &p->numberOfTokens, " ");
  sput_fail_unless(parsePOLISH(p,&result)==1 && fabs(result-(20.2*20.2-20.2))<epsilon, "A A * A - ; } is valid syntax, result should be A*A - A and returns 1.");

  displayErrors(p);
  freeParser(p);
  p=initParser();
  setVarValue(p, 'A', 20.2);

  p->progArray = tokenise("A A * z - ;  }", &p->numberOfTokens, " ");
  sput_fail_unless(parsePOLISH(p,&result)==0, "A A * z - ; } is invalid syntax, should push error & return 0.");
  displayErrors(p);

  freeParser(p);
}

void testParseSet()
{
  parser * p = initParser();
  p->progArray = tokenise("NOTSET A * z - ;  }", &p->numberOfTokens, " ");
  sput_fail_unless(parseSET(p)==0, "first token is not SET so should return 0");
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("SET", &p->numberOfTokens, " ");
  sput_fail_unless(parseSET(p)==0, "Not enough tokens. Sould return 0.");
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("SET a }", &p->numberOfTokens, " ");
  sput_fail_unless(parseSET(p)==0, "a is not a valid variable. Sould return 0.");
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("SET A }", &p->numberOfTokens, " ");
  sput_fail_unless(parseSET(p)==0, "There should be a assignment after A. Sould return 0.");
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("SET A :=", &p->numberOfTokens, " ");
  sput_fail_unless(parseSET(p)==0, "Not enough tokens. Sould return 0.");
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("SET A := invalid", &p->numberOfTokens, " ");
  sput_fail_unless(parseSET(p)==0, "invalid polish epression. Sould return 0.");
  freeParser(p);
    
  p = initParser();
  setVarValue(p, 'B', 20.2);
  float epsilon = 0.02;
  p->progArray = tokenise("SET A := B C + ; }", &p->numberOfTokens, " ");
  sput_fail_unless(parseSET(p)==1 &&
		   fabs( getVarValue(p, 'A')-(getVarValue(p, 'C')+getVarValue(p, 'B')) )<epsilon,
		   "valid polish epression. Sould return 1 and set A to B^2.");

  freeParser(p);

}

void testParseFd()
{
  //test parseFD
  parser * p = initParser();
  p->progArray = tokenise("notFD 50.2 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseFD(p)==0, "first token is not FD so should return 0");
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("FD x }", &p->numberOfTokens, " ");
  sput_fail_unless(parseFD(p)==0 && p->numberOfErrors>0, "2nd token is not a valid VARNUM. should add error and return 0.");
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("FD 0 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseFD(p)==1 && p->numberOfErrors>0, "FD 0 }is a valid but redundant instruction. SHould add error and return 1.");
  freeParser(p);
    
  p = initParser();
  setVarValue(p, 'X', 202);
  p->progArray = tokenise("FD X }", &p->numberOfTokens, " ");
  sput_fail_unless(parseFD(p)==1 && p->numberOfErrors==0, "FD X } is a valid. Should return 1.");
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("FD 200 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseFD(p)==1 && p->numberOfErrors==0, "FD 200 } is a valid. Should return 1.");
  freeParser(p);
    

}

void testParseRt()
{
  //test parseFD
  parser * p = initParser();
  p->progArray = tokenise("notRT 50.2 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseFD(p)==0, "first token is not FD so should return 0");
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("RT x }", &p->numberOfTokens, " ");
  sput_fail_unless(parseRT(p)==0 && p->numberOfErrors>0, "2nd token is not a valid VARNUM. should add error and return 0.");
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("RT 0 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseRT(p)==1 && p->numberOfErrors>0, "RT 0 is a valid but redundant instruction. SHould add error and return 1.");
  freeParser(p);
    
  p = initParser();
  setVarValue(p, 'X', 202);
  p->progArray = tokenise("RT X }", &p->numberOfTokens, " ");
  sput_fail_unless(parseRT(p)==1 && p->numberOfErrors==0, "RT 0 is a valid. Should return 1.");
  freeParser(p);
    
  p = initParser();
  setVarValue(p, 'X', 202);
  p->progArray = tokenise("RT 200 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseRT(p)==1 && p->numberOfErrors==0, "RT 200 is a valid. Should return 1.");
  freeParser(p);
    
}

void testParseLt()
{
  parser * p = initParser();
  p->progArray = tokenise("notLT 50.2 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseLT(p)==0, "first token is not LT so should return 0");
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("LT x }", &p->numberOfTokens, " ");
  sput_fail_unless(parseLT(p)==0 && p->numberOfErrors>0, "2nd token is not a valid VARNUM. should add error and return 0.");
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("LT 0 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseLT(p)==1 && p->numberOfErrors>0, "LT 0 is a valid but redundant instruction. SHould add error and return 1.");
  freeParser(p);
    
  p = initParser();
  setVarValue(p, 'X', 202);
  p->progArray = tokenise("LT X }", &p->numberOfTokens, " ");
  sput_fail_unless(parseLT(p)==1 && p->numberOfErrors==0, "LT 0 is a valid. Should return 1.");
  freeParser(p);
    
  p = initParser();
  setVarValue(p, 'X', 202);
  p->progArray = tokenise("LT 200 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseLT(p)==1 && p->numberOfErrors==0, "LT 200 is a valid. Should return 1.");
  freeParser(p);
    
}

void testParseDo()
{
  parser * p = initParser();
  p->progArray = tokenise("notDO {", &p->numberOfTokens, " ");
  sput_fail_unless(parseDO(p)==0 , "called with notDO } should return 0 as this is not DO.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("DO notAVar {", &p->numberOfTokens, " ");
  sput_fail_unless(parseDO(p)==0 && p->numberOfErrors>0, "called with DO notAVar } should return 0 and add an error as this is not a VAR.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("DO A", &p->numberOfTokens, " ");
  sput_fail_unless(parseDO(p)==0 && p->numberOfErrors>0, "called with DO A  should return 0 and add an error as A is the last token.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("DO A notFROM {", &p->numberOfTokens, " ");
  sput_fail_unless(parseDO(p)==0 && p->numberOfErrors>0, "called with DO A notFROM  should return 0 and add an error as it expects to read FROM.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("DO A FROM notAVar {", &p->numberOfTokens, " ");
  sput_fail_unless(parseDO(p)==0 && p->numberOfErrors>0, "called with DO A FROM notAVar }  should return 0 and add an error as notAVar is not a VAR.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("DO A FROM 1 notTo {", &p->numberOfTokens, " ");
  sput_fail_unless(parseDO(p)==0 && p->numberOfErrors>0, "called with DO A FROM 1 noTo }  should return 0 and add an error it expects to read TO.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("DO A FROM 1 TO notAVar {", &p->numberOfTokens, " ");
  sput_fail_unless(parseDO(p)==0 && p->numberOfErrors>0, "called with DO A FROM 1 To notAVar }  should return 0 and add an error it expects to read a Var.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("DO A FROM 1 TO 5 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseDO(p)==0 && p->numberOfErrors>0, "called with DO A FROM 1 TO 5 }  should return 0 and add an error it expects to read a {.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("DO A FROM 1 TO 5 {", &p->numberOfTokens, " ");
  sput_fail_unless(parseDO(p)==0 && p->numberOfErrors>0, "called with DO A FROM 1 TO 5 {  should return 0 and add an error it expects to read a an instructionList.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("DO A FROM 1 TO 5 { FD 5 } }", &p->numberOfTokens, " ");
  sput_fail_unless(parseDO(p)==1, "called with DO A FROM 1 TO 5 { FD 5 } }  should return 1.");
  displayErrors(p);
  freeParser(p);
}

void testParseInstruction()
{
  parser * p = initParser();
  p->progArray = tokenise("z }", &p->numberOfTokens, " ");
  sput_fail_unless(parseINSTRUCTION(p)==0 , "called with z } should return 0 as this is not an instruction.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("FD 9 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseINSTRUCTION(p)==1 , "called with FD 9 } should return 1.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("RT 9 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseINSTRUCTION(p)==1 , "called with RT 9 } should return 1.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("LT 9 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseINSTRUCTION(p)==1 , "called with LT 9 } should return 1.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("SET A := 9 ; }", &p->numberOfTokens, " ");
  sput_fail_unless(parseINSTRUCTION(p)==1 , "called with SET A := 9 } should return 1.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("DO A FROM 1 TO 5 { FD 5 } }", &p->numberOfTokens, " ");
  sput_fail_unless(parseINSTRUCTION(p)==1 , "called with DO A FROM 1 TO 5 { FD 5 }} should return 1.");
  displayErrors(p);

  freeParser(p);
}

void testParseInstrctlst()
{
  parser * p = initParser();
  p->progArray = tokenise("FD 9 }", &p->numberOfTokens, " ");
  sput_fail_unless(parseINSTRUCTION(p)==1 , "called with FD 9 } should return 1.");
  displayErrors(p);
  freeParser(p);

  p = initParser();
  p->progArray = tokenise("FD z }", &p->numberOfTokens, " ");
  sput_fail_unless(parseINSTRUCTION(p)==0 , "called with FD 9 } should return 0 and add an error.");
  displayErrors(p);
  freeParser(p);
}

void testParseMain()
{
  parser * p = initParser();
  p->progArray = tokenise(">", &p->numberOfTokens, " ");
  sput_fail_unless(parseMAIN(p)==0 , "called with > should return 0 and display an error.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("{", &p->numberOfTokens, " ");
  sput_fail_unless(parseMAIN(p)==0 , "called with {, not enough tokens, should return 0 and display an error.");
  displayErrors(p);
  freeParser(p);
    
  p = initParser();
  p->progArray = tokenise("{ RT 9 } }", &p->numberOfTokens, " ");
  sput_fail_unless(parseMAIN(p)==1 , "called with { RT 9 }, this is valid, should return 1.");
  displayErrors(p);
  freeParser(p);
}

/**
   Builds a symbolList for use in path.c unit tests, if you change this you
   need to update void testBuildPath() in path.c and void testGetScaler() in draw.c
*/
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
