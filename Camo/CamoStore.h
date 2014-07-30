
#import <stdlib.h>
#import <vector>

//
class CamoStore : public std::vector<char *>
{
//public:
//	size_t maxLength;
	
public:
//	inline CamoStore()
//	{
//		maxLength = 0;
//	}
	
	//
	inline ~CamoStore()
	{
		for (CamoStore::iterator it = begin(); it != end(); ++it)
		{
			free(*it);
		}
	}
	
public:
	//
	char *PushSymbol(const char *string, size_t length)
	{
		for (CamoStore::iterator it = begin(); it != end(); ++it)
		{
			if (!strncmp(*it, string, length))
			{
				return NULL;
			}
		}
		
//		if (maxLength < length)
//		{
//			maxLength = length;
//		}
		
		char *symbol = (char *)malloc(length + 1);
		memcpy(symbol, string, length);
		symbol[length] = 0;
		//puts(symbol);
		push_back(symbol);
		return symbol;
	}
};
