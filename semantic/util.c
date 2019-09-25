#include "util.h"
#include "annotation.h"

struct symtab * create_tables_tree_annotation (struct node * ast_root) {
    ast_root = ast_root->child;
    struct symtab * class_table = create_class_table(ast_root->lval);
    struct method_link * linked_methods = NULL;
    while(ast_root) {
        if(!strcmp(ast_root->token, "FieldDecl")) {
            struct node * type_node = ast_root->child;
            struct node * id_node = type_node->sibling;
            struct table_line * field_decl = create_other_line(id_node->lval, id_node->line, id_node->col, NO_FLAG, string_to_data_type(type_node->token));
            //Check
            if(check_field_redeclaration(class_table, id_node->lval) == 0) {
                throw_semantics_error(SE_REDEF, id_node, NULL, NULL);
                id_node->annotation->single_type = DT_NULL;

            } else {
                append_line_to_table(class_table, field_decl);
            }
        } else if(!strcmp(ast_root->token, "MethodDecl")) {
            struct node * type_node = ast_root->child->child;

            struct symtab * method_table = build_method_table(ast_root->child, ast_root->child->sibling);

            struct table_line * method_line = create_method_line(method_table->name, type_node->sibling->line, type_node->sibling->col, method_table->params_type, string_to_data_type(type_node->token));
            
            if(check_method_redeclaration (class_table, method_line->type.method_params, method_line->name) == 0) {
                throw_semantics_error(SE_REDEF, type_node->sibling, get_param_types(method_line->type.method_params,count_line_params(method_line->type.method_params)), NULL);
                type_node->sibling->annotation->single_type = DT_NULL;
            } else {
                append_line_to_table(class_table, method_line);
                append_table_to_list(class_table, method_table);

                if(linked_methods == NULL){
                    struct method_link * new_method = (struct method_link *) malloc(sizeof(struct method_link));
                    new_method->method_header = ast_root->child->sibling;
                    new_method->next = NULL;
                    linked_methods = new_method;
                }
                else{
                    struct method_link * linking = linked_methods;

                    while(linking->next){
                        linking = linking->next;
                    }

                    struct method_link * new_method = (struct method_link *) malloc(sizeof(struct method_link));
                    new_method->method_header = ast_root->child->sibling;
                    new_method->next = NULL;
                    linking->next = new_method;
                }
            }
           
        }
         ast_root = ast_root->sibling;
    }
    struct symtab * method_table = class_table->next;
    struct method_link * first = linked_methods;
    while(method_table && linked_methods){
        annotate_method(method_table, class_table, NULL, linked_methods->method_header);
        method_table = method_table->next;
        linked_methods = linked_methods->next;
    }
    
    free_linking(first);

    return class_table;
}

void free_linking(struct method_link * link) {
    if(!link)
        return;
    if(link->next)
        free_linking(link->next);
    link->method_header = NULL;
    link->next = NULL;
    free(link);
}

int check_field_redeclaration(struct symtab * class_table, char * field_name) {
    struct table_line *line = class_table->first_line;

    while(line) {
        if(line->line_type == T_OTHER && !strcmp(field_name, line->name)) {
            return 0;
        }
        line = line->next_line;
    }

    return 1;
}

struct params_type_list * create_param_node(char * p_type, char * p_name) {
    struct params_type_list * new_param_node = (struct params_type_list*) malloc(sizeof(struct params_type_list));

    new_param_node->name = p_name;
    new_param_node->type = string_to_data_type(p_type);
    new_param_node->next = NULL;

    return new_param_node;
}

void print_param_types(struct params_type_list * method_params) {
    printf("(");
    while(method_params) {
        printf("%s", data_type_to_string(method_params->type));
        if(method_params->next)
            printf(",");
        method_params = method_params->next;
    }
    printf(")");
}


int count_siblings(struct node * target){
    int n = 0;
    while(target){
        n++;
        target = target->sibling;
    }
    return n;
}


int count_line_params(struct params_type_list * t_line_params){
    int n = 0;
    while(t_line_params){
        n++;
        t_line_params = t_line_params->next;
    }
    return n;

}


char * get_param_types(struct params_type_list * method_params, int n_params){
    char * params = (char *) malloc(8 * n_params + 3);
    strcpy(params, "(");
    while(method_params) {
        strcat(params, data_type_to_string(method_params->type));
        if(method_params->next)
            strcat(params, ",");
        method_params = method_params->next;
    }
    strcat(params, ")");
    return params;
}


char * get_node_param_types(struct node * method_params, int n_params){
    char * params = (char *) malloc(8 * n_params + 3);
    strcpy(params, "(");
    while(method_params) {
        strcat(params, data_type_to_string(method_params->annotation->single_type));
        if(method_params->sibling)
            strcat(params, ",");
        method_params = method_params->sibling;
    }
    strcat(params, ")");
    return params;
}


enum data_type string_to_data_type (char * word) {
    enum data_type type;

    if(!strcmp(word, "Int"))
        type = DT_DECLIT;
    else if(!strcmp(word, "Double"))
        type = DT_REALLIT;
    else if(!strcmp(word, "StringArray"))
        type = DT_STRING_ARRAY;
    else if(!strcmp(word, "StrlLit"))
        type = DT_STRING;
    else if(!strcmp(word, "Bool"))
        type = DT_BOOLEAN;
    else if(!strcmp(word, "Void"))
        type = DT_VOID;
    else
        type = DT_UNDEF;

    return type;
}

char * data_type_to_string (enum data_type type) {
    switch(type) {
        case DT_UNDEF:
            return "undef";
        case DT_DECLIT:
            return "int";
        case DT_REALLIT:
            return "double";
        case DT_STRING_ARRAY:
            return "String[]";
        case DT_STRING:
            return "String";
        case DT_BOOLEAN:
            return "boolean";
        case DT_VOID:
            return "void";
        case DT_NULL:
            return "";
        case DT_HIDE:
            return "";
		case DT_NO_PARAMS:
			return "()";
    }
}

char * table_line_type_to_string (enum table_line_type type) {
    switch(type) {
        case T_CLASS:
            return "Class";
        case T_METHOD:
            return "Method";
        case T_OTHER:
            return "";
    }
}

char * table_line_flag_to_string (enum table_line_flag type) {
    switch(type) {
        case NO_FLAG:
            return "";
        case PARAM:
            return "param";
    }
    return "";
}

char * remove_char_from_string(char * str, char ch){

    int i = 0, j = 0;

    int size = strlen(str);

    char * new_string = (char *) malloc(sizeof(size + 1));



    while(i < size) {

        if(str[i] != ch) {

            new_string[j++] = str[i];

        }

        i++;

    }

    new_string[j] = '\0';
    return new_string;

}