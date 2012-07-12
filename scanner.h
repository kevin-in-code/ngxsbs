
#ifndef scanner_h
#define scanner_h
#pragma once

#include <stdio.h>

#define MAX_TOKEN_LEN 1024
#define SCANNER_BUFFER_LEN (MAX_TOKEN_LEN*2)

typedef enum {
    NK_EOF,
    NK_EOL,
    NK_WS,
    NK_COMMENT,
    NK_STRING,
    NK_CONTENT,
    NK_WORD,
    NK_LB,
    NK_RB,
    NK_SEMI,
    NK_BREAK,
    NK_CHAIN
} NodeKindT;

struct NodeS;
typedef struct NodeS NodeT;

struct NodeS {
    NodeKindT kind;
    char* text;
    NodeT* children;
    NodeT* next;
};

typedef struct {
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
} ScannerT;

void release(NodeT* token);

int init_scanner(ScannerT* scanner, const char* filename);
void close_scanner(ScannerT* scanner);

NodeT* scan(ScannerT* scanner, int allowWord, int allowBreak);

#endif
