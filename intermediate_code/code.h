

#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "symtab.h"
#include "util.h"

#define PARAM_TYPE_MAX_SIZE 6   //double

char * class_name;


void generate_code                  (struct node *, struct symtab *);
void generate_methods_names         (struct symtab *);
void generate_global_vars           (struct symtab *);
void generate_method_code           (struct node *, struct node *, int *);
void build_method_var_decls         (struct symtab *);
char * add_terminal_char            (char * str, int * size);
char *build_method_name             (char *, struct params_type_list *);
char *build_method_params_llvm      (struct params_type_list *);
char *build_var_name                (char *, char);
void generate_methods               (struct symtab *);
void find_prints                    (struct node * root);
