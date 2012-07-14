
#ifndef scanner_h
#define scanner_h
#pragma once

#include <stdio.h>
#include "bindings.h"

#define SCANNER_BUFFER_LEN 2048

typedef enum {
    NK_EOF,
    NK_SKIP = 1,
    NK_STRING = 2,
    NK_CONTENT = 4,
    NK_WORD = 8,
    NK_LB = 16,
    NK_RB = 32,
    NK_SEMI = 64,
    NK_BREAK = 128,
    NK_CHAIN = 256
} NodeKindT;

struct NodeS;
typedef struct NodeS NodeT;

struct NodeS {
    NodeKindT kind;
    char* text;
    NodeT* children;
    NodeT* next;
    int ends_line;
    int line;
};

typedef struct {
    const char* filename;
    BindingT* bindings;
    
    char buffer[SCANNER_BUFFER_LEN];
    char sidebuf[128];
    char* text;
    char* injection;
    FILE* file;

    /* scanner input buffer */
    int index;
    int size;

    /* token text buffer */
    int capacity;
    int used;
    
    int line_start;
    int line;
} ScannerT;

void release_node(NodeT* token);

ScannerT* init_scanner(const char* filename, BindingT* bindings);
void release_scanner(ScannerT* scanner);

NodeT* scanner_scan(ScannerT* scanner, int allowWord, int allowBreak);

#endif
