
Camo Preprocessor
======
<pre>
Camo Preprocessor 1.0.10.0909
Symbol Confusion for Objective C/C++
Copyleft(L) 2014, Yonsm.NET, No Rights Reserved.

USAGE: Camo [OutFile|@[Prefix]] [-][Dir1|File1|@Code1] [-][Dir2|File2|@Code2] ...

       OutFile     Output symbols to file
       @           Output symbols to stdout
       Prefix      Fixed prefix instead of random

       Dir         Include symbols from dir
       File        Include symbols from file
       @Code       Include symbols from code

       -Dir        Exclude symbols from dir
       -File       Exclude symbols from file
       -@Code      Exclude symbols from code
       -           Exclude symbols from iPhone SDK

EXAMPLE: Camo $ ./Sources

EXAMPLE: Camo ./CamoPrefix.h ./Sources -

EXAMPLE: Camo @ABC . -./Base -./Logic/RPC -../Pods - | grep "IMPL"

EXAMPLE: Camo ./CamoPrefix.h ./Sources -./Sources/Export.h -./Pods -

EXAMPLE: Camo $ "@interface MyObj - (void)myMethod; @property BOOL myProp; @end"

IMPORTANT: CHECK OUTPUT SYMBOLS CAREFULLY, ESPECIALLY UNDER COMPLEX CIRCUMSTANCE
</pre>

Download Binary: [Camo for Mac OS X 10.9+](https://raw.githubusercontent.com/Yonsm/Camo/master/Release/Camo)

More Info: <http://yonsm.net/camo>
