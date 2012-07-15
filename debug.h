
#pragma once

#ifndef debug_h
#define debug_h

#include <stdio.h>
#include "scanner.h"
#include "parser.h"
#include "bindings.h"

void test();
void show_node(FILE* out, NodeT* node, int indent);

#endif
