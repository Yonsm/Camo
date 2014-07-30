
#import "CamoStore.h"

//
class CamoProducer
{
public:
	//
	void ProduceNewSymbols(int fd, CamoStore &symbols, unsigned int begin = 0)
	{
		srand((unsigned)time(NULL));
		size_t count = symbols.size();
		for (unsigned int i = begin; i < count; i++)
		{
			unsigned int length;
			char *symbol = symbols[i];
			write(fd, "#define ", sizeof("#define ") - 1);
			write(fd, symbol, strlen(symbol));
			write(fd, NewSymbol(length), length);
			write(fd, "\n", 1);
		}
	}

private:
	//
	char *NewSymbol(unsigned int &length)
	{
		const static char newSymbolChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
		while (true)
		{
			char buffer[256];
			length = 5 + rand() % 10;
			for (unsigned int i = 0; i < length; i++)
			{
				buffer[i] = newSymbolChars[rand() % (sizeof(newSymbolChars) - 1)];
			}
			
			char *newSymbol = _newSymbols.PushSymbol(buffer, length);
			if (newSymbol)
			{
				return newSymbol;
			}
		}
	}
	
private:
	CamoStore _newSymbols;
};
