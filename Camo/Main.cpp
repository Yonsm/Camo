
#import "CamoParser.h"
#import "CamoProducer.h"
#import <errno.h>

//
int main(int argc, char * argv[])
{
	puts("Camo Preprocessor 1.0.5\n"
		 "Symbol Confusion for Objective C/C++\n"
		 "Copyleft(L) 2014, Yonsm.NET, No Rights Reserved.\n");
	
	if (argc < 3)
	{
		puts("Usage: Camo <OutFile|$> [-]<File1|Dir1|@Code> [-]<File2|Dir2|@Code> [-] ...\n\n"
			 "       OutFile     Output symbols redifinition to file\n"
			 "       $           Output symbols redifinition to stdout\n"
			 "       File|Dir    Include symbols from file or dir\n"
			 "       -File|-Dir  Exclude symbols from file or dir\n"
			 "       @Code       Include symbols from input code\n"
			 "       -@Code      Exclude symbols from input code\n"
			 "       -           Exclude symbols from Xcode iPhone SDK\n\n"
			 "Example: Camo $ ./Sources\n\n"
			 "Example: Camo ./CamoPrefix.h ./Sources -\n\n"
			 "Example: Camo ./CamoPrefix.h ./Sources -./Sources/Export.h -./Pods -\n\n"
			 "Example: Camo $ \"@interface MyObj - (void)myMethod; @property BOOL myProp; @end\"\n");
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
			parser.Parse(path);
		}
	}
	
	unsigned exclude = (unsigned)parser.symbols.size();
	if (exclude)
	{
		printf("EXCLUDE: %d symbols\n\n", (int)exclude);
		
		parser.symbols.maxLength = 0; // Reset
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
		
		if (argv[1][0] == '$' && argv[1][1] == 0)
		{
			CamoProducer producer(fileno(stdout), parser.symbols, exclude);
		}
		else
		{
			int fd = creat(argv[1], S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
			if (fd == -1)
			{
				printf("ERROR: Could not create %s\n\n", argv[1]);
				return errno;
			}
			
			CamoProducer producer(fd, parser.symbols, exclude);
			close(fd);
			printf("PRODUCE: %s\n\n", argv[1]);
		}
	}
	else
	{
		puts("WARNING: No symbols found.\n");
	}
	
	return EXIT_SUCCESS;
}
