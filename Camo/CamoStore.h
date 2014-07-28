
#include <stdlib.h>
#include <vector>

//
class CamoStore
{
public:
	//
	std::vector<char *> _symbols;
	
public:
	//
	~CamoStore()
	{
		for (std::vector<char *>::iterator it = _symbols.begin(); it != _symbols.end(); ++it)
		{
			free(*it);
		}
	}
	
public:
	//
	bool PushSymbol(const char *code, size_t size)
	{
		for (std::vector<char *>::iterator it = _symbols.begin(); it != _symbols.end(); ++it)
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
		
		puts(symbol);
		
		_symbols.push_back(symbol);
		return true;
	}
};
