# gettext-prof
A gettext profiler / dynamic POT file generation

Generates POT files based on messages that were actually requested during a program's execution.

Quick Start Guide
-----------------

1. Compile it:

`make`

1. Use it with an application, such as gcc:

`path/dynamic-gettext gcc`

1. In the end, a `*_dynamic.pot` file(s) is/are created in CWD.
This `.pot` will contain exactly the messages that were really requested from gettext at runtime.

1. Merge your translation with the `*_dynamic.pot`:

`msgmerge -U gcc-9.1.xx.po gcc*_dynamic.pot`
