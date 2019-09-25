#include "ast.h"

extern int actual_line, actual_col;
struct node * create_node( char * name, struct var_data * value ){
    struct node * tree_node;
    tree_node = (struct node *) malloc(sizeof(struct node));
    tree_node->token = name;
    tree_node->lval = value==NULL?NULL:value->strlit;
    tree_node->line = value==NULL?0:value->line;
    tree_node->col = value==NULL?0:value->col;
    if(value != NULL){
        free(value);
    }
    tree_node->annotation = malloc(sizeof(struct tables_content_type));
    tree_node->annotation->single_type = DT_NULL;
    tree_node->annotation->method_params = NULL;
    tree_node->sibling = NULL;
    tree_node->child = NULL;
    return tree_node;
}

void add_sibling (struct node* adding_node, struct node* node_to_add) {
    struct node * temp = adding_node;
    while(temp->sibling != NULL){
      temp = temp->sibling;
    }
    temp->sibling = node_to_add;
}

void add_child (struct node* adding_node, struct node* node_to_add) {
    struct node * temp = adding_node;
    if(temp->child != NULL){
        add_sibling(temp->child, node_to_add);
    } else {
        temp->child = node_to_add;
    }
}

struct node * check_one_child(struct node * node){
    if(node->child){
        if(node->child->sibling == NULL){
            struct node * temp =  node->child;
            free(node);
            return temp;
        }
  }
  return node;
}

void print_tree(struct node * node, int points, int annot) {
    int i;
    struct node * temp = node;
    for(i = 0; i < points; i++){
        printf("..");
    }
    printf("%s", node->token);
    if(!strcmp(node->token, "Id") || !strcmp(node->token, "BoolLit") ||
        !strcmp(node->token, "DecLit") || !strcmp(node->token, "RealLit") ||
        !strcmp(node->token, "StrLit")) {
        printf("(%s)", node->lval);
    }

    if(node->annotation->single_type != DT_NULL && 
       node->annotation->single_type != DT_HIDE && annot){
        printf(" - %s", data_type_to_string(node->annotation->single_type));
    }
    else if(node->annotation->method_params && annot){
        printf(" - ");
        print_param_types(node->annotation->method_params);
    }

    printf("\n");
    if(temp->child != NULL) {
        print_tree(temp->child, points+1, annot);
    }
    if(temp->sibling != NULL) {
        print_tree(temp->sibling, points, annot);
    }
}

void free_tree(struct node * to_free) {
    if(to_free == NULL)
        return;
    if(to_free->child) {
        free_tree(to_free->child);
    }
    if(to_free->sibling) {
        free_tree(to_free->sibling);
    }
    if(to_free->lval)
        free(to_free->lval);
    //free(to_free->annotation);
    free(to_free);
}

struct node * create_program(struct var_data * id, struct node * child) {
    struct node * program_node = create_node("Program", NULL);
    add_child(program_node, create_node("Id", id));
    add_child(program_node, child);

    return program_node;
}

struct node * create_field_decl(struct node * type, struct var_data * id) {
    struct node * field_decl = create_node("FieldDecl", NULL);

    add_child(field_decl, type);
    add_child(field_decl, create_node("Id", id));

    return field_decl;
}

struct node * create_var_decl(struct node * type, struct var_data * id) {
    struct node * var_decl = create_node("VarDecl", NULL);

    add_child(var_decl, type);
    add_child(var_decl, create_node("Id", id));

    return var_decl;
}

struct node * create_method_decl(struct node * header, struct node * body) {
    struct node * method_decl = create_node("MethodDecl", NULL);

    add_child(method_decl, header);
    add_child(method_decl, body);

    return method_decl;
}

struct node * create_param_decl(struct node * type, struct var_data * id) {
    struct node * param_decl = create_node("ParamDecl", NULL);

    add_child(param_decl, type);
    add_child(param_decl, create_node("Id", id));

    return param_decl;
}

struct node * create_method_header(struct node * type, struct var_data * id, struct node * params) {
    struct node * header = create_node("MethodHeader", NULL);

    add_child(header, type);
    add_child(header, create_node("Id", id));
    add_child(header, params==NULL? create_node("MethodParams", NULL):params);

    return header;
}

struct node * create_method_params(struct node * child) {
    struct node * params = create_node("MethodParams", NULL);

    add_child(params, child);

    return params;
}

struct node * create_method_body(struct node * child) {
    struct node * method_body = create_node("MethodBody", NULL);

    add_child(method_body, child);

    return method_body;
}

struct node * create_expression(char * name, struct var_data * data, struct node * left, struct node * right) {
    struct node * operation = create_node(name, data);
    add_child(operation, left);
    add_child(operation, right);

    return operation;
}
