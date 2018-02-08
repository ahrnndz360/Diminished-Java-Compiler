/* 
 File typecheck.c
 author - Anthony Hernandez
 "I pledge my Honor that I have not cheated, and will not cheat, on this assignment." - Anthony Hernandez
 Implementation of level 2 typechecker for DJ 
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "typecheck.h"

/*****************************************Begin extra helper functions*********************************************/
/*    Extra helper functions that are not given in typechecker.h These helper 
    functions are used in a level 2 implementation of  As5 however some are
    written as if they are for a level 3 implementation of As5, e.g. FindVarName()
*/

//Helper functions for explaining errors encountered 
void HandleInternalError(){
  printf("Internal error, NULL dereference\n");
  exit(-1);
}

void HandleExternalError(char *errorMsg, int lineNum){
  printf("Semantic error on line %d:\n", lineNum);
  printf(errorMsg);
  exit(-1);
}

//Checks to see if types in the ST are valid, currently only handles MainBlockLocals
void ValidateTables(){
  int i = 0;
  for(i; i<numMainBlockLocals; i++){
    if(mainBlockST[i].type < -1){
      printf("Invalid variable type on line %d:\n", mainBlockST[i].typeLineNumber);
      exit(-1);
    } 
  }
}


//Searches for a class name 
//Returns its type if cound, else return -4(invalid type)
int FindClassName(char *id){
  int i = 0;
  for(i; i<numClasses; i++){
  	int val = strcmp(id, classesST[i].className);
    if(strcmp(id, classesST[i].className) == 0) {return i;}
  }
  return -4;
}


//Searches for a Variable name in the given context
//Returns type of variable if found, else return -4 (invalid type)
int FindVarName(char *id, int class, int method){
  int i = 0;
  int type = -4;
  //If the variable is being searched in MainBlock locals
  if (class == -1){
    for(i; i<numMainBlockLocals; i++){
      if(strcmp(id, mainBlockST[i].varName) == 0){
        return mainBlockST[i].type;
      }
    }
  }
  //If the variable is being searched in a class; assumes class and method context given exists
  //Partial implementation for level3; will suffice for level 2 implementation
  else{
    if(classesST[class].numVars == 0){return -4;}
    else{
      for(i; i<classesST[class].numVars; i++){
        if(strcmp(id, classesST[class].varList[i].varName) == 0){
          return classesST[class].varList[i].type; 
        }
      }
    }
  }
  return -4;  // the variable was never found
}


//Checks to see if locals in the MainBlock are unique
//Uses double for loop for simplicty, results in O(n^2) can be faster if needed
void CheckUniquenessMain(){
  int i = 0;
  for(i; i < numMainBlockLocals-1; i++){
  	 int j = i+1;
    for(j; j<numMainBlockLocals; j++){
      char *str1 = mainBlockST[i].varName;
      char* str2 = mainBlockST[j].varName;
      if (strcmp(str1, str2) == 0){
        printf("Variable declared multiple times on lines %d and %d\n", 
          mainBlockST[i].typeLineNumber, mainBlockST[j].typeLineNumber);  
        exit(-1); 
      }
    }
  }
}

//Finds the join of 2 given classes. Assumes classes are joinable
//returns the type of the join
int FindJoin(int c1, int c2){
  if (isSubtype(c1, c2)) {return c2;}
  else if (isSubtype(c2, c1)) {return c1;}
  else { return FindJoin(classesST[c1].superclass, c2);}
}


/* Returns nonzero iff sub is a subtype of super */
int isSubtype(int sub, int super){
  if (sub == -1 && super == -1) {return 1;}
  else if(sub == super){return 1;}
  else{
    if(sub == -2 && (super >=0 || super == -2)) {return 1;} //handle null subtyping
    else if(sub >=0 && super >=0) {return isSubtype(classesST[sub].superclass, super);} //handle typical subtyping
    else {return 0;} 
  }
}
/********************************End extra helper functions*********************************************************/



/********************************Given function defititions*********************************************************/
//Typechecks a DJ program. Written for level 2 implementation 
void typecheckProgram(){
  //validate types in ST
  ValidateTables();
  //check local names are unique in MainBlock 
  if (numMainBlockLocals > 1){CheckUniquenessMain();}
  //TC ExprBlock in Main
  typeExprs(mainExprs, -1, -1);
}

/* Returns the type of the EXPR_LIST AST in the given context. */
int typeExprs(ASTree *t, int classContainingExprs, int methodContainingExprs){
  if (t->typ!=EXPR_LIST){
    printf("AST node is not an expression list\n");
      exit(-1);	
  }
  else{ //get list of children of EXPR_LIST
    ASTList *exprList = t->children;
    while(exprList->next != NULL){ //iterate over and TC each expr in the 
      //exprlist end loop return type of last expr 
      int typ = typeExpr(exprList->data, classContainingExprs, methodContainingExprs);
      if (typ < -2){HandleExternalError("invalid type encountered in body of while loop\n", t->lineNumber);} 
      //handle values for typ(different types for exprs)
      exprList = exprList->next;		
    }
    //handle last child in exprList final type of exprList is type of this
    int typ = typeExpr(exprList->data, classContainingExprs, methodContainingExprs); 
    return typ;
  }
}


//Typechecks DJ exprs
//Returns type of expr if well typed, else prints error information and exits
//Expressions involving method calls are automatically considered semantically
//due to the assumption that no classes have been declared and Object has no fields
int typeExpr(ASTree *t, int classContainingExpr, int methodContainingExpr){
  //current implementation assumes no classes are declared thus 2nd and 3rd paramaters are always -1
  if (t == NULL){
    HandleInternalError(); // helper function to handle NULL pointer where there shouldn't be one 
  }
  else{
    if(t->typ == NAT_LITERAL_EXPR) {return -1;}
    else if(t->typ == NULL_EXPR) {return -2;}
    else if(t->typ == THIS_EXPR) {HandleExternalError("reference to 'this' outside of class\n", t->lineNumber);} //for level 2 implementation, this expr will only be found MainBlock
    else if(t->typ == PLUS_EXPR) {
       int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
       int t2 = typeExpr(t->childrenTail->data, classContainingExpr, methodContainingExpr);
       if(t1 == -1 && t2 == -1) {return -1;}
       else {HandleExternalError("Non-nat type encountered in '+' expression\n", t->lineNumber);} 
    }
    else if(t->typ == MINUS_EXPR){
       int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
       int t2 = typeExpr(t->childrenTail->data, classContainingExpr, methodContainingExpr);
       if(t1 == -1 && t2 == -1){return -1;}
       else{HandleExternalError("Non-nat type encountered in '-' expression\n", t->lineNumber);}
    }
    else if(t->typ == TIMES_EXPR){
      int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
      int t2 = typeExpr(t->childrenTail->data, classContainingExpr, methodContainingExpr);
      if(t1 == -1 && t2 == -1) {return -1;}
      else {HandleExternalError("Non-nat type encountered in '*' expression\n", t->lineNumber);}
    }
    else if(t->typ == EQUALITY_EXPR){
      int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
      int t2 = typeExpr(t->childrenTail->data, classContainingExpr, methodContainingExpr);
      if(isSubtype(t1,t2)) {return t2;}
      else if(isSubtype(t2,t1)) {return t1;}
      else {HandleExternalError("Type mismatch in '==' expression (subtype not found)\n", t->lineNumber);}
    }
    else if(t->typ == LESS_THAN_EXPR){
      int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
      int t2 = typeExpr(t->childrenTail->data, classContainingExpr, methodContainingExpr);
     //if(isSubtype(t1,t2)) {return t2;}
      //else if(isSubtype(t2,t1)) {return t1;}
      if(t1 == -1 && t2 == -1){return -1;}
      //else{HandleExternalError("Type mismatch in '<' expression (subtype not found)\n", t->lineNumber);}
      else{HandleExternalError("Non-nat type in '<' expression\n", t->lineNumber);}
    }
    else if (t->typ == NOT_EXPR) {
      int subT = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
  	  if (subT == -1) {return -1;}
      else {HandleExternalError("Non-nat type ecountered in '!' expression\n", t->lineNumber);}
    }
    else if(t->typ == AND_EXPR){
      int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
      int t2 = typeExpr(t->childrenTail->data, classContainingExpr, methodContainingExpr);
      //if(isSubtype(t1,t2)) {return t2;}
      //else if(isSubtype(t2,t1)) {return t1;}
      if(t1 == -1 && t2 == -1){return -1;}
      //else{HandleExternalError("Type mismatch in '&&' expression (subtype not found)\n", t->lineNumber);}
      else{HandleExternalError("Non-nat type in '&&' expression\n", t->lineNumber);}
    }
    else if(t->typ == IF_THEN_ELSE_EXPR){
      int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
      int tThen = typeExprs(t->children->next->data, classContainingExpr, methodContainingExpr);
      int tElse = typeExprs(t->childrenTail->data, classContainingExpr, methodContainingExpr);
      if (t1 == -1){
        if(tThen  == -1 && tElse == -1) {return -1;}
        else if ((tThen >=0 || tThen == -2) && (tElse >=0|| tThen == -2)) {return FindJoin(tThen, tElse);}
        else {HandleExternalError("Type mismatch between the bodies of if-then-else expression\n", t->lineNumber);}
      }
      else {HandleExternalError("Non-nat type encountered in conditional of if-then-else expression\n", t->lineNumber);}
    }
    else if(t->typ == WHILE_EXPR){
      int tTest = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
      int tBody = typeExprs(t->childrenTail->data, classContainingExpr, methodContainingExpr);
      
      if(tTest == -1){
        if(tBody >=-2) {return -1;}
        else {HandleExternalError("Invalid type encountered body of while loop\n", t->lineNumber);}
      }
      else{HandleExternalError("Non-nat type encountered in conditional of while loop\n", t->lineNumber);}
    }
    else if(t->typ == NEW_EXPR){
      char *id = t->children->data->idVal;
      int t1 = FindClassName(id);
      if(t1>=0) {return t1;}
      else {HandleExternalError("Undeclared Class\n", t->lineNumber);}
     } 
    else if(t->typ == PRINT_EXPR){
      int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
      if(t1 == -1){return -1;}
      else{HandleExternalError("Non-nat type ecnountered in printNat\n", t->lineNumber);}
    }
    else if(t->typ == READ_EXPR){
      return -1;
    }
    else if(t->typ == ID_EXPR){
      int t1 = FindVarName(t->children->data->idVal, classContainingExpr, methodContainingExpr);
        if(t1>=-2){return t1;}
        else{HandleExternalError("Undeclared identifier\n", t->lineNumber);}
    }
    else if(t->typ == DOT_ID_EXPR){
      int tClass = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
      if (tClass >= 0){
        //Search for variable in class# classType level 2 implentation will always result in semantic error
        int idType = FindVarName(t->childrenTail->data->idVal, tClass, methodContainingExpr);
        if (idType == -4){HandleExternalError("Undeclared identifier in '.' expression\n", t->lineNumber);}
        else {return idType;}
     }
     else{HandleExternalError("Undeclared class in '.' expression", t->lineNumber);}
    }
    else if(t->typ == ASSIGN_EXPR){
      int tVar = FindVarName(t->children->data->idVal, classContainingExpr, methodContainingExpr);
      if (tVar == -4) {HandleExternalError("Undeclared identifier in LHS of assignment\n", t->lineNumber);}
      else{
        int tExpr = typeExpr(t->childrenTail->data, classContainingExpr, methodContainingExpr);
        if(isSubtype(tExpr, tVar)) {return tVar;}
        else {HandleExternalError("Invalid type encountered in RHS of assignment (subtype not found)\n", t->lineNumber);}
      }
    }
    else if(t->typ == DOT_ASSIGN_EXPR){
      int tClass = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
      if (tClass >= 0){
        //Search for variable in class# tClass level 2 implementation will always result in semantic error
        int tVar = FindVarName(t->children->next->data->idVal, tClass, methodContainingExpr);
        if (tVar == -4){HandleExternalError("Undeclared identifier in LHS dotted assignment\n", t->lineNumber);}
        else{
          int tExpr = typeExpr(t->childrenTail->data, classContainingExpr, methodContainingExpr);
          if(isSubtype(tExpr, tVar)) {return tVar;}
          else{HandleExternalError("Invalid type encountered in RHS of dotted assignment (subtype not found)\n", t->lineNumber);} 
        }
      }
      else {HandleExternalError("Undeclared class\n", t->lineNumber);} 
    }                             
    else{HandleInternalError("Invalid expression encountered\n", t->lineNumber);} 
  }//end typying exprs
}//end function

