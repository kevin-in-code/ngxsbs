
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "debug.h"
#include "error.h"
#include "generate.h"

void print_indent(FILE* out, int indent) {
    while (indent > 0) {
        fputs("    ", out);
        indent--;
    }
}

void print_indented_text(FILE* out, int indent, const char* text) {
    print_indent(out, indent);
    fputs(text, out);
}

void print_wrapped_text(FILE* out, int indent, const char* start, const char* text, const char* stop) {
    print_indent(out, indent);
    fputs(start, out);
    fputs(text, out);
    fputs(stop, out);
}

void show_node(FILE* out, NodeT* node, int indent) {
    if (node) {
        switch (node->kind) {
            case NK_CHAIN:
                print_indented_text(out, indent, "<chain>\n");
                node = node->children;
                while (node) {
                    show_node(out, node, indent + 1);
                    node = node->next;
                }
                print_indented_text(out, indent, "</chain>\n");
                break;

            case NK_EOF:
                print_indented_text(out, indent, "<eof/>\n");
                break;

            case NK_SKIP:
                print_wrapped_text(out, indent, "<skip>", node->text, "</skip>\n");
                break;

            case NK_STRING:
                print_wrapped_text(out, indent, "<str>", node->text, "</str>\n");
                break;

            case NK_CONTENT:
                print_wrapped_text(out, indent, "<content>", node->text, "</content>\n");
                break;

            case NK_WORD:
                print_wrapped_text(out, indent, "<word>", node->text, "</word>\n");
                break;

            case NK_BREAK:
                print_wrapped_text(out, indent, "<break>", node->text, "</break>\n");
                break;

            case NK_LB:
                print_indented_text(out, indent, "<{>\n");
                break;

            case NK_RB:
                print_indented_text(out, indent, "<}>\n");
                break;

            case NK_SEMI:
                print_indented_text(out, indent, "<;>\n");
                break;

            default:
                print_wrapped_text(out, indent, "<other>", node->text, "</other>\n");
                break;
        }
    }
}

void test() {
    BindingT* bindings = decode_bindings("DOMAIN=example.com,HOME=/home/john");
    BindingT* constraints = NULL;
    TemplateT* tmplt = NULL;
    NodeT* user_conf = NULL;

    tmplt = parse_template("test/addlogs.template", "example.com", bindings, &constraints);
    user_conf = parse_user_conf("test/example.com.server", "example.com", constraints);

    show_node(stdout, user_conf, 0);
    fputs("\n\n-----------------\n\n", stdout);
    show_node(stdout, tmplt->headPattern, 0);
    fputs("\n\n-----------------\n\n", stdout);
    show_node(stdout, tmplt->tailPattern, 0);
    fputs("\n\n-----------------\n\n", stdout);
    write_node(stdout, user_conf);
    fputs("\n\n-----------------\n\n", stdout);
    render_servers(stdout, user_conf, tmplt);

    release_node(user_conf);
    release_template(tmplt);
    release_binding(bindings);
}

int main() {
    test();
}

