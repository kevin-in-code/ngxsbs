
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "generate.h"
#include "error.h"

/*
render server blocks with template:
    for each server block:
        find applicable template block

        emit conf start

        render head template wrt server block

        flush empty line
        output server block content

        render tail template wrt server block

        emit conf end

render template chain wrt server block:
    for each chain section:
        if empty:
            note pending empty line
        else
        if skippable:
            flush empty line
            output section
        else
        if required
           or (overridable and not found in server block)
           or (helper      and     found in server block):
            flush empty line
            output section
*/

NodeT* first_item(NodeT* items) {
    while ((items) && (items->kind & (NK_SKIP|NK_BREAK))) {
        items = items->next;
    }
    return items;
}

NodeT* second_item(NodeT* items) {
    items = first_item(items);
    if (items) {
        items = items->next;
        items = first_item(items);
    }
    return items;
}

TemplateT* find_template_block(NodeT* server, TemplateT* templates);
int directive_in_server_block(NodeT* dir, NodeT* server);
void write_node(FILE* out, NodeT* node);
void write_nodes(FILE* out, NodeT* node);
void write_until(FILE* out, NodeT* chain, NodeT* sentinel);

void render_template_wrt_server(FILE* out, NodeT* tmplt, NodeT* server, int flush);

void render_servers(FILE* out, NodeT* servers, TemplateT* templates) {
    if (servers->kind == NK_CHAIN) servers = servers->children;
    while (servers) {
        NodeT* server = servers->children;
        NodeT* chain = NULL;
        TemplateT* tmplt = find_template_block(server, templates);

        if (tmplt) {
            fputs("server {\n", out);
            
            NodeT* domain = second_item(server);
            while ((domain) && (domain->kind == NK_CONTENT)) {
                fprintf(out, "    server_name %s;\n", domain->text);
                domain = second_item(domain);
            }
            fputs("\n", out);

            chain = domain;
            while ((chain) && (chain->kind != NK_CHAIN)) {
                chain = second_item(chain);
            }
            if (chain) chain = chain->children;
            
            render_template_wrt_server(out, tmplt->headPattern, chain, 1);

            write_nodes(out, chain);

            render_template_wrt_server(out, tmplt->tailPattern, chain, 0);
            fputs("}\n", out);
        }

        servers = servers->next;
        if (servers) {
            fputs("\n", out);
        }
    }
}

void render_template_wrt_server(FILE* out, NodeT* tmplt, NodeT* server, int flush) {
    NodeT* sep = NULL;
    tmplt = tmplt->children;
    while (tmplt) {
        NodeT* section = tmplt;
        if (section) {
            if (section->empty) {
                if (!sep) sep = section;
            }
            else
            if (section->kind != NK_CHAIN) {
                if (sep) {
                    fputs(sep->text, out);
                    sep = NULL;
                }
                fputs(section->text, out);
            }
            else {
                int normal = 0;
                section = section->children;
                NodeT* first = first_item(section);
                NodeT* second = second_item(first);
                NodeT* third = second_item(second);

                if (second) {
                    if (strcmp(second->text, "?") == 0) {
                        if (!directive_in_server_block(first, server)) {
                            write_until(out, section, second);
                            write_nodes(out, second->next);
                        }
                    }
                    else
                    if (strcmp(second->text, ":") == 0) {
                        if (directive_in_server_block(first, server)) {
                            write_until(out, section, first);
                            write_nodes(out, third);
                        }
                    }
                    else {
                        normal = 1;
                    }
                }
                else {
                    normal = 1;
                }

                if (normal) write_nodes(out, section);
            }
        }

        tmplt = tmplt->next;
    }
}


TemplateT* find_template_block(NodeT* server, TemplateT* templates) {
    while (templates) {
        int ok = 1;
        TemplateT* tmplt = templates;
        templates = templates->next;

        NodeT* domain = second_item(server);
        while ((domain) && (domain->kind == NK_CONTENT)) {
            if (!valid_prefix(tmplt->prefix, domain->text)) {
                ok = 0;
                break;
            }
            domain = second_item(domain);
        }

        if (ok) return tmplt;
    }
    return NULL;
}

int directive_in_server_block(NodeT* dir, NodeT* server) {
    while (server) {
        NodeT* section = server;
        server = server->next;

        if (section->kind == NK_CHAIN) {
            NodeT* the_dir = first_item(section->children);
            if (strcmp(dir->text, the_dir->text) == 0) return 1;
        }
    }
    return 0;
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

void write_nodes(FILE* out, NodeT* node) {
    while (node) {
        write_node(out, node);
        node = node->next;
    }
}

void write_until(FILE* out, NodeT* chain, NodeT* sentinel) {
    if ((chain) && (chain != sentinel)) {
        if (chain->kind == NK_CHAIN) chain = chain->children;
        while ((chain) && (chain != sentinel)) {
            write_node(out, chain);
            chain = chain->next;
        }
    }
}

