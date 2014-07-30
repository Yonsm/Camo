
//#include "CamoStore.h"
#import "CamoParser.h"
#import "CamoProducer.h"

//
int main(int argc, char * argv[])
{
	CamoParser parser;
	//parser.ParseFile("/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS7.1.sdk/System/Library/Frameworks/Foundation.framework/Headers/NSByteCountFormatter.h");
	parser.ParseDir("/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer");
	//parser.ParseDir("/Users/Yonsm/Documents/GitHub/Sample");
	//parser.ParseMethod(" - (void)setAbc:(BOOL)a; @property(XXX) BOOL *** ddd;");
	
	{
		printf("Camo Preprocessor 1.0\n"
			   "Symbol Confusion for Objective C/C++\n"
			   "Copyleft (L) 2014, Yonsm.NET, No Rights Reserved.\n\n");
	}
	
	CamoProducer producer;
	producer.ProduceNewSymbols(fileno(stdout), parser);

	return 0;
}

