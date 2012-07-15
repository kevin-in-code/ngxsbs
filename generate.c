
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "generate.h"
#include "error.h"

/*
for each server block:
    find applicable template block
    for each head template section:
        if required or not found in server block:
            output section
    output server block content
    for each tail template section:
        if required or not found in server block:
            output section
*/

TemplateT* find_template_block(NodeT* server_block, TemplateT* templates);
int section_in_server_block(NodeT* section, NodeT* server_block);
void write_node(FILE* out, NodeT* node);


TemplateT* find_template_block(NodeT* server_block, TemplateT* templates) {
}

int section_in_server_block(NodeT* section, NodeT* server_block) {
}

void write_node(FILE* out, NodeT* node) {
    if (node) {
        if (node->kind == NK_CHAIN) {
            node = node->children;
            while (node) {
                write_node(out, node);
                node = node->next;
            }
        }
        else {
            fputs(node->text, out);
        }
    }
}


