
#import "CamoStore.h"

//
class CamoProducer
{
public:
	CamoProducer(CamoStore &store, unsigned int begin = 0)
	{
		size_t count = store.size();
		for (unsigned int i = begin; i < count; i++)
		{
			char *symbol = store[i];
			puts(symbol);
		}
	}
	
public:
	void ProduceCamo()
	{
		
	}
};
