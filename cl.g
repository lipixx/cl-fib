/* ===== Cl.g: parser para el lenguaje fuente CL de Compiladors ===== */
#header
<<
#include <string>
#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <fstream>

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include "ptype.hh"
#include "symtab.hh"
#include "codegest.hh"

/// struct to contain information about a token.
typedef struct {
    string kind;
    string text;
    int line;
} Attrib;

/// function called by the scanner when a new token is found
/// Predeclared here, definition below.
void zzcr_attr(Attrib *attr,int type,char *text);

/// Include AST node fields declaration
#include "myASTnode.hh"

/// Macro called by the parser when a new AST node is to be created
/// createASTnode function doing the actual job is defined below.
#define zzcr_ast(as,attr,tttype,textt) as=createASTnode(attr,tttype,textt);
AST* createASTnode(Attrib* attr, int ttype, char *textt);
>>


<<
#include "semantic.hh"
#include "codegen.hh"
#include "util.hh"

/// -----------------------------------------------------------
/// function called by the scanner when a new token is found.
void zzcr_attr(Attrib *attr,int type,char *text)
{
  switch (type) {
  case IDENT:
    attr->kind="ident";
    attr->text=text;
    break;
  case INTCONST:
    attr->kind="intconst";
    attr->text=text;
    break;
  case STRING:
    attr->kind="string";
    attr->text=text;
    break;
  default:
    attr->kind=lowercase(text);
    attr->text="";
    break;
  }
  attr->line = zzline;
  if (type!=INPUTEND) cout<<text;

}


/// ----------- Several AST-related functions -----------

/// create a new AST node with given token information
AST* createASTnode(Attrib* attr, int ttype, char *textt) {
   AST *as = new AST;
   as->kind=(attr)->kind;
   as->text=(attr)->text;
   as->line=(attr)->line;
   as->right=NULL;
   as->down=NULL;
   as->sc=0;
   as->tp=create_type("error",0,0);
   as->ref=0;
   return as;   
}

/// create a new "list" AST node with one element
AST* createASTlist(AST *child) {
 AST *as=new AST;
 as->kind="list";
 as->right=NULL;
 as->down=child;
 return as;
}


/// create a new empty "list" AST node 
AST* createASTlist() {
 AST *as=new AST;
 as->kind="list";
 as->right=NULL;
 as->down=NULL;
 return as;
} 


/// get nth child of a tree. Count starts at 0.
/// if no such child, returns NULL
AST* child(AST *a,int n) {
 AST *c=a->down;
 for (int i=0; c!=NULL && i<n; i++) c=c->right;
 return c;
} 


void ASTPrintIndent(AST *a,string s)
{
  if (a==NULL) return;

  cout<<a->kind;
  if (a->text!="") cout<<"("<<a->text<<")";
  cout<<endl;

  AST *i = a->down;
  while (i!=NULL && i->right!=NULL) {
    cout<<s+"  \\__";
    ASTPrintIndent(i,s+"  |"+string(i->kind.size()+i->text.size(),' '));
    i=i->right;
  }
  
  if (i!=NULL) {
      cout<<s+"  \\__";
      ASTPrintIndent(i,s+"   "+string(i->kind.size()+i->text.size(),' '));
      i=i->right;
  }
}

/// print AST 
void ASTPrint(AST *a)
{
  cout<<endl;
  ASTPrintIndent(a,"  ");
}




/// ------------------  MAIN PROGRAM ---------------------

/// Main program
int main(int argc,char *argv[])
{
  AST *root=NULL;
  cout<<endl<<endl<<"  1: ";

  ANTLR(program(&root),stdin);

  // if there are errors, say so and stop
  if (zzLexErrCount>0) {
    cout<<endl<<"There were lexical errors."<<endl;
    exit(0);
  } 
  if (zzSyntaxErrCount>0) {
    cout<<endl<<"There were syntax errors."<<endl;
    exit(0);
  }

  // no lexical or syntax errors found. Check types
  cout<<endl<<endl;
  ASTPrint(root);

  cout<<endl<<endl<<"Type Checking:"<<endl<<endl;
  TypeCheck(root);

  // if there are type errors, say so and stop
  if (TypeError) {
    cout<<"There are errors: no code generated"<<endl;
    exit(0);
  } 

  // no errors found. Generate code
  cout<<"Generating code:"<<endl;
  codeglobal cg;
  CodeGen(root,cg);
  writecodeglobal(cg);
  
  if (argc==2 && string(argv[1])=="execute") {
    cout<<endl<<"Executing code:"<<endl;
    executecodeglobal(cg,10000,0);
  }
  else if (argc==2 && string(argv[1])=="executestep") {
    cout<<endl<<"Executing code:"<<endl;
    executecodeglobal(cg,10000,1);
  }
  else if (argc>=2 && string(argv[1])=="writefile") {
    ofstream ofs;
    if (argc>=3) ofs.open(argv[2]);  // Open given filename or
    else ofs.open("program.t");      // use default if none given.
    writecodeglobal(cg,ofs);
  }
}

/// -----------------------------------------------------
>>

#lexclass START
#token PROGRAM      "PROGRAM"
#token ENDPROGRAM   "ENDPROGRAM"
#token PROCEDURE    "PROCEDURE"
#token ENDPROCEDURE "ENDPROCEDURE"
#token FUNCTION     "FUNCTION"
#token ENDFUNCTION  "ENDFUNCTION"
#token COMA         ","
#token VARS         "VARS"
#token VAL          "VAL"
#token REF          "REF"
#token RETURN       "RETURN"
#token ENDVARS      "ENDVARS"
#token INT          "INT"
#token BOOL         "BOOL"
#token STRUCT       "STRUCT"
#token ARRAY        "ARRAY"
#token OF           "OF"
#token ENDSTRUCT    "ENDSTRUCT"
#token READ         "READ"
#token WRITE        "WRITE"
#token WRITELN      "WRITELN"
#token IF           "IF"
#token THEN         "THEN"
#token ELSE         "ELSE"
#token ENDIF        "ENDIF"
#token WHILE        "WHILE"
#token DO           "DO"
#token ENDWHILE     "ENDWHILE"
#token DOT          "."
#token OPENPAR      "\("
#token CLOSEPAR     "\)"
#token OPENBRACK    "\["
#token CLOSEBRACK   "\]"
#token NOT          "NOT"
#token DIV          "\/"
#token MUL          "\*"
#token PLUS         "\+"
#token MINUS         "\-"
#token EQ           "="
#token GT           ">"
#token LT           "<"
#token AND          "AND"
#token OR           "OR"
#token ASIG         ":="
#token TRUEKWD      "TRUE"
#token FALSEKWD     "FALSE"
#token STRING       " \" ~[\"]* \" "
#token IDENT        "[a-zA-Z][a-zA-Z0-9]*"
#token INTCONST     "[0-9]+"
#token COMMENT      "//~[\n]*" << printf("%s",zzlextext); zzskip(); >>
#token WHITESPACE   "[\ \t]+"  << printf("%s",zzlextext); zzskip(); >>
#token NEWLINE      "\n"       << zzline++; printf("\n%3d: ", zzline); zzskip(); >>
#token LEXICALERROR "~[]"   << printf("Lexical error: symbol '%s' ignored!\n", zzlextext);
                               zzLexErrCount++;
                               zzskip(); >>
#token INPUTEND   "@"

program: PROGRAM^ dec_vars l_dec_blocs l_instrs ENDPROGRAM! INPUTEND!;


dec_vars: (VARS! l_dec_vars ENDVARS! | ) <<#0=createASTlist(_sibling);>>;
l_dec_blocs: ( dec_bloc )* <<#0=createASTlist(_sibling);>> ;
l_instrs: (instruction)* <<#0=createASTlist(_sibling);>>;

l_dec_vars: (dec_var)* ;
dec_var: IDENT^ constr_type;


dec_bloc: (PROCEDURE^ dec_procedure procedure_instr ENDPROCEDURE!
        | FUNCTION^ dec_function procedure_instr RETURN! expression ENDFUNCTION!);

dec_function: IDENT^ dec_params RETURN! constr_type;
dec_procedure: IDENT^ dec_params;
procedure_instr: dec_vars l_dec_blocs l_instrs;

dec_params: OPENPAR! l_dec_params CLOSEPAR! <<#0=createASTlist(_sibling);>>;
l_dec_params: (dec_param (COMA! dec_param)*|);
dec_param: (VAL^ | REF^) IDENT constr_type;

call_params: (expression (COMA! expression)* | ) <<#0=createASTlist(_sibling);>>;
constr_type: INT | BOOL | STRUCT^ (field)* ENDSTRUCT! | ARRAY^ OPENBRACK! INTCONST CLOSEBRACK! OF! constr_type;
field: IDENT^ constr_type;


instruction:
        expr_id (ASIG^ expression| )
      |	(WRITE^ | WRITELN^) OPENPAR! ( expression | STRING ) CLOSEPAR!
      | READ^ OPENPAR! IDENT CLOSEPAR!
      | IF^ expression THEN! l_instrs (ELSE! l_instrs| ) ENDIF! 
      | WHILE^ expression DO! l_instrs ENDWHILE! ;

expression: expr_comparacio ((AND^|OR^)expr_comparacio)*;

expr_comparacio: expr_aritm ((GT^|LT^|EQ^)expr_aritm)*;

expr_aritm: expr_muldiv (PLUS^ expr_muldiv | MINUS^ expr_muldiv)*;

expr_muldiv: exprsimple (MUL^ exprsimple | DIV^ exprsimple)*;

exprsimple: expr_id 
           | INTCONST 
           | OPENPAR! expression CLOSEPAR! 
           | TRUEKWD | FALSEKWD 
           | (NOT^ | MINUS^) exprsimple;
        
expr_id: IDENT (OPENPAR^ call_params CLOSEPAR! | )((DOT^ IDENT) | (OPENBRACK^ expression CLOSEBRACK!))*;
