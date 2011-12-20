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
#include "codegest.hh"

#include "myASTnode.hh"

#include "util.hh"
#include "semantic.hh"

#include "codegen.hh"

// symbol table with information about identifiers in the program
// declared in symtab.cc
extern symtab symboltable;

// When dealing with a list of instructions, it contains the maximum auxiliar space
// needed by any of the instructions for keeping non-referenceable non-basic-type values.
int maxoffsetauxspace;

// When dealing with one instruction, it contains the auxiliar space
// needed for keeping non-referenceable values.
int offsetauxspace;

// For distinghishing different labels for different if's and while's.
int newLabelWhile(bool inicialitzar = false){
  static int comptador = 1;
  if (inicialitzar) comptador = 0;
  return comptador++;
}

int newLabelIf(bool inicialitzar = false){
  static int comptador = 1;
  if (inicialitzar) comptador = 0;
  return comptador++;
}


codechain indirections(int jumped_scopes,int t)
{
  codechain c;
  if (jumped_scopes==0) {
    c="aload static_link t"+itostring(t);
  }
  else {
    c="load static_link t"+itostring(t);
    for (int i=1;i<jumped_scopes;i++) {
      c=c||"load t"+itostring(t)+" t"+itostring(t);
    }
  }
  return c;
}

/*Retorna la mida d'un tipus, ja sigui basic, elements de l'array, o mida total de l'struct*/
int compute_size(ptype tp)
{
  if (isbasickind(tp->kind)) {
    tp->size=4;
  }
  else if (tp->kind=="array") {
    tp->size=tp->numelemsarray*compute_size(tp->down);
  }
  else if (tp->kind=="struct") {
    tp->size=0;
    for (list<string>::iterator it=tp->ids.begin();it!=tp->ids.end();it++) {
      tp->offset[*it]=tp->size;
      tp->size+=compute_size(tp->struct_field[*it]);
    }
  }
  return tp->size;
}

void gencodevariablesandsetsizes(scope *sc,codesubroutine &cs,bool isfunction=0)
{
  if (isfunction) cs.parameters.push_back("returnvalue");
  for (list<string>::iterator it=sc->ids.begin();it!=sc->ids.end();it++) {
    if (sc->m[*it].kind=="idvarlocal") {
      variable_data vd;
      vd.name="_"+(*it);
      vd.size=compute_size(sc->m[*it].tp);
      cs.localvariables.push_back(vd);
    } else if (sc->m[*it].kind=="idparval" || sc->m[*it].kind=="idparref") {
      compute_size(sc->m[*it].tp);
      cs.parameters.push_back("_"+(*it));
    } else if (sc->m[*it].kind=="idfunc") {
      // Here it is assumed that in tp->right is kept the return value type
      // for the case of functions. If this is not the case you must
      // change this line conveniently in order to force the computation
      // of the size of the type returned by the function.
      compute_size(sc->m[*it].tp->right);
    } else if (sc->m[*it].kind=="idproc") {
      // Nothing to be done.
    }
  }
  cs.parameters.push_back("static_link");
}

codechain GenAddress(AST *a,int t); //Was GenLeft
codechain GenValue(AST *a,int t);   //Was GenRight

/*Funció cridada quan generem els paràmetres d'una funció o procedure*/
void CodeGenRealParams(AST *a,ptype tp,codechain &cpushparam,codechain &cremoveparam,int t)
{
  if (!a) return;
  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;

  /*Generar tots els paràmetres, deixar el resultat dins cpushparam i cremoveparam, ja s'imprimirà des de
   un altre lloc*/
  for (AST * a1 = a; a1 != 0; a1 = a1->right)
    {
      /*Paràmetre per referència*/
      if (tp->kind=="parref")
	{
	  cpushparam=cpushparam
	    ||GenAddress(a1,t);
	} 
      else 
	{  /*El paràmetre és per valor*/
	  if (a->ref)/*i és referenciable*/
	    {
	      if (isbasickind(a->tp->kind))
		{
		  cpushparam=cpushparam
		    ||GenValue(a1,t);
		}
	      else 
		{
		  /*Si no és tipus bàsic, hem de fer una còpia*/
		  cpushparam = cpushparam
		    ||GenAddress(a1,t+1)
		    ||"aload aux_space t" + itostring(t)					
		    ||"addi t"+itostring(t)+" "+itostring(offsetauxspace)+" t"+itostring(t)
		    ||"copy t"+ itostring(t+1) + " t" + itostring(t) +  " " + itostring(a->tp->size);
		  
		  offsetauxspace = offsetauxspace + a->tp->size;
		  if (offsetauxspace > maxoffsetauxspace) maxoffsetauxspace = offsetauxspace;
		}
	    }
	  else /*i no és referenciable*/
	    {
	      cpushparam=cpushparam
		||GenValue(a1,t);
	    }
	}
      cpushparam = cpushparam
	||"pushparam t" + itostring(t);
      cremoveparam = cremoveparam
	||"killparam";
      
      if (tp) /*Continuem amb el següent*/
	tp = tp->right;
    }
  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
}


codechain GenAddress(AST *a,int t)
{
  codechain c;

  if (!a) {
    return c;
  }

  /*Si generem l'adreça d'un identificador, i es troba  a l'scope actual*/
  if (a->kind=="ident" && symboltable.jumped_scopes(a->text)==0) {
    /*En cas que el paràmetre sigui d'una funció o procedure, i sigui per referència o per (valor i no tipus basic),
     *Per ex:  function p(Array * a, Struct s)
     *Ja tenim l'adreça que apunta a l'objecte dins l'identificador, per tant amb un load ens basta.
    */
    if ((symboltable[a->text].kind=="idparref") || (symboltable[a->text].kind=="idparval" && !isbasickind(a->tp->kind)) ) {
      c="load _"+a->text+" t"+itostring(t);
    }
    /*En el cas normal, hem de carregar l'adreça de l'objecte*/
    else {
      c="aload _"+a->text+" t"+itostring(t);
    }
    
    /*Si l'identificador està a fora del nostre scope, hem de calcular les indireccions,
     *afegir l'offset de la variable a l'adreça calculada (que apunta al bloc on està la variable)
     *i finalment, si la variable és un punter hem de carregar el contingut, ja que sinó tindríem un punter a punter.
     */
  } else if(a->kind=="ident" && symboltable.jumped_scopes(a->text)!=0) {
    c=indirections(symboltable.jumped_scopes(a->text),t)
      ||"addi t" + itostring(t) + " offset(" + symboltable.idtable(a->text) + ":_" + a->text + ") t" + itostring(t);
    
    /*Si es parref per a carregar l'adreça em de carregar el valor del parametre*/
    if (symboltable[a->text].kind=="idparref") {
      c=c||"load t"+itostring(t)+" t"+itostring(t);
    }
  }

  /*Generar l'adreça d'un accés a struct:
   *1. Carregar l'adreça de l'struct
   *2. Sumar l'offset de la variable a l'adreça de l'struct (1)
   *ptype.hh, tp->offset[string] retorna un int. És una operació pròpia del map.
   */
  else if (a->kind==".") {
    c=GenAddress(child(a,0),t)||
      "addi t"+itostring(t)+" "+itostring(child(a,0)->tp->offset[child(a,1)->text])+" t"+itostring(t);
  }
  
  /*Generar l'adreça d'una posició de l'Array
   *Només cal obtenir l'adreça base de l'array i sumar-hi N*sizeof(elems), on N pot ser una expressió
   *(per això el GenValue).
   * a->tp->size : Mida dels elements de l'Array
   */
  else if (a->kind=="[") {
    c=GenAddress(child(a,0),t)
      ||GenValue(child(a,1),t+1)
      ||"muli t"+itostring(t+1)+" "+itostring(a->tp->size)+" t"+itostring(t+1)
      ||"addi t"+itostring(t)+ " t"+itostring(t+1)+ " t"+itostring(t);
  }
  
  else {
    cout<<"BIG PROBLEM! No case defined for kind "<<a->kind<<endl;
  }
  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
  return c;
}


codechain GenValue(AST *a,int t)
{
  codechain c;

  if (!a) {
    return c;
  }

  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  if (a->ref)
    {
      if (a->kind=="ident" 
	  && symboltable.jumped_scopes(a->text)==0
	  && isbasickind(symboltable[a->text].tp->kind)
	  && symboltable[a->text].kind!="idparref") 
	{
	  c="load _"+a->text+" t"+itostring(t);
	}

      /*Cas en que tenim A[i] i el tipus dels elements de l'array és bàsic. "a" correspon al node "[",
       *que té com a fills ident de l'array i l'expressió del nº d'elements.
       */
      else if (isbasickind(a->tp->kind))
	{
	  c=GenAddress(a,t)
	    ||"load t"+itostring(t)+" t"+itostring(t);
	}
  
      else if (a->kind=="[")
	{
	  c = GenAddress(a,0)
	    || "iload t0 t0";
	}
      
    } 
  
  else if (a->kind=="intconst") 
    {
      c="iload "+a->text+" t"+itostring(t);
    }

  else if (a->kind=="+")
    {
      c=GenValue(child(a,0),t)|| GenValue(child(a,1),t+1)||
	"addi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
    }  
  else if (a->kind=="-")
    {
      /*Operador unari canvi de signe*/
      if (child(a,1) == 0) 
	{
	  c=GenValue(child(a,0),t)||"mini t"+itostring(t)+" t"+itostring(t);
	}     
      else /*Resta aritmètica*/	
	{
	  c=GenValue(child(a,0),t)|| GenValue(child(a,1),t+1)||
	    "subi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
	}
    }
  else if (a->kind=="/")
    {
      c=GenValue(child(a,0),t)|| GenValue(child(a,1),t+1)||
	"divi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
    }  
  else if (a->kind=="*")
    {
      c=GenValue(child(a,0),t)|| GenValue(child(a,1),t+1)||
	"muli t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
    }

  else if (a->kind=="false")
    {
      c="iload 0 t"+itostring(t);
    }
  else if (a->kind=="true")
    {
      c="iload 1 t"+itostring(t);
    }
   
  else if (a->kind=="<")
    {
      c=GenValue(child(a,0),t) 
	|| GenValue(child(a,1),t+1)
	|| "lesi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
    }
  else if (a->kind==">")
    {
      c=GenValue(child(a,0),t) 
	|| GenValue(child(a,1),t+1)
	|| "grti t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
    }
  else if (a->kind=="=")
    {
      c=GenValue(child(a,0),t) 
	|| GenValue(child(a,1),t+1)
	|| "equi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
    }
  else if (a->kind=="and")
    {
      c=GenValue(child(a,0),t) 
	|| GenValue(child(a,1),t+1)
	|| "land t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
    }
  else if (a->kind=="or")
    {
      c=GenValue(child(a,0),t) 
	|| GenValue(child(a,1),t+1)
	|| "loor t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
    }
  else if (a->kind=="not") {
		c=GenValue(child(a,0),t)||
		"lnot t"+itostring(t)+" t"+itostring(t);
  }
  else if (a->kind=="."){
    c=GenValue(child(a,0),t)
      || "addi t"+itostring(t)+" "+itostring(child(a,0)->tp->offset[child(a,1)->text])+" t"+itostring(t);
  }

  else if (a->kind=="(")
    { 
      //child(a,0)->tp->kind ens diu si és "procedure" o "function".
      //child(a,0)->tp->down és la llista de paràmetres, amb kind parref i parval.
      //Els paràmetres son també de tipus ttypenode (ptype.hh).
      
      codechain cpushparam,cremoveparam;
      bool function=false;

      /*En cas de que hi hagi retorn*/
      if (child(a, 0)->tp->right) {

	function = true;

	/*Si el retorn es tipus bàsic, basta fer un push d'un enter per reservar l'espai*/
	if (isbasickind(child(a, 0)->tp->right->kind)) {
	  cpushparam = cpushparam||"pushparam 0";
	}
	/*Si no és bàsic carreguem l'aux_space, sumem a aux_space l'offset actual, i fem un push del paràmetre.
	 *Llavors haurem d'incrementar l'offsetauxspace segons la mida del retorn (tp->right->size).
	 *maxoffsetauxspace ens serveix per saber el total utilitzat.
	 */
	else
	  {
	    cpushparam = cpushparam
	      ||"aload aux_space t" + itostring(t)
	      ||"addi t" + itostring(t) + " " + itostring(offsetauxspace) + " t" + itostring(t)
	      ||"pushparam t" + itostring(t);
	    
	    /*Al temporal t tenim guardat aux_space, per tant no el volem sobreescriure:*/
	    t++;

	    offsetauxspace = offsetauxspace + child(a, 0)->tp->right->size;
	    if (offsetauxspace > maxoffsetauxspace) maxoffsetauxspace = offsetauxspace;	  
	}
      }
      
      /*Generem els paràmetres: Node AST del primer parametre i ptype de la funcio*/
      CodeGenRealParams(child(child(a, 1), 0), child(a, 0)->tp->down, cpushparam, cremoveparam,t);
      
      /*Carreguem l'Static link*/
      cpushparam = cpushparam
	||indirections(symboltable.jumped_scopes(child(a,0)->text),t)
	||"pushparam t" + itostring(t);
      
      cremoveparam = cremoveparam
	||"killparam";
      
      /*Recuperem el retorn de la funció*/
      if (function) {
	if (isbasickind(child(a, 0)->tp->right->kind)) {
	  cremoveparam = cremoveparam
	    ||"popparam t" + itostring(t);
	}
	else
	  {
	    t--;
	    cremoveparam = cremoveparam
	      ||"killparam";
	  }
      }
      
      /*Cridem a la funció fent els pushparam, després el call, i finalment el killparam i recull de resultats*/
      c=cpushparam
	||"call "+symboltable.idtable(child(a,0)->text)+"_"+child(a,0)->text
	||cremoveparam;      
    }
  
  else 
    {
      cout<<"BIG PROBLEM! No case defined for kind "<<a->kind<<endl;
    }

  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
  return c;
}

/*
 * Aquesta funció és la primera que es crida quan es va a tractar una nova
 * instrucció de codi font d'alt nivell. Podríem dir que són els "pares" de
 * l'AST que venen just després de Program.
 */
codechain CodeGenInstruction(AST *a,string info="")
{
  codechain c;

  if (!a) {
    return c;
  }
  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  offsetauxspace=0;

  if (a->kind=="list")
    {
      for (AST *a1=a->down;a1!=0;a1=a1->right) {
	c=c||CodeGenInstruction(a1,info);
      }
    }

  else if (a->kind==":=") 
    {
      if (isbasickind(child(a,0)->tp->kind)) {
	c=GenAddress(child(a,0),0)||GenValue(child(a,1),1)||"stor t1 t0";
      }
      else if (child(a,1)->ref) {
	c=GenAddress(child(a,0),0)||GenAddress(child(a,1),1)||"copy t1 t0 "+itostring(child(a,1)->tp->size);
      }
      else {
	c=GenAddress(child(a,0),0)||GenValue(child(a,1),1)||"copy t1 t0 "+itostring(child(a,1)->tp->size);
      }
    } 

  else if (a->kind=="write" || a->kind=="writeln") 
    {
      if (child(a,0)->kind=="string") {
	c="wris "+child(a,0)->text;
      } 
      else {
	c=GenValue(child(a,0),0)||"wrii t0";
      }
      
      if (a->kind=="writeln") {
	c=c||"wrln";
      }
    }

  else if (a->kind=="while")
    {
      int label=newLabelWhile();
      
      c="etiq while_"+itostring(label)
	||GenValue(child(a,0),0)
	||"fjmp t0 endwhile_"+itostring(label)
	||CodeGenInstruction(child(a,1),info)
	||"ujmp while_"+itostring(label)
	||"etiq endwhile_"+itostring(label);
    }

  else if (a->kind=="if")
    {
      int label = newLabelIf();

      /*En cas de voler etiqueta inici de if, afegir: "etiq if_"+itostring(label)"*/
      c=GenValue(child(a,0),0);
	
      /*Si no hi ha else*/
      if (!child(a,2)) 
	{
	  c = c||"fjmp t0 endif_"+itostring(label)
	    ||CodeGenInstruction(child(a,1),info);
	}
      else
	{
	  c = c||"fjmp t0 else_"+itostring(label)
	    ||CodeGenInstruction(child(a,1),info)
	    ||"ujmp endif_"+itostring(label)
	    ||"etiq else_"+itostring(label)
	    ||CodeGenInstruction(child(a,2),info);
	}
      c = c||"etiq endif_"+itostring(label);	
    }

  else if (a->kind=="("){
    c=GenValue(a,0);
  }
  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
  
  return c;
}

void CodeGenSubroutine(AST *a,list<codesubroutine> &l)
{
  codesubroutine cs;

  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  string idtable=symboltable.idtable(child(a,0)->text);
  cs.name=idtable+"_"+child(a,0)->text;
  symboltable.push(a->sc);
  symboltable.setidtable(idtable+"_"+child(a,0)->text);

  /*Crea el troç de codi de definició variables.*/
  gencodevariablesandsetsizes(a->sc,cs,a->kind=="function");

  /*Subrutines encastades*/
  for (AST *a1=child(child(a,2),0);a1!=0;a1=a1->right) {
    CodeGenSubroutine(a1,l);
  }
  maxoffsetauxspace=0; 
  newLabelIf(true); 
  newLabelWhile(true);
  cs.c=CodeGenInstruction(child(a,3));
  
  /*Si és un funcio s'ha de retornar el valor*/
  if (child(a,4))
    {
      if (isbasickind(child(a,4)->tp->kind))
	{
	  cs.c = cs.c
	    ||GenValue(child(a,4),0)
	    ||"stor t0 returnvalue";
	} 
      else
	{
	  if (child(a,4)->ref)
	    {
	      cs.c = cs.c
		||GenAddress(child(a,4),1)
		||"load returnvalue t0"
		||"copy t1 t0 "+itostring(child(a,4)->tp->size);
	    }
	  else
	    {
	    cs.c = cs.c
	      ||GenValue(child(a,4),1)
	      ||"load returnvalue t0"
	      ||"copy t1 t0 " + itostring(child(a,4)->tp->size);
	    }
	}
    }
  cs.c = cs.c
    ||"retu";
  
  if (maxoffsetauxspace>0)
    {
      variable_data vd;
      vd.name="aux_space";
      vd.size=maxoffsetauxspace;
      cs.localvariables.push_back(vd);
    }
  symboltable.pop();
  l.push_back(cs);
  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
}

void CodeGen(AST *a,codeglobal &cg)
{
  initnumops();
  securemodeon();
  cg.mainsub.name="program";
  symboltable.push(a->sc);
  symboltable.setidtable("program");
  gencodevariablesandsetsizes(a->sc,cg.mainsub);
  for (AST *a1=child(child(a,1),0);a1!=0;a1=a1->right) {
    CodeGenSubroutine(a1,cg.l);
  }
  maxoffsetauxspace=0; newLabelIf(true); newLabelWhile(true);
  cg.mainsub.c=CodeGenInstruction(child(a,2))||"stop";
  if (maxoffsetauxspace>0) {
    variable_data vd;
    vd.name="aux_space";
    vd.size=maxoffsetauxspace;
    cg.mainsub.localvariables.push_back(vd);
  }
  symboltable.pop();
}

