/* 
 File codegen.c
 author - Anthony Hernandez
 "I pledge my Honor that I have not cheated, and will not cheat, on this assignment." - Anthony Hernandez
 Implementation of level 2 DISM code generator for DJ 

*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "typecheck.h"

#define MAX_DISM_ADDR 65535

/* Global for the DISM output file */
FILE *fout;

/* Global to remember the next unique label number to use */
unsigned int labelNumber = 0;

/* Declare mutually recursive functions (defs and docs appear below) */
void codeGenExpr(ASTree *t, int classNumber, int methodNumber);
void codeGenExprs(ASTree *expList, int classNumber, int methodNumber);


/* Print a message and exit under an exceptional condition */
void internalCGerror(char *msg){
  printf("InternalError: %s", msg);
  exit(-1);
};

/* Using the global classesST, calculate the total number of fields,
 including inherited fields, in an object of the given type */
int getNumObjectFields(int type);

/* Generate code that increments the stack pointer */
void incSP(){
  fprintf(fout, "mov 1 1\n");
  fprintf(fout, "add 6 6 1 ;SP++\n");
};

/* Generate code that decrements the stack pointer */
void decSP(){
  fprintf(fout, "mov 1 1\n");
  fprintf(fout, "sub 6 6 1  ;SP --\n");
  fprintf(fout, "blt 5 6 #%d\n", labelNumber);
  fprintf(fout, "mov 1 77\n");
  fprintf(fout, "hlt 1\n");
  fprintf(fout, "#%d:  mov 0 0\n", labelNumber);
  labelNumber++;
};



/* Output code to check for a null value at the top of the stack.
 If the top stack value (at M[SP+1]) is null (0), the DISM code
 output will halt. */
void checkNullDereference(){
  int error = labelNumber++;
  int end = labelNumber++;
  fprintf(fout, "lod 1 6 1 ;SP --\n");
  fprintf(fout, "beq 0 1 #%d\n", error);  //jump to handle error
  fprintf(fout, "jmp 0 #%d\n", end);  //stepthrough to a jump to next part of code
  fprintf(fout, "#%d: mov 1 88\n", error);//handle error of derefrence 
  fprintf(fout, "hlt 1  ;halted on error code\n");
  fprintf(fout, "#%d: mov 0 0\n", end);
};

//Searches for a class index
//Returns its type/index
int FindClassIndex(char *id){
  int i = 0;
  for(i; i<numClasses; i++){
  	int val = strcmp(id, classesST[i].className);
    if(strcmp(id, classesST[i].className) == 0) {return i;}
  }
  return -4;
}

//Searches for a Variable name in the given context
//Returns offset of var
int FindVarOffset(char *id, int class, int method){
  int i = 0;
  //If the variable is being searched in MainBlock locals
  if (class < 0){
    for(i; i<numMainBlockLocals; i++){
      if(strcmp(id, mainBlockST[i].varName) == 0){
        return i;
      }
    }
  }
  //If the variable is being searched in a class; assumes class and method context given exists
  //Partial implementation for level3; will suffice for level 2 implementation
  else{
    if(classesST[class].numVars == 0){return -4;}
    else{
      for(i; i<classesST[class].numVars; i++) {
        if(strcmp(id, classesST[class].varList[i].varName) == 0){
          return i; 
        }
      }
    }
  }
  return -4;  // the variable was never found
}


/* Generate DISM code for the given single expression, which appears
 in the given class and method (or main block).
 If classNumber < 0 then methodNumber may be anything and we assume
 we are generating code for the program's main block. */
void codeGenExpr(ASTree *t, int classNumber, int methodNumber){
   if (t == NULL){
    internalCGerror("NULL Derefence during codegen expr");
  }
  else{
  if(t->typ == NAT_LITERAL_EXPR){
    fprintf(fout, "mov 1 %d \n", t->natVal);
    fprintf(fout, "str 6 0 1 ;M[SP] <-R[r1] (a nat literal)\n");
    decSP();	
  }
  else if(t->typ == NULL_EXPR){
    fprintf(fout, "str 6 0 0\n");
    decSP();
  }
  else if(t->typ == PLUS_EXPR){
    codeGenExpr(t->children->data, classNumber, methodNumber);
    codeGenExpr(t->children->next->data, classNumber, methodNumber);
    fprintf(fout, "lod 1 6 2\n");
    fprintf(fout, "lod 2 6 1\n");
    fprintf(fout, "add 1 1 2\n");
    fprintf(fout, "str 6 2 1\n");
    incSP();  
  }
  else if(t->typ == MINUS_EXPR){
    codeGenExpr(t->children->data, classNumber, methodNumber);
    codeGenExpr(t->children->next->data, classNumber, methodNumber);
    fprintf(fout, "lod 1 6 2\n");
    fprintf(fout, "lod 2 6 1\n");
    fprintf(fout, "sub 1 1 2\n");
    fprintf(fout, "str 6 2 1\n");
    incSP();  
  }
  else if(t->typ == TIMES_EXPR){
    codeGenExpr(t->children->data, classNumber, methodNumber);
    codeGenExpr(t->children->next->data, classNumber, methodNumber);
    fprintf(fout, "lod 1 6 2\n");
    fprintf(fout, "lod 2 6 1\n");
    fprintf(fout, "mul 1 1 2\n");
    fprintf(fout, "str 6 2 1\n");
    incSP();  
  }
//increment may be wrong in str for EQUALITY EXPR
  else if(t->typ == EQUALITY_EXPR){
    int isEqual = labelNumber++;
    int contin = labelNumber++;
    codeGenExpr(t->children->data, classNumber, methodNumber);
    codeGenExpr(t->children->next->data, classNumber, methodNumber);
    fprintf(fout, "lod 1 6 2\n");
    fprintf(fout, "lod 2 6 1\n");
    fprintf(fout, "beq 1 2 #%d\n", isEqual);
    fprintf(fout, "str 6 2 0\n");
    fprintf(fout, "jmp 0 #%d", contin);
    fprintf(fout, "#%d:  mov 1 1\n", isEqual);
    fprintf(fout, "str 6 2 1\n");
    fprintf(fout, "#%d:  mov 0 0\n", contin); 
    incSP();  
  }
  else if(t->typ == NOT_EXPR){
    int isZero = labelNumber++;
    int contin = labelNumber++; 
    codeGenExpr(t->children->data, classNumber, methodNumber);
    fprintf(fout, "lod 1 6 1\n");
    fprintf(fout, "beq 0 1 #%d\n", isZero);
    fprintf(fout, "str 6 1 0\n");  //store 0 if non-zero
    fprintf(fout, "jmp 0 #%d\n", contin);
    fprintf(fout, "#%d:  mov 1 1\n", isZero);
    fprintf(fout, "str 6 1 1\n"); //store 1 if 0
    fprintf(fout, "#%d:  mov 0 0\n", contin);
  }
  else if(t->typ == LESS_THAN_EXPR){
    int isLess = labelNumber++;
    int end = labelNumber++;
    codeGenExpr(t->children->data, classNumber, methodNumber);
    codeGenExpr(t->children->next->data, classNumber, methodNumber);
    fprintf(fout, "lod 1 6 2\n");
    fprintf(fout, "lod 2 6 1\n");
    fprintf(fout, "blt 1 2 #%d\n", isLess);
    fprintf(fout, "str 6 2 0\n");
    fprintf(fout, "jmp 0 #%d\n", end);    
    fprintf(fout, "#%d:  mov 1 1\n", isLess);
    fprintf(fout, "str 6 2 1\n");
    fprintf(fout, "#%d: mov 0 0\n", end);
    incSP();
  }
  else if(t->typ == AND_EXPR){
    int isFalse = labelNumber++;
    int contin = labelNumber++;
    codeGenExpr(t->children->data, classNumber, methodNumber);
    fprintf(fout, "lod 1 6 1  ;Begin AND_EXPR\n");
    fprintf(fout, "beq 1 0 #%d  ;check left side truth\n", isFalse);
    codeGenExpr(t->children->next->data, classNumber, methodNumber);
    incSP();
    fprintf(fout, "lod 1 6 0\n");
    fprintf(fout, "beq 1 0 #%d  ;check right-side truth\n", isFalse);
    fprintf(fout, "mov 1 1\n");
    fprintf(fout, "str 6 1 1\n");
    fprintf(fout, "jmp 0 #%d\n", contin);
    fprintf(fout, "#%d: str 6 1 0\n", isFalse);
    fprintf(fout, "#%d: mov 0 0  ;End AND_EXPR\n", contin);
  }
  else if(t->typ == IF_THEN_ELSE_EXPR) {
    int elseLabel = labelNumber++;
    int endLabel = labelNumber++;
    codeGenExpr(t->children->data, classNumber, methodNumber);
    fprintf(fout, "lod 1 6 1  ;IF_THEN_ELSE\n");
    fprintf(fout, "beq 1 0 #%d  ;check test | below is then body\n", elseLabel);
    incSP();
    codeGenExprs(t->children->next->data, classNumber, methodNumber);
    fprintf(fout, "jmp 0 #%d\n", endLabel);
    fprintf(fout, "#%d:  mov 0 0  ;below is else body\n", elseLabel);
    incSP();
    codeGenExprs(t->childrenTail->data, classNumber, methodNumber);
    fprintf(fout, "#%d:  mov 0 0  ;End of IF_THEN_ELSE\n", endLabel);
    }
 //Both the test condition and exp-list temporaries are written to the same addr
  else if(t->typ == WHILE_EXPR){
   int startBody = labelNumber++;
   fprintf(fout, "#%d: mov 0 0\n", startBody);
   codeGenExprs(t->childrenTail->data, classNumber, methodNumber);
   incSP();
   codeGenExpr(t->children->data, classNumber, methodNumber); // rewrites the temporary made by the expr-list
   incSP();
   fprintf(fout, "lod 1 6 0\n");
   fprintf(fout, "mov 2 1\n");
   fprintf(fout, "beq 1 2 #%d\n", startBody);
   fprintf(fout, "str 6 0 0\n");
   decSP();
  }
  else if(t->typ == PRINT_EXPR){
     codeGenExpr(t->children->data, classNumber, methodNumber);
     fprintf(fout, "lod 1 6 1\n");
     fprintf(fout, "ptn 1 ;print nat number\n");
  }
  else if(t->typ == READ_EXPR){
    fprintf(fout, "rdn 1 ;read nat number\n");
    fprintf(fout, "str 6 0 1\n");
    decSP();
  }
  else if(t->typ == NEW_EXPR){
    int freeSpace = labelNumber++;
    int tNum = FindClassIndex(t->children->data->idVal);
    int n = 0; // for level 2, implmentation the only available class will have 0 fields. 
    //(For level 3, use getNumObjFields helper to traverse inheritance tree summing fields)
    // check if HP+n+1 < MAX_DISM_ADDR
    fprintf(fout, "mov 1 %d  ;checking available space for object\n", MAX_DISM_ADDR);
    fprintf(fout, "mov 2 1  \n");
    fprintf(fout, "add 2 2 5\n");
    fprintf(fout, "blt 2 1 %d  ;R[2] =Hp+n+1 | R[1] = MAX_DISM_ADDR\n", freeSpace);
    fprintf(fout, "mov 1 66\n");
    fprintf(fout, "hlt 1 ; error code 66, object too large\n");
    fprintf(fout, "#%d: mov 1 1  ;free space is available\n", freeSpace);
    //store field values
    if(n > 0){
      int i = 0;
      for(i; i<n; i++){
          fprintf(fout, "str 5 0 0\n");
          fprintf(fout, "add 5 5 1 \n");
      }
    }
    fprintf(fout, "mov 2 %d\n", tNum);
    fprintf(fout, "str 5 0 2  ;Store at Hp, Obj number\n");
    fprintf(fout, "str 6 0 5  ;Store at Sp, Obj addr in Heap\n");
    fprintf(fout, "add 5 5 1  ;inc Hp\n");
    decSP(); //check for heap/stack clash is done here, not when adding fields for the Obj    
  }
  else if(t->typ == ID_EXPR){
    int offset = FindVarOffset(t->children->data->idVal, classNumber, methodNumber);
    fprintf(fout, "mov 2 %d\n", MAX_DISM_ADDR);
    fprintf(fout, "mov 3 %d\n", offset);
    fprintf(fout, "sub 2 2 3  ;calculate offset\n");
    fprintf(fout, "lod 1 2 0  ;load rval of ID\n");
    fprintf(fout, "str 6 0 1  ;store rval into SP\n");
    decSP();
  }
 else if(t->typ == DOT_ID_EXPR){
   //Level 2 typechecker will halt compilation on this expr. This expr
   //Shouldn't be encountered during codeGen
   internalCGerror("DOT_ID_EXPR encountered during codeGen");
 }
 else if(t->typ == ASSIGN_EXPR){
   codeGenExpr(t->childrenTail->data, classNumber, methodNumber);
   int offset = FindVarOffset(t->children->data->idVal, classNumber, methodNumber);
   fprintf(fout, "lod 1 6 1  ;load results of RHS\n");
   fprintf(fout, "mov 2 %d\n", MAX_DISM_ADDR);
   fprintf(fout, "mov 3 %d\n", offset);
   fprintf(fout, "sub 2 2 3  ;calculate offset\n");
   fprintf(fout, "str 2 0 1  ;str results of RHS into addr of LHS\n");
   fprintf(fout, "lod 1 2 0  ;load addr of ID\n");
   fprintf(fout, "str 1 1 6  ;str addr of ID in SP\n", offset);
 } 
 else if(t->typ == DOT_ASSIGN_EXPR){
   //Level 2 typechecker will halt compilation on this expr. This expr
   //Shouldn't be encountered during codeGen
   internalCGerror("DOT_ID_ASSIGN_EXPR encountered during codeGen");
 }
 }
  
};


/* Generate DISM code for an expression list, which appears in
 the given class and method (or main block).
 If classNumber < 0 then methodNumber may be anything and we assume
 we are generating code for the program's main block. */
void codeGenExprs(ASTree *expList, int classNumber, int methodNumber){
  if (expList->typ !=EXPR_LIST){
    printf("AST node is not an expression list\n");//change to handle error
    exit(-1);	
  }
  else{
    fprintf(fout, "mov 0 0 ;start of expr-list\n"); 
    ASTList *exprList = expList->children;
    while(exprList->next != NULL){ //iterate over and CG each expr in the expr-list 
      codeGenExpr(exprList->data, classNumber, methodNumber); 
      incSP();
      exprList = exprList->next;		
    }
    //temporary val of expr-list is temporary val of last expr
    codeGenExpr(exprList->data, classNumber, methodNumber);
     fprintf(fout, "mov 0 0 ;end of expr-list\n"); 
  } 
};



/* Generate DISM code as the prologue to the given method or main
 block. If classNumber < 0 then methodNumber may be anything and we
 assume we are generating code for the program's main block. */
void genPrologue(int classNumber, int methodNumber){
  //Mainblock Prologue 
  if(classNumber < 0){
      fprintf(fout, "mov 0 0  ;Generating Mainblock prologue\n");
      fprintf(fout, "mov 7 %d  ;init SP\n", MAX_DISM_ADDR);
      fprintf(fout, "mov 6 %d  ;init FP\n", MAX_DISM_ADDR);
      fprintf(fout, "mov 5 1   ;init HP\n", MAX_DISM_ADDR);
      int i = 0;
      for(i; i<numMainBlockLocals; i++){
         fprintf(fout, "str 6 0 0 ;local stored\n");
         decSP();
      }

  }
};

/* Generate DISM code as the epilogue to the given method or main
 block. If classNumber < 0 then methodNumber may be anything and we
 assume we are generating code for the program's main block. */
void genEpilogue(int classNumber, int methodNumber){
  if(classNumber < 0){
    fprintf(fout, "mov 1 1  ;Generating Mainblock epilogue\n");
    fprintf(fout, "hlt 1\n");
  }
};

/* Generate DISM code for the given method or main block.
 If classNumber < 0 then methodNumber may be anything and we assume
 we are generating code for the program's main block. */
void genBody(int classNumber, int methodNumber){
  if(classNumber < 0){
    fprintf(fout, "mov 0 0  ;Generating Mainblock code\n");
    codeGenExprs(mainExprs, classNumber, methodNumber);
  }
};

/* Map a given (1) static class number, (2) a method number defined
 in that class, and (3) a dynamic object's type to:
 (a) the dynamic class number and (b) the dynamic method number that
 actually get called when an object of type (3) dynamically invokes
 method (2).
 This method assumes that dynamicType is a subtype of staticClass. */
void getDynamicMethodInfo(int staticClass, int staticMethod,
 int dynamicType, int *dynamicClassToCall, int *dynamicMethodToCall);

/* Emit code for the program's vtable, beginning at label #VTABLE.
 The vtable jumps (i.e., dispatches) to code based on
 (1) the number of arguments on the stack (at M[SP+1]),
 (2) the dynamic calling object's address (at M[SP+4+numArguments]),
 (3) the calling object's static type (at M[SP+3+numArguments]), and
 (4) the static method number (at M[SP+2+numArguments]). */
void genVTable();



void generateDISM(FILE *outputFile){
  fout = outputFile;
  genPrologue(-1, -1);
  genBody(-1, -1);
  genEpilogue(-1, -1);
}








