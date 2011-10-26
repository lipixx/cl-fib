/* ===== semantic.c ===== */
#include <string>
#include <iostream>
#include <map>
#include <list>
#include <vector>
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include "ptype.hh"
#include "symtab.hh"
#include "myASTnode.hh"
#include "semantic.hh"
#include "util.hh"

// feedback the main program with our error status
int TypeError = 0;


/// ---------- Error reporting routines --------------

void errornumparam(int l) {
  TypeError = 1;
  cout<<"L. "<<l<<": The number of parameters in the call do not match."<<endl;
}

void errorincompatibleparam(int l,int n) {
  TypeError = 1;
  cout<<"L. "<<l<<": Parameter "<<n<<" with incompatible types."<<endl;
}

void errorreferenceableparam(int l,int n) {
  TypeError = 1;
  cout<<"L. "<<l<<": Parameter "<<n<<" is expected to be referenceable but it is not."<<endl;
}

void errordeclaredident(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Identifier "<<s<<" already declared."<<endl;
}

void errornondeclaredident(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Identifier "<<s<<" is undeclared."<<endl;
}

void errornonreferenceableleft(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Left expression of assignment is not referenceable."<<endl;
}

void errorincompatibleassignment(int l) {
  TypeError = 1;
  cout<<"L. "<<l<<": Assignment with incompatible types."<<endl;
}

void errorbooleanrequired(int l,string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Instruction "<<s<<" requires a boolean condition."<<endl;
}

void errorisnotprocedure(int l) {
  TypeError = 1;
  cout<<"L. "<<l<<": Operator ( must be applied to a procedure in an instruction."<<endl;
}

void errorisnotfunction(int l) {
  TypeError = 1;
  cout<<"L. "<<l<<": Operator ( must be applied to a function in an expression."<<endl;
}

void errorincompatibleoperator(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Operator "<<s<<" with incompatible types."<<endl;
}

void errorincompatiblereturn(int l) {
  TypeError = 1;
  cout<<"L. "<<l<<": Return with incompatible type."<<endl;
}

void errorreadwriterequirebasic(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Basic type required in "<<s<<"."<<endl;
}

void errornonreferenceableexpression(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Referenceable expression required in "<<s<<"."<<endl;
}

void errornonfielddefined(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Field "<<s<<" is not defined in the struct."<<endl;
}

void errorfielddefined(int l, string s) {
  TypeError = 1;
  cout<<"L. "<<l<<": Field "<<s<<" already defined in the struct."<<endl;
}

/// ------------------------------------------------------------
/// Table to store information about program identifiers
symtab symboltable;

static void InsertintoST(int line,string kind,string id,ptype tp)
{
  infosym p;

  /*Si el símbol està ja a la taula i a més trobem que està declarat a l'scope
    actual, error: symbol ja declarat!!. Per anar bé, hauría d'estar, o no declarat (.find(id)=0)
    o declarat a un altre scope que no sigui el nostre.
    Si no hi ha error, el declarem a l'scope actual.
  */
  if (symboltable.find(id) && symboltable.jumped_scopes(id)==0) errordeclaredident(line,id);
  else {
    symboltable.createsymbol(id);
    symboltable[id].kind=kind;
    symboltable[id].tp=tp;
  }
}

/// ------------------------------------------------------------

bool isbasickind(string kind) {
  return kind=="int" || kind=="bool";
}

bool isvalidkind(string kind) {
  return kind == "int" || kind == "bool" || kind == "array" || kind == "struct";
}

void check_params(AST *a,ptype tp,int line,int numparam)
{
  int numCalledParams = 0;

  /*child(a,1): _list
    child(_list,0): primer paràmetre de la llista
    Iterant amb els fills de _list podem obtenir el nombre de paràmetres que ens
    han passat a la funció.
   */
  AST * a1 = child(child(a,1),0);
  
  for (; a1 != 0; a1 = a1->right) {
    numCalledParams++;
  }
  
  /*Llavors si el nº de paràmetres no coincideix amb la declaració, llançem error*/
  if (numparam != numCalledParams)
      errornumparam(a->line);
  else
    {
      /*La llista de params. coincideix en nombre, però ara hem de comprovar cada paràmetre
	si coincideix amb el lloc de la seva declaració.*/

      //Actualment tp té el tipus del primer paràmetre de la declaració
      int i = 1;

      //Obtenim el primer paràmetre
      a1 = child(child(a,1),0);

      for (;a1 != 0; a1 = a1->right)
	{
	  //Comprovem el primer param.
	  TypeCheck(a1);
	  //El kind parref indica si és per ref. o per valor. (veure ptype.hh).
	  if (tp->kind=="parref" && !a1->ref)
	    errorreferenceableparam(a->line, i);
	  
	  //Si coincideix la declaració amb el que ens passen
	  if (a1->tp->kind != "error" && !equivalent_types(a1->tp, tp->down))
	    errorincompatibleparam(a->line, i);
	  
	  //Iterem pel següent.
	  if (tp) tp = tp->right;
	  i++;
	}
    }
}

/*Func. cridada des del typecheck de Program, secció de declaració de variables*/
void insert_vars(AST *a)
{
  /*Com veiem és una funció recursiva que en primera instància és cridada per TypeCheck(program),
   amb input *a = l'arrel de la primera declaració.*/
  if (!a) return; //Condició que fa que haguem acabat amb totes les declaracions de variables
  /*Es comprova la var. actual*/
  TypeCheck(child(a,0));
  /*Es posa la variable a l'scope actual*/
  InsertintoST(a->line,"idvarlocal",a->text,child(a,0)->tp);
  /*Continua amb la següent variable (germà següent a l'arbre)*/
  insert_vars(a->right); 
}

void construct_struct(AST *a)
{
  AST *a1=child(a,0);
  a->tp=create_type("struct",0,0);

  while (a1!=0) {
    TypeCheck(child(a1,0));
    if (a->tp->struct_field.find(a1->text)==a->tp->struct_field.end()) {
      a->tp->struct_field[a1->text]=child(a1,0)->tp;
      a->tp->ids.push_back(a1->text);
     } else {
      errorfielddefined(a1->line,a1->text);
    }
    a1=a1->right;
  }
}

void create_header(AST *a)
{
  //...
}


void insert_header(AST *a)
{
  //...
}

void insert_headers(AST *a)
{
  while (a!=0) {
    insert_header(a);
    a=a->right;
  }
}

/*info pot ser NULL*/
void TypeCheck(AST *a,string info)
{
  /*Veure l'AST que es genera quan executem ./cl, per tal de tenir una idea
   de quins fills te cada node.*/

  if (!a) {
    return;
  }

  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  if (a->kind=="program") {
    /*Creem un nou scope a la pila*/
    a->sc=symboltable.push();

    /*Secció de variables*/
    insert_vars(child(child(a,0),0));

    /*Secció de blocs*/
    //insert_headers(child(child(a,1),0));
    //TypeCheck(child(a,1));

    /*Secció d'instruccions*/
    TypeCheck(child(a,2),"instruction");
    symboltable.pop();
  }
  else if (a->kind=="list") {
    // At this point only instruction, procedures or parameters lists are possible.
    for (AST *a1=a->down;a1!=0;a1=a1->right) {
      TypeCheck(a1,info);
    }
  } 
  else if (a->kind=="ident") {
    if (!symboltable.find(a->text)) {
      errornondeclaredident(a->line, a->text);
    } 
    else {
      a->tp=symboltable[a->text].tp;
      a->ref=1;
    }
  } 
  else if (a->kind=="struct") {
    construct_struct(a);
  }
  else if (a->kind=="array"){
    a->tp=create_type("array",0,0);
    TypeCheck(child(a,0));
    TypeCheck(child(a,1));
    
    /*Comprovem els dos fills de l'array que són, fill 0: nºelements, i fill 1: tipus de
      l'array*/
    if (child(a,0)->tp->kind != "error" && child(a,1)->tp->kind != "error"
	&& (child(a,0)->tp->kind!="int" && !isvalidkind(child(a,1)->tp->kind)))
      {
	errorincompatibleoperator(a->line,a->kind);
      }
    else /*Si tenim una definició bona d'array ens guardem la informació, veure ptype.hh*/
      {
	a->tp->numelemsarray = stringtoint(child(a,0)->text);
	/*Guardem el tipus de l'array al down, per conveni*/
	a->tp->down = child(a,1)->tp;
      }
  }  
  else if (a->kind=="["){
    /*Suposem x[expr].
      En aquest cas hem de comprovar que la variable x és de tipus array. Un [ té com a fill
      0 l'identificador, i com a fill dret una expressió que ha de resultar de tipus enter.*/
    TypeCheck(child(a,0));
    TypeCheck(child(a,1));

    //Si el fill 0 (id. x) és una expressió per l'esquerra, "[" també..
    a->ref = child(a,0)->ref;
    
    if (child(a,0)->tp->kind!="error")
      {
	if (child(a,0)->tp->kind!="array")
	  errorincompatibleoperator(a->line,"array[]");
	else
	  //Els tipus d'un array els guardem a down per conveni. Veure ptype.hh.
	  a->tp = child(a,0)->tp->down;
      }

    //Comprovem que l'expressió que vé despres de [ és un enter
    if (child(a,1)->tp->kind!="int" && child(a,1)->tp->kind!="error")
      errorincompatibleoperator(a->line,"[]");
  }
  else if (a->kind== "("){
    /*Suposem x(expr):
      En aquest cas x és l'identificador i és el fill 0 de (.
      Expr no ens importa de quin tipus sigui i és el fill 1 de (.

      Llavors podem tenir una funció o un procedure.

      Suposem (x+10):
      En aquest cas si ens fixem en l'arbre generat, els parèntesis són implicits i no apareixen.
      Per tant no els hem de tractar i no se'ns pot donar el cas.
    */
    TypeCheck(child(a, 0));

    if (child(a, 0)->tp->kind != "error")
      {
	if (child(a, 0)->tp->kind != "procedure" && info == "instruction")
	  errorisnotprocedure(a->line);       
	else 
	  if (child(a, 0)->tp->kind != "function" && info != "instruction")	    
	    errorisnotfunction(a->line);
      }
    
    /*En cas de tenir un proc. o una func. hem de comprovar els paràmetres*/
    if (child(a, 0)->tp->kind == "procedure" || child(a, 0)->tp->kind == "function")
      //Revisar el pas de paràmetres a aquesta crida
      check_params(a,child(a,0)->tp->down,a->line,child(a,0)->tp->size);     

    /*Per conveni guardem el tipus de retorn de la funció a right (veure ptype.hh)*/
    if (child(a, 0)->tp->kind == "function")
      a->tp = child(a, 0)->tp->right;
  }

  else if (a->kind=="if"){
    //Comprovem que la part que segueix a l'if sigui una expressió booleana
    TypeCheck(child(a,0));
    if (child(a,0)->tp->kind!="bool")
	errorbooleanrequired(a->line,a->kind);

    //Comprovem la segona part de l'if
    TypeCheck(child(a,1),info);

    //Si hi ha tercera part d'if (else), comprovar-ho també.
    if (child(a,2)!=0)
	TypeCheck(child(a,2),info);
  }
  
  else if (a->kind=="while"){
    TypeCheck(child(a,0));
    if (child(a,0)->tp->kind!="bool")
	errorbooleanrequired(a->line,a->kind);
    //Continuem processant while
    TypeCheck(child(a,1),info);
  }

  else if (a->kind==":=") {
    TypeCheck(child(a,0));
    TypeCheck(child(a,1));
    if (!child(a,0)->ref) {
      errornonreferenceableleft(a->line,child(a,0)->text);
    }
    else if (child(a,0)->tp->kind!="error" && child(a,1)->tp->kind!="error" &&
	     !equivalent_types(child(a,0)->tp,child(a,1)->tp)) {
      errorincompatibleassignment(a->line);
    } 
    else {
      a->tp=child(a,0)->tp;
    }
  } 
  else if (a->kind=="intconst") {
    a->tp=create_type("int",0,0);
  } 
  else if (a->kind == "string") {
    a->tp = create_type("string", 0, 0);
  }
  else if (a->kind=="+" || (a->kind=="-" && child(a,1)!=0) || a->kind=="*"
	   || a->kind=="/") {
    TypeCheck(child(a,0));
    TypeCheck(child(a,1));
    if ((child(a,0)->tp->kind!="error" && child(a,0)->tp->kind!="int") ||
	(child(a,1)->tp->kind!="error" && child(a,1)->tp->kind!="int")) {
      errorincompatibleoperator(a->line,a->kind);
    }
    a->tp=create_type("int",0,0);
  }
  else if (a->kind=="-" && child(a,1)==0)
    {
      TypeCheck(child(a,0));
      if ((child(a,0)->tp->kind!="error" && child(a,0)->tp->kind!="int"))
	  errorincompatibleoperator(a->line,a->kind);
      a->tp=create_type("int",0,0);	  
    }
  else if (a->kind=="true" || a->kind=="false") {
    a->tp=create_type("bool",0,0);
  }
  else if (a->kind=="="){
    TypeCheck(child(a,0));
    TypeCheck(child(a,1));
    if (child(a,0)->tp->kind!="error" && child(a,1)->tp->kind!="error"
	&& !equivalent_types(child(a,0)->tp,child(a,1)->tp)
	|| !isbasickind(child(a,0)->tp->kind) || !isbasickind(child(a,1)->tp->kind))
      errorincompatibleoperator(a->line,a->kind);
    a->tp=create_type("bool",0,0);
  }
  else if (a->kind=="or" || a->kind=="and"){
    TypeCheck(child(a,0));
    TypeCheck(child(a,1));
    if ((child(a,0)->tp->kind!="error" && child(a,0)->tp->kind!="bool") ||
	(child(a,1)->tp->kind!="error" && child(a,1)->tp->kind!="bool")) {
      errorincompatibleoperator(a->line,a->kind);
    }
    a->tp=create_type("bool",0,0);
  }
  else if (a->kind=="not"){
    TypeCheck(child(a,0));
    if (child(a,0)->tp->kind!="error" && child(a,0)->tp->kind!="bool")
      errorincompatibleoperator(a->line,a->kind);
    a->tp=create_type("bool",0,0);
  }
  else if (a->kind=="<" || a->kind==">")
    {
      TypeCheck(child(a,0));
    TypeCheck(child(a,1));
    if ((child(a,0)->tp->kind!="error" && child(a,0)->tp->kind!="int") ||
	(child(a,1)->tp->kind!="error" && child(a,1)->tp->kind!="int")) {
      errorincompatibleoperator(a->line,a->kind);
    }
    a->tp=create_type("bool",0,0);
    }
  else if (isbasickind(a->kind)) {
    a->tp=create_type(a->kind,0,0);
  }
  else if (a->kind=="writeln" || a->kind=="write") {
    TypeCheck(child(a,0));
    if (child(a,0)->tp->kind!="error" && !isbasickind(child(a,0)->tp->kind)
	&& child(a, 0)->tp->kind != "string")
      {
	errorreadwriterequirebasic(a->line,a->kind);
      }
  }
  else if (a->kind == "read") {
    /*Aquest cas és igual que el write, però si l'expressió que hi ha dins el write
    no és un tipus simple (expressió per l'esquerra), llavors fallem.*/
    TypeCheck(child(a, 0));
    if (child(a, 0)->tp->kind != "error" && !child(a, 0)->ref)
      errornonreferenceableexpression(a->line, a->kind);   
    else
      if (child(a, 0)->tp->kind != "error" && !isbasickind(child(a, 0)->tp->kind))
	errorreadwriterequirebasic(a->line, a->kind);
  }
  else if (a->kind == "string"){
    
  }
  else if (a->kind==".") {
    TypeCheck(child(a,0));
    a->ref=child(a,0)->ref;
    if (child(a,0)->tp->kind!="error") {
      if (child(a,0)->tp->kind!="struct") {
	errorincompatibleoperator(a->line,"struct.");
      }
      else if (child(a,0)->tp->struct_field.find(child(a,1)->text)==
	       child(a,0)->tp->struct_field.end()) {
	errornonfielddefined(a->line,child(a,1)->text);
      } 
      else {
	a->tp=child(a,0)->tp->struct_field[child(a,1)->text];
      }
    }
  } 
  else {
    cout<<"BIG PROBLEM! No case defined for kind "<<a->kind<<endl;
  }

  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
}
