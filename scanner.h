
#ifndef scanner_h
#define scanner_h
#pragma once

#include <stdio.h>

#define SCANNER_BUFFER_LEN 2048

typedef enum {
    NK_EOF,
    NK_SKIP,
    NK_STRING,
    NK_CONTENT,
    NK_WORD,
    NK_LB,
    NK_RB,
    NK_SEMI,
    NK_BREAK,
    NK_VAR,
    NK_CHAIN
} NodeKindT;

struct NodeS;
typedef struct NodeS NodeT;

struct NodeS {
    NodeKindT kind;
    char* text;
    NodeT* children;
    NodeT* next;
    int line;
};

typedef struct {
    const char* filename;
    
    char buffer[SCANNER_BUFFER_LEN];
    char* text;
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

void release_token(NodeT* token);

ScannerT* init_scanner(const char* filename);
void release_scanner(ScannerT* scanner);

NodeT* scanner_scan(ScannerT* scanner, int allowWord, int allowBreak, int allowVar);

#endif
