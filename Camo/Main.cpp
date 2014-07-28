
#include "CamoProducer.h"

//
int main(int argc, char * argv[])
{
	CamoParser parser;
//	//parser.ParseFile("/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS7.1.sdk/System/Library/Frameworks/Foundation.framework/Headers/NSRunLoop.h");
	parser.ParseDir("/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer");
	//parser.ParseDir("/Users/Yonsm/Documents/GitHub/Sample");
	return 0;
}

