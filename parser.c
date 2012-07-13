
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "parser.h"
#include "error.h"

typedef struct {
    const char* filename;
    ScannerT* scanner;
    NodeT* pending_token;
    BindingT* bindings;
    int primed;
    int allowBreak;
    int allowVar;
} ParserT;


NodeT* parser_parse_user_conf(ParserT* parser);
NodeT* parser_parse_template(ParserT* parser);

NodeT* parser_parse_conf(ParserT* parser);
NodeT* parser_parse_server_block(ParserT* parser);
NodeT* parser_parse_section_list(ParserT* parser);
NodeT* parser_parse_section(ParserT* parser);
NodeT* parser_parse_block_list(ParserT* parser);
NodeT* parser_parse_block(ParserT* parser);


TemplateT* init_template() {
    TemplateT* t = (TemplateT*) malloc(sizeof(TemplateT));
    if (!t) error("Out of memory");
    return t;
}

void release_template(TemplateT* t) {
    if (t) {
        release_binding(t->constraints);
        release_node(t->headPattern);
        release_node(t->tailPattern);
        free(t);
    }
}


void release_parser(ParserT* parser) {
    if (parser) {
        release_scanner(parser->scanner);
        free(parser);
    }
}

ParserT* init_parser(const char* filename, int allowBreak, BindingT* bindings) {
    ParserT* parser = (ParserT*) malloc(sizeof(ParserT));
    if (!parser) error("Out of memory");

    parser->filename = filename;
    parser->scanner = init_scanner(filename, bindings);
    parser->pending_token = NULL;
    parser->primed = 0;
    parser->allowBreak = allowBreak;
    return parser;
}

NodeT* next_token(ParserT* parser, int allowWord) {
    NodeT* token;
    if (!parser->primed) {
        parser->pending_token = scanner_scan(parser->scanner, parser->bindings, allowWord, parser->allowBreak);
        parser->primed = 1;
    }
    token = parser->pending_token;
    return token;
}

NodeT* consume_token(ParserT* parser) {
    NodeT* token = parser->pending_token;
    parser->primed = 0;
    parser->pending_token = NULL;
    return token;
}

NodeT* parse_user_conf(const char* filename) {
    ParserT* parser = init_parser(filename, 0, NULL);
    NodeT* node = parser_parse_user_conf(parser);
    release_parser(parser);
    return node;
}

NodeT* parse_template(const char* filename, BindingT* bindings) {
    ParserT* parser = init_parser(filename, 1, bindings);
    parser->bindings = bindings;
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

int chain_take(ParserT* parser, NodeT* chain, NodeT** last, NodeT** token, int kinds, int allowWord) {
    *token = next_token(parser, allowWord);
    if ((*token)->kind & kinds) {
        *last = chain_add(chain, *last, *token);
        consume_token(parser);
        return 1;
    }
    else {
        return 0;
    }
}

int chain_gobble_skip(ParserT* parser, NodeT* chain, NodeT** last, NodeT** token, int allowWord, int allowEOL, int continueAfterEOL) {
    int done = 0;
    *token = next_token(parser, allowWord);
    while ((*token)->kind == NK_SKIP) {
        if ((*token)->ends_line && !allowEOL) break;
        *last = chain_add(chain, *last, *token);
        consume_token(parser);
        if ((*token)->ends_line && !continueAfterEOL) break;
    }
    return done != 0;
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
    NodeT* chain;
    NodeT* last;
    NodeT* token = next_token(parser, 1);

    if ((token->kind == NK_SKIP) && token->ends_line) return consume_token(parser);
    
    chain = init_chain();
    last = NULL;
    
    chain_gobble_skip(parser, chain, &last, &token, 1, 0, 0);
    
    if (!chain_take(parser, chain, &last, &token, NK_WORD, 1)) error("%s:%d: server directive expected", parser->filename, token->line);
    
    while (chain_take(parser, chain, &last, &token, NK_WORD|NK_SKIP, 1));
    
    if (!chain_take(parser, chain, &last, &token, NK_LB, 1)) error("%s:%d: { expected", parser->filename, token->line);
    if (!token->ends_line) chain_gobble_skip(parser, chain, &last, &token, 1, 1, 0);
    
    last = chain_add(chain, last, parser_parse_section_list(parser));

    if (!chain_take(parser, chain, &last, &token, NK_RB, 1)) error("%s:%d: } expected", parser->filename, token->line);
    if (!token->ends_line) chain_gobble_skip(parser, chain, &last, &token, 1, 1, 0);

    return chain;
}

NodeT* parser_parse_section_list(ParserT* parser) {
    NodeT* chain = init_chain();
    NodeT* last = NULL;
    
    NodeT* item = parser_parse_section(parser);
    while (item) {
        last = chain_add(chain, last, item);
        item = parser_parse_section(parser);
    }
    return chain;
}

NodeT* parser_parse_section(ParserT* parser) {
    NodeT* chain;
    NodeT* last;
    NodeT* token = next_token(parser, 1);

    if ((token->kind == NK_SKIP) && token->ends_line) return consume_token(parser);
    
    chain = init_chain();
    last = NULL;
    
    chain_gobble_skip(parser, chain, &last, &token, 1, 0, 0);
    
    if (!chain_take(parser, chain, &last, &token, NK_WORD, 1)) error("%s:%d: directive expected", parser->filename, token->line);
    
    while (chain_take(parser, chain, &last, &token, NK_CONTENT|NK_STRING|NK_SKIP, 0));
    
    if (token->kind == NK_LB) {
        chain_take(parser, chain, &last, &token, NK_LB, 0);
        if (!token->ends_line) chain_gobble_skip(parser, chain, &last, &token, 0, 1, 0);
        
        last = chain_add(chain, last, parser_parse_block_list(parser));

        if (!chain_take(parser, chain, &last, &token, NK_RB, 1)) error("%s:%d: } expected", parser->filename, token->line);
    }
    else
    if (token->kind == NK_SEMI) {
        chain_take(parser, chain, &last, &token, NK_SEMI, 1);
    }
    else {
        error("%s:%d: { or ; expected", parser->filename, token->line);
    }
    if (!token->ends_line) chain_gobble_skip(parser, chain, &last, &token, 1, 1, 0);

    return chain;
}

NodeT* parser_parse_block_list(ParserT* parser) {
    NodeT* chain = init_chain();
    NodeT* last = NULL;
    
    NodeT* item = parser_parse_block(parser);
    while (item) {
        last = chain_add(chain, last, item);
        item = parser_parse_block(parser);
    }
    return chain;
}

NodeT* parser_parse_block(ParserT* parser) {
    NodeT* chain;
    NodeT* last;
    NodeT* token = next_token(parser, 0);

    if ((token->kind == NK_SKIP) && token->ends_line) return consume_token(parser);
    
    chain = init_chain();
    last = NULL;
    
    while (chain_take(parser, chain, &last, &token, NK_CONTENT|NK_STRING|NK_SKIP, 0));
    
    if (token->kind == NK_LB) {
        chain_take(parser, chain, &last, &token, NK_LB, 0);
        if (!token->ends_line) chain_gobble_skip(parser, chain, &last, &token, 0, 1, 0);
        
        last = chain_add(chain, last, parser_parse_block_list(parser));

        if (!chain_take(parser, chain, &last, &token, NK_RB, 0)) error("%s:%d: } expected", parser->filename, token->line);
    }
    else
    if (token->kind == NK_SEMI) {
        chain_take(parser, chain, &last, &token, NK_SEMI, 0);
    }
    else {
        error("%s:%d: { or ; expected", parser->filename, token->line);
    }
    if (!token->ends_line) chain_gobble_skip(parser, chain, &last, &token, 0, 1, 0);

    return chain;
}

NodeT* skip_tokens(ParserT* parser, int allowWord, int kinds, int allowEOL) {
    NodeT* token = next_token(parser, allowWord);
    while ((token->kind & kinds) && (allowEOL || !token->ends_line)) {
        consume_token(parser);
        release_node(token);
        token = next_token(parser, allowWord);
    }
    return token;
}

NodeT* take_token(ParserT* parser, int allowWord, int kinds, int skips, int allowEOL, int failOnError) {
    NodeT* token = next_token(parser, allowWord);
    while ((token->kind & skips) && (allowEOL || !token->ends_line)) {
        consume_token(parser);
        release_node(token);
        token = next_token(parser, allowWord);
    }
    if (!(token->kind & kinds) && failOnError) {
        error("%s:%d: template format error", parser->filename, token->line);
    }
    else {
        token = NULL;
    }
    return token;
}

BindingT* parser_parse_pair(ParserT* parser) {
    BindingT* binding = NULL;
    NodeT* name;
    NodeT* value;
    name = take_token(parser, 1, NK_WORD, NK_SKIP, 1, 0);
    if (name) {
        value = take_token(parser, 0, NK_CONTENT, NK_SKIP, 0, 1);
        if (!value->ends_line) skip_tokens(parser, 1, NK_SKIP, 0);
        binding = init_binding(name->text, value->text);
        release_node(name);
        release_node(value);
    }
    return binding;
}

NodeT* parser_parse_template(ParserT* parser) {
    BindingT* list = NULL;
    BindingT* item = NULL;
    NodeT* headPattern = NULL;
    NodeT* tailPattern = NULL;
    TemplateT* t = NULL;

    release_node(take_token(parser, 0, NK_BREAK, NK_SKIP, 1, 1));

    item = parser_parse_pair(parser);
    while (item) {
        list = add_known_binding(list, item);
        item = parser_parse_pair(parser);
    }

    release_node(take_token(parser, 0, NK_BREAK, NK_SKIP, 1, 1));

    headPattern = parser_parse_section_list(parser);

    release_node(take_token(parser, 0, NK_BREAK, NK_SKIP, 1, 1));

    tailPattern = parser_parse_section_list(parser);
    
    release_node(take_token(parser, 0, NK_BREAK, NK_SKIP, 1, 1));
    
    t = init_template();
    t->constraints = list;
    t->headPattern = headPattern;
    t->tailPattern = tailPattern;
}

