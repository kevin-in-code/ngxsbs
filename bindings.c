
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "bindings.h"
#include "error.h"

BindingT* init_binding(const char* name, const char* value) {
    char* new_name;
    char* new_value;
    int nlen, vlen = 0;

    nlen = strlen(name);

    if (value != NULL) {
        vlen = strlen(value);
    }

    BindingT* binding = malloc(sizeof(BindingT));
    if (!binding) error("Out of memory");

    new_name = (char*) malloc(nlen + 1);
    if (!new_name) error("Out of memory");
    memcpy(new_name, name, nlen + 1);

    new_value = (char*) malloc(vlen + 1);
    if (!new_value) error("Out of memory");
    if (value) {
        memcpy(new_value, value, vlen + 1);
    }
    else {
        new_value[0] = '\0';
    }

    binding->next = NULL;
    binding->name = new_name;
    binding->value = new_value;
    return binding;
}

BindingT* add_binding(BindingT* list, const char* name, const char* value) {
    BindingT* binding = init_binding(name, value);
    binding->next = list;
    return binding;
}

BindingT* add_known_binding(BindingT* list, BindingT* item) {
    item->next = list;
    return item;
}

BindingT* decode_bindings(const char* csv) {
    int nameIndex, valueIndex;
    int index = 0;
    int len = strlen(csv);
    BindingT* list = NULL;
    char* buffer = (char*) malloc(len + 1);
    if (!buffer) error("Out of memory");
    memcpy(buffer, csv, len + 1);

    while (index < len) {
        nameIndex = index;
        for (; (index < len) && (buffer[index] != '='); index++);
        if (index + 1 >= len) error("Format error in variable bindings");
        buffer[index] = '\0';
        index++;
        valueIndex = index;
        for (; (index < len) && (buffer[index] != ','); index++);
        if ((index < len) && (index + 1 == len)) error("Format error in variable bindings");
        buffer[index] = '\0';
        index++;

        list = add_binding(list, &buffer[nameIndex], &buffer[valueIndex]);
    }

    free(buffer);
    return list;
}

BindingT* lookup_binding(BindingT* list, const char* name) {
    while (list) {
        if (strcmp(list->name, name) == 0) {
            return list;
        }
        else {
            list = list->next;
        }
    }
    return NULL;
}

void release_binding(BindingT* binding) {
    BindingT* rest = binding;
    while (rest) {
        binding = rest;
        rest = binding->next;
        binding->next = NULL;

        free(binding->name);
        free(binding->value);
        free(binding);
    }
}
