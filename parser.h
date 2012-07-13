
#ifndef parser_h
#define parser_h
#pragma once

#include "scanner.h"
#include "bindings.h"

typedef struct {
    BindingT* constraints;
    NodeT* headPattern;
    NodeT* tailPattern;
} TemplateT;

TemplateT* init_template();
void release_template(TemplateT* t);

NodeT* parse_user_conf(const char* filename);
NodeT* parse_template(const char* filename, BindingT* bindings);

#endif
