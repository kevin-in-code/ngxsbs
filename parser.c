
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "parser.h"
#include "error.h"

typedef struct {
    const char* filename;
    ScannerT* scanner;
    NodeT* pending_token;
    int primed;
    int allowBreak;
    int allowVar;
} ParserT;

void release_parser(ParserT* parser) {
    if (parser) {
        release_scanner(parser->scanner);
        free(parser);
    }
}

ParserT* init_parser(const char* filename, int allowBreak, int allowVar) {
    ParserT* parser = (ParserT*) malloc(sizeof(ParserT));
    if (!parser) error("Out of memory");

    parser->filename = filename;
    parser->scanner = init_scanner(filename);
    parser->pending_token = NULL;
    parser->primed = 0;
    parser->allowBreak = allowBreak;
    parser->allowVar = allowVar;
    return parser;
}

NodeT* next_token(ParserT* parser, int allowWord) {
    NodeT* token;
    if (!parser->primed) {
        parser->pending_token = scanner_scan(parser->scanner, allowWord, parser->allowBreak, parser->allowVar);
        parser->primed = 1;
    }
    token = parser->pending_token;
    return token;
}

void consume_token(ParserT* parser) {
    parser->primed = 0;
    parser->pending_token = NULL;
}

NodeT* parse_user_conf(const char* filename) {
    ParserT* parser = init_parser(filename, 0, 0);
    NodeT* node = parser_parse_user_conf(parser);
    release_parser(parser);
    return node;
}

NodeT* parse_template(const char* filename) {
    ParserT* parser = init_parser(filename, 1, 1);
    NodeT* node = parser_parse_template(parser);
    release_parser(parser);
    return node;
}

NodeT* init_chain() {
    NodeT* node = (NodeT*) malloc(sizeof(NodeT));
    if (!node) error("Out of memory");
    
    node->kind = NK_CHAIN;
    node->text = NULL;
    node->children = NULL;
    node->next = NULL;
    node->line = 0;
    return node;
}

NodeT* chain_add(NodeT* chain, NodeT* last, NodeT* item) {
    /* returns the new last item */
    
    if (last) {
        last->next = item;
    }
    else {
        chain->children = item;
    }
    return item;
}

NodeT* parser_parse_user_conf(ParserT* parser) {
    NodeT* conf = init_chain();
    NodeT* last = NULL;
    
    NodeT* item = parser_parse_conf(parser);
    while (item) {
        last = chain_add(conf, last, item);
        item = parser_parse_conf(parser);
    }
    return conf;
}

NodeT* parser_parse_conf(ParserT* parser) {
    NodeT* conf = init_chain();
    NodeT* last = NULL;
    
    NodeT* item = parser_parse_server_block(parser);
    while (item) {
        last = chain_add(conf, last, item);
        item = parser_parse_server_block(parser);
    }
    return conf;
}

NodeT* parser_parse_server_block(ParserT* parser) {
    NodeT* token = next_token(parser, 1);
    NodeT* last = NULL;
    
    NodeT* item = parser_parse_conf(parser);
    while (item) {
        last = chain_add(conf, last, item);
        item = parser_parse_conf(parser);
    }
    return conf;
}


