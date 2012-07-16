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
1. Verifies that server blocks only use NginX directives selected from a safe list.
1. Generates a potentially valid NginX conf file, but with the .test extension.
1. Launchs NginX's built-in configuration validator for the generated configuration file.
1. Deletes invalid .test files, and renames valid ones with the .conf extension, subsequently moving these to the output directory.

Command-line Usage
------------------

`ngxsbs [options] domain [infile [outfile]]`

`ngxsbs [info-options]`

Permitted options are:

    -t templatefile : a text file that constrains and augments the generated configuration
    -op out-path    : the directory in which the output should be placed (defaults to current directory)
    -tp temp-path   : the directory for temporary configuration files (defaults to current directory)
    -d string       : a string of comma-separated name=value variable bindings
    -l              : display the license (info option)
    -u              : display usage (info option, default)

Default file names:

    infile    : ${DOMAIN}.server
    outfile   : ${out-path}${infile sans extension}.conf
    tempfiles : ${temp-path}ngxsbs.test, ${temp-path}ngxsbs.test.context

### A Complete Usage Example

`ngxsbs -t addlogs.template -d "HOME=/home/john" example.com example.com.server example.com.conf`

Given **example.com.server** as follows:

    server www.example.com {
        root /home/john/www/example.com/;
        return 301 $scheme://example.com$request_uri;
    }
    
    server example.com *.example.com {
        location / { }
    }


and the **addlogs.template** file as follows:

    === allow these directives ===
        access_log ${HOME}/
        error_log  ${HOME}/
        root       ${HOME}/
        include    /opt/nginx/includes/
        log_format
        location
        return
        error_page
        rewrite
        if
        try_files
    === host =====================
        root? ${HOME}/host/${DOMAIN}/;
    
        access_log ${DOMAIN}.access.log AccessFormat;
        error_log  ${DOMAIN}.error.log  ErrorFormat;
    
        no_log_favicon: location = /favicon.ico {
            access_log off;
            log_not_found off;
        }
    
        # USER SECTION FOLLOWS
    
        --------------------------
    
        # USER SECTION ENDS
    
    === host update.${DOMAIN} ====
        access_log? ${DOMAIN}.access.log AccessFormat;
        error_log?  ${DOMAIN}.error.log  ErrorFormat;
    
        # USER SECTION FOLLOWS
    
        --------------------------
    
        # USER SECTION ENDS
    

Note the question-mark preceding the root entry in the template.  Such entries are optional, and are overridden by any like-named directive in the user-provided configuration.  In this case, such an overriding root path is further constrained in that it must be within the user's home directory.  For the input shown here, ngxsbs will produce **example.com.conf** as follows:

    server {
        server_name www.example.com;
    
        access_log example.com.access.log AccessFormat;
        error_log  example.com.error.log  ErrorFormat;
    
        # USER SECTION FOLLOWS
    
        root /home/john/www/example.com/;
        return 301 $scheme://example.com$request_uri;
    
        # USER SECTION ENDS
    }
    
    server {
        server_name example.com *.example.com;
    
        root /home/john/host/example.com/;
    
        access_log example.com.access.log AccessFormat;
        error_log  example.com.error.log  ErrorFormat;
    
        location = /favicon.ico {
            access_log off;
            log_not_found off;
        }
    
        # USER SECTION FOLLOWS
    
        location / { }
    
        # USER SECTION ENDS
    }

Many such configurations cannot co-exist without conflicting, as long as no two configurations use the same domain name.  In addition, most of the configuration flexibility within NginX can be made available within the ngxsbs configuration files.

Server Configuration Grammar
----------------------------

The following is an almost complete description of the configuration grammar.  The main differences are the subtle details about the treatment of whitespace, and some small details pertaining to templates.  ngxsbs retains as much of the user's whitespace and comments in the generated file.  Although, this has complicated the implementation considerably, it is an important feature.  By keeping the user formatting and comments it becomes far easier to diagnose any problems that may occur.  For example, the source and generated configurations can be viewed under a diff program without the change-noise that would result from thinning whitespace and dropping comments.

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
