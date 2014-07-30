
Camo Preprocessor
======

<pre>
Camo Preprocessor 1.0
Symbol Confusion for Objective C/C++
Copyleft(L) 2014, Yonsm.NET, No Rights Reserved.

Usage: ./Camo <OutFile|$> [-]<File1|Dir1> [-]<File2|Dir2> [-] ...
       OutFile     Output symbols redifinition to file
       $           Output symbols redifinition to stdout
       File|Dir    Include symbols from file or dir
       -File|-Dir  Exclude symbols from file or dir
       -           Exclude symbols from Xcode iPhone SDK

Example: ./Camo ./CamoPrefix.h . -
</pre>
