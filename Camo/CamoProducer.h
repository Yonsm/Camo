
#import "CamoStore.h"

//
class CamoProducer
{
public:
	CamoProducer(CamoStore &symbols, unsigned int begin = 0)
	{
		srand((unsigned)time(NULL));
		size_t count = symbols.size();
		for (unsigned int i = begin; i < count; i++)
		{
			char *symbol = symbols[i];
			printf("#define %s %s\n", symbol, NewSymbol());
		}
	}
	
private:
	//
	char *NewSymbol()
	{
		const static char newSymbolChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
		char symbol[256];
		while (true)
		{
			unsigned int length = 5 + rand() % 10;
			for (unsigned int i = 0; i < length; i++)
			{
				symbol[i] = newSymbolChars[rand() % (sizeof(newSymbolChars) - 1)];
			}
			
			char *newSymbol = _newSymbols.PushSymbol(symbol, length);
			if (newSymbol)
			{
				return newSymbol;
			}
		}
	}
	
private:
	CamoStore _newSymbols;
};
