
#import <stdlib.h>
#import <vector>

//
class CamoVector : public std::vector<char *>
{
public:
	unsigned maxLength;
	
public:
	inline CamoVector()
	{
		maxLength = 0;
	}
	
	//
	inline ~CamoVector()
	{
		for (CamoVector::iterator it = begin(); it != end(); ++it)
		{
			free(*it);
		}
	}
	
public:
	//
	char *PushSymbol(const char *string, unsigned length)
	{
		// Skip duplicate
		for (CamoVector::iterator it = begin(); it != end(); ++it)
		{
			const char *p1 = *it;
			const char *p2 = string;
			for (unsigned i = 0; ; i++)
			{
				if (i == length)
				{
					if (p1[i] == 0) return NULL;
					else break;
				}
				else if (p1[i] != p2[i])
				{
					break;
				}
			}
		}
		
		//
		if (maxLength < length)
		{
			maxLength = length;
		}
		
		char *symbol = (char *)malloc(length + 1);
		memcpy(symbol, string, length);
		symbol[length] = 0;
		push_back(symbol);
		return symbol;
	}
};
