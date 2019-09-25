#include "data_types.h"


struct params_type_list * append_param_node (struct params_type_list * head, struct params_type_list * new_node) {
    if(!head)
        return new_node;
    struct params_type_list * aux = head;

    while(aux->next)
        aux = aux->next;
    aux->next = new_node;
    return head;
}