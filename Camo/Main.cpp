
#import "CamoParser.h"
#import "CamoProducer.h"

//
int main(int argc, char *argv[])
{
	puts("Camo Preprocessor 2.0.19.1206\n"
		 "Symbol Confusion for Objective C/C++\n"
		 "Copyleft(L) 2014-2019, Yonsm.NET, No Rights Reserved.\n");
	
	if (argc < 3)
	{
		puts("USAGE: Camo [OutFile|@[Prefix]] [-][Dir1|!Dir1|File1|@Code1] [-][Dir2|!Dir2|File2|@Code2] ...\n\n"
			 "       OutFile     Output symbols to file\n"
			 "       @           Output symbols to stdout\n"
			 "       Prefix      Fixed prefix instead of random\n\n"
			 "       Dir         Include symbols from dir recursive\n"
			 "       !Dir        Include symbols from dir only\n"
			 "       File        Include symbols from file\n"
			 "       @Code       Include symbols from code\n\n"
			 "       -Dir        Exclude symbols from dir recursive\n"
			 "       -!Dir       Exclude symbols from dir only\n"
			 "       -File       Exclude symbols from file\n"
			 "       -@Code      Exclude symbols from code\n"
			 "       -           Exclude symbols from iPhone SDK\n\n"
			 "EXAMPLE: Camo @ ./Sources\n\n"
			 "EXAMPLE: Camo ./CamoPrefix.h ./Sources -\n\n"
			 "EXAMPLE: Camo @ABC . -./Base -./Logic/RPC -../Pods - | grep \"IMPL\"\n\n"
			 "EXAMPLE: Camo ./CamoPrefix.h ./Sources -./Sources/Export.h -./Pods -\n\n"
			 "EXAMPLE: Camo @ \"@interface MyObj - (void)myMethod; @property BOOL myProp; @end\"\n\n"
			 "IMPORTANT: CHECK OUTPUT SYMBOLS CAREFULLY, ESPECIALLY UNDER COMPLEX CIRCUMSTANCE\n");
		return EXIT_FAILURE;
	}
	
	CamoParser parser;
	
	for (int i = 2; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			const char *path = argv[i] + 1;
			if (*path == 0) path = "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs";
			printf("EXCLUDE: %s\n", path);
			parser.Parse(path, true);
		}
	}
	
	unsigned exclude = (unsigned)parser.symbols.size();
	if (exclude)
	{
		printf("EXCLUDE: %d symbols\n\n", (int)exclude);
		
#ifdef _SUPPORT_ALIGN
		parser.symbols.maxLength = 0; // Reset
#endif
	}
	
	for (int i = 2; i < argc; i++)
	{
		if (argv[i][0] != '-')
		{
			printf("INCLUDE: %s\n", argv[i]);
			parser.Parse(argv[i]);
		}
	}

	unsigned total = (unsigned)parser.symbols.size();
	if (total > exclude)
	{
		printf("INCLUDE: %d symbols\n\n", (int)(total - exclude));
		
		CamoProducer producer(argv[1]);
		producer.ProduceSymbols(parser.symbols, exclude);
		producer.ProduceStrings(parser.strings);
	}
	else
	{
		puts("WARNING: No symbols found.\n");
	}
	
	return EXIT_SUCCESS;
}
