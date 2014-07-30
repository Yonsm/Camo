
#import "CamoVector.h"

//
class CamoProducer
{
public:
	//
	CamoProducer(int fd, CamoVector &items, unsigned begin = 0)
	{
		srand((unsigned)time(NULL));
		unsigned count = (unsigned)items.size();
		for (unsigned i = begin; i < count; i++)
		{
			CamoItem &item = *items[i];
			if (item.type == CamoItemIgnore)
			{
				write(fd, "//", 2);
			}
			
			write(fd, "#define ", sizeof("#define ") - 1);
			write(fd, item.symbol, item.length);
			
			for (unsigned j = item.length; j <= items.maxLength; j++)
			{
				write(fd, " ", 1);
			}
			
			CamoItem &newItem = *NewItem();
			write(fd, newItem.symbol, newItem.length);
			
			write(fd, "\n", 1);
			
			if (item.type == CamoItemProperty)
			{
				write(fd, "#define ", sizeof("#define ") - 1);
				write(fd, "set", sizeof("set") - 1);
				char ch1 = toupper(item.symbol[0]);
				write(fd, &ch1, 1);
				write(fd, item.symbol + 1, item.length - 1);
				
				for (unsigned j = item.length; j <= items.maxLength; j++)
				{
					write(fd, " ", 1);
				}
				
				write(fd, "set", sizeof("set") - 1);
				char ch2 = toupper(newItem.symbol[0]);
				write(fd, &ch2, 1);
				write(fd, newItem.symbol + 1, newItem.length - 1);
				
				write(fd, "\n", 1);
			}
		}
	}
	
private:
	//
	CamoItem *NewItem()
	{
		char buffer[256];
		unsigned length = 5 + rand() % 10;
		const static char newSymbolChars[] = "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		while (true)
		{
			buffer[0] = newSymbolChars[rand() % 26];	// To ensure lower case property
			for (unsigned i = 1; i < length; i++)
			{
				buffer[i] = newSymbolChars[rand() % (sizeof(newSymbolChars) - 1)];
			}
			
			CamoItem *newItem = _newItems.PushSymbol(buffer, length);
			if (newItem)
			{
				return newItem;
			}
		}
	}
	
private:
	CamoVector _newItems;
};
