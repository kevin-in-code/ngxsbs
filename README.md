ngxsbs
======

NginX Server Block Sanitiser - ensures that user supplied NginX server config is well behaved in a larger/shared context.

This is a command-line program that parses conf files containing definitions very similar to NginX server blocks.  The input format is slightly altered, but ngxsbs is able to validate the configuration and generate the actual NginX conf files.

Why use ngxsbs?
---------------

ngxsbs does the following important things:
#. Accepts server blocks for a specified domain name, or its subdomains.
#. Rejects server blocks for arbitrary domains not related to the specified domain name.
#. Rejects domain-less server blocks. More than one of these would cause conflicts.
#. Verifies that server blocks are broadly wellformed in isolation. This prevents injects that violate the above rules.
#. Generates a potential valid NginX conf file, but with the .conftest extension.
#. Launchs NginX's built-in configuration validator for the generated conf file.
#. Deletes invalid .conftest files, and renames valid ones with the .conf extension.

Command-line Usage:
-------------------

ngxsbs domain in-file out-file

e.g. ngxsbs example.com example.com.userconf example.com.conf
