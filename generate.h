
#pragma once

#ifndef generate_h
#define generate_h

#include <stdio.h>
#include "scanner.h"
#include "parser.h"
#include "bindings.h"

void generate_conf(FILE* out, NodeT* userconf, TemplateT* tmplt);
void generate_context(const char* path);
void write_node(FILE* out, NodeT* node);
void render_template_wrt_server(FILE* out, NodeT* tmplt, NodeT* server, int flush);
void render_servers(FILE* out, NodeT* servers, TemplateT* templates);

#endif
