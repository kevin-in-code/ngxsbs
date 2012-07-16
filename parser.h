
#pragma once

#ifndef parser_h
#define parser_h

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

int valid_prefix(const char* sub, const char* text);

TemplateT* init_template();
void release_template(TemplateT* t);

NodeT* parse_user_conf(const char* filename, const char* domain, BindingT* constraints);
TemplateT* parse_template(const char* filename, const char* domain, BindingT* bindings, BindingT** constraints);

#endif
