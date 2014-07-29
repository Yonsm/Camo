
#include "CamoProducer.h"

//
int main(int argc, char * argv[])
{
	CamoParser parser;
	//parser.ParseFile("/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/Library/Xcode/Templates/File Templates/Cocoa Touch/Objective-C class.xctemplate/UITableViewCell/___FILEBASENAME___.m");
	parser.ParseDir("/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer");
	//parser.ParseDir("/Users/Yonsm/Documents/GitHub/Sample");
	//parser.ParseMethod(" - (void)setAbc:(BOOL)a; @property(XXX) BOOL *** ddd;");
	return 0;
}

