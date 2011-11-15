/*
 * A n t l r  T r a n s l a t i o n  H e a d e r
 *
 * Terence Parr, Will Cohen, and Hank Dietz: 1989-2001
 * Purdue University Electrical Engineering
 * With AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR33
 *
 *   antlr -gt cl.g
 *
 */

#define ANTLR_VERSION	13333
#include "pcctscfg.h"
#include "pccts_stdio.h"

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
#define GENAST

#include "ast.h"

#define zzSET_SIZE 8
#include "antlr.h"
#include "tokens.h"
#include "dlgdef.h"
#include "mode.h"

/* MR23 In order to remove calls to PURIFY use the antlr -nopurify option */

#ifndef PCCTS_PURIFY
#define PCCTS_PURIFY(r,s) memset((char *) &(r),'\0',(s));
#endif

#include "ast.c"
zzASTgvars

ANTLR_INFO

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
    case REALCONST:
    attr->kind="realconst";
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

void
#ifdef __USE_PROTOS
program(AST**_root)
#else
program(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  zzmatch(PROGRAM); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
  dec_vars(zzSTR); zzlink(_root, &_sibling, &_tail);
  l_dec_blocs(zzSTR); zzlink(_root, &_sibling, &_tail);
  l_instrs(zzSTR); zzlink(_root, &_sibling, &_tail);
  zzmatch(ENDPROGRAM);  zzCONSUME;
  zzmatch(INPUTEND);  zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd1, 0x1);
  }
}

void
#ifdef __USE_PROTOS
dec_vars(AST**_root)
#else
dec_vars(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (LA(1)==VARS) ) {
      zzmatch(VARS);  zzCONSUME;
      l_dec_vars(zzSTR); zzlink(_root, &_sibling, &_tail);
      zzmatch(ENDVARS);  zzCONSUME;
    }
    else {
      if ( (setwd1[LA(1)]&0x2) ) {
      }
      else {zzFAIL(1,zzerr1,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
  (*_root)=createASTlist(_sibling);
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd1, 0x4);
  }
}

void
#ifdef __USE_PROTOS
l_dec_blocs(AST**_root)
#else
l_dec_blocs(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (setwd1[LA(1)]&0x8) ) {
      dec_bloc(zzSTR); zzlink(_root, &_sibling, &_tail);
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
  (*_root)=createASTlist(_sibling);
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd1, 0x10);
  }
}

void
#ifdef __USE_PROTOS
l_instrs(AST**_root)
#else
l_instrs(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (setwd1[LA(1)]&0x20) ) {
      instruction(zzSTR); zzlink(_root, &_sibling, &_tail);
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
  (*_root)=createASTlist(_sibling);
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd1, 0x40);
  }
}

void
#ifdef __USE_PROTOS
l_dec_vars(AST**_root)
#else
l_dec_vars(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (LA(1)==IDENT) ) {
      dec_var(zzSTR); zzlink(_root, &_sibling, &_tail);
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd1, 0x80);
  }
}

void
#ifdef __USE_PROTOS
dec_var(AST**_root)
#else
dec_var(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  zzmatch(IDENT); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
  constr_type(zzSTR); zzlink(_root, &_sibling, &_tail);
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd2, 0x1);
  }
}

void
#ifdef __USE_PROTOS
dec_bloc(AST**_root)
#else
dec_bloc(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (LA(1)==PROCEDURE) ) {
      zzmatch(PROCEDURE); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
      dec_procedure(zzSTR); zzlink(_root, &_sibling, &_tail);
      procedure_instr(zzSTR); zzlink(_root, &_sibling, &_tail);
      zzmatch(ENDPROCEDURE);  zzCONSUME;
    }
    else {
      if ( (LA(1)==FUNCTION) ) {
        zzmatch(FUNCTION); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
        dec_function(zzSTR); zzlink(_root, &_sibling, &_tail);
        procedure_instr(zzSTR); zzlink(_root, &_sibling, &_tail);
        zzmatch(RETURN);  zzCONSUME;
        expression(zzSTR); zzlink(_root, &_sibling, &_tail);
        zzmatch(ENDFUNCTION);  zzCONSUME;
      }
      else {zzFAIL(1,zzerr2,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd2, 0x2);
  }
}

void
#ifdef __USE_PROTOS
dec_function(AST**_root)
#else
dec_function(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  zzmatch(IDENT); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
  dec_params(zzSTR); zzlink(_root, &_sibling, &_tail);
  zzmatch(RETURN);  zzCONSUME;
  constr_type(zzSTR); zzlink(_root, &_sibling, &_tail);
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd2, 0x4);
  }
}

void
#ifdef __USE_PROTOS
dec_procedure(AST**_root)
#else
dec_procedure(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  zzmatch(IDENT); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
  dec_params(zzSTR); zzlink(_root, &_sibling, &_tail);
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd2, 0x8);
  }
}

void
#ifdef __USE_PROTOS
procedure_instr(AST**_root)
#else
procedure_instr(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  dec_vars(zzSTR); zzlink(_root, &_sibling, &_tail);
  l_dec_blocs(zzSTR); zzlink(_root, &_sibling, &_tail);
  l_instrs(zzSTR); zzlink(_root, &_sibling, &_tail);
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd2, 0x10);
  }
}

void
#ifdef __USE_PROTOS
dec_params(AST**_root)
#else
dec_params(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  zzmatch(OPENPAR);  zzCONSUME;
  l_dec_params(zzSTR); zzlink(_root, &_sibling, &_tail);
  zzmatch(CLOSEPAR); 
  (*_root)=createASTlist(_sibling);
 zzCONSUME;

  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd2, 0x20);
  }
}

void
#ifdef __USE_PROTOS
l_dec_params(AST**_root)
#else
l_dec_params(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd2[LA(1)]&0x40) ) {
      dec_param(zzSTR); zzlink(_root, &_sibling, &_tail);
      {
        zzBLOCK(zztasp3);
        zzMake0;
        {
        while ( (LA(1)==COMA) ) {
          zzmatch(COMA);  zzCONSUME;
          dec_param(zzSTR); zzlink(_root, &_sibling, &_tail);
          zzLOOP(zztasp3);
        }
        zzEXIT(zztasp3);
        }
      }
    }
    else {
      if ( (LA(1)==CLOSEPAR) ) {
      }
      else {zzFAIL(1,zzerr3,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd2, 0x80);
  }
}

void
#ifdef __USE_PROTOS
dec_param(AST**_root)
#else
dec_param(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (LA(1)==VAL) ) {
      zzmatch(VAL); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
    }
    else {
      if ( (LA(1)==REF) ) {
        zzmatch(REF); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
      }
      else {zzFAIL(1,zzerr4,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
  zzmatch(IDENT); zzsubchild(_root, &_sibling, &_tail); zzCONSUME;
  constr_type(zzSTR); zzlink(_root, &_sibling, &_tail);
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd3, 0x1);
  }
}

void
#ifdef __USE_PROTOS
call_params(AST**_root)
#else
call_params(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd3[LA(1)]&0x2) ) {
      expression(zzSTR); zzlink(_root, &_sibling, &_tail);
      {
        zzBLOCK(zztasp3);
        zzMake0;
        {
        while ( (LA(1)==COMA) ) {
          zzmatch(COMA);  zzCONSUME;
          expression(zzSTR); zzlink(_root, &_sibling, &_tail);
          zzLOOP(zztasp3);
        }
        zzEXIT(zztasp3);
        }
      }
    }
    else {
      if ( (LA(1)==CLOSEPAR) ) {
      }
      else {zzFAIL(1,zzerr5,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
  (*_root)=createASTlist(_sibling);
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd3, 0x4);
  }
}

void
#ifdef __USE_PROTOS
constr_type(AST**_root)
#else
constr_type(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==INT) ) {
    zzmatch(INT); zzsubchild(_root, &_sibling, &_tail); zzCONSUME;
  }
  else {
    if ( (LA(1)==BOOL) ) {
      zzmatch(BOOL); zzsubchild(_root, &_sibling, &_tail); zzCONSUME;
    }
    else {
      if ( (LA(1)==REAL) ) {
        zzmatch(REAL); zzsubchild(_root, &_sibling, &_tail); zzCONSUME;
      }
      else {
        if ( (LA(1)==STRUCT) ) {
          zzmatch(STRUCT); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
          {
            zzBLOCK(zztasp2);
            zzMake0;
            {
            while ( (LA(1)==IDENT) ) {
              field(zzSTR); zzlink(_root, &_sibling, &_tail);
              zzLOOP(zztasp2);
            }
            zzEXIT(zztasp2);
            }
          }
          zzmatch(ENDSTRUCT);  zzCONSUME;
        }
        else {
          if ( (LA(1)==ARRAY) ) {
            zzmatch(ARRAY); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
            zzmatch(OPENBRACK);  zzCONSUME;
            zzmatch(INTCONST); zzsubchild(_root, &_sibling, &_tail); zzCONSUME;
            zzmatch(CLOSEBRACK);  zzCONSUME;
            zzmatch(OF);  zzCONSUME;
            constr_type(zzSTR); zzlink(_root, &_sibling, &_tail);
          }
          else {zzFAIL(1,zzerr6,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
        }
      }
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd3, 0x8);
  }
}

void
#ifdef __USE_PROTOS
field(AST**_root)
#else
field(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  zzmatch(IDENT); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
  constr_type(zzSTR); zzlink(_root, &_sibling, &_tail);
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd3, 0x10);
  }
}

void
#ifdef __USE_PROTOS
instruction(AST**_root)
#else
instruction(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==IDENT) ) {
    expr_id(zzSTR); zzlink(_root, &_sibling, &_tail);
    {
      zzBLOCK(zztasp2);
      zzMake0;
      {
      if ( (LA(1)==ASIG) ) {
        zzmatch(ASIG); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
        expression(zzSTR); zzlink(_root, &_sibling, &_tail);
      }
      else {
        if ( (setwd3[LA(1)]&0x20) ) {
        }
        else {zzFAIL(1,zzerr7,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
      }
      zzEXIT(zztasp2);
      }
    }
  }
  else {
    if ( (setwd3[LA(1)]&0x40) ) {
      {
        zzBLOCK(zztasp2);
        zzMake0;
        {
        if ( (LA(1)==WRITE) ) {
          zzmatch(WRITE); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
        }
        else {
          if ( (LA(1)==WRITELN) ) {
            zzmatch(WRITELN); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
          }
          else {zzFAIL(1,zzerr8,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
        }
        zzEXIT(zztasp2);
        }
      }
      zzmatch(OPENPAR);  zzCONSUME;
      {
        zzBLOCK(zztasp2);
        zzMake0;
        {
        if ( (setwd3[LA(1)]&0x80) ) {
          expression(zzSTR); zzlink(_root, &_sibling, &_tail);
        }
        else {
          if ( (LA(1)==STRING) ) {
            zzmatch(STRING); zzsubchild(_root, &_sibling, &_tail); zzCONSUME;
          }
          else {zzFAIL(1,zzerr9,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
        }
        zzEXIT(zztasp2);
        }
      }
      zzmatch(CLOSEPAR);  zzCONSUME;
    }
    else {
      if ( (LA(1)==READ) ) {
        zzmatch(READ); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
        zzmatch(OPENPAR);  zzCONSUME;
        zzmatch(IDENT); zzsubchild(_root, &_sibling, &_tail); zzCONSUME;
        zzmatch(CLOSEPAR);  zzCONSUME;
      }
      else {
        if ( (LA(1)==IF) ) {
          zzmatch(IF); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
          expression(zzSTR); zzlink(_root, &_sibling, &_tail);
          zzmatch(THEN);  zzCONSUME;
          l_instrs(zzSTR); zzlink(_root, &_sibling, &_tail);
          {
            zzBLOCK(zztasp2);
            zzMake0;
            {
            if ( (LA(1)==ELSE) ) {
              zzmatch(ELSE);  zzCONSUME;
              l_instrs(zzSTR); zzlink(_root, &_sibling, &_tail);
            }
            else {
              if ( (LA(1)==ENDIF) ) {
              }
              else {zzFAIL(1,zzerr10,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
            }
            zzEXIT(zztasp2);
            }
          }
          zzmatch(ENDIF);  zzCONSUME;
        }
        else {
          if ( (LA(1)==WHILE) ) {
            zzmatch(WHILE); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
            expression(zzSTR); zzlink(_root, &_sibling, &_tail);
            zzmatch(DO);  zzCONSUME;
            l_instrs(zzSTR); zzlink(_root, &_sibling, &_tail);
            zzmatch(ENDWHILE);  zzCONSUME;
          }
          else {zzFAIL(1,zzerr11,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
        }
      }
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd4, 0x1);
  }
}

void
#ifdef __USE_PROTOS
expression(AST**_root)
#else
expression(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  expr_comparacio(zzSTR); zzlink(_root, &_sibling, &_tail);
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (setwd4[LA(1)]&0x2) ) {
      {
        zzBLOCK(zztasp3);
        zzMake0;
        {
        if ( (LA(1)==AND) ) {
          zzmatch(AND); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
        }
        else {
          if ( (LA(1)==OR) ) {
            zzmatch(OR); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
          }
          else {zzFAIL(1,zzerr12,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
        }
        zzEXIT(zztasp3);
        }
      }
      expr_comparacio(zzSTR); zzlink(_root, &_sibling, &_tail);
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd4, 0x4);
  }
}

void
#ifdef __USE_PROTOS
expr_comparacio(AST**_root)
#else
expr_comparacio(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  expr_aritm(zzSTR); zzlink(_root, &_sibling, &_tail);
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (setwd4[LA(1)]&0x8) ) {
      {
        zzBLOCK(zztasp3);
        zzMake0;
        {
        if ( (LA(1)==GT) ) {
          zzmatch(GT); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
        }
        else {
          if ( (LA(1)==LT) ) {
            zzmatch(LT); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
          }
          else {
            if ( (LA(1)==EQ) ) {
              zzmatch(EQ); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
            }
            else {zzFAIL(1,zzerr13,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
          }
        }
        zzEXIT(zztasp3);
        }
      }
      expr_aritm(zzSTR); zzlink(_root, &_sibling, &_tail);
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd4, 0x10);
  }
}

void
#ifdef __USE_PROTOS
expr_aritm(AST**_root)
#else
expr_aritm(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  expr_muldiv(zzSTR); zzlink(_root, &_sibling, &_tail);
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    for (;;) {
      if ( !((setwd4[LA(1)]&0x20))) break;
      if ( (LA(1)==PLUS) ) {
        zzmatch(PLUS); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
        expr_muldiv(zzSTR); zzlink(_root, &_sibling, &_tail);
      }
      else {
        if ( (LA(1)==MINUS) ) {
          zzmatch(MINUS); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
          expr_muldiv(zzSTR); zzlink(_root, &_sibling, &_tail);
        }
        else break; /* MR6 code for exiting loop "for sure" */
      }
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd4, 0x40);
  }
}

void
#ifdef __USE_PROTOS
expr_muldiv(AST**_root)
#else
expr_muldiv(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  exprsimple(zzSTR); zzlink(_root, &_sibling, &_tail);
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    for (;;) {
      if ( !((setwd4[LA(1)]&0x80))) break;
      if ( (LA(1)==MUL) ) {
        zzmatch(MUL); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
        exprsimple(zzSTR); zzlink(_root, &_sibling, &_tail);
      }
      else {
        if ( (LA(1)==DIV) ) {
          zzmatch(DIV); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
          exprsimple(zzSTR); zzlink(_root, &_sibling, &_tail);
        }
        else break; /* MR6 code for exiting loop "for sure" */
      }
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd5, 0x1);
  }
}

void
#ifdef __USE_PROTOS
exprsimple(AST**_root)
#else
exprsimple(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==IDENT) ) {
    expr_id(zzSTR); zzlink(_root, &_sibling, &_tail);
  }
  else {
    if ( (LA(1)==INTCONST) ) {
      zzmatch(INTCONST); zzsubchild(_root, &_sibling, &_tail); zzCONSUME;
    }
    else {
      if ( (LA(1)==REALCONST) ) {
        zzmatch(REALCONST); zzsubchild(_root, &_sibling, &_tail); zzCONSUME;
      }
      else {
        if ( (LA(1)==OPENPAR) ) {
          zzmatch(OPENPAR);  zzCONSUME;
          expression(zzSTR); zzlink(_root, &_sibling, &_tail);
          zzmatch(CLOSEPAR);  zzCONSUME;
        }
        else {
          if ( (LA(1)==TRUEKWD) ) {
            zzmatch(TRUEKWD); zzsubchild(_root, &_sibling, &_tail); zzCONSUME;
          }
          else {
            if ( (LA(1)==FALSEKWD) ) {
              zzmatch(FALSEKWD); zzsubchild(_root, &_sibling, &_tail); zzCONSUME;
            }
            else {
              if ( (setwd5[LA(1)]&0x2) ) {
                {
                  zzBLOCK(zztasp2);
                  zzMake0;
                  {
                  if ( (LA(1)==CASTINGR) ) {
                    zzmatch(CASTINGR); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
                  }
                  else {
                    if ( (LA(1)==CASTINGI) ) {
                      zzmatch(CASTINGI); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
                    }
                    else {
                      if ( (LA(1)==FLOOR) ) {
                        zzmatch(FLOOR); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
                      }
                      else {
                        if ( (LA(1)==NOT) ) {
                          zzmatch(NOT); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
                        }
                        else {
                          if ( (LA(1)==MINUS) ) {
                            zzmatch(MINUS); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
                          }
                          else {zzFAIL(1,zzerr14,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
                        }
                      }
                    }
                  }
                  zzEXIT(zztasp2);
                  }
                }
                exprsimple(zzSTR); zzlink(_root, &_sibling, &_tail);
              }
              else {zzFAIL(1,zzerr15,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
            }
          }
        }
      }
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd5, 0x4);
  }
}

void
#ifdef __USE_PROTOS
expr_id(AST**_root)
#else
expr_id(_root)
AST **_root;
#endif
{
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  zzmatch(IDENT); zzsubchild(_root, &_sibling, &_tail); zzCONSUME;
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (LA(1)==OPENPAR) ) {
      zzmatch(OPENPAR); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
      call_params(zzSTR); zzlink(_root, &_sibling, &_tail);
      zzmatch(CLOSEPAR);  zzCONSUME;
    }
    else {
      if ( (setwd5[LA(1)]&0x8) ) {
      }
      else {zzFAIL(1,zzerr16,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    for (;;) {
      if ( !((setwd5[LA(1)]&0x10))) break;
      if ( (LA(1)==DOT) ) {
        {
          zzBLOCK(zztasp3);
          zzMake0;
          {
          zzmatch(DOT); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
          zzmatch(IDENT); zzsubchild(_root, &_sibling, &_tail); zzCONSUME;
          zzEXIT(zztasp3);
          }
        }
      }
      else {
        if ( (LA(1)==OPENBRACK) ) {
          {
            zzBLOCK(zztasp3);
            zzMake0;
            {
            zzmatch(OPENBRACK); zzsubroot(_root, &_sibling, &_tail); zzCONSUME;
            expression(zzSTR); zzlink(_root, &_sibling, &_tail);
            zzmatch(CLOSEBRACK);  zzCONSUME;
            zzEXIT(zztasp3);
            }
          }
        }
        else break; /* MR6 code for exiting loop "for sure" */
      }
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd5, 0x20);
  }
}
