#ifndef AST_H_
#define AST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
    char * token;
    char * lval;
    struct node * sibling;
    struct node * child;
};

struct node * create_node          (char * , char *  );
struct node * create_field_decl    (struct node * , char * );
struct node * create_var_decl      (struct node * , char * );
struct node * create_program       (char * , struct node * );
struct node * create_param_decl    (struct node * , char * );
struct node * create_method_decl   (struct node * , struct node * ) ;
struct node * create_method_header (struct node * , char * , struct node * );
struct node * create_method_params (struct node * );
struct node * create_method_body   (struct node * );
struct node * create_expression    (char *, struct node * , struct node * );
struct node * check_one_child      (struct node * node);

void          add_sibling          (struct node *, struct node *);
void          add_child            (struct node *, struct node *);

void          print_tree           (struct node *, int);
void          free_tree            (struct node *);

#endif //ast.h
