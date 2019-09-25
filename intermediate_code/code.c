
#include "code.h"


void generate_code(struct node * ast, struct symtab * class_table){
    class_name = ast->child->lval;
    find_prints(ast);
    printf("\n");
    generate_global_vars(class_table);
    printf("\n\n");
    generate_methods_names(class_table);
    generate_methods(class_table);

    class_name = NULL;
}

void generate_global_vars(struct symtab * class_table){

    struct table_line * table_line = class_table->first_line;
    printf("@.print.int = private unnamed_addr constant [4 x i8] c\"%%d\\0A\\00\"\n");
    printf("@.print.float = private unnamed_addr constant [7 x i8] c\"%%.16E\\0A\\00\"\n");
    printf("@.print.str = private unnamed_addr constant [4 x i8] c\"%%s\\0A\\00\"\n");
    printf("@.print.true = private unnamed_addr constant [6 x i8] c\"true\\0A\\00\"\n");
    printf("@.print.false = private unnamed_addr constant [7 x i8] c\"false\\0A\\00\"\n");
    printf("declare i32 @printf(i8*, ...)\n");
    printf("declare i32 @atoi(i8*)\n");

    while(table_line){
        if(table_line->line_type == T_OTHER){
            //TODO: Store var name in table
            table_line->llvm_name = build_var_name(table_line->name, '@');
            printf("%s = global %s %s\n",table_line->llvm_name , data_type_to_llvm(table_line->type.single_type) ,table_line->type.single_type == DT_REALLIT ? "0.0" : "0");
        }
        table_line = table_line->next_line;
    }

}

void generate_methods_names(struct symtab * class_table) {
    struct table_line *line = class_table->first_line;

    while(line) {
        if(line->line_type == T_METHOD) {
			 line->llvm_name = build_method_name(line->name, line->type.method_params);
        }
        line = line->next_line;
    }
}

void generate_methods(struct symtab * class_table){
    struct symtab * table = class_table->next;
    struct table_line * cur_table_line = NULL;
    struct table_line * table_line = class_table->first_line;
    while(table){
            cur_table_line = table->first_line;
            while(table_line->line_type != T_METHOD){
                table_line = table_line->next_line;
            }

            //Actual main function that calls the class main function
            if(!strcmp(table->name, "main") && table_line->type.method_params != NULL && table_line->type.method_params->type == DT_STRING_ARRAY){

                printf("\ndefine i32 @main(i32 %%argc, i8** %%argv) {\n\n");
                printf("\t%%1 = sub i32 %%argc, 1\n");
                printf("%%2 = getelementptr inbounds i8*, i8** %%argv, i32 1\n");
                printf("\tcall void @%s.main.i8.(i32 %%1, i8** %%2)\n", class_name);
                printf("\tret i32 0\n}\n");
            }

            int is_string_array = table_line->type.method_params && table_line->type.method_params->type == DT_STRING_ARRAY;

            if(!strcmp("main",table_line->name) && is_string_array){
                 printf("\ndefine void @%s.main.i8.(i32 %%.%s.argc,i8** %%%s.%s) {\n\n", class_name,class_name,class_name, table_line->type.method_params->name);

            }
            else if(is_string_array) {
                printf("\ndefine %s %s (i32 %%.%s.argc, i8** %%%s.%s) {\n\n",
                data_type_to_llvm(table->first_line->type.single_type),
                table_line->llvm_name, class_name, class_name, table_line->type.method_params->name);
            }
            else
                printf("\ndefine %s %s (%s) {\n\n", data_type_to_llvm(table->first_line->type.single_type), table_line->llvm_name, build_method_params_llvm(table_line->type.method_params));


            int n_params = 1;
            if(table->first_line->type.single_type != DT_VOID){
                char * ret_type = data_type_to_llvm(table->first_line->type.single_type);
                printf("%%1 = alloca %s\n", ret_type);
                printf("store %s %s, %s* %%1\n", ret_type,table->first_line->type.single_type == DT_REALLIT ? "0.0" : "0", ret_type);
                cur_table_line->temp_var = n_params;
            }
            else n_params--;


            cur_table_line = cur_table_line->next_line;
            struct params_type_list * params = table_line->type.method_params;
            while(params){
                char * type = data_type_to_llvm(params->type);
                int temp_var = ++n_params;
                printf("%%%d = alloca %s\n", temp_var, type);
                printf("store %s %s, %s* %%%d\n", type, build_var_name(params->name, '%') , type, temp_var);
                cur_table_line->temp_var = temp_var;
                params = params->next;
                cur_table_line = cur_table_line->next_line;
            }

            //TODO: Call function to generate method code!!!
            build_method_var_decls(table);
            generate_method_code(table_line->ast_method_decl->child, NULL, &n_params);
            char * c = data_type_to_llvm(table->first_line->type.single_type);
            if(!strcmp(c, "void")) {
                printf("%s", "ret void\n");
            }
            else{
                printf("ret %s %s\n", c, table->first_line->type.single_type == DT_REALLIT ? "0.0" : "0");
            }
            printf("}\n\n");
            table = table->next;
            table_line = table_line->next_line;
    }
}

void build_method_var_decls(struct symtab * method_table){
    struct table_line * line = method_table->first_line->next_line;
    while(line){
        if(line->flag != PARAM){
        line->llvm_name = build_var_name(line->name, '%');
        printf("%s = alloca %s\n", line->llvm_name,data_type_to_llvm(line->type.single_type));

        }
    line = line->next_line;}


}

int n_prints = 0;
void find_prints(struct node * root){

    if(!strcmp(root->token, "Print") && !strcmp(root->child->token, "StrLit")) {
        int size = 0;
        char * prin = add_terminal_char(root->child->lval, &size);
        printf("@.print.%d = private unnamed_addr constant [%d x i8] c%s\\00\"\n", ++n_prints, size, prin );
        root->temp_var = n_prints;

    }
    if(root->child) find_prints(root->child);
    if(root->sibling) find_prints(root->sibling);

}

char * add_terminal_char(char * str, int * size){
    int i, count_size = 0;
    char * new_str = malloc(strlen(str) * 3 + 1);
    int new_str_size = 0;
    new_str[0] = 0;
    for(i = 0 ; i < strlen(str)-1; i++){
        if(str[i] == '\\'){
            if(str[i+1] == 't'){
                strcat(new_str,"\\09");

            }
            else if(str[i+1] == 'n'){
                strcat(new_str,"\\0A");
            }
            else if(str[i+1] == 'f'){
                strcat(new_str,"\\0C");
            }
            else if(str[i+1] == 'r'){
                strcat(new_str,"\\0D");
            }
            else if(str[i+1] == '"'){
                strcat(new_str,"\\22");
            }
            else if(str[i+1] == '\\'){
                strcat(new_str,"\\5C");
            }
            count_size++; i++; new_str_size+=3;
        }
        else
            new_str[new_str_size++] = str[i];
            new_str[new_str_size] = 0;
       }

    *size = strlen(new_str)- 2*count_size;
    return new_str;
}

char * get_var(struct node * n){
    if(n->table_entry){
        if(n->table_entry->llvm_name)
            return n->table_entry->llvm_name;
        else if(n->table_entry->temp_var != 0){
            char buff[1024];
            sprintf(buff, "%%%d", n->table_entry->temp_var);
            return strdup(buff);
        }
    }
    char buff[1024];
    sprintf(buff, "%%%d", n->temp_var);
    return strdup(buff);
}

int g_if_labels = 1, g_while_labels = 1, g_dowhile_labels = 1, gn_ands = 1;
int last_label = 0;
void generate_method_code(struct node * ast_node, struct node * parent, int * n_params){
    if(!strcmp(ast_node->token, "If")){
      int temp1;
        generate_method_code(ast_node->child, ast_node, n_params);

        if(ast_node->child->table_entry != NULL){
            temp1 = ++(*n_params);
            printf("%%%d = load i1, i1* %s\n",
            *n_params,
            get_var(ast_node->child));
        }
        else temp1 = ast_node->child->temp_var;

        int n_labels = g_if_labels++;

        printf("%%%d = icmp eq i1 %%%d, 1\n", ++(*n_params), temp1);

        printf("br i1 %%%d, label %%if%d, label %%else%d\n", *n_params, n_labels, n_labels);

        printf(" if%d: \n", n_labels);


        generate_method_code(ast_node->child->sibling, ast_node,  n_params);

        printf("br label %%end%d\n", n_labels);

        printf(" else%d: \n", n_labels);


        generate_method_code(ast_node->child->sibling->sibling,ast_node, n_params);

        printf("br label %%end%d\n", n_labels);

        printf(" end%d: \n", n_labels);


    }

      else if(!strcmp(ast_node->token, "And") || !strcmp(ast_node->token, "Or")){
int temp1, temp2, a;
        int n_ands = gn_ands;
        gn_ands+=3;

        printf("\tbr label %%ao%d\n", n_ands);

        printf("ao%d:\n", n_ands);

        struct node * s = ast_node->child->sibling;
        ast_node->child->sibling = NULL;
        ast_node->child->phi_label = n_ands;
        generate_method_code(ast_node->child, ast_node, n_params);


        if(ast_node->child->table_entry != NULL){
            printf("%%%d = load i1, i1* %s\n",++(*n_params), get_var(ast_node->child));
            temp1 = *n_params;
        }

        else temp1 = ast_node->child->temp_var;
        printf("%%%d = icmp ne i1 %%%d, 0\n", ++(*n_params), temp1);
        if(!strcmp(ast_node->token, "And") ){
            printf("br i1 %%%d, label %%ao%d, label %%ao%d\n", *n_params, n_ands+1, n_ands+2);
        }
        else
             printf("br i1 %%%d, label %%ao%d, label %%ao%d\n", *n_params,n_ands+2, n_ands+1);

        printf("ao%d:\n",n_ands+1);
        ast_node->child->sibling = s;
        ast_node->child->sibling->phi_label = n_ands+1;
        generate_method_code(ast_node->child->sibling, ast_node, n_params);

        a = *n_params;

        if(ast_node->child->sibling->table_entry != NULL){
            printf("%%%d = load i1, i1* %s\n",++(*n_params), get_var(ast_node->child->sibling));
            temp2 = *n_params;
        }

        else{
            temp2 = ast_node->child->sibling->temp_var;
            printf("%%%d = add i32 0, 0\n", ++(*n_params));
        }

        printf("%%%d = icmp ne i1 %%%d, 0\n", ++(*n_params), temp2);

        printf("br label %%ao%d\n", n_ands+2);

        printf(" ao%d:\n", n_ands+2);

        printf("%%%d = phi i1 [%s, %%ao%d], [%%%d, %%ao%d]\n",


        ++(*n_params),

        !strcmp(ast_node->token, "And") ? "false" : "true" ,

        ast_node->child->phi_label,

        *n_params - 1,

        ast_node->child->sibling->phi_label);

        ast_node->phi_label = n_ands+2;

        last_label = *n_params-1;

        ast_node->temp_var = *n_params;

    }
    else if(!strcmp(ast_node->token, "DoWhile")){
        int temp1;

            int n_labels = g_dowhile_labels++;

            printf("br label %%dowhile%d\n", n_labels);

            printf("dowhile%d:\n", n_labels);

            generate_method_code(ast_node->child, ast_node, n_params);

            generate_method_code(ast_node->child->sibling, ast_node, n_params);

            if(ast_node->child->sibling->table_entry != NULL){
                temp1 = ++(*n_params);
                printf("%%%d = load i1, i1* %s\n",
                *n_params,
                get_var(ast_node->child->sibling));
            }
            else temp1 = ast_node->child->sibling->temp_var;

            printf("%%%d = icmp eq i1 %%%d, 1\n", ++(*n_params), temp1);

            printf("br i1 %%%d, label %%dowhile%d, label %%dofinal%d\n", *n_params, n_labels, n_labels);

            printf("dofinal%d:\n", n_labels);


    }



    else if(!strcmp(ast_node->token, "While")){
            int temp1 = 0;

            int n_labels = g_while_labels++;

            printf("br label %%while%d\n", n_labels);

            printf("while%d:\n", n_labels);

            generate_method_code(ast_node->child, ast_node, n_params);

        if(ast_node->child->table_entry != NULL){
            temp1 = ++(*n_params);
            printf("%%%d = load i1, i1* %s\n",
            *n_params,
            get_var(ast_node->child));
        }
        else temp1 = ast_node->child->temp_var;

            printf("%%%d = icmp eq i1 %%%d, 1\n", ++(*n_params), temp1);

            printf("br i1 %%%d, label %%body%d, label %%final%d\n", *n_params, n_labels, n_labels);

            printf("body%d:\n", n_labels);

            generate_method_code(ast_node->child->sibling, ast_node, n_params);

            printf("br label %%while%d\n", n_labels);

            printf("final%d:\n", n_labels);
    }



    else if(ast_node->child){
        generate_method_code(ast_node->child, ast_node, n_params);
    }


    if(!strcmp(ast_node->token, "Add")|| !strcmp(ast_node->token, "Sub") || !strcmp(ast_node->token, "Mul")
    || !strcmp(ast_node->token, "Div") || !strcmp(ast_node->token, "Mod")){
        char op[5];
        int temp1 = -1, temp2 = -1;

        if(!strcmp(ast_node->token, "Add")) strcpy(op, "add");
        else if(!strcmp(ast_node->token, "Sub")) strcpy(op, "sub");
        else if(!strcmp(ast_node->token, "Mul")) strcpy(op, "mul");
        else if(!strcmp(ast_node->token, "Div")) strcpy(op, "div");
        else if(!strcmp(ast_node->token, "Mod")) strcpy(op, "srem");

        if(ast_node->child->table_entry != NULL){
            temp1 = ++(*n_params);
            printf("%%%d = load %s, %s* %s\n",
            *n_params,
            data_type_to_llvm(ast_node->child->annotation->single_type),
            data_type_to_llvm(ast_node->child->annotation->single_type),
            get_var(ast_node->child));
        }
        else temp1 = ast_node->child->temp_var;

        int flag = 0;

        if(ast_node->child->sibling->table_entry != NULL){
            temp2 = ++(*n_params);
            printf("%%%d = load %s, %s* %s\n",
            *n_params,
            data_type_to_llvm(ast_node->child->sibling->annotation->single_type),
            data_type_to_llvm(ast_node->child->sibling->annotation->single_type),
            get_var(ast_node->child->sibling));
        } else temp2 = ast_node->child->sibling->temp_var;

        if(ast_node->child->annotation->single_type == DT_REALLIT && ast_node->child->sibling->annotation->single_type == DT_DECLIT){
            printf("%%%d = sitofp i32 %%%d to double\n", ++(*n_params), temp2);
            temp2 = *n_params;
            flag = 1;
        }
        else if(ast_node->child->annotation->single_type == DT_DECLIT && ast_node->child->sibling->annotation->single_type == DT_REALLIT){
            printf("%%%d = sitofp i32 %%%d to double\n", ++(*n_params), temp1);
            temp1 = *n_params;
            flag = 1;
        }

        int exists_double = ast_node->child->sibling->annotation->single_type == DT_REALLIT || ast_node->child->annotation->single_type == DT_REALLIT;

        if(exists_double) {
            if(!strcmp(ast_node->token, "Add")) strcpy(op, "fadd");
            else if(!strcmp(ast_node->token, "Div")) strcpy(op, "fdiv");
            else if(!strcmp(ast_node->token, "Mul")) strcpy(op, "fmul");
            else if(!strcmp(ast_node->token, "Sub")) strcpy(op, "fsub");
            else if(!strcmp(ast_node->token, "Mod")) strcpy(op, "frem");
        } else {
            if(!strcmp(ast_node->token, "Add")) strcpy(op, "add");
            else if(!strcmp(ast_node->token, "Div")) strcpy(op, "sdiv");
            else if(!strcmp(ast_node->token, "Mul")) strcpy(op, "mul");
            else if(!strcmp(ast_node->token, "Sub")) strcpy(op, "sub");
            else if(!strcmp(ast_node->token, "Mod")) strcpy(op, "srem");
        }
/*
        printf("%%%d = %s%s%s %s %%%d, %%%d\n",
        ++(*n_params),
        !strcmp("div", op) && (ast_node->child->annotation->single_type == DT_DECLIT && ast_node->child->sibling->annotation->single_type == DT_DECLIT) ? "s" : "",
        strcmp("div", op) && (ast_node->child->annotation->single_type == DT_REALLIT || ast_node->child->sibling->annotation->single_type == DT_REALLIT) ? "f" : "",
        op,
        ast_node->child->annotation->single_type == DT_REALLIT || ast_node->child->sibling->annotation->single_type == DT_REALLIT ? "double" : data_type_to_llvm(ast_node->child->annotation->single_type),
        temp1,
        temp2);*/

        printf("%%%d = %s %s %%%d, %%%d\n", ++(*n_params),
               op,
               flag == 1 ? "double" : data_type_to_llvm(ast_node->child->annotation->single_type),
               temp1,
               temp2);

        ast_node->temp_var = *n_params;
    }



    else if(!strcmp(ast_node->token, "Geq") || !strcmp(ast_node->token, "Gt")
         || !strcmp(ast_node->token, "Leq") || !strcmp(ast_node->token, "Lt")
         || !strcmp(ast_node->token, "Neq") || !strcmp(ast_node->token, "Eq")){

        char op[5];

        int temp1 = -1, temp2 = -1;

        if(ast_node->child->table_entry != NULL){
            temp1 = ++(*n_params);
            printf("%%%d = load %s, %s* %s\n",
            *n_params,
            data_type_to_llvm(ast_node->child->annotation->single_type),
            data_type_to_llvm(ast_node->child->annotation->single_type),
            get_var(ast_node->child));
        }
        else temp1 = ast_node->child->temp_var;

        if(ast_node->child->sibling->table_entry != NULL){
            temp2 = ++(*n_params);
            printf("%%%d = load %s, %s* %s\n",
            *n_params,
            data_type_to_llvm(ast_node->child->sibling->annotation->single_type),
            data_type_to_llvm(ast_node->child->sibling->annotation->single_type),
            get_var(ast_node->child->sibling));
        } else temp2 = ast_node->child->sibling->temp_var;

        int flag = 0;
        if(ast_node->child->annotation->single_type == DT_REALLIT && ast_node->child->sibling->annotation->single_type == DT_DECLIT){
            printf("%%%d = sitofp i32 %%%d to double\n", ++(*n_params), temp2);
            temp2 = *n_params;
            flag = 1;
        }
        else if(ast_node->child->annotation->single_type == DT_DECLIT && ast_node->child->sibling->annotation->single_type == DT_REALLIT){
            printf("%%%d = sitofp i32 %%%d to double\n", ++(*n_params), temp1);
            temp1 = *n_params;
            flag = 1;
        }

        int exists_double = ast_node->child->sibling->annotation->single_type == DT_REALLIT || ast_node->child->annotation->single_type == DT_REALLIT;

        if(exists_double) {
            if(!strcmp(ast_node->token, "Lt")) strcpy(op, "olt");
            else if(!strcmp(ast_node->token, "Leq")) strcpy(op, "ole");
            else if(!strcmp(ast_node->token, "Gt")) strcpy(op, "ogt");
            else if(!strcmp(ast_node->token, "Geq")) strcpy(op, "oge");
            else if(!strcmp(ast_node->token, "Neq")) strcpy(op, "une");
            else if(!strcmp(ast_node->token, "Eq")) strcpy(op, "oeq");
        } else {
            if(!strcmp(ast_node->token, "Lt")) strcpy(op, "slt");
            else if(!strcmp(ast_node->token, "Leq")) strcpy(op, "sle");
            else if(!strcmp(ast_node->token, "Gt")) strcpy(op, "sgt");
            else if(!strcmp(ast_node->token, "Geq")) strcpy(op, "sge");
            else if(!strcmp(ast_node->token, "Neq")) strcpy(op, "ne");
            else if(!strcmp(ast_node->token, "Eq")) strcpy(op, "eq");
        }

        printf("%%%d = %scmp %s %s %%%d, %%%d\n", ++(*n_params),
               exists_double ? "f" : "i",
               op,
               flag == 1 ? "double" : data_type_to_llvm(ast_node->child->annotation->single_type),
               temp1,
               temp2);
        ast_node->temp_var = *n_params;
    }

    else if(!strcmp(ast_node->token, "Not")){
        int temp;
        if(ast_node->child->table_entry != NULL){
            printf("%%%d = load i1, i1* %s\n",++(*n_params), get_var(ast_node->child));
            temp = *n_params;
        }
        else temp = ast_node->child->temp_var;

        printf("%%%d = icmp ne i1 %%%d, 0\n", ++(*n_params), temp);
        printf("%%%d = xor i1 %%%d, true\n", ++(*n_params), *n_params - 1);
        ast_node->temp_var = *n_params;
    }


    else if(!strcmp(ast_node->token, "Assign")) {
        char * var = get_var(ast_node->child->sibling);
        if(ast_node->child->annotation->single_type == DT_REALLIT && ast_node->child->sibling->annotation->single_type == DT_DECLIT){
            if(ast_node->child->sibling->table_entry != NULL){
                printf("%%%d = load i32, i32* %s\n", ++(*n_params), var);
                printf("%%%d = sitofp i32 %%%d to double\n", ++(*n_params), *n_params-1);
                printf("store double %%%d, double* %s\n", *n_params, get_var(ast_node->child));
            }
            else{
                printf("%%%d = sitofp i32 %s to double\n", ++(*n_params), var);
                printf("store double %%%d, double* %s\n", *n_params, get_var(ast_node->child));
            }
        } else {
            if(ast_node->child->sibling->table_entry != NULL){
                printf("%%%d = load %s, %s* %s\n",
                ++(*n_params),
                data_type_to_llvm(ast_node->child->sibling->annotation->single_type),
                data_type_to_llvm(ast_node->child->sibling->annotation->single_type),
                var);
                printf("store %s %%%d, %s* %s\n",
                data_type_to_llvm(ast_node->child->sibling->annotation->single_type),
                *n_params,
                data_type_to_llvm(ast_node->child->sibling->annotation->single_type),
                get_var(ast_node->child));
            }
            else
                printf("store %s%s %s, %s* %s\n", data_type_to_llvm(ast_node->child->annotation->single_type),
                        var[0] == '@' ? "*" : "",
                        var,
                        data_type_to_llvm(ast_node->annotation->single_type),
                        get_var(ast_node->child));
        }
        ast_node->table_entry = ast_node->child->table_entry;
    }


    else if(!strcmp(ast_node->token, "Minus")) {

        int a = ast_node->child->annotation->single_type == DT_DECLIT ? 1 : 0;
        if(ast_node->child->table_entry == NULL){
            ast_node->temp_var = ++(*n_params);
            printf("%%%d = %ssub %s -%s, %%%d\n",
                *n_params, a == 1 ? "" : "f",
                data_type_to_llvm(ast_node->child->annotation->single_type),
                a == 1 ? "0" : "0.0",
                ast_node->child->temp_var);

        } else{
            ast_node->temp_var = *n_params + 2;
            printf("%%%d = load %s, %s* %s\n", ++(*n_params),
                data_type_to_llvm(ast_node->child->annotation->single_type),
                data_type_to_llvm(ast_node->child->annotation->single_type),
                get_var(ast_node->child));

            printf("%%%d = %ssub %s %s, %%%d\n",
                ++(*n_params), a == 1 ? "" : "f",
                data_type_to_llvm(ast_node->child->annotation->single_type),
                a == 1 ? "0" : "0.0",
                *n_params-1);
        }
    }

    else if(!strcmp(ast_node->token, "Plus")) {

        if(ast_node->child->table_entry == NULL)
            ast_node->temp_var = ast_node->child->temp_var;
        else
            ast_node->table_entry = ast_node->child->table_entry;
    }

    else if(!strcmp(ast_node->token, "DecLit") ){
        ast_node->temp_var = ++(*n_params);
        printf("%%%d = add %s 0, %s\n", *n_params, data_type_to_llvm(ast_node->annotation->single_type), remove_char_from_string(ast_node->lval, '_'));
    }

    else if(!strcmp(ast_node->token, "RealLit") ){
        ast_node->temp_var = ++(*n_params);
        printf("%%%d = fadd %s 0.0, %.16E\n", *n_params, data_type_to_llvm(ast_node->annotation->single_type), atof(remove_char_from_string(ast_node->lval, '_')));
    }

    else if(!strcmp(ast_node->token, "BoolLit")){
        ast_node->temp_var = ++(*n_params);
        printf("%%%d = add %s 0, %d\n", *n_params,
                data_type_to_llvm(ast_node->annotation->single_type),
                strcmp(ast_node->lval, "true") ? 0 : 1 );
    }

        else if(!strcmp(ast_node->token, "Call")) {
            struct node * aux = ast_node->child->sibling;

            int param_len = 0;
            char * params;

            int is_string_array = aux && aux->annotation->single_type == DT_STRING_ARRAY;
            int array_var = 0;
            if (is_string_array) {
                if(aux->table_entry) {
                    array_var = ++(*n_params);
                    printf("%%%d = load i8**, i8*** %s\n", array_var, get_var(aux));
                } else
                    array_var = aux->temp_var;

                //aux = NULL;
            }

            while(aux){
                if(aux->table_entry && aux->table_entry->llvm_name){
                    param_len += 10 + strlen(aux->table_entry->llvm_name);
                }
                else param_len += 10;
                aux = aux->sibling;
            }
            if(param_len!=0){
                params = (char *) malloc(param_len + 1);
                params[0] = 0;
                aux = ast_node->child->sibling;
                struct params_type_list * expected_param = ast_node->child->annotation->method_params;
                while (aux) {
                    char * type = data_type_to_llvm(aux->annotation->single_type);
                    int needs_promotion = aux->annotation->single_type != expected_param->type;
                    strcat(params, needs_promotion?"double" : type);
                    strcat(params, " ");
                    if(aux->table_entry){
                        printf("%%%d = load %s, %s* %s\n", ++(*n_params), type, type, get_var(aux));
                        if(needs_promotion) {
                            int last_param = *n_params;
                            printf("%%%d = sitofp i32 %%%d to double\n", ++(*n_params), last_param);
                        }
                        char new_temp[10];
                        sprintf(new_temp,"%%%d", *n_params);
                        strcat(params, new_temp);
                    }
                    else {
                        if(needs_promotion) {
                            printf("%%%d = sitofp i32 %s to double\n", ++(*n_params), get_var(aux));
                            char new_temp[10];
                            sprintf(new_temp,"%%%d", *n_params);
                            strcat(params, new_temp);
                        }
                        else
                            strcat(params, get_var(aux));
                    }
                    if(aux->sibling){
                        strcat(params, ",");
                    }
                    aux = aux->sibling;
                    expected_param = expected_param->next;
                }
            }

            if(is_string_array) {
                if(ast_node->annotation->single_type != DT_VOID) {
                    printf("%%%d = ", ++(*n_params));
                }

                printf("call %s %s (i32 %%.%s.argc,i8** %%%d)\n",
                       data_type_to_llvm(ast_node->annotation->single_type),
                       ast_node->child->table_entry->llvm_name,
                       class_name, array_var);

            } else {

                if(ast_node->annotation->single_type != DT_VOID)
                printf("%%%d = call %s %s(%s)\n", ++(*n_params),
                data_type_to_llvm(ast_node->annotation->single_type), ast_node->child->table_entry->llvm_name, param_len == 0 ? "" : params);

                else
                printf("call void %s(%s)\n", ast_node->child->table_entry->llvm_name,
                param_len == 0 ? "" : params);
            }

            ast_node->temp_var = *n_params;

    }

    else if(!strcmp(ast_node->token, "Return")){
            if(ast_node->child){
                char * c = data_type_to_llvm(ast_node->child->annotation->single_type);
                int needs_promotion = ast_node->table_entry->type.single_type != ast_node->child->annotation->single_type;
                int var = 0;
                if(ast_node->child->table_entry != NULL) {
                    var = ++(*n_params);
                    printf("%%%d = load %s, %s* %s\n", var, c, c, get_var(ast_node->child));
                } else
                    var = ast_node->child->temp_var;

                if(needs_promotion) {
                    printf("%%%d = sitofp i32 %%%d to double\n", ++(*n_params), var);
                    var = *n_params;
                }

                printf("ret %s %%%d\n", needs_promotion?"double":c, var);
            }
            else printf("%s", "ret void\n");
            ++(*n_params);
    }

    else if(!strcmp(ast_node->token, "Print")){
        if(ast_node->child->annotation->single_type == DT_BOOLEAN){

            if(ast_node->child->table_entry != NULL){
                printf("%%%d = load %s, %s* %s\n", ++(*n_params),
                        data_type_to_llvm(ast_node->child->annotation->single_type),
                        data_type_to_llvm(ast_node->child->annotation->single_type),
                        get_var(ast_node->child));

                printf("%%%d = icmp eq i1 %%%d, 1\n", ++(*n_params),*n_params-1);
                printf("br i1 %%%d, label %%%d, label %%%d\n", *n_params, *n_params + 1, *n_params + 3);
            }
            else{
                printf("%%%d = icmp eq i1 %%%d, 1\n", ++(*n_params), ast_node->child->temp_var);
                printf("br i1 %%%d, label %%%d, label %%%d\n", *n_params, *n_params + 1, *n_params + 3);
            }
            printf("\n; <btrue>:%d\n%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.print.true, i32 0, i32 0))\nbr label %%%d", ++(*n_params), ++(*n_params), *n_params+3);

            printf("\n; <bfalse>:%d\n%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.print.false, i32 0, i32 0))\nbr label %%%d", ++(*n_params), ++(*n_params), *n_params+1);

            printf("\n; <end>:%d\n", ++(*n_params));


        } else if(ast_node->child->annotation->single_type == DT_DECLIT || ast_node->child->annotation->single_type == DT_REALLIT){
            int a = ast_node->child->annotation->single_type == DT_DECLIT ? 4 : 7;
            if(ast_node->child->table_entry != NULL){
                printf("%%%d = load %s, %s* %s\n", ++(*n_params),
                        data_type_to_llvm(ast_node->child->annotation->single_type),
                        data_type_to_llvm(ast_node->child->annotation->single_type),
                        get_var(ast_node->child) );



                printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([%d x i8], [%d x i8]* @.print.%s, i32 0, i32 0), %s %%%d)\n",
                        ++(*n_params), a, a, ast_node->child->annotation->single_type == DT_DECLIT ? "int" : "float" ,
                        data_type_to_llvm(ast_node->child->annotation->single_type),
                        *n_params - 1  );
            }
            else
            printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([%d x i8], [%d x i8]* @.print.%s, i32 0, i32 0), %s %s)\n",
                    ++(*n_params), a,a,
                    ast_node->child->annotation->single_type == DT_DECLIT ? "int" : "float" ,
                    data_type_to_llvm(ast_node->child->annotation->single_type),
                    get_var(ast_node->child) );
        }

        else if(ast_node->child->annotation->single_type == DT_STRING){
            int size = 0;
            add_terminal_char(ast_node->child->lval, &size);
            printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.print.str, i32 0, i32 0), [%d x i8]* @.print.%d)\n", ++(*n_params),
                    size,ast_node->temp_var );
        }



    }

    else if(!strcmp(ast_node->token, "ParseArgs")) {
        int args_var_ind = ast_node->child->table_entry->temp_var;
        int args_temp = ++(*n_params);
        //load do array
        printf("%%%d = load i8**, i8*** %%%d\n", args_temp, args_var_ind);
        //load do valor do indice
        int index_val = ++(*n_params);
        if(ast_node->child->sibling->table_entry == NULL) {
            printf("%%%d = add i32 0, %%%d\n", index_val, ast_node->child->sibling->temp_var);
        } else {
            printf("%%%d = load i32, i32* %s\n", index_val, get_var(ast_node->child->sibling));
        }

        //get element in array
        int temp_var_element = ++(*n_params);
        printf("%%%d = getelementptr inbounds i8*, i8** %%%d, i32 %%%d\n", temp_var_element, args_temp, index_val);

        //load array's element value
        int temp_var_element_val = ++(*n_params);
        printf("%%%d = load i8*, i8** %%%d\n", temp_var_element_val, temp_var_element);

        //call atoi
        int int_element_val = ++(*n_params);
        printf("%%%d = call i32 @atoi(i8* %%%d)\n", int_element_val, temp_var_element_val);

        //add temp var to ParseArgs node
        ast_node->temp_var = int_element_val;

    }

    else if(!strcmp(ast_node->token, "Length")) {

        printf("%%%d = add i32 0, %%.%s.argc\n", ++(*n_params), class_name);
        ast_node->temp_var = *n_params;
    }




    if((ast_node->sibling && !parent) || (ast_node->sibling && parent && strcmp("If", parent->token) && strcmp("While", parent->token)  && strcmp("DoWhile", parent->token))){
        generate_method_code(ast_node->sibling,NULL, n_params);
    }


}


char * build_method_name(char* real_name, struct params_type_list * params) {
    int param_max_size = PARAM_TYPE_MAX_SIZE;
    int n_params = count_line_params(params);

    int allocated = strlen(class_name) + 1 /*DOT*/ + strlen(real_name) + 1 /*DOT*/ + (n_params * (param_max_size + 1/*DOT*/)) + 2 /* '\0' */;
    char *method_params = (char *)malloc(allocated * sizeof(char));
    method_params[0] = 0;
    sprintf(method_params, "%c%s.%s.",'@', class_name, real_name);

    if(n_params == 0) return method_params;

    while(params) {
        char param_to_add[param_max_size + 1];
        sprintf(param_to_add, "%s.", !strcmp("i8**", data_type_to_llvm(params->type)) ? "i8": data_type_to_llvm(params->type));

        strcat(method_params, param_to_add);
        params = params->next;
    }

    return method_params;
}


char * build_method_params_llvm(struct params_type_list * params) {
    int n_params = 0, max_param_name_size = 0, max_param_type_size = PARAM_TYPE_MAX_SIZE;
    struct params_type_list * p = params;
    while(p) {
        int len = strlen(p->name);
        max_param_name_size = len > max_param_name_size ? len : max_param_name_size;
        n_params++;
        p = p->next;
    }

    if(n_params == 0)return "";
    int allocated =
            /*<type>_<name>*/ (max_param_name_size + max_param_type_size + 1 + strlen(class_name) + 2) * n_params +
            /*commas&spaces*/ (n_params) * 2 + 1 /*'\0'*/;

    char * method_params = (char *) malloc(allocated * sizeof(char));
    method_params[0] = 0;
    int i;
    for(i = 1; params != NULL; ++i) {
        char buff[max_param_name_size + max_param_type_size + 2];
        buff[0] = 0;
        char * name = build_var_name(params->name, '%');
        char * type = data_type_to_llvm(params->type);
        char * app  = i == n_params ? "" : ", ";
        sprintf(buff, "%s %s%s", type, name, app);
        strcat(method_params, buff);
        params = params->next;
    }

    method_params[allocated-1] = '\0';
    return method_params;
}

char * build_var_name(char * real_name, char scope){
    char * new_name = malloc(strlen(real_name) + strlen(class_name) + 3);
    sprintf(new_name,"%c%s.%s", scope, class_name, real_name);
    return new_name;
}
