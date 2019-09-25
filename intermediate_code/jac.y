%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "symtab.h"
    #include "ast.h"
    #include "code.h"

    int yylex(void);
    void yyerror (const char *s);
    void yylex_destroy();

    extern int line, col;
    int flag=0, token_flag=0, err = 0, semantics_err = 0;
    extern const char * yytext;
    extern int actual_line, actual_col;

    struct node * root = NULL;
%}
%union {
 
  struct node * node;
   struct var_data * var_data;

}

%token BOOL BOOLLIT CLASS DO DOTLENGTH DOUBLE ELSE IF INT PARSEINT
%token PRINT PUBLIC RETURN STATIC STRING VOID WHILE OCURV CCURV OBRACE
%token CBRACE OSQUARE CSQUARE AND OR LT GT EQ NEQ LEQ GEQ PLUS MINUS
%token STAR DIV MOD NOT ASSIGN SEMI COMMA RESERVED DECLIT REALLIT STRLIT ID

%type <node> Program AuxProgram MethodDecl AuxMethodBody FormalParams AuxFormalParams Assigment ParseArgs MethodInvocation Expr1
%type <node> Type AuxFieldDecl FieldDecl MethodHeader MethodBody VarDecl AuxVarDecl Expr AuxMethodInvocation Statement AuxStatement

%type <var_data> ID BOOLLIT STRLIT DECLIT REALLIT AND OR LT GT EQ NEQ LEQ GEQ PLUS RETURN
%type <var_data> MINUS DIV MOD STAR NOT ASSIGN DOTLENGTH PARSEINT PRINT IF WHILE DO


%right THEN ELSE
%right ASSIGN
%left OR
%left AND
%left EQ NEQ
%left GEQ GT LEQ LT
%left PLUS MINUS
%left STAR DIV MOD
%left NOT UNARY_M

%%

Program : CLASS ID OBRACE AuxProgram CBRACE {root = create_program($2, $4);}
        ;

AuxProgram : %empty {$$ = NULL;}
        | AuxProgram FieldDecl {if($1 != NULL) {
                                    add_sibling($1, $2);
                                    $$ = $1;
                                } else {
                                    $$ = $2;
                                }}
        | AuxProgram MethodDecl {if($1 != NULL) {
                                    add_sibling($1, $2);
                                    $$=$1;
                                } else{
                                    $$ = $2;
                                }}
        | AuxProgram SEMI {$$ = $1;}
        ;

FieldDecl : PUBLIC STATIC AuxFieldDecl SEMI {$$=$3;}
        |   error SEMI { $$ = create_node("Block", NULL);}
        ;

AuxFieldDecl : Type ID {$$ = create_field_decl($1, $2);}
        | AuxFieldDecl COMMA ID {struct node* t = create_node("FieldDecl", NULL);
                                add_child(t, create_node($$->child->token, NULL));
                                add_child(t, create_node("Id", $3));
                                add_sibling($$, t);
                                }
        ;

MethodDecl : PUBLIC STATIC MethodHeader MethodBody { $$ = create_method_decl($3, $4);}
        ;

MethodHeader: Type ID OCURV FormalParams CCURV { $$ = create_method_header($1, $2, $4); }
        | VOID ID OCURV FormalParams CCURV { $$ = create_method_header(create_node("Void", NULL), $2, $4); }
        ;

MethodBody : OBRACE AuxMethodBody CBRACE { $$ = create_method_body($2); }
        ;

AuxMethodBody : %empty {$$=NULL;}
        | AuxMethodBody VarDecl {   if($1 != NULL) {
                                        add_sibling($1, $2);
                                        $$ = $1;
                                    } else {
                                        $$ = $2;
                                    }}
        | AuxMethodBody Statement {
                                    if($1 != NULL) {
                                        add_sibling($1, $2);
                                        $$ = $1;
                                    } else {
                                        $$ = $2;
                                    }}
        ;

FormalParams : %empty {$$=NULL;}
        | Type ID AuxFormalParams { struct node * t = create_param_decl($1, $2);
                                    add_sibling(t, $3);
                                    $$ = create_method_params(t);
                                  }
        | STRING OSQUARE CSQUARE ID { struct node *t = create_param_decl(create_node("StringArray", NULL), $4);
                                      $$ = create_method_params(t);
                                    }
        ;

AuxFormalParams : %empty {$$ = NULL;}
        | AuxFormalParams COMMA Type ID { struct node *t = create_param_decl($3, $4);
                                        if($1 != NULL) {
                                            add_sibling($1, t);
                                            $$ = $1;
                                        } else {
                                            $$ = t;
                                        }}
        ;

VarDecl : AuxVarDecl SEMI { $$ = $1;}
        ;

AuxVarDecl : Type ID { $$ = create_var_decl($1, $2); }
        |  AuxVarDecl COMMA ID {struct node* t = create_node("VarDecl", NULL);
                                  add_child(t, create_node($$->child->token, NULL));
                                  add_child(t, create_node("Id", $3));
                                  add_sibling($$, t);
                                  }
        ;

Type :    BOOL { $$ = create_node("Bool", NULL);}
        | INT { $$ = create_node("Int", NULL);}
        | DOUBLE { $$ = create_node("Double", NULL);}
        ;

Statement : OBRACE AuxStatement CBRACE {if($2 != NULL){
                                            $$=create_node("Block", NULL);
                                            add_child($$, $2);
                                            $$=check_one_child($$);
                                        } else
                                            $$=$2;
                                        }
        | IF OCURV Expr CCURV Statement	%prec THEN {$$ = create_node("If", $1); add_child($$, $3);
                                                      add_child($$, $5 == NULL ? create_node("Block", NULL) : $5);
                                                      add_child($$,create_node("Block", NULL));
                                                    }
        | IF OCURV Expr CCURV Statement ELSE Statement {$$ = create_node("If", $1);
                                                        add_child($$, $3);
                                                        add_child($$, $5 == NULL ? create_node("Block", NULL) : $5);
                                                        add_child($$, $7 == NULL ? create_node("Block", NULL) : $7);
                                                       }
        | WHILE OCURV Expr CCURV Statement {$$ = create_node("While", $1);
                                            add_child($$, $3);
                                            add_child($$, $5 == NULL ? create_node("Block", NULL) : $5);
                                           }
        | DO Statement WHILE OCURV Expr CCURV SEMI {$$=create_node("DoWhile", $1);
                                                    add_child($$, $2 == NULL ? create_node("Block", NULL) : $2);
                                                    add_child($$, $5);
                                                   }
        | PRINT OCURV Expr CCURV SEMI   {$$ = create_node("Print", $1); add_child($$, $3);}
        | PRINT OCURV STRLIT CCURV SEMI {$$ = create_node("Print", $1); add_child($$, create_node("StrLit", $3));}
        | Assigment SEMI                {$$ = $1;}
        | MethodInvocation SEMI         {$$ = $1;}
        | ParseArgs SEMI                {$$ = $1;}
        | SEMI                          {$$ = NULL;}
        | RETURN Expr SEMI              {$$ = create_node("Return", $1);  add_child($$, $2);}
        | RETURN SEMI                   {$$ = create_node("Return", $1);}
        | error SEMI                    {$$ = create_node("Block", NULL);}
        ;

AuxStatement : Statement AuxStatement  { if($1 != NULL && $2 != NULL){
                                            add_sibling($1,$2);
                                            $$ = $1;
                                        } else if($1 != NULL){
                                            $$ = $1;
                                        } else if($2 != NULL) {
                                            $$ = $2;
                                        }
                                       }
        | %empty {$$ = NULL;}
        ;

Assigment : ID ASSIGN Expr              {$$=create_expression("Assign", $2, create_node("Id", $1), $3);}
        ;

MethodInvocation : ID OCURV CCURV       {$$ = create_node("Call", NULL); $$->line = $1->line; $$->col = $1->col; add_child($$, create_node("Id", $1));}
        | ID OCURV  AuxMethodInvocation CCURV  { $$ = create_node("Call", NULL); $$->line = $1->line; $$->col = $1->col; add_child($$, create_node("Id", $1));
                                                 add_child($$, $3);
                                               }
        | ID OCURV error CCURV {$$ = create_node("Block", NULL);}
        ;

AuxMethodInvocation : Expr {$$ = $1;}
        | AuxMethodInvocation COMMA Expr {add_sibling($1,$3); $$ = $1;}
        ;

ParseArgs : PARSEINT OCURV ID OSQUARE Expr CSQUARE CCURV  {$$ = create_node("ParseArgs", $1);
                                                           add_child($$, create_node("Id", $3));
                                                           add_child($$, $5);
                                                          }
        | PARSEINT OCURV error CCURV { $$ = create_node("Block", NULL);}
        ;

Expr:   Expr1                             {$$ = $1;}
        | Assigment                       {$$ = $1;}
        ;

Expr1 :   MethodInvocation                {$$ = $1;}
        | ParseArgs                       {$$ = $1;}
        | Expr1 AND Expr1                 {$$=create_expression("And", $2, $1, $3);}
        | Expr1 OR Expr1                  {$$=create_expression("Or", $2, $1, $3);}
        | Expr1 EQ Expr1                  {$$=create_expression("Eq", $2, $1, $3);}
        | Expr1 GEQ Expr1                 {$$=create_expression("Geq", $2, $1, $3);}
        | Expr1 GT Expr1                  {$$=create_expression("Gt", $2, $1, $3);}
        | Expr1 LEQ Expr1                 {$$=create_expression("Leq", $2, $1, $3);}
        | Expr1 LT Expr1                  {$$=create_expression("Lt", $2, $1, $3);}
        | Expr1 NEQ Expr1                 {$$=create_expression("Neq", $2, $1, $3);}
        | Expr1 PLUS Expr1                {$$=create_expression("Add", $2, $1, $3);}
        | Expr1 MINUS Expr1               {$$=create_expression("Sub", $2, $1, $3);}
        | Expr1 STAR Expr1                {$$=create_expression("Mul", $2, $1, $3);}
        | Expr1 DIV Expr1                 {$$=create_expression("Div", $2, $1, $3);}
        | Expr1 MOD Expr1                 {$$=create_expression("Mod", $2, $1, $3);}
        | PLUS Expr1    %prec UNARY_M     {$$=create_expression("Plus", $1, $2, NULL);}
        | MINUS Expr1   %prec UNARY_M     {$$=create_expression("Minus", $1, $2, NULL);}
        | NOT Expr1                       {$$=create_expression("Not", $1, $2, NULL);}
        | ID DOTLENGTH                    {$$=create_expression("Length", $2, create_node("Id", $1), NULL);}
        | ID                              {$$=create_node("Id", $1);}
        | OCURV Expr CCURV                {$$=$2;}
        | BOOLLIT                         {$$=create_node("BoolLit", $1);}
        | DECLIT                          {$$=create_node("DecLit", $1);}
        | REALLIT                         {$$=create_node("RealLit", $1);}
        | OCURV error CCURV               {$$ = create_node("Block", NULL);}
        ;

%%

int main(int argc, char * argv[]){
    if(argc == 2) {
        if(!strcmp(argv[1],"-l")){
            flag = 1;
            yylex();
        } else if(!strcmp(argv[1], "-t")) {
            token_flag = 1;
            yyparse();
            if(!err){
            print_tree(root, 0, 0);
            free_tree(root);
        }
        } else if(!strcmp(argv[1], "-1")){
            yylex();
        } else if(!strcmp(argv[1], "-2")){
            token_flag = 1;
            yyparse();
            free_tree(root);
        } else if(!strcmp(argv[1], "-3")){
            token_flag = 1;
            yyparse(); 
            if(!err) {
                struct symtab * t = create_tables_tree_annotation(root);
                free_symtab(t);
            }
            free_tree(root);          
        } else if(!strcmp(argv[1], "-s")){
            token_flag = 1;
            yyparse();
            if(!err) {
                struct symtab * t = create_tables_tree_annotation(root);
                print_symtab(t);
                print_tree(root,0, 1);
                free_symtab(t);
            }
            free_tree(root);
        } else {
            token_flag = 1;
            yyparse();
            free_tree(root);
        }
    }
    else {
        token_flag = 1;
        yyparse();
        if(!err) {
            struct symtab * t = create_tables_tree_annotation(root);
            if(!semantics_err){
                generate_code(root,t);
            }
            free_symtab(t);
        }
        free_tree(root);
    }
    yylex_destroy();
    return 0;
}

void yyerror (const char *s) {
    err = 1;
    printf("Line %d, col %d: %s: %s\n", actual_line, actual_col, s, yytext);
}
