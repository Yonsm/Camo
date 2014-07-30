
Camo Preprocessor
======

<pre>
Camo Preprocessor 1.0.3
Symbol Confusion for Objective C/C++
Copyleft(L) 2014, Yonsm.NET, No Rights Reserved.

Usage: %s <OutFile|$> [-]<File1|Dir1> [-]<File2|Dir2> [-] ...

       OutFile     Output symbols redifinition to file
       $           Output symbols redifinition to stdout
       File|Dir    Include symbols from file or dir
       -File|-Dir  Exclude symbols from file or dir
       -           Exclude symbols from Xcode iPhone SDK

Example: Camo $ ./Sources

Example: Camo ./CamoPrefix.h ./Sources -

Example: Camo ./CamoPrefix.h ./Sources -./Sources/Export.h -./Pods -

</pre>
