
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
			if (items[i]->type != CamoItemIgnore)
			{
				write(fd, "#define ", sizeof("#define ") - 1);
				write(fd, items[i]->symbol, items[i]->length);
				
				for (unsigned j = items[i]->length; j <= items.maxLength; j++)
				{
					write(fd, " ", 1);
				}
				
				CamoItem *newItem = NewItem();
				write(fd, newItem->symbol, newItem->length);
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
			for (unsigned i = 0; i < length; i++)
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
