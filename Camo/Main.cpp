
#import "CamoParser.h"
#import "CamoProducer.h"

#define MAX_SLUGIFY 64
unsigned slugify(char *dst, const char *src, unsigned length)
{
	signed j = 0;
	for (signed i = 0; (i < length && j < MAX_SLUGIFY - 1); i++)
	{
		char c = src[i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
		{
			dst[j++] = c;
		}
		else if (j && dst[j - 1] != '_')
		{
			dst[j++] = '_';
		}
	}
	if (j && dst[j - 1] == '_')
		j--;
	dst[j] = 0;
	return j;
}

//
int main(int argc, char *argv[])
{
	puts("Camo Preprocessor 1.0.10.0909\n"
		 "Symbol Confusion for Objective C/C++\n"
		 "Copyleft(L) 2014, Yonsm.NET, No Rights Reserved.\n");
	
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
	
	write(fileno(stdout), "\n", 1);
	unsigned end = (unsigned)parser.strings.size();
	for (unsigned i = 0; i < end; i++)
	{
		CamoItem &item = *parser.strings[i];
		char name[MAX_SLUGIFY];
		unsigned length = slugify(name, item.symbol, item.length);

		write(fileno(stdout), item.type == CamoItemString ? "// " : "//@", 3);
		write(fileno(stdout), item.symbol, item.length);
		write(fileno(stdout), "\n", 1);

		write(fileno(stdout), "#define CAMO_", 13);
		write(fileno(stdout), name, length);
		write(fileno(stdout), " ", 1);
		
		if (item.type == CamoItemString)
		{
			write(fileno(stdout), "char camo_", 10);
			write(fileno(stdout), name, length);
			printf("[%d];", item.length + 1);

			write(fileno(stdout), " CamoStringDecode(camo_", 23);
			write(fileno(stdout), name, length);
			write(fileno(stdout), ", \"", 3);
		}
		else
		{
			write(fileno(stdout), " CFBridgingRelease(CamoStringDecode2(\"", 38);
		}

		for (unsigned k = 0; k < item.length; k++)
		{
			char c = item.symbol[k];
			if (c == '\\')
			{
				c = item.symbol[++k];
				if (c == 'r') c = '\r';
				else if (c == 'n') c = '\n';
				else if (c == 't') c = '\t';
			}
			c = (c ^ (unsigned char)item.length) - k;
			printf("\\x%02x", c);
		}
		printf("\", %d)", item.length);
		printf((item.type == CamoItemNSString) ? ")\n" : "\n");
			
	}
	write(fileno(stdout), "\n", 1);

	unsigned total = (unsigned)parser.symbols.size();
	if (total > exclude)
	{
		printf("INCLUDE: %d symbols\n\n", (int)(total - exclude));
		
		CamoProducer producer;
		
		return producer.Produce(argv[1], parser.symbols, exclude);
	}
	else
	{
		puts("WARNING: No symbols found.\n");
	}
	
	return EXIT_SUCCESS;
}
