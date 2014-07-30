
#import <stdlib.h>
#import <vector>

//
enum CamoItemType
{
	CamoItemIgnore,
	CamoItemMethod,

	CamoItemProperty,
	CamoItemReadOnly,
	CamoItemSetter,
	CamoItemGetter,
	
	CamoItemProtocol,
	CamoItemInterface,
	CamoItemImplementation,
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
	CamoItem *PushSymbol(const char *symbol, unsigned length, CamoItemType type = CamoItemIgnore)
	{
		if (length == 0)
		{
			return NULL;
		}
		
		// Skip duplicate
		for (CamoVector::iterator it = begin(); it != end(); ++it)
		{
			CamoItem &item = **it;
			if ((length == item.length) && !memcmp(symbol, item.symbol, length))
			{
				item.type = type;
				return NULL;
			}
		}
		
		CamoItem *item = (CamoItem *)malloc(sizeof(CamoItem) + length);
		item->type = type;
		item->length = length;
		memcpy(item->symbol, symbol, length);
		push_back(item);
		
		//
		if (type == CamoItemProperty)
		{
			length += 3;
		}
		if (maxLength < length)
		{
			maxLength = length;
		}
		
		return item;
	}
};
