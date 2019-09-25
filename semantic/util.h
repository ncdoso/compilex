#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "data_types.h"

struct method_link {
    struct node * method_header;
    struct method_link * next;
};


struct symtab * create_tables_tree_annotation (struct node *);
struct params_type_list * create_param_node   (char *, char *);
enum data_type string_to_data_type            (char *);

int check_field_redeclaration                 (struct symtab *, char * );
int count_siblings                            (struct node *);
int count_line_params                         (struct params_type_list *);

void free_linking                             (struct method_link *); 
void print_param_types                        (struct params_type_list *);

char * get_param_types                        (struct params_type_list *, int);
char * get_node_param_types                   (struct node *, int);
char * data_type_to_string                    (enum data_type);
char * table_line_type_to_string              (enum table_line_type);
char * table_line_flag_to_string              (enum table_line_flag);

char * remove_char_from_string                (char * str, char ch);
#endif  //util.h
