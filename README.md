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

`ngxsbs domain in-file out-file`

e.g. `ngxsbs example.com example.com.userconf example.com.conf`

Server Configuration Grammar
----------------------------

Tokens:

    string -> "[^"\\\r\n]*((\\(\")?)[^"\\\r\n]*)*" | '[^'\\\r\n]*((\\(\')?)[^'\\\r\n]*)*'
    content -> [^\s\r\n"'\\]+ | [^\s\r\n"'\\]*((\\[^\s\r\n]?[^\s\r\n"'\\]*)+``
    lb -> \{
    rb -> \}
    semi-> ;
    server -> server
    server_name -> server_name

Ignore:

    ws -> [\s\r\n]+
    comment -> #[^\r\n]*

Grammar:

    Conf -> Server+
    Server -> server content* lb Section rb
    Section -> content (content | string)* (lb Block rb | semi)
    Block -> (content | string | server | server_name)*  (lb Block rb | semi)
