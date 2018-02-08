/* 
   File ast.c
   author - Anthony Hernandez
   "I pledge my Honor that I have not cheated, and will not cheat, on this assignment." - Anthony Hernandez
   Implementation for DJ Parser, constructs AST but currently does not type-check
*/

%code provides {
  #include <stdio.h>
  #include "lex.yy.c"
  #include "ast.h"
  #include "symtbl.h"
  #include "typecheck.h"
  #include "codegen.h" 

  /* Symbols in this grammar are represented as ASTs */ 
  #define YYSTYPE ASTree *

  /* Declare global AST for entire program */
  ASTree *pgmAST;

  /* Function for printing generic syntax-error messages */
  void yyerror(const char *str) {
    printf("Syntax error on line %d at token %s\n",yylineno,yytext);
    printf("(This version of the compiler exits after finding the first ");
    printf("syntax error.)\n");
    exit(-1);
  }

}

%token MAIN CLASS EXTENDS NATTYPE IF ELSE WHILE
%token PRINTNAT READNAT THIS NEW NUL NATLITERAL 
%token ID ASSIGN PLUS MINUS TIMES EQUALITY LESS
%token AND NOT DOT SEMICOLON COMMA LBRACE RBRACE 
%token LPAREN RPAREN ENDOFFILE

%start pgm

%right ASSIGN
%left AND
%nonassoc EQUALITY LESS
%left PLUS MINUS
%left TIMES
%right NOT
%left DOT

%%

//May need to modify this grammar 
pgm : Class_Decl_List MAIN LBRACE Variable_Decl_List Expr_List RBRACE ENDOFFILE 
       { pgmAST = newAST(PROGRAM, $1, 0, NULL, yylineno); appendToChildrenList(pgmAST, $4); 
 	 appendToChildrenList(pgmAST, $5); return 0;}
    ;
//Generic ID and NAT type
Ast_Id: ID {$$ = newAST(AST_ID, NULL, 0, yytext, yylineno);};

Nat_Type: NATTYPE {$$ = newAST(NAT_TYPE, NULL, 0, NULL, yylineno);};


/*productions for classes in Dj (excluding expressions)*/

Class_Decl_List: Class_Decl_List Class_Decl 
	          { appendToChildrenList($1, $2);}
	       | 
	          {$$ = newAST(CLASS_DECL_LIST, NULL, 0, NULL, yylineno);}
	       ;
Class_Decl: CLASS Ast_Id EXTENDS Ast_Id LBRACE Variable_Decl_List Method_Decl_List RBRACE
	     { $$ = newAST(CLASS_DECL, $2, 0, NULL, yylineno); appendToChildrenList($$, $4);
	      appendToChildrenList($$, $6); appendToChildrenList($$, $7);}
	  | CLASS Ast_Id EXTENDS Ast_Id LBRACE Variable_Decl_List RBRACE	//Empty Method_Decl_Seq;
	     { $$ = newAST(CLASS_DECL, $2, 0, NULL, yylineno); appendToChildrenList($$, $4);
	      appendToChildrenList($$, $6); appendToChildrenList($$, newAST(METHOD_DECL_LIST, NULL, 0, NULL, yylineno));}
	  ;

/*Sequences for Classes*/

Variable_Decl_List: Variable_Decl_List  Variable_Decl  
		     {appendToChildrenList($1, $2);}
		  |
		     {$$= newAST(VAR_DECL_LIST, NULL, 0, NULL, yylineno);}
		  ;

Variable_Decl: Ast_Id Ast_Id SEMICOLON
	        {$$ = newAST(VAR_DECL, $1, 0, NULL, yylineno); appendToChildrenList($$, $2);}
	     | Nat_Type Ast_Id SEMICOLON
	        {$$ = newAST(VAR_DECL, $1, 0, NULL, yylineno); appendToChildrenList($$, $2);}
	     ;

Method_Decl_List: Method_Decl_List Method_Decl
		   { appendToChildrenList($1, $2);}
		| Method_Decl	
		   {$$ = newAST(METHOD_DECL_LIST, $1, 0, NULL, yylineno);}	
		;

Method_Decl: Ast_Id Ast_Id LPAREN Parameter_Decl_List RPAREN LBRACE Variable_Decl_List Expr_List RBRACE
	      {$$ = newAST(METHOD_DECL, $1, 0, NULL, yylineno);  appendToChildrenList($$, $2); 
	        appendToChildrenList($$, $4);  appendToChildrenList($$, $7);  appendToChildrenList($$,$8);}
	   | Nat_Type Ast_Id LPAREN Parameter_Decl_List RPAREN LBRACE Variable_Decl_List Expr_List RBRACE
	      {$$ = newAST(METHOD_DECL, $1, 0, NULL, yylineno);  appendToChildrenList($$, $2); 
	        appendToChildrenList($$, $4);  appendToChildrenList($$, $7);  appendToChildrenList($$,$8);}
	   ;

Parameter_Decl_List: Parameter_Decl_List Parameter_Decl
		     {appendToChildrenList($1, $2);}
	           |
		     {$$ = newAST(PARAM_DECL_LIST, NULL, 0, NULL, yylineno);}
		   ;

Parameter_Decl: Ast_Id Ast_Id
	         {$$ = newAST(PARAM_DECL, $1, 0, NULL, yylineno); appendToChildrenList($$, $2);}  
	      | Ast_Id Ast_Id COMMA
		 {$$ = newAST(PARAM_DECL, $1, 0, NULL, yylineno); appendToChildrenList($$, $2);}
	      | Nat_Type Ast_Id COMMA
		 {$$ = newAST(PARAM_DECL, $1, 0, NULL, yylineno); appendToChildrenList($$, $2);}
	      | Nat_Type Ast_Id
		 {$$ = newAST(PARAM_DECL, $1, 0, NULL, yylineno); appendToChildrenList($$, $2);}
	      ; 




/*Productions for expressions*/
Expr_List: Expr_List Expr SEMICOLON
	    {appendToChildrenList($1, $2);}
	 | Expr SEMICOLON
	    {$$ = newAST(EXPR_LIST, $1, 0, NULL,  yylineno);}
	 ;

Expr: NUL
       {$$ = newAST(NULL_EXPR, NULL, 0, NULL, yylineno);}

    | THIS
      {$$ = newAST(THIS_EXPR, NULL, 0, NULL, yylineno);}

    | NATLITERAL //double check on getting this attribute value
	{$$ = newAST(NAT_LITERAL_EXPR, NULL, atoi(yytext), NULL, yylineno);}	

    | Expr PLUS Expr 
	{$$ = newAST(PLUS_EXPR, $1, 0, NULL, yylineno); appendToChildrenList($$, $3);}
    | Expr MINUS Expr
	{$$ = newAST(MINUS_EXPR, $1, 0, NULL, yylineno); appendToChildrenList($$, $3);}

    | Expr TIMES Expr
	{$$ = newAST(TIMES_EXPR, $1, 0, NULL, yylineno); appendToChildrenList($$, $3);}

    | Expr EQUALITY Expr
	{$$ = newAST(EQUALITY_EXPR, $1, 0, NULL, yylineno); appendToChildrenList($$, $3);}

    | Expr LESS Expr 
	{$$ = newAST(LESS_THAN_EXPR, $1, 0, NULL, yylineno); appendToChildrenList($$, $3);}

    | NOT Expr 
	{$$ = newAST(NOT_EXPR, $2, 0, NULL, yylineno);}

    | Expr AND Expr
	{$$ = newAST(AND_EXPR, $1, 0, NULL, yylineno); appendToChildrenList($$, $3);}

    | IF LPAREN Expr RPAREN LBRACE Expr_List RBRACE ELSE LBRACE Expr_List RBRACE
	{$$ = newAST(IF_THEN_ELSE_EXPR, $3, 0, NULL, yylineno); appendToChildrenList($$, $6);
	  appendToChildrenList($$, $10);}

    | WHILE LPAREN Expr RPAREN LBRACE Expr_List RBRACE
	{$$ = newAST(WHILE_EXPR, $3, 0, NULL, yylineno); appendToChildrenList($$, $6);}

    | NEW Ast_Id LPAREN RPAREN
	{$$ = newAST(NEW_EXPR, $2, 0, NULL, yylineno);}

    | PRINTNAT LPAREN Expr RPAREN
	{$$ = newAST(PRINT_EXPR, $3, 0, NULL, yylineno);}

    | READNAT LPAREN RPAREN
	{$$ = newAST(READ_EXPR, $3, 0, NULL, yylineno);}

    | Ast_Id
	{$$ = newAST(ID_EXPR, $1, 0, NULL, yylineno);}

    | Expr DOT Ast_Id
	{$$ = newAST(DOT_ID_EXPR, $1, 0, NULL, yylineno); appendToChildrenList($$, $3);}
    | Ast_Id ASSIGN Expr
	{$$ = newAST(ASSIGN_EXPR, $1, 0, NULL, yylineno); appendToChildrenList($$, $3);}

    | Expr DOT Ast_Id ASSIGN Expr
	{$$ = newAST(DOT_ASSIGN_EXPR, $1, 0, NULL, yylineno); appendToChildrenList($$, $3);
		appendToChildrenList($$, $5);}

    | Ast_Id LPAREN Method_Arg_List RPAREN
	{$$ = newAST(METHOD_CALL_EXPR, $1, 0, NULL, yylineno); appendToChildrenList($$, $3);}

    | Ast_Id LPAREN RPAREN //empty Method_Arg_Seq for undotted method call
	{$$ = newAST(METHOD_CALL_EXPR, $1, 0, NULL, yylineno); 
          appendToChildrenList($$, newAST(ARG_LIST, NULL, 0, NULL, yylineno));}

    | Expr DOT Ast_Id LPAREN Method_Arg_List RPAREN
	{$$ = newAST(DOT_METHOD_CALL_EXPR, $1, 0, NULL, yylineno); appendToChildrenList($$, $3);
	  appendToChildrenList($$, $5);}
    | Expr DOT Ast_Id LPAREN RPAREN 	//empty Method_Arg_Seq for dotted method call
	{$$ = newAST(DOT_METHOD_CALL_EXPR, $1, 0, NULL, yylineno); appendToChildrenList($$, $3);
	  appendToChildrenList($$, newAST(ARG_LIST, NULL, 0, NULL, yylineno));}

    | LPAREN Expr RPAREN 
	{$$ = $2; }
    ;

Method_Arg_List: Method_Arg_List COMMA Expr
	          {appendToChildrenList($1, $3);}
	       | Expr
		  {$$ = newAST(ARG_LIST, $1, 0, NULL, yylineno);}
	       ;
%%

int main(int argc, char **argv) {
  if(argc!=2) {
    printf("Usage: dj-parse filename\n");
    exit(-1);
  }
  yyin = fopen(argv[1],"r");
  if(yyin==NULL) {
    printf("ERROR: could not open file %s\n",argv[1]);
    exit(-1);
  }

  //Checks to see if file has the right entension to compile
  char *extension = strrchr(argv[1], '.');
  if(strcmp(".dj", extension) != 0) {
    printf("file extension must be: .dj "); 
    exit(-1);
  }
  
  yyparse();
  setupSymbolTables(pgmAST);
  typecheckProgram();

  //Snippet to change file extension and open file with the new name
  char *fileName = malloc(strlen(argv[1])+3);
  *extension = '\0';
  strcat(strcat(fileName, argv[1]),".dism");
  //printf("New File: %s\n", fileName);
  FILE *target = fopen(fileName, "w");
  generateDISM(target);
  fclose(target);
  
  return 0;
}

