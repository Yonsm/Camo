
Camo Preprocessor
======
<pre>
Camo Preprocessor 1.0.6.0731
Symbol Confusion for Objective C/C++
Copyleft(L) 2014, Yonsm.NET, No Rights Reserved.

Usage: Camo &lt;OutFile|$&gt; [-]&lt;File1|Dir1|@Code&gt; [-]&lt;File2|Dir2|@Code&gt; [-] ...

       OutFile     Output symbols redifinition to file
       $           Output symbols redifinition to stdout
       File|Dir    Include symbols from file or dir
       -File|-Dir  Exclude symbols from file or dir
       @Code       Include symbols from input code
       -@Code      Exclude symbols from input code
       -           Exclude symbols from Xcode iPhone SDK

Example: Camo $ ./Sources

Example: Camo ./CamoPrefix.h ./Sources -

Example: Camo ./CamoPrefix.h ./Sources -./Sources/Export.h -./Pods -

Example: Camo $ "@interface MyObj - (void)myMethod; @property BOOL myProp; @end"
</pre>

Download Binary: [Camo for Mac OS X 10.9+](https://raw.githubusercontent.com/Yonsm/Camo/master/Release/Camo)

More Info: <http://yonsm.net/camo>
