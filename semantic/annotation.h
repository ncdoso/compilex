#ifndef ANNOTATION_H_
#define ANNOTATION_H_

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include "symtab.h"

enum data_type check_method_call (struct symtab *, struct node *);

int search_method_table          (struct symtab * , struct node * cur_node);
int evaluate_candidate_call      (struct node *, struct params_type_list *);
int check_double_issue           (enum data_type, enum data_type);
int check_bounds                 (struct node *);
int same_parameters              (struct params_type_list *, struct params_type_list *);
int check_method_redeclaration   (struct symtab *, struct params_type_list *, char *);

void check_if_redeclared         (struct symtab *, struct node *);
void throw_semantics_error       (enum sem_error, struct node *, char *, struct node *);
void annotate_method             (struct symtab *, struct symtab *, struct node *, struct node *);

#endif  //annotation.h
