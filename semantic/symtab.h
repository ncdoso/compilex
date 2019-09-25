#ifndef SYMTAB_H_
#define SYMTAB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "data_types.h"
#include "util.h"
#include "annotation.h"

/**
 * Estrutura que armazena a informação referente a uma linha de tabela.
 *
 * name: contem o nome da linha
 * flag: possui flags relevantes para o conteudo da linha ou NULL
 * line_type: mantem a info necessária para identificar o tipo da linha
 * type: struct com os tipos de dados do identificador ou parametros do
 *      metodo, depende de `line_type`
 * next_line: apontador para a linha seguinte da tabela ou NULL.
 */
struct table_line {
    char * name;
    int field_line, field_col;
    enum table_line_flag flag;
    enum table_line_type line_type;
    struct tables_content_type type;
    struct table_line * next_line;
};

/**
 * Estrutura que armazena a informação refente a uma tabela, de classe ou método.
 *
 * name: nome da class ou do método a que pertence a tabela
 * table_type: tipo da tabela, classe ou método
 * params_type: lista do tipo de dados dos parametros do método, ou NULL caso seja de classe
 * first_line: apontador para a primeira linha da tabela
 * next: apontador para a próxima tabela.
 */
struct symtab {
    char * name;
    enum table_line_type table_type;
    struct params_type_list * params_type;
    struct table_line * first_line;
    struct symtab * next;
};

struct symtab * create_table                      (char *, enum table_line_type, struct params_type_list *);
struct symtab * create_class_table                (char *);
struct symtab * create_method_table               (char *, struct params_type_list *);
struct symtab * build_method_table                (struct node *, struct node *);
struct params_type_list * build_params_types_list (struct node *);

void append_line_to_table                         (struct symtab *, struct table_line *);
void append_table_to_list                         (struct symtab *, struct symtab *);
void print_symtab                                 (struct symtab *);
void free_symtab                                  (struct symtab *);
void free_table_line                              (struct table_line *); 
void free_params_list                             (struct params_type_list *);

struct table_line * create_line                   (char *, int, int, enum table_line_flag, enum table_line_type, struct params_type_list *, enum data_type);
struct table_line * create_method_line            (char *, int, int, struct params_type_list *, enum data_type);
struct table_line * create_other_line             (char *, int, int, enum table_line_flag, enum data_type);




#endif  //symtab.h
