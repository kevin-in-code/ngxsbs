
#ifndef scanner_h
#define scanner_h
#pragma once

#include <stdio.h>

#define MAX_TOKEN_LEN 1024
#define SCANNER_BUFFER_LEN (MAX_TOKEN_LEN*2)

typedef enum {
    TT_EOF,
    TT_EOL,
    TT_WS,
    TT_COMMENT,
    TT_STRING,
    TT_CONTENT,
    TT_LB,
    TT_RB,
    TT_SEMI,
    TT_SERVER,
    TT_SERVER_NAME
} TokenKindT;

typedef struct {
    TokenKindT kind;
    char text[MAX_TOKEN_LEN];
    int length;
} TokenT;

typedef struct {
    char buffer[SCANNER_BUFFER_LEN];
    FILE* file;
    int index;
    int size;
} ScannerT;

int init_scanner(ScannerT* scanner, const char* filename);
void close_scanner(ScannerT* scanner);

int scan(ScannerT* scanner, TokenT* token);

#endif
