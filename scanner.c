
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "scanner.h"

int init_scanner(ScannerT* scanner, const char* filename) {
    FILE* file = fopen(filename, "r");
    scanner->file = file;
    scanner->index = 0;
    scanner->size = 0;
    return (file)? 1 : 0;
}

void close_scanner(ScannerT* scanner) {
    fclose(scanner->file);
}

int scan(ScannerT* scanner, TokenT* token) {
    char next;
    int index;
    int total;
    if ((scanner->index >= MAX_TOKEN_LEN) && (scanner->size == SCANNER_BUFFER_LEN)) {
        /* Time to pull more data from the source file. */
        int size;
        int free = scanner->index;
        int count = scanner->size - scanner->index;
        if (scanner->index > 0) memcpy(&scanner->buffer[0], &scanner->buffer[free], count);
        size = fread(&scanner->buffer[count], 1, SCANNER_BUFFER_LEN - count, scanner->file);
        scanner->index = 0;
        scanner->size = size + count;
    }
    index = scanner->index;
    total = scanner->size;
    if (index == total) {
        /* We've run out of input. */
        token->kind = TT_EOF;
        token->length = 0;
        return 0;
    }
    else {
        /* Finally, now we're ready to scan. */
        next = scanner->buffer[index];
    }
}
