
#import <stdlib.h>
#import <vector>

//
enum CamoItemType
{
	CamoItemNormal,
	CamoItemIgnore,
	CamoItemProperty,
	CamoItemReadOnlyProperty,
};

//
struct CamoItem
{
	CamoItemType type;
	unsigned length;
	char symbol[];
};

//
class CamoVector : public std::vector<CamoItem *>
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
	CamoItem *PushSymbol(const char *symbol, unsigned length, CamoItemType type = CamoItemNormal)
	{
		if (length == 0)
		{
			return NULL;
		}
		
		// Skip duplicate
		for (CamoVector::iterator it = begin(); it != end(); ++it)
		{
			if ((length == (*it)->length) && !memcmp(symbol, (*it)->symbol, length))
			{
				(*it)->type = type;
				return NULL;
			}
		}
		
		//
		if (maxLength < length)
		{
			maxLength = length;
		}
		
		CamoItem *item = (CamoItem *)malloc(sizeof(CamoItem) + length);
		item->type = type;
		item->length = length;
		memcpy(item->symbol, symbol, length);
		push_back(item);
		return item;
	}
};
