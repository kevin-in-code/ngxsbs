
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "scanner.h"
#include "error.h"

void release_node(NodeT* token) {
    if (token) {
        NodeT* next;
        if (token->text) free(token->text);
        if (token->children) release_node(token->children);
        next = token;
        free(token);
        token = next->next;
        
        while (token) {
            next = token->next;
            token->next = NULL;
            release_node(token);
            token = next;
        }
    }
}

ScannerT* init_scanner(const char* filename, BindingT* bindings) {
    ScannerT* scanner;
    char* buffer;

    scanner = (ScannerT*) malloc(sizeof(ScannerT));
    if (!scanner) error("Out of memory");

    FILE* file = fopen(filename, "r");
    if (!file) error("Could not open file: %s", filename);

    buffer = (char*) malloc(1024);
    if (!buffer) error("Out of memory");
    
    scanner->bindings = bindings;
    scanner->injection = NULL;
    
    scanner->filename = filename;
    scanner->file = file;

    scanner->text = buffer;
    scanner->capacity = 1023;
    scanner->used = 0;
    
    scanner->index = 0;
    scanner->size = 0;
    scanner->line_start = 1;
    scanner->line = 1;
    return scanner;
}

void release_scanner(ScannerT* scanner) {
    if (scanner) {
        free(scanner->text);
        fclose(scanner->file);
    }
}

char prime_buffer(ScannerT* scanner) {
    if ((scanner->index + 128 >= SCANNER_BUFFER_LEN) && (scanner->size == SCANNER_BUFFER_LEN)) {
        /* Time to pull more data from the source file. */
        int size, t;
        int free = scanner->index;
        int count = scanner->size - scanner->index;
        if (scanner->index > 0) memcpy(&scanner->buffer[0], &scanner->buffer[free], count);
        size = fread(&scanner->buffer[count], 1, SCANNER_BUFFER_LEN - count, scanner->file);
        scanner->index = 0;
        scanner->size = size + count;
        
        for (t = size; t < SCANNER_BUFFER_LEN; t++) {
            scanner->buffer[t] = '\0';
        }            
    }
    return scanner->buffer[scanner->index];
}

int is_word_char(char c) {
    switch (c) {
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y':
        case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y':
        case 'Z':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case '_':
            return 1;
        default:
            return 0;
    }
}

int is_content_char(char c) {
    return ((c != '\r') || (c != '\n') ||
            (c != ' ') || (c != '\t') ||
            (c != '\'') || (c != '\"') || (c != '\\'));
}

char peek(ScannerT* scanner, int c) {
    if (scanner->index + c < scanner->size) {
        return scanner->buffer[scanner->index + c];
    }
    else {
        return '\0';
    }
}

void peek_bindings(ScannerT* scanner) {
    BindingT* nv;
    char* cs;
    int k = 0;

    if (peek(scanner, 0) != '{') return;
    if (peek(scanner, 1) != '$') return;
    scanner->index += 2;

    for (; k < 126; k++) {
        char c = peek(scanner, k);
        if (is_word_char(c)) {
            scanner->sidebuf[k] = c;
        }
        else {
            break;
        }
    }
    scanner->sidebuf[k] = '\0';
    if (peek(scanner, k) != '}') error("%s:%d: invalid template variable on this line", scanner->filename, scanner->line);
    k++;
    scanner->index += k;
    
    nv = lookup_binding(scanner->bindings, &scanner->sidebuf[0]);
    if (!nv) error("%s:%d: unrecognised template variable on this line", scanner->filename, scanner->line);
    scanner->injection = nv->value;
}

char take_char(ScannerT* scanner, char c) {
    /* returns next char */

    if (scanner->index + 128 >= scanner->size) {
        prime_buffer(scanner);
    }

    if (scanner->used == scanner->capacity) {
        char* buffer = (char*) malloc(scanner->capacity + 1024 + 1);
        if (!buffer) error("Out of memory");
        memcpy(buffer, scanner->text, scanner->used);
        free(scanner->text);
        scanner->text = buffer;
        scanner->capacity += 1024 + 1;
    }
    
    scanner->text[scanner->used] = c;
    scanner->used++;

    if (scanner->bindings && (!scanner->injection) && (scanner->buffer[scanner->index] == '{')) {
        peek_bindings(scanner);
    }
    
    if (scanner->injection) {
        char next = scanner->injection[0];
        if (next) {
            scanner->injection = &scanner->injection[1];
            if (scanner->injection[0] == '\0') scanner->injection = NULL;
            return next;
        }
        else {
            scanner->injection = NULL;
        }
    }

    if (scanner->index < scanner->size) {
        scanner->index++;
        if (scanner->index < scanner->size) {
            char next = scanner->buffer[scanner->index];
            if (next == '\0') error("Input contains nul");
            
            return next;
        }
        else
            return '\0';
    }
    else {
        return '\0';
    }
}


NodeT* scanner_scan(ScannerT* scanner, BindingT* bindings, int allowWord, int allowBreak) {
    NodeT* token;
    char next = prime_buffer(scanner);
    
    token = (NodeT*) malloc(sizeof(NodeT));
    if (!token) error("Out of memory");
    
    scanner->used = 0;
    
    token->line = scanner->line;
    token->text = NULL;
    token->children = NULL;
    token->next = NULL;
    token->ends_line = 0;
    if (scanner->index == scanner->size) {
        /* We've run out of input. */
        token->kind = NK_EOF;
        return token;
    }
    else {
        int lf = 0;
        /* Finally, now we're ready to scan. */
        switch (next) {
            case ' ':
            case '\t':
                token->kind = NK_SKIP;
                scanner->line_start = 0;
                next = take_char(scanner, next);
                while ((next) && ((next == ' ') || (next == '\t'))) {
                    next = take_char(scanner, next);
                }
                if (next == '#') {
                    while ((next) && (next != '\r') && (next != '\n')) {
                        next = take_char(scanner, next);
                    }
                }
                lf = 0;
                if (next == '\r') {
                    next = take_char(scanner, next);
                    lf = 1;
                }
                if (next == '\n') {
                    next = take_char(scanner, next);
                    lf = 1;
                }
                scanner->line += lf;
                token->ends_line = lf;
                break;
                
            case '\r':
                token->kind = NK_SKIP;
                scanner->line_start = 1;
                scanner->line++;
                next = take_char(scanner, next);
                if (next == '\n') next = take_char(scanner, next);
                token->ends_line = 1;
                break;

            case '\n':
                token->kind = NK_SKIP;
                scanner->line_start = 1;
                scanner->line++;
                next = take_char(scanner, next);
                token->ends_line = 1;
                break;
            
            case '#':
                token->kind = NK_SKIP;
                scanner->line_start = 0;
                next = take_char(scanner, next);
                while ((next) && (next != '\r') && (next != '\n')) {
                    next = take_char(scanner, next);
                }
                lf = 0;
                if (next == '\r') {
                    next = take_char(scanner, next);
                    lf = 1;
                }
                if (next == '\n') {
                    next = take_char(scanner, next);
                    lf = 1;
                }
                scanner->line += lf;
                token->ends_line = lf;
                break;
            
            case '-':
                if (!allowBreak || !scanner->line_start || (peek(scanner, 1) != '-') || (peek(scanner, 2) != '-')) goto content;
                token->kind = NK_BREAK;
                scanner->line_start = 0;
                next = take_char(scanner, next);
                while ((next) && (next != '\r') && (next != '\n')) {
                    next = take_char(scanner, next);
                }
                lf = 0;
                if (next == '\r') {
                    next = take_char(scanner, next);
                    lf = 1;
                }
                if (next == '\n') {
                    next = take_char(scanner, next);
                    lf = 1;
                }
                scanner->line += lf;
                token->ends_line = lf;
                break;
            
            case '\"':
                token->kind = NK_STRING;
                scanner->line_start = 0;
                next = take_char(scanner, next);
                do {
                    while ((next) && (next != '\r') && (next != '\n') && (next != '\"') && (next != '\\')) {
                        next = take_char(scanner, next);
                    }
                    while (next == '\\') {
                        next = take_char(scanner, next);
                        if ((next) && (next != '\r') && (next != '\n')) next = take_char(scanner, next);
                    }
                } while ((next) && (next != '\r') && (next != '\n') && (next != '\"'));
                if (next == '\"') {
                    next = take_char(scanner, next);
                }
                else {
                    error("%s:%d: string does not end on this line", scanner->filename, token->line);
                }
                break;
            
            case '\'':
                token->kind = NK_STRING;
                scanner->line_start = 0;
                next = take_char(scanner, next);
                do {
                    while ((next) && (next != '\r') && (next != '\n') && (next != '\'') && (next != '\\')) {
                        next = take_char(scanner, next);
                    }
                    while (next == '\\') {
                        next = take_char(scanner, next);
                        if ((next) && (next != '\r') && (next != '\n')) next = take_char(scanner, next);
                    }
                } while ((next) && (next != '\r') && (next != '\n') && (next != '\''));
                if (next == '\'') {
                    next = take_char(scanner, next);
                }
                else {
                    error("%s:%d: string does not end on this line", scanner->filename, token->line);
                }
                break;
            
            case 'a': case 'b': case 'c': case 'd': case 'e':
            case 'f': case 'g': case 'h': case 'i': case 'j':
            case 'k': case 'l': case 'm': case 'n': case 'o':
            case 'p': case 'q': case 'r': case 's': case 't':
            case 'u': case 'v': case 'w': case 'x': case 'y':
            case 'z':
            case 'A': case 'B': case 'C': case 'D': case 'E':
            case 'F': case 'G': case 'H': case 'I': case 'J':
            case 'K': case 'L': case 'M': case 'N': case 'O':
            case 'P': case 'Q': case 'R': case 'S': case 'T':
            case 'U': case 'V': case 'W': case 'X': case 'Y':
            case 'Z':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            case '_':
                token->kind = NK_WORD;
                scanner->line_start = 0;
                next = take_char(scanner, next);
                while ((next) && is_word_char(next)) {
                    next = take_char(scanner, next);
                }
                if (!allowWord) {
                    token->kind = NK_CONTENT;
                    if ((next) && (is_content_char(next) || (next == '\\'))) {
                        goto content;
                    }
                }
                break;
            
            case '{':
                token->kind = NK_LB;
                scanner->line_start = 0;
                next = take_char(scanner, next);
                break;
            
            case '}':
                token->kind = NK_RB;
                scanner->line_start = 0;
                next = take_char(scanner, next);
                break;
            
            case ';':
                token->kind = NK_SEMI;
                scanner->line_start = 0;
                next = take_char(scanner, next);
                break;
            
            default:
            content:
                token->kind = NK_CONTENT;
                scanner->line_start = 0;
                do {
                    while (next == '\\') {
                        next = take_char(scanner, next);
                        if ((next) && ((next != ' ') || (next != '\t') || (next != '\r') || (next != '\n'))) {
                            next = take_char(scanner, next);
                        }
                    }
                    while ((next) && is_content_char(next)) {
                        next = take_char(scanner, next);
                    }
                } while (next == '\\');
                break;
        }

        token->text = (char*) malloc(scanner->used+1);
        if (!token->text) error("Out of memory");
        memcpy(token->text, scanner->text, scanner->used+1);
        token->text[scanner->used] = '\0';

        return token;
    }
}
