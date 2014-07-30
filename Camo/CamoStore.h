
#import <stdlib.h>
#import <vector>

//
class CamoStore : public std::vector<char *>
{
public:
	//
	~CamoStore()
	{
		for (CamoStore::iterator it = begin(); it != end(); ++it)
		{
			free(*it);
		}
	}
	
public:
	//
	bool PushSymbol(const char *code, size_t size)
	{
		for (CamoStore::iterator it = begin(); it != end(); ++it)
		{
			if (!strncmp(*it, code, size))
			{
				return false;
			}
		}
		
		if (code[0] == '<')
		{
			return false;
		}
		char *symbol = (char *)malloc(size + 1);
		memcpy(symbol, code, size);
		symbol[size] = 0;
		//puts(symbol);
		push_back(symbol);
		return true;
	}
};
