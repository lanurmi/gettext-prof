# gettext-prof
A gettext profiler / dynamic POT file generation

Generates POT files based on messages that were actually requested during a program's execution.

This tool is meant for situations where translating all messages of a multi-thousand-message
translation catalog is unrealistic, and the important messages that are actually used during
regular execution should somehow be cherry-picked.

Quick Start Guide
-----------------

1. Compile it:

`make`

2. Use it to execute an application, in this case 'gcc':

`path/dynamic-gettext gcc`

3. In the end, a `*_dynamic.pot` file(s) is/are created in CWD.
This `.pot` will contain exactly the messages that were really requested from gettext at runtime.

4. Merge your translation with the `*_dynamic.pot`:

`msgmerge -U gcc-9.1.xx.po gcc*_dynamic.pot`

5. Complete the translation of your `.po`.

6. Merge your .po again, now against the upstream `.pot`

`msgmerge -U gcc-9.1.xx.po gcc-9.1.pot`

Now your `.po` contains translations for the messages found through profiling, as well as
translations that possibly were in the `.po` initially.
