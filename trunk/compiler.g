#header
<<
#include <string>
#include <iostream>

using namespace std;

typedef struct {
    string kind;
    string text;
} Attrib;
void zzcr_attr(Attrib *attr,int type,char *text);

#define AST_FIELDS string kind;string text;
#define zzcr_ast(as,attr,ttype,textt) as=new AST;\
(as)->kind=(attr)->kind;(as)->text=(attr)->text;\
(as)->right=NULL;(as)->down=NULL;
>>

<<
#include <stdlib.h>
#include <math.h>

void zzcr_attr(Attrib *attr,int type,char *text)
{
    attr->kind=text;
    attr->text="";
    if (type==NUM) {
        attr->kind="intconst";
        attr->text=text;
    }
}
void ASTPrintIndent(AST *a,string s)
{
  if (a==NULL) return;
  cout<<s<<" "<<a->kind;
  if (a->text!="") cout<<"("<<a->text<<")";
  cout<<endl;
  ASTPrintIndent(a->down,s+" |");
  ASTPrintIndent(a->right,s);
}

void ASTPrint(AST *a)
{
  cout<<endl;
  ASTPrintIndent(a,"");
}

int main()
{
  AST *root=NULL;
  ANTLR(expr(&root),stdin);
  ASTPrint(root);
}
>>

#lexclass START
#token NUM "[0-9]+"
#token PLUS "\+"
#token SPACE "[\ \n]" << zzskip();>>

expr: NUM (PLUS^ NUM )*;
