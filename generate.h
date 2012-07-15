
#pragma once

#ifndef generate_h
#define generate_h

#include <stdio.h>
#include "scanner.h"
#include "parser.h"
#include "bindings.h"

void generate_conf(FILE* out, NodeT* userconf, TemplateT* tmplt);
void generate_context(const char* path);

#endif
