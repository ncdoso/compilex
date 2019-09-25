#ifndef AST_H_
#define AST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_types.h"
#include "util.h"

struct node {
    char * token;
    char * lval;
    struct tables_content_type * annotation;
    int line, col, temp_var, phi_label;
    struct table_line * table_entry;
    struct node * sibling;
    struct node * child;
};

struct var_data{
    char * strlit;
    int line, col;
};

struct node * create_node          (char * , struct var_data *  );
struct node * create_field_decl    (struct node * , struct var_data * );
struct node * create_var_decl      (struct node * , struct var_data * );
struct node * create_program       (struct var_data * , struct node * );
struct node * create_param_decl    (struct node * , struct var_data * );
struct node * create_method_decl   (struct node * , struct node * ) ;
struct node * create_method_header (struct node * , struct var_data * , struct node * );
struct node * create_method_params (struct node * );
struct node * create_method_body   (struct node * );
struct node * create_expression    (char *, struct var_data * , struct node * , struct node * );
struct node * check_one_child      (struct node * node);

void          add_sibling          (struct node *, struct node *);
void          add_child            (struct node *, struct node *);
void          print_tree           (struct node *, int, int);
void          free_tree            (struct node *);

#endif //ast.h
