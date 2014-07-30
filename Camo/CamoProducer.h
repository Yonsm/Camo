
#import "CamoVector.h"

//
class CamoProducer
{
public:
	//
	CamoProducer(int fd, CamoVector &symbols, unsigned begin = 0)
	{
		srand((unsigned)time(NULL));
		unsigned count = (unsigned)symbols.size();
		for (unsigned i = begin; i < count; i++)
		{
			write(fd, "#define ", sizeof("#define ") - 1);

			char *symbol = symbols[i];
			unsigned length = (unsigned)strlen(symbol);
			write(fd, symbol, length);
			
			for (unsigned j = length; j <= symbols.maxLength; j++)
			{
				write(fd, " ", 1);
			}
			
			write(fd, NewSymbol(length), length);
			write(fd, "\n", 1);
		}
	}

private:
	//
	char *NewSymbol(unsigned &length)
	{
		const static char newSymbolChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
		while (true)
		{
			char buffer[256];
			length = 5 + rand() % 10;
			for (unsigned i = 0; i < length; i++)
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
	CamoVector _newSymbols;
};
