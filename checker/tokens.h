#ifndef tokens_h
#define tokens_h
/* tokens.h -- List of labelled tokens and stuff
 *
 * Generated from: cl.g
 *
 * Terence Parr, Will Cohen, and Hank Dietz: 1989-2001
 * Purdue University Electrical Engineering
 * ANTLR Version 1.33MR33
 */
#define zzEOF_TOKEN 1
#define INPUTEND 1
#define PROGRAM 2
#define ENDPROGRAM 3
#define PROCEDURE 4
#define ENDPROCEDURE 5
#define FUNCTION 6
#define ENDFUNCTION 7
#define FLOOR 8
#define CASTINGR 9
#define CASTINGI 10
#define COMA 11
#define VARS 12
#define VAL 13
#define REF 14
#define RETURN 15
#define ENDVARS 16
#define INT 17
#define REAL 18
#define BOOL 19
#define STRUCT 20
#define ARRAY 21
#define OF 22
#define ENDSTRUCT 23
#define READ 24
#define WRITE 25
#define WRITELN 26
#define IF 27
#define THEN 28
#define ELSE 29
#define ENDIF 30
#define WHILE 31
#define DO 32
#define ENDWHILE 33
#define DOT 34
#define OPENPAR 35
#define CLOSEPAR 36
#define OPENBRACK 37
#define CLOSEBRACK 38
#define NOT 39
#define DIV 40
#define MUL 41
#define PLUS 42
#define MINUS 43
#define EQ 44
#define GT 45
#define LT 46
#define AND 47
#define OR 48
#define ASIG 49
#define TRUEKWD 50
#define FALSEKWD 51
#define STRING 52
#define IDENT 53
#define INTCONST 54
#define REALCONST 55
#define COMMENT 56
#define WHITESPACE 57
#define NEWLINE 58
#define LEXICALERROR 59

#ifdef __USE_PROTOS
void program(AST**_root);
#else
extern void program();
#endif

#ifdef __USE_PROTOS
void dec_vars(AST**_root);
#else
extern void dec_vars();
#endif

#ifdef __USE_PROTOS
void l_dec_blocs(AST**_root);
#else
extern void l_dec_blocs();
#endif

#ifdef __USE_PROTOS
void l_instrs(AST**_root);
#else
extern void l_instrs();
#endif

#ifdef __USE_PROTOS
void l_dec_vars(AST**_root);
#else
extern void l_dec_vars();
#endif

#ifdef __USE_PROTOS
void dec_var(AST**_root);
#else
extern void dec_var();
#endif

#ifdef __USE_PROTOS
void dec_bloc(AST**_root);
#else
extern void dec_bloc();
#endif

#ifdef __USE_PROTOS
void dec_function(AST**_root);
#else
extern void dec_function();
#endif

#ifdef __USE_PROTOS
void dec_procedure(AST**_root);
#else
extern void dec_procedure();
#endif

#ifdef __USE_PROTOS
void procedure_instr(AST**_root);
#else
extern void procedure_instr();
#endif

#ifdef __USE_PROTOS
void dec_params(AST**_root);
#else
extern void dec_params();
#endif

#ifdef __USE_PROTOS
void l_dec_params(AST**_root);
#else
extern void l_dec_params();
#endif

#ifdef __USE_PROTOS
void dec_param(AST**_root);
#else
extern void dec_param();
#endif

#ifdef __USE_PROTOS
void call_params(AST**_root);
#else
extern void call_params();
#endif

#ifdef __USE_PROTOS
void constr_type(AST**_root);
#else
extern void constr_type();
#endif

#ifdef __USE_PROTOS
void field(AST**_root);
#else
extern void field();
#endif

#ifdef __USE_PROTOS
void instruction(AST**_root);
#else
extern void instruction();
#endif

#ifdef __USE_PROTOS
void expression(AST**_root);
#else
extern void expression();
#endif

#ifdef __USE_PROTOS
void expr_comparacio(AST**_root);
#else
extern void expr_comparacio();
#endif

#ifdef __USE_PROTOS
void expr_aritm(AST**_root);
#else
extern void expr_aritm();
#endif

#ifdef __USE_PROTOS
void expr_muldiv(AST**_root);
#else
extern void expr_muldiv();
#endif

#ifdef __USE_PROTOS
void exprsimple(AST**_root);
#else
extern void exprsimple();
#endif

#ifdef __USE_PROTOS
void expr_id(AST**_root);
#else
extern void expr_id();
#endif

#endif
extern SetWordType zzerr1[];
extern SetWordType setwd1[];
extern SetWordType zzerr2[];
extern SetWordType zzerr3[];
extern SetWordType zzerr4[];
extern SetWordType setwd2[];
extern SetWordType zzerr5[];
extern SetWordType zzerr6[];
extern SetWordType zzerr7[];
extern SetWordType zzerr8[];
extern SetWordType zzerr9[];
extern SetWordType zzerr10[];
extern SetWordType zzerr11[];
extern SetWordType setwd3[];
extern SetWordType zzerr12[];
extern SetWordType zzerr13[];
extern SetWordType setwd4[];
extern SetWordType zzerr14[];
extern SetWordType zzerr15[];
extern SetWordType zzerr16[];
extern SetWordType setwd5[];
