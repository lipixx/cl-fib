#header
<<
#include "charptr.h"
>>

<<
#include "charptr.c"
int main() {
    ANTLR(expr(), stdin);
}
>>

#lexclass START
#token NUM "[0-9]+"
#token PLUS "\+"
#token MINUS "\-"
#token MUL "\*"
#token DIV "\/"
#token SPACE "[\ \n]" << zzskip(); >>
input: expr "@";
expr: NUM ((PLUS | MINUS) NUM)*; 
