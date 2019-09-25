#ifndef DATA_TYPES_H_
#define DATA_TYPES_H_


#include <stdio.h>
#include <stdlib.h>

enum data_type {
    DT_UNDEF, DT_DECLIT, DT_REALLIT, DT_STRING_ARRAY, DT_STRING, DT_BOOLEAN, DT_VOID, DT_NULL, DT_HIDE, DT_NO_PARAMS
};

enum sem_error{
    SE_NOT_FOUND, SE_INCOMPATIBLE, SE_BOUNDS, SE_BAD_TYPE, SE_BAD_TYPES, SE_AMBIGUOUS, SE_REDEF
};

enum table_line_type {
    T_CLASS, T_METHOD, T_OTHER
};

enum table_line_flag {
    NO_FLAG, PARAM
};


struct params_type_list {
    char * name;
    enum data_type type;
    struct params_type_list * next;
};

/**
 * Struct que armazena tipos de dados
 * single_type: armazena o tipo de dados de um identificador
 * method_params: lista de tipo de dados de um método.
 */
struct tables_content_type {
    enum data_type single_type;
    struct params_type_list * method_params;
};

struct params_type_list * append_param_node (struct params_type_list *, struct params_type_list *);


#endif  //symbols.h

