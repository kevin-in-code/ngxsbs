
#ifndef parser_h
#define parser_h
#pragma once

#include "scanner.h"
#include "bindings.h"

struct TemplateS;
typedef struct TemplateS TemplateT;

struct TemplateS {
    char* prefix;
    NodeT* headPattern;
    NodeT* tailPattern;
    TemplateT* next;
};

TemplateT* init_template();
void release_template(TemplateT* t);

NodeT* parse_user_conf(const char* filename, const char* domain, BindingT* constraints);
TemplateT* parse_template(const char* filename, const char* domain, BindingT* bindings, BindingT** constraints);

#endif
