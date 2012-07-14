ngxsbs
======

NginX Server Block Sanitiser - ensures that user supplied NginX server config is well behaved in a larger/shared context.

This is a command-line program that parses conf files containing definitions very similar to NginX server blocks.  The input format is slightly altered, but ngxsbs is able to validate the configuration and generate the actual NginX conf files.

Why use ngxsbs?
---------------

ngxsbs does the following important things:

1. Accepts server blocks for a specified domain name, or its subdomains.
1. Rejects server blocks for arbitrary domains not related to the specified domain name.
1. Rejects domain-less server blocks. More than one of these would cause conflicts.
1. Verifies that server blocks are broadly wellformed in isolation. This prevents injects that violate the above rules.
1. Generates a potential valid NginX conf file, but with the .conftest extension.
1. Launchs NginX's built-in configuration validator for the generated conf file.
1. Deletes invalid .conftest files, and renames valid ones with the .conf extension.

Command-line Usage
------------------

`ngxsbs [options] domain infile [outfile]`

`ngxsbs [info-options]`

Permitted options are:

    -t templatefile : a text file that constrains and augments the generated configuration
    -d string       : a string of comma-separated name=value variable bindings
    -l              : display the license (info option)
    -u              : display usage (info option, default)

### A Complete Usage Example

`ngxsbs -t addlogs.template -d "HOME=/home/john" example.com example.com.userconf example.com.conf`

Given example.com.userconf as follows:

    server www.example.com {
        access_log /home/john/logs/www.example.com.access.log AccessFormat;
        return 301 $scheme://example.com$request_uri;
    }

    server example.com *.example.com {
        location / { }
    }


and the addlogs.template file as follows:

    === allow these directives ===
        access_log ${HOME}/
        error_log  ${HOME}/
        include    /opt/nginx/includes/
        log_format
        location
        return
        error_page
        rewrite
        if
        try_files
    === host =====================
        access_log? ${DOMAIN}.access.log AccessFormat;
        error_log   ${DOMAIN}.error.log  ErrorFormat;

        # USER SECTION FOLLOWS

    ------------------------------

        # USER SECTION ENDS
    === host update.${DOMAIN} ====
        access_log? ${DOMAIN}.access.log AccessFormat;
        error_log   ${DOMAIN}.error.log  ErrorFormat;

        # USER SECTION FOLLOWS

    ------------------------------

        # USER SECTION ENDS

Note the question-mark preceding the access_log entry in the template.  Such entries are optional, and are overridden by any like-named directive in the user-provided configuration.  ngxsbs will produce example.com.conf as follows:

    server {
        server_name www.example.com;
        error_log  example.com.error.log  ErrorFormat;

        # USER SECTION FOLLOWS

        access_log example.com.access.log AccessFormat;
        return 301 $scheme://example.com$request_uri;

        # USER SECTION ENDS
    }

    server {
        server_name example.com *.example.com;
        access_log example.com.access.log AccessFormat;
        error_log  example.com.error.log  ErrorFormat;

        # USER SECTION FOLLOWS

        location / { }

        # USER SECTION ENDS
    }

Many such configurations cannot co-exist without conflicting, as long as no two configurations use the same domain name.  In addition, most of the configuration flexibility within NginX can be made available within the ngxsbs configuration files.

Server Configuration Grammar
----------------------------

The following is an almost complete description of the configuration grammar.  The main differences are the subtle details about the treatment of whitespace, and some small details pertaining to templates.  ngxsbs retains whitespace and comments in the generated file.  This should help in diagnosing any problems that may occur.

Tokens:

    string      -> "[^"\\\r\n]*((\\(\")?)[^"\\\r\n]*)*" | '[^'\\\r\n]*((\\(\')?)[^'\\\r\n]*)*'
    content     -> [^\s\r\n"'\\]+ | [^\s\r\n"'\\]*((\\[^\s\r\n]?[^\s\r\n"'\\]*)+
    lb          -> \{
    rb          -> \}
    semi        -> ;
    server      -> server
    directive   -> [A-Za-z0-9_]+

Ignore:

    ws      -> [\s\r\n]+
    comment -> #[^\r\n]*

Grammar:

    Conf    -> Server*
    Server  -> server content* lb Section* rb
    Section -> directive (content | string)* (lb Section* rb | semi)
