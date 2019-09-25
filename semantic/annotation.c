
#include "annotation.h"
#include <errno.h>
//FIXME ENUM TYPE FOR VARDECLS: DT_OTHER
int search_method_table(struct symtab * method_table, struct node * cur_node){

    struct table_line * cur_line = method_table->first_line;
    while(cur_line){

        if(cur_line->line_type == T_OTHER && !strcmp(cur_line->name, cur_node->lval)){
            cur_node->annotation->single_type = cur_line->type.single_type;
            return cur_node->line;
        }
        cur_line = cur_line->next_line;
    }
    return 0;
}


void check_if_redeclared(struct symtab * method_table, struct node * cur_node){
    struct table_line * cur_line = method_table->first_line;
    while(cur_line){

        if(cur_line->line_type == T_OTHER && !strcmp(cur_line->name, cur_node->lval)
           &&
           (cur_node->line > cur_line->field_line  || (cur_node->line == cur_line->field_line && cur_node->col > cur_line->field_col))){
            throw_semantics_error(SE_REDEF, cur_node, NULL,NULL);
            return;
        }
        cur_line = cur_line->next_line;
    }
}

int check_method_redeclaration(struct symtab * class_table, struct params_type_list * method_params, char * method_name) {
    struct table_line * class_table_line = class_table->first_line;

    while(class_table_line) {
        if(class_table_line->line_type == T_METHOD &&
            !strcmp(method_name, class_table_line->name) &&
                    same_parameters(method_params, class_table_line->type.method_params) == 0) {
                return 0;
        }
        class_table_line = class_table_line->next_line;
    }
    return 1;
}

int same_parameters(struct params_type_list * params1, struct params_type_list * params2) {
    int n_params1 = count_line_params(params1),
        n_params2 = count_line_params(params2);

    if(n_params1 == n_params2) {
        while(params1 && params2) {
            if(params1->type != params2->type)
                return 1;
            params1 = params1->next;
            params2 = params2->next;
        }
        return 0;
    }
    return 1;
}

int evaluate_candidate_call(struct node * params, struct params_type_list * t_line_params){
    int legit = 1;
    while(params && t_line_params){
        if(t_line_params->type != params->annotation->single_type){
            legit = 0;
        }

		if(params->annotation->single_type == DT_UNDEF || t_line_params->type == DT_UNDEF){
			return 0;
		}

        else if(t_line_params->type == DT_DECLIT && params->annotation->single_type != DT_DECLIT){
            return 0;
        }
        else if(t_line_params->type == DT_REALLIT &&
               !(params->annotation->single_type == DT_REALLIT ||
                   params->annotation->single_type == DT_DECLIT )){
            return 0;
        }
        else if(t_line_params->type == DT_BOOLEAN && params->annotation->single_type != DT_BOOLEAN){
            return 0;
        }
        else if(t_line_params->type == DT_STRING_ARRAY && params->annotation->single_type != DT_STRING_ARRAY){
            return 0;
        }
        else if(t_line_params->type == DT_VOID && params->annotation->single_type != DT_VOID){
            return 0;
        }

        params = params->sibling;
        t_line_params = t_line_params->next;
    }

    if(legit)
        return 2;

    return 1;
}


void throw_semantics_error(enum sem_error error, struct node * token, char * types, struct node * expr){
    switch(error){
        case SE_NOT_FOUND:
            printf("Line %d, col %d: Cannot find symbol %s", token->line, token->col, token->lval);
            if(types){
                printf("%s", types);
            }
            break;
        case SE_INCOMPATIBLE:
            printf("Line %d, col %d: Incompatible type %s in %s statement", expr->line, expr->col, types, token->lval);
            break;
        case SE_BOUNDS:
            printf("Line %d, col %d: Number %s out of bounds", token->line, token->col, token->lval);
            break;
        case SE_BAD_TYPES:
            printf("Line %d, col %d: Operator %s cannot be applied to types %s", token->line, token->col, token->lval, types);
            break;
        case SE_BAD_TYPE:
            printf("Line %d, col %d: Operator %s cannot be applied to type %s", expr != NULL ? expr->line : token->line, expr != NULL ? expr->col : token->col, token->lval, types);
            break;
;
        case SE_AMBIGUOUS:
            printf("Line %d, col %d: Reference to method %s%s is ambiguous", token->line, token->col, token->lval, types);
            break;
        case SE_REDEF:
            printf("Line %d, col %d: Symbol %s%s already defined", token->line, token->col, token->lval,  types == NULL ? "" : types);

            break;

    }
    printf("\n");
}


int check_double_issue(enum data_type dt1, enum data_type dt2){
    if((dt1 == DT_REALLIT && dt2 == DT_DECLIT) ||
        (dt1 == DT_DECLIT && dt2 == DT_REALLIT)){
            return 1;
    }
    return 0;
}

int is_reallit_zero(char * str) {
    int size = strlen(str);
    int i;
    for (i = 0; i < size; i++) {
        if (str[i] == 'e' || str[i] == 'E') {
                return 1;
            }

        if (str[i] != '.' && str[i] != '0' && str[i] != '_') {
            return 0;
        }
    }
    return 1;
}

//TODO: Check this

int check_bounds(struct node * target) {

    if(!strcmp(target->token, "RealLit")) {
        //submit it
        //strtod(target->lval, &ptr);
	char * to_free = remove_char_from_string(target->lval , '_');
        if(atof(to_free) > DBL_MAX ||
            (atof(to_free) == 0 && is_reallit_zero(to_free) == 0) ) {
            throw_semantics_error(SE_BOUNDS, target, NULL, NULL);
	    //free(to_free);
            return 0;
        }
	//free(to_free);
	return 1;
    }
    else{
     	 char * to_free = remove_char_from_string(target->lval , '_');
         int size = strlen("2147483648");
         int str_size = strlen(to_free);
         if( str_size >  size || (str_size == size &&
                strcmp(to_free, "2147483648") >= 0)) {
            throw_semantics_error(SE_BOUNDS, target, NULL, NULL);
            //target->annotation->single_type = DT_UNDEF;
            //free(to_free);
            return 0;
         }
         //free(to_free);
/*
         long int val = atol(target->lval);
        if(val > INT_MAX){
            throw_semantics_error(SE_BOUNDS, target, NULL, NULL);
            target->annotation->single_type = DT_UNDEF;
            return 0;
        }*/
    }
    return 1;

}
/*int check_bounds(struct node * target){

    if(!strcmp(target->token, "RealLit")){
        double val = atof(target->lval);
        if(val > DBL_MAX){
            throw_semantics_error(SE_BOUNDS, target, NULL,NULL);
            return 0;
        }
    }
    else{

        char * to_free = remove_char_from_string(target->lval , '_');
        long int val = atol(to_free);

        if(val > INT_MAX){
            throw_semantics_error(SE_BOUNDS, target, NULL,NULL);
            target->annotation->single_type = DT_UNDEF;
            //target->annotation->method_params = NULL;
            return 0;
        }
    }
    return 1;

}*/

enum data_type check_method_call(struct symtab * class_table, struct node * method_id){
    struct table_line * cur_line = class_table->first_line;
    struct table_line * candidate;

    int n_candidates = 0;
    while(cur_line){

        if(cur_line->line_type == T_METHOD && !strcmp(cur_line->name, method_id->lval)){

            struct node * params = method_id->sibling;
            struct params_type_list * t_line_params = cur_line->type.method_params;
            if(count_siblings(params) != count_line_params(t_line_params)){
                cur_line = cur_line->next_line;
                continue;
            }

            int call = evaluate_candidate_call(params, t_line_params);

            if(call == 2){
                method_id->annotation->method_params = cur_line->type.method_params;
				if(!method_id->annotation->method_params){
					method_id->annotation->single_type = DT_NO_PARAMS;
				}
                return cur_line->type.single_type;
            }
            else if(call == 1){
                candidate = cur_line;
                n_candidates++;

            }
        }

        cur_line = cur_line->next_line;
    }

    if(n_candidates == 1){
        method_id->annotation->method_params = candidate->type.method_params;
        return candidate->type.single_type;
    }

	else if(n_candidates > 1){
		throw_semantics_error(SE_AMBIGUOUS, method_id, get_node_param_types(method_id->sibling,count_siblings(method_id->sibling)), NULL);
		method_id->annotation->single_type = DT_UNDEF;
        return DT_UNDEF;

	}
    else{

        method_id->annotation->single_type = DT_UNDEF;
        throw_semantics_error(SE_NOT_FOUND, method_id, get_node_param_types(method_id->sibling, count_siblings(method_id)), NULL);
        return DT_UNDEF;
    }
}

void annotate_method(struct symtab * method_table, struct symtab * class_table, struct node * parent, struct node * first_child){


	if(!strcmp(first_child->token, "VarDecl")) {
            struct node * type_node = first_child->child;
            struct node * id_node = type_node->sibling;

            struct table_line * var_line = create_other_line(id_node->lval, first_child->child->sibling->line, first_child->child->sibling->col, NO_FLAG, string_to_data_type(type_node->token));

            if(check_field_redeclaration(method_table, id_node->lval) == 0) {
                throw_semantics_error(SE_REDEF, id_node, NULL, NULL);
                id_node->annotation->single_type = DT_NULL;
            }
			else{
				first_child->child->sibling->annotation->single_type = DT_HIDE;
				append_line_to_table(method_table, var_line);
			}


    }


    else if(first_child->child)
        annotate_method(method_table, class_table, first_child, first_child->child);

    if(first_child->annotation->single_type != DT_NULL || first_child->annotation->method_params){
    }

    else if(!strcmp(first_child->token, "Id") && first_child->annotation->single_type != DT_UNDEF){


        //This ID is a method name
        if(parent != NULL && !strcmp(parent->token, "Call")){

            //annotate params
            struct node * prov = first_child->sibling;
            while(prov){
		        annotate_method(method_table, class_table, NULL, prov);
                prov = prov->sibling;

            }
            parent->annotation->single_type = check_method_call(class_table, first_child);
        }

        else if(!search_method_table(method_table, first_child) && !search_method_table(class_table, first_child)){
            throw_semantics_error(SE_NOT_FOUND, first_child, NULL, NULL);
            first_child->annotation->single_type = DT_UNDEF;
            first_child->annotation->method_params = NULL;

        }

    }
    //undef undef
    else if(!strcmp(first_child->token, "Assign")){

	first_child->annotation->single_type = first_child->child->annotation->single_type;

        if(( first_child->child->annotation->single_type != DT_UNDEF && first_child->child->sibling->annotation->single_type != DT_UNDEF
        && first_child->child->annotation->single_type != DT_STRING_ARRAY && first_child->child->sibling->annotation->single_type != DT_STRING_ARRAY
        && first_child->child->annotation->single_type == first_child->child->sibling->annotation->single_type)
		|| (first_child->child->annotation->single_type == DT_REALLIT &&
            	first_child->child->sibling->annotation->single_type == DT_DECLIT)){

        }

        else{
            char incompatible_types[15];
            sprintf(incompatible_types, "%s, %s" , data_type_to_string(first_child->child->annotation->single_type),
            data_type_to_string(first_child->child->sibling->annotation->single_type), NULL);
            throw_semantics_error(SE_BAD_TYPES, first_child, incompatible_types, NULL);

        }
    }

    else if(!strcmp(first_child->token, "Add") || !strcmp(first_child->token, "Sub") ||
            !strcmp(first_child->token, "Mul") || !strcmp(first_child->token, "Div") ||
            !strcmp(first_child->token, "Mod")){

        if(first_child->child->annotation->single_type != DT_BOOLEAN &&
		   first_child->child->sibling->annotation->single_type != DT_BOOLEAN &&
           first_child->child->annotation->single_type != DT_UNDEF &&
           first_child->child->sibling->annotation->single_type != DT_UNDEF &&
           first_child->child->annotation->single_type != DT_VOID &&
           first_child->child->sibling->annotation->single_type != DT_VOID &&
           first_child->child->annotation->single_type != DT_STRING_ARRAY &&
           first_child->child->sibling->annotation->single_type != DT_STRING_ARRAY &&
		   first_child->child->annotation->single_type == first_child->child->sibling->annotation->single_type){
            first_child->annotation->single_type = first_child->child->annotation->single_type;
        }
        else if(check_double_issue(first_child->child->annotation->single_type, first_child->child->sibling->annotation->single_type)){
            first_child->annotation->single_type = DT_REALLIT;

        }
        else{
            char incompatible_types[15];
            sprintf(incompatible_types, "%s, %s" , data_type_to_string(first_child->child->annotation->single_type),
            data_type_to_string(first_child->child->sibling->annotation->single_type));
            throw_semantics_error(SE_BAD_TYPES, first_child, incompatible_types, NULL);
            first_child->annotation->single_type = DT_UNDEF;
        }
    }

    else if(!strcmp(first_child->token, "And") || !strcmp(first_child->token, "Or")){
        first_child->annotation->single_type = DT_BOOLEAN;
        if(first_child->child->annotation->single_type == DT_BOOLEAN &&
           first_child->child->sibling->annotation->single_type == DT_BOOLEAN){
                //first_child->annotation->single_type = DT_BOOLEAN;
        }
        else{
            char incompatible_types[15];
            sprintf(incompatible_types, "%s, %s" , data_type_to_string(first_child->child->annotation->single_type),
            data_type_to_string(first_child->child->sibling->annotation->single_type));
            throw_semantics_error(SE_BAD_TYPES, first_child, incompatible_types, NULL);
            //first_child->annotation->single_type = DT_UNDEF;
        }

    }
    else if(!strcmp(first_child->token, "Eq") || !strcmp(first_child->token, "Neq")) {
            first_child->annotation->single_type = DT_BOOLEAN;
                if((first_child->child->annotation->single_type == first_child->child->sibling->annotation->single_type &&
                first_child->child->annotation->single_type != DT_VOID && first_child->child->sibling->annotation->single_type != DT_VOID &&
               first_child->child->annotation->single_type != DT_UNDEF && first_child->child->sibling->annotation->single_type != DT_UNDEF &&
               first_child->child->annotation->single_type != DT_STRING_ARRAY && first_child->child->sibling->annotation->single_type != DT_STRING_ARRAY) ||
                check_double_issue(first_child->child->annotation->single_type, first_child->child->sibling->annotation->single_type)){
                //first_child->annotation->single_type = DT_BOOLEAN;
            }

            else{
                //CREATE METHOD TO ADD TYPES
                char incompatible_types[15];
                sprintf(incompatible_types, "%s, %s" , data_type_to_string(first_child->child->annotation->single_type),
                data_type_to_string(first_child->child->sibling->annotation->single_type));
                throw_semantics_error(SE_BAD_TYPES, first_child, incompatible_types, NULL);
                //first_child->annotation->single_type = DT_UNDEF;
            }
    }

    else if(!strcmp(first_child->token, "Geq") ||
            !strcmp(first_child->token, "Gt") || !strcmp(first_child->token, "Leq") ||
            !strcmp(first_child->token, "Lt") ){
                first_child->annotation->single_type = DT_BOOLEAN;
                if((first_child->child->annotation->single_type == first_child->child->sibling->annotation->single_type &&
                first_child->child->annotation->single_type != DT_VOID && first_child->child->sibling->annotation->single_type != DT_VOID &&
                first_child->child->annotation->single_type != DT_BOOLEAN && first_child->child->sibling->annotation->single_type != DT_BOOLEAN &&
               first_child->child->annotation->single_type != DT_UNDEF && first_child->child->sibling->annotation->single_type != DT_UNDEF &&
               first_child->child->annotation->single_type != DT_STRING_ARRAY && first_child->child->sibling->annotation->single_type != DT_STRING_ARRAY) ||
                check_double_issue(first_child->child->annotation->single_type, first_child->child->sibling->annotation->single_type)){
                //first_child->annotation->single_type = DT_BOOLEAN;
            }

            else{
                //CREATE METHOD TO ADD TYPES
                char incompatible_types[15];
                sprintf(incompatible_types, "%s, %s" , data_type_to_string(first_child->child->annotation->single_type),
                data_type_to_string(first_child->child->sibling->annotation->single_type));
                throw_semantics_error(SE_BAD_TYPES, first_child, incompatible_types, NULL);
                //first_child->annotation->single_type = DT_UNDEF;
            }

    }

    else if(!strcmp(first_child->token, "Plus") || !strcmp(first_child->token, "Minus")){
        if(first_child->child->annotation->single_type == DT_DECLIT || first_child->child->annotation->single_type == DT_REALLIT){
            first_child->annotation->single_type = first_child->child->annotation->single_type;

        }
        else{
            throw_semantics_error(SE_BAD_TYPE, first_child, data_type_to_string(first_child->child->annotation->single_type), NULL);
            first_child->annotation->single_type = DT_UNDEF;
        }

    }

    else if(!strcmp(first_child->token, "Not")){
        first_child->annotation->single_type = DT_BOOLEAN;
        if(first_child->child->annotation->single_type == DT_BOOLEAN){
            //first_child->annotation->single_type = DT_BOOLEAN;
        }
        else{
            throw_semantics_error(SE_BAD_TYPE, first_child, data_type_to_string(first_child->child->annotation->single_type), NULL);
            //first_child->annotation->single_type = DT_UNDEF;
        }

    }
    else if(!strcmp(first_child->token, "ParseArgs")) {
        first_child->annotation->single_type = DT_DECLIT;
        if(first_child->child->annotation->single_type != DT_STRING_ARRAY ||
            first_child->child->sibling->annotation->single_type != DT_DECLIT) {
                char incompatible_types[15];
            sprintf(incompatible_types, "%s, %s" , data_type_to_string(first_child->child->annotation->single_type),
            data_type_to_string(first_child->child->sibling->annotation->single_type));
            throw_semantics_error(SE_BAD_TYPES, first_child, incompatible_types, first_child->child);
        }
    }
    //TODO: Add Error 1
    else if(!strcmp(first_child->token, "Length") ){
        first_child->annotation->single_type = DT_DECLIT;
        //STRING AND STRINGARRAY? E O PRINT?
        if(first_child->child->annotation->single_type != DT_STRING_ARRAY){
            throw_semantics_error(SE_BAD_TYPE, first_child, data_type_to_string(first_child->child->annotation->single_type), NULL);
        }

    }

    else if(!strcmp(first_child->token, "Print")){
        if(first_child->child->annotation->single_type == DT_VOID ||
           first_child->child->annotation->single_type == DT_STRING_ARRAY ||
           first_child->child->annotation->single_type == DT_UNDEF){
            throw_semantics_error(SE_INCOMPATIBLE, first_child, data_type_to_string(first_child->child->annotation->single_type), first_child->child);

        }
    }

    else if(!strcmp(first_child->token, "DecLit") || !strcmp(first_child->token, "RealLit")){
            first_child->annotation->single_type = string_to_data_type(!strcmp(first_child->token, "DecLit") ? "Int" : "Double");
            check_bounds(first_child);

            /*}
		else
			first_child->annotation->single_type = DT_UNDEF;
*/
    }

    else if(!strcmp(first_child->token, "BoolLit") || !strcmp(first_child->token, "StrLit")){
            first_child->annotation->single_type = !strcmp(first_child->token, "BoolLit") ? DT_BOOLEAN : DT_STRING ;
    }


    //TODO: TH
    else if(!strcmp(first_child->token, "Return")){
        if((!first_child->child && method_table->first_line->type.single_type != DT_VOID)){
            throw_semantics_error(SE_INCOMPATIBLE, first_child, "void", first_child);
        }
        else if((first_child->child && method_table->first_line->type.single_type == DT_VOID)) {
            throw_semantics_error(SE_INCOMPATIBLE, first_child, data_type_to_string(first_child->child->annotation->single_type), first_child->child);
        }
        //Há filho mas não é o esperado
        else if(first_child->child &&
                (method_table->first_line->type.single_type != first_child->child->annotation->single_type &&
                !(method_table->first_line->type.single_type == DT_REALLIT && first_child->child->annotation->single_type == DT_DECLIT))){
            throw_semantics_error(SE_INCOMPATIBLE, first_child, data_type_to_string(first_child->child->annotation->single_type), first_child->child);
        }
    }

    else if(!strcmp(first_child->token, "DoWhile")){
        if(first_child->child->sibling->annotation->single_type != DT_BOOLEAN){
            throw_semantics_error(SE_INCOMPATIBLE, first_child, data_type_to_string(first_child->child->sibling->annotation->single_type), first_child->child->sibling );

        }
    }

        //STATEMENTS
    if(parent && (!strcmp(parent->token, "If") || !strcmp(parent->token, "While"))){

        if(first_child->annotation->single_type != DT_BOOLEAN){
            throw_semantics_error(SE_INCOMPATIBLE, parent, data_type_to_string(first_child->annotation->single_type), first_child);

        }
    }

    if(first_child->sibling)
        annotate_method(method_table, class_table, NULL, first_child->sibling);

}
