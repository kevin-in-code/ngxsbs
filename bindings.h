
#ifndef bindings_h
#define bindings_h
#pragma once

struct BindingS;
typedef struct BindingS BindingT;

struct BindingS {
    BindingT* next;
    char* name;
    char* value;
};

BindingT* init_binding(const char* name, const char* value);
BindingT* add_binding(BindingT* list, const char* name, const char* value);
BindingT* add_known_binding(BindingT* list, BindingT* item);
BindingT* decode_bindings(const char* csv);
BindingT* lookup_binding(BindingT* list, const char* name);
void release_binding(BindingT* binding);

#endif
