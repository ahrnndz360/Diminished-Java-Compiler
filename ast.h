/* File ast.h: Abstract-syntax-tree data structure for DJ */

#ifndef AST_H
#define AST_H

#include <stdlib.h>

/* define types of AST nodes */
typedef enum {
  /* program, class, field, method, and parameter declarations: */
  PROGRAM, 
  CLASS_DECL_LIST,
  CLASS_DECL,
  VAR_DECL_LIST,
  VAR_DECL,   
  METHOD_DECL_LIST,
  METHOD_DECL, 
  PARAM_DECL_LIST,
  PARAM_DECL, 
  /* types, including generic IDs: */
  NAT_TYPE, 
  AST_ID, 
  /* expression-lists: */
  EXPR_LIST,
  /* expressions: */
  DOT_METHOD_CALL_EXPR, /* E.ID(As) */
  METHOD_CALL_EXPR,     /* ID(As) 		DONE*/
  DOT_ID_EXPR,          /* E.ID 		DONE*/
  ID_EXPR,              /* ID 			DONE*/
  DOT_ASSIGN_EXPR,      /* E.ID = E		DONE */
  ASSIGN_EXPR,          /* ID = E 		DONE*/
  PLUS_EXPR,            /* E + E 		DONE*/
  MINUS_EXPR,           /* E - E 		DONE*/
  TIMES_EXPR,           /* E * E 		DONE*/
  EQUALITY_EXPR,        /* E==E 		DONE*/
  LESS_THAN_EXPR,       /* E < E 		DONE*/
  NOT_EXPR,             /* !E 			DONE*/
  AND_EXPR,              /* E&&E 		DONE*/
  IF_THEN_ELSE_EXPR,    /* if(E) {Es} else {Es} DONE*/
  WHILE_EXPR,           /* while(E) {Es} 	DONE*/
  PRINT_EXPR,           /* printNat(E) 		DONE*/
  READ_EXPR,            /* readNat() 		DONE*/
  THIS_EXPR,            /* this			DONE */
  NEW_EXPR,             /* new 			DONE*/
  NULL_EXPR,            /* null 		DONE*/
  NAT_LITERAL_EXPR,     /* N 			DONE*/
  /* argument-lists: */
  ARG_LIST,
} ASTNodeType;

/* define a list of AST nodes */
typedef struct astlistnode {
  struct astnode *data;
  struct astlistnode *next;
} ASTList;

/* define the actual AST nodes */
typedef struct astnode {
  ASTNodeType typ;
  /* list of children nodes: */
  ASTList *children; /* head of the list of children */
  ASTList *childrenTail;
  /* which source-program line does this node end on: */
  unsigned int lineNumber;
  /* node attributes: */
  unsigned int natVal;
  char *idVal;
  /* The following node attributes are used on the first 6 kinds of expressions
     enumerated above (E.ID(As), ID(As), E.ID, ID, E.ID = E, and ID = E).
     These attributes get set during type checking and used during code gen,
     so code gen doesn't duplicate the work of the type checker.
     The attributes store the statically determined class and member number
     of an ID that refers to a class method or class variable.
     When these attributes are both 0, the ID refers not to a member of a class
     but instead to a local/parameter variable. */
  unsigned int staticClassNum; /* class number in which this member resides */
  unsigned int staticMemberNum; /* when set to i, this member is the ith
                                   method/var in the staticClassNum-th class */
} ASTree;


/* METHODS TO CREATE AND MANIPULATE THE AST */

/* Create a new AST node of type t having only one child.
   (That is, create a one-node list of children for the new tree.)
   If the child argument is NULL, then the single list node in the 
   new AST node's list of children will have a NULL data field.
   If t is NAT_LITERAL_EXPR then the proper natAttribute should be
   given; otherwise natAttribute is ignored.
   If t is AST_ID then the proper idAttribute should be given;
   otherwise idAttribute is ignored.
*/
ASTree *newAST(ASTNodeType t, ASTree *child, unsigned int natAttribute, 
  char *idAttribute, unsigned int lineNum);

/* Append an AST node onto a parent's list of children */
void appendToChildrenList(ASTree *parent, ASTree *newChild);

/* Print the AST to stdout with indentations marking tree depth. */
void printAST(ASTree *t);

#endif



































