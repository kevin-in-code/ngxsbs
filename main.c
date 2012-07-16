
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "generate.h"
#include "bindings.h"
#include "error.h"

char* change_path(const char* prefix, const char* filename, const char* oldext, const char* ext) {
    int plen = strlen(prefix);
    int flen = strlen(filename);
    int oelen = strlen(oldext);
    int elen = strlen(ext);
    char* r;

    if ((flen > oelen) && (strcmp(&filename[flen - oelen], oldext) == 0)) {
        flen -= oelen;
    }

    r = (char*) malloc(plen + flen + elen + 1);
    if (!r) error("Out of memory");
    memcpy(r, prefix, plen);
    memcpy(&r[plen], filename, flen);
    memcpy(&r[plen+flen], ext, elen);
    r[plen+flen+elen] = '\0';

    return r;
}

const char* file_name(const char* fname) {
    int len = strlen(fname);
    int c;
    for (c = len; (c > 0) && (fname[c-1] != '/') && (fname[c-1] != '\\'); c--);
    return &fname[c];
}

int file_exists(const char* fname) {
    FILE *file = fopen(fname, "r");
    if (file)
    {
        fclose(file);
        return 1;
    }
    return 0;
}

void process(const char* domain, const char* vars,
             const char* tfile, const char* infile, const char* outfile,
             const char* temppath, const char* outpath,
             int dry_run) {

    char* in_file; 
    char* test_file; 
    char* test_context_file; 
    char* out_file;

                 FILE* context;
    FILE* test = stdout;

    BindingT* constraints = NULL;
    TemplateT* tmplt = NULL;
    NodeT* user_conf = NULL;
    BindingT* bindings = decode_bindings(vars);
    bindings = add_binding(bindings, "DOMAIN", domain);

    if (infile)
        in_file = change_path("", infile, "", "");
    else
        in_file = change_path("", domain, "", ".server");

    if (outfile)
        in_file = change_path(outpath, outfile, "", "");
    else
        out_file = change_path(outpath, file_name(infile), ".server", ".conf");
                 
    test_file = change_path(temppath, "ngxsbs.test", "", ""); 
    test_context_file = change_path(temppath, "ngxsbs.test.context", "", ""); 

    tmplt = parse_template(tfile, domain, bindings, &constraints);
    user_conf = parse_user_conf(infile, domain, constraints);

    if (!dry_run) {
        test = fopen(test_file, "w");
        if (!test) error("In processing %s: could not create: %s", infile, test_file);
    }

    render_servers(test, user_conf, tmplt);

    if (!dry_run) {
        fclose(test);

        if (!file_exists(test_context_file)) {
            context = fopen(test_context_file, "w");
            if (!context) error("In processing %s: could not create: %s", infile, test_context_file);

            fprintf(context, "http {\n    include %s;\n}", test_file);

            fclose(context);
        }

        /* NginX testing here */

        if (rename(test_file, out_file)) {
            error("In processing %s: could not move: %s -> %s", infile, test_file, out_file);
        }
    }
    
    release_node(user_conf);
    release_template(tmplt);
    release_binding(bindings);

    free(out_file);
    free(test_context_file);
    free(test_file);
    free(in_file);
}

int main() {
    process("example.com", "HOME=/home/john",
             "test/addlogs.template", "test/example.com.server", "example.com.conf",
             "", "",
             0);
    return 0;
}
