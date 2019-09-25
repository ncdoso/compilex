#include "symtab.h"

struct symtab * create_table (char * p_name, enum table_line_type p_type, struct params_type_list * p_param_type) {
    struct symtab * table = (struct symtab *) malloc(sizeof(struct symtab));
    table->name = strdup(p_name);
    table->table_type = p_type;
    table->params_type = p_param_type;
    table->first_line = NULL;
    table->next = NULL;

    return table;
}

struct symtab * create_class_table (char * p_name) {
    return create_table(p_name, T_CLASS, NULL);
}

struct symtab * create_method_table (char * p_name, struct params_type_list * p_param_type) {
    return create_table(p_name, T_METHOD, p_param_type);
}

struct symtab * build_method_table (struct node * method_decl, struct node * method_body) {
    struct node * method_header = method_decl->child;
    struct node * type_node = method_header->child;
    struct node * id_node = type_node->sibling;
    struct node * param_decl = id_node->sibling->child;
    //struct node * body = method_body->child;
    struct table_line * return_line = create_other_line(strdup("return"), 0, 0, NO_FLAG, string_to_data_type(type_node->token));

    struct params_type_list * params_list = build_params_types_list(param_decl);
    struct symtab * method_table = create_method_table(id_node->lval, params_list);

    append_line_to_table(method_table, return_line);

    while(params_list) {
        struct table_line *param_line = create_other_line(params_list->name, param_decl->child->sibling->line, param_decl->child->sibling->col, PARAM, params_list->type);
        
        //TODO: PARAMETER REDECLARATION
        if(check_field_redeclaration(method_table, params_list->name) == 0) 
                throw_semantics_error(SE_REDEF, param_decl->child->sibling, NULL, NULL);
        else
            append_line_to_table(method_table, param_line);
   
        params_list = params_list->next;
        param_decl = param_decl->sibling;
    }

    /*while(body) {
        if(!strcmp(body->token, "VarDecl")) {
            struct node * type_node = body->child;
            struct node * id_node = type_node->sibling;
            
            struct table_line * var_line = create_other_line(id_node->lval, body->child->sibling->line, body->child->sibling->col, NO_FLAG, string_to_data_type(type_node->token));

            if(check_field_redeclaration(method_table, id_node->lval) == 0) {
                throw_semantics_error(SE_REDEF, id_node, NULL);
                id_node->annotation->single_type = DT_NULL;
            }
            append_line_to_table(method_table, var_line);
        }
        body = body->sibling;
    }*/

    return method_table;
}

void free_symtab(struct symtab * table) {
    if(!table) 
        return;
    
    if(table->next)
        free_symtab(table->next);
    free_table_line(table->first_line);
 //   free_params_list(table->params_type);

    free(table->name);
    table->name = NULL;
    free(table);
}

void free_table_line(struct table_line * line) {
    if(!line)
        return;

    if(line->next_line)
        free_table_line(line->next_line);
   // free(line->name);
    free_params_list(line->type.method_params);
    line->next_line = NULL;
    free(line);
}

void free_params_list(struct params_type_list * params) {
    if(!params)
        return;
    if(params->next)
        free_params_list(params->next);
    //free(params->name);
    params->next = NULL;
    free(params);
}

struct params_type_list * build_params_types_list (struct node * param_decl) {
    struct params_type_list * params_list = NULL;

    while(param_decl) {
        struct node * node_type = param_decl->child;
        struct node * node_id = node_type->sibling;
        params_list = append_param_node(params_list, create_param_node(node_type->token, node_id->lval));
        param_decl = param_decl->sibling;
    }

    return params_list;
}

void append_line_to_table (struct symtab * table, struct table_line * line) {
    if(!table->first_line) {
        table->first_line = line;
        return;
    }

    struct table_line * line_aux = table->first_line;
    while(line_aux->next_line)
        line_aux = line_aux->next_line;

    line_aux->next_line = line;
}

void append_table_to_list (struct symtab * first_table, struct symtab * table) {
    while(first_table->next)
        first_table = first_table->next;

    first_table->next = table;
}

void print_symtab (struct symtab * table) {
    if(!table)
        return;

    struct table_line * line = table->first_line;

    printf("===== %s %s", table_line_type_to_string(table->table_type), table->name);
    if(table->table_type == T_METHOD) {
        print_param_types(table->params_type);
    }
    printf(" Symbol Table =====\n");

    while(line) {
        printf("%s\t", line->name);
        if (line->line_type == T_METHOD) {
          print_param_types(line->type.method_params);
          printf("\t%s", data_type_to_string(line->type.single_type));
          if (line->flag != NO_FLAG) {
            printf("\t%s", table_line_flag_to_string(line->flag));
          }
        }
        else if (line->line_type == T_OTHER) {
            printf("\t%s", data_type_to_string(line->type.single_type));
            if(line->flag != NO_FLAG)
                printf("\t%s", table_line_flag_to_string(line->flag));
        }
        printf("\n");
        line = line->next_line;
    }
    printf("\n");
    print_symtab(table->next);
}

struct table_line * create_line (char * p_name, int p_line, int p_col, enum table_line_flag p_flag, enum table_line_type p_line_type, struct params_type_list * p_type_list, enum data_type p_type) {
    struct table_line * line = (struct table_line *) malloc(sizeof(struct table_line));
    line->name = p_name;
    line->field_line = p_line;
    line->field_col = p_col;
    line->temp_var = 0;
    line->llvm_name = NULL;
    line->ast_method_decl = NULL;
    line->flag = p_flag;
    line->line_type = p_line_type;
    line->type.method_params = p_type_list;
    line->type.single_type = p_type;
    line->next_line = NULL;
    return line;
}

struct table_line * create_method_line (char * p_name, int p_line, int p_col, struct params_type_list * p_type, enum data_type return_type) {
    return create_line(p_name, p_line, p_col, NO_FLAG, T_METHOD, p_type, return_type);
}

struct table_line * create_other_line (char * p_name, int p_line, int p_col, enum table_line_flag p_flag, enum data_type p_type) {
    return create_line(p_name, p_line, p_col, p_flag, T_OTHER, NULL, p_type);
}


