%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fstream>
#include "traverser.h"

#define YYLTYPE LocType

#define MAX_LINE_LENG      256
extern int line_no, col_no, opt_list;
extern char buffer[MAX_LINE_LENG];
extern FILE *yyin;        /* declared by lex */
extern char *yytext;      /* declared by lex */
extern int yyleng;

int yylex();
int yyparse();
void yyerror(const char *msg);

int pass_error = 0;
char *output = NULL;
Node* root = NULL;
%}

%locations

%token PROGRAM VAR ARRAY OF INTEGER REAL STRING FUNCTION PROCEDURE PBEGIN END IF THEN ELSE WHILE DO NOT AND OR

%token LPAREN RPAREN SEMICOLON DOT COMMA COLON LBRACE RBRACE DOTDOT ASSIGNMENT ADDOP SUBOP MULOP DIVOP LTOP GTOP EQOP GETOP LETOP NEQOP

%token IDENTIFIER REALNUMBER INTEGERNUM SCIENTIFIC LITERALSTR

%union {
  int ival;
  double dval;
  char* sval;
  IDType tval;
  OpType oval;
  Node* nval;
}

%type <ival> INTEGERNUM
%type <dval> REALNUMBER SCIENTIFIC
%type <sval> IDENTIFIER LITERALSTR
%type <tval> standard_type
%type <oval> addop mulop relop
%type <nval> prog identifier_list declarations type subprogram_declarations subprogram_head arguments parameter_list parameter compound_statement statement_list statement variable tail procedure_statement expression_list expression boolexpression simple_expression term factor signed_num num

%%

    /* define your snytax here */
    /* @n return the sturct LocType of "n-th node", ex: @1 return the PROGRAM node's locType
       $n return the $$ result you assigned to the rule, ex: $1 */
prog: PROGRAM IDENTIFIER LPAREN identifier_list RPAREN SEMICOLON
      declarations
      subprogram_declarations
      compound_statement
      DOT 
{
    root = new Node{NodeType::PROG, @2, {$4, $7, $8, $9}, nullptr};
    root->metadata.sval = $2;
    /*
    printf("program node is @ line: %d, column: %d\n",
                @1.first_line, @1.first_column);
    yylval.val, yylval.text, yylval.dval to get the data (type defined in %union) you assigned by scanner.
    */
};

identifier_list: IDENTIFIER
{
    $$ = new Node{NodeType::ID_LIST, @1, {}, nullptr};
    $$->metadata.sval = $1;
}
               | IDENTIFIER COMMA identifier_list 
{
    $$ = new Node{NodeType::ID_LIST, @1, {}, $3};
    $$->metadata.sval = $1;
}
               ;
               
declarations: VAR identifier_list COLON type SEMICOLON declarations 
{
    $$ = new Node{NodeType::DECL_LIST, {}, {$2, $4}, $6};
}
            |
{
    $$ = nullptr;
}
            ;
            
type: standard_type
{
    $$ = new Node{NodeType::TYPE, @1, {}, nullptr};
    $$->metadata.tval = $1;
}
    | ARRAY LBRACE INTEGERNUM DOTDOT INTEGERNUM RBRACE OF type
{
    Node *lower_bound = new Node{NodeType::LITERAL_INT, @3, {}, nullptr};
    lower_bound->metadata.ival = $3;
    Node *upper_bound = new Node{NodeType::LITERAL_INT, @5, {}, nullptr};
    upper_bound->metadata.ival = $5;
    $$ = new Node{NodeType::TYPE, @1, {lower_bound, upper_bound, $8}, nullptr};
    $$->metadata.tval = IDType::ARRAY;
}
    ;
    
standard_type: INTEGER
{
    $$ = IDType::INT;
}
             | REAL
{
    $$ = IDType::REAL;
}
             | STRING
{
    $$ = IDType::STRING;
}
             ;

subprogram_declarations: subprogram_head 
                         declarations 
                         subprogram_declarations 
                         compound_statement SEMICOLON
                         subprogram_declarations
{
    $$ = new Node{NodeType::SUBPROG_DECL_LIST, {}, {$1, $2, $3, $4}, $6};
}
                       |
{
    $$ = nullptr;
}
                       ;

subprogram_head: FUNCTION IDENTIFIER arguments COLON type SEMICOLON
{
    $$ = new Node{NodeType::SUBPROG_HEAD, @1, {$3, $5}, nullptr};
    $$->metadata.sval = $2;
}
               | PROCEDURE IDENTIFIER arguments SEMICOLON
{
    Node *type_node = new Node{NodeType::TYPE, {}, {}, nullptr};
    type_node->metadata.tval = IDType::VOID;
    $$ = new Node{NodeType::SUBPROG_HEAD, @1, {$3, type_node}, nullptr};
    $$->metadata.sval = $2;
}
               ;

arguments: LPAREN parameter_list RPAREN
{
    $$ = $2;
}
         |
{
    $$ = nullptr;
}
         ;
         
parameter_list: parameter
{
    $$ = $1;
}
              | parameter SEMICOLON parameter_list
{
    $$ = $1;
    $$->next = $3;
}
              ;

parameter: optional_var identifier_list COLON type
{
    $$ = new Node{NodeType::PARAM_LIST, {}, {$2, $4}, nullptr};
}
              
optional_var: VAR
            |
            ;

compound_statement: PBEGIN
                    statement_list 
                    END
{
    $$ = $2;
}
                  ;
                   
statement_list: statement
{
    $$ = new Node{NodeType::STMT_LIST, {}, {$1}, nullptr};
}
              | statement SEMICOLON statement_list
{
    $$ = new Node{NodeType::STMT_LIST, {}, {$1}, $3};
}
              ;
              
statement: variable ASSIGNMENT expression
{
    $$ = new Node{NodeType::ASSIGN, {}, {$1, $3}, nullptr};
}
         | procedure_statement
{
    $$ = $1;
}
         | compound_statement
{
    $$ = $1;
}
         | IF expression THEN statement ELSE statement
{
    $$ = new Node{NodeType::IF, {}, {$2, $4, $6}, nullptr};
}
         | WHILE expression DO statement
{
    $$ = new Node{NodeType::WHILE, {}, {$2, $4}, nullptr};
}
         |
{
    $$ = nullptr;
}
         ;

variable: IDENTIFIER tail
{
    $$ = new Node{NodeType::VAR, @1, {$2}, nullptr};
    $$->metadata.sval = $1;
}
        ;

tail: LBRACE expression RBRACE tail
{
    $$ = new Node{NodeType::EXPR_LIST, {}, {$2}, $4};
}
    |
{
    $$ = nullptr;
}
    ;

procedure_statement: IDENTIFIER
{
    $$ = new Node{NodeType::PROCEDURE, @1, {nullptr}, nullptr};
    $$->metadata.sval = $1;
}
                   | IDENTIFIER LPAREN expression_list RPAREN
{
    $$ = new Node{NodeType::PROCEDURE, @1, {$3}, nullptr};
    $$->metadata.sval = $1;
}
                   ;
                   
expression_list: expression
{
    $$ = new Node{NodeType::EXPR_LIST, {}, {$1}, nullptr};
}
               | expression COMMA expression_list
{
    $$ = new Node{NodeType::EXPR_LIST, {}, {$1}, $3};
}
               ;
               
expression: boolexpression
{
    $$ = $1;
}
          | boolexpression AND boolexpression
{
    $$ = new Node{NodeType::OP, @2, {$1, $3}, nullptr};
    $$->metadata.oval = OpType::AND;
}
          | boolexpression OR boolexpression
{
    $$ = new Node{NodeType::OP, @2, {$1, $3}, nullptr};
    $$->metadata.oval = OpType::OR;
}
          ;
          
boolexpression: simple_expression
{
    $$ = $1;
}
              | simple_expression relop simple_expression
{
    $$ = new Node{NodeType::OP, @2, {$1, $3}, nullptr};
    $$->metadata.oval = $2;
}
              ;
              
simple_expression: term
{
    $$ = $1;
}
                 | simple_expression addop term
{
    $$ = new Node{NodeType::OP, @2, {$1, $3}, nullptr};
    $$->metadata.oval = $2;
}
                 ;
                 
term: factor
{
    $$ = $1;
}
    | term mulop factor
{
    $$ = new Node{NodeType::OP, @2, {$1, $3}, nullptr};
    $$->metadata.oval = $2;
}
    ;
    
factor: variable
{
    $$ = $1;
}
      | IDENTIFIER LPAREN expression_list RPAREN
{
    $$ = new Node{NodeType::VAR, @1, {$3}, nullptr};
    $$->metadata.sval = $1; 
}
      | signed_num
{
    $$ = $1;
}
      | LITERALSTR
{
    $$ = new Node{NodeType::LITERAL_STR, @1, {}, nullptr};
    $$->metadata.sval = $1;
}
      | LPAREN expression RPAREN
{
    $$ = $2;
}
      | NOT factor
{
    $$ = new Node{NodeType::NOT, {}, {$2}, nullptr};
}
      ;
      
addop: ADDOP
{
    $$ = OpType::ADD;
}
     | SUBOP
{
    $$ = OpType::SUB;
}
     ;

mulop: MULOP
{
    $$ = OpType::MUL;
}
     | DIVOP
{
    $$ = OpType::DIV;
}
     ;
     
relop: LTOP
{
    $$ = OpType::LT;
}
     | GTOP
{
    $$ = OpType::GT;
}
     | EQOP
{
    $$ = OpType::EQ;
}
     | LETOP
{
    $$ = OpType::LET;
}
     | GETOP
{
    $$ = OpType::GET;
}
     | NEQOP
{
    $$ = OpType::NEQ;
}
     ;

signed_num: num
{
    $$ = $1;
}
          | SUBOP signed_num
{
    $$ = new Node{NodeType::NEGATE, {}, $2, nullptr};
}
          ;
     
num: REALNUMBER
{
    $$ = new Node{NodeType::LITERAL_DBL, @1, {}, nullptr};
    $$->metadata.dval = $1;
}
   | INTEGERNUM
{
    $$ = new Node{NodeType::LITERAL_INT, @1, {}, nullptr};
    $$->metadata.ival = $1;
}
   | SCIENTIFIC
{
    $$ = new Node{NodeType::LITERAL_DBL, @1, {}, nullptr};
    $$->metadata.dval = $1;
}
   ;

%%

void yyerror(const char *msg) {
    fprintf(stderr,
            "[ERROR] line %4d:%3d %s, Unmatched token: %s\n",
            line_no, col_no - yyleng, buffer, yytext);
    pass_error = 1;
}

int main(int argc, char *argv[]) {
    char c;
    while((c=getopt(argc, argv, "o:")) != -1){
      switch(c){
        case 'o':
          output = optarg;
          break;
        case '?':
            fprintf(stderr, "Illegal option:-%c\n", isprint(optopt)?optopt:'#');
            break;
        default:
            fprintf( stderr, "Usage: %s [-o output] filename\n", argv[0]), exit(0);
            break;
      }
    }

    FILE *fp = argc == 1 ? stdin : fopen(argv[optind], "r");

    if(fp == NULL)
        fprintf( stderr, "Open file error\n" ), exit(-1);

    yyin = fp;
    yyparse();
    
    std::string out_str(output);
    std::ofstream out_file(out_str);
    
    std::string basename = out_str.substr(out_str.find_last_of('/') + 1);
    basename = basename.substr(0, basename.find_last_of('.'));
    
    Traverser traverser(out_file, basename);
    if (!pass_error && root) {
        traverser.gen_prog(root);
    }
    
    out_file.close();
    return 0;
}
