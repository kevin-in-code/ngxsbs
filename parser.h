
#ifndef parser_h
#define parser_h
#pragma once

#include "scanner.h"

NodeT* parse_user_conf(const char* filename);
NodeT* parse_template(const char* filename);

#endif
