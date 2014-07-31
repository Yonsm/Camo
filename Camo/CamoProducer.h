
#import "CamoVector.h"
#import <errno.h>

//
class CamoProducer
{
public:
	//
	int Produce(const char *file, CamoVector &items, unsigned begin = 0)
	{
		bool stdo = (file[0] == '$' && file[1] == 0);
		int fd = stdo ? fileno(stdout) : creat(file, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if (fd == -1)
		{
			printf("ERROR: Could not create %s\n\n",file);
			return errno;
		}
		
		//
		const static struct {const char type[12]; const char *desc;} _comments[] =
		{
			{"/* IGNO*///", "NSClassFromString/NSSelectorFromString Ignored Symbol"},
			{"/* METH */ ", "Method Symbol"},
			
			{"/* PROP */ ", "Property Symbol"},
			{"/* REAP */ ", "Readonly Property Symbol"},
			{"/* SETP */ ", "Property Setter Symbol"},
			{"/* GETP */ ", "Property Getter Symbol"},
			
			{"/* PROT */ ", "Protocol Symbol"},
			{"/* INTE */ ", "Interface Symbol"},
			{"/* IMPL */ ", "Implementation Symbol"},
		};
		
		for (unsigned i = 0; i < sizeof(_comments) / sizeof(_comments[0]); i++)
		{
			write(fd, "//", 2);
			write(fd, _comments[i].type + 2, 5);
			write(fd, ": ", 2);
			write(fd, _comments[i].desc, strlen(_comments[i].desc));
			write(fd, "\n", 1);
		}
		write(fd, "\n", 1);
		
#ifdef _SUPPORT_ALIGN
		unsigned align = (stdo ? 0 : items.maxLength);
#define _ALIGN_ARG		, align
#define _ALIGN_PARAM	, unsigned align
#define _ALIGN_LOOP(n)	if (align) for (unsigned j = n; j <= align; j++) write(fd, " ", 1); else write(fd, " ", 1)
#else
#define _ALIGN_ARG
#define _ALIGN_PARAM
#define _ALIGN_LOOP(n)	write(fd, " ", 1)
#endif
		
		//
		signed count = 0;
		srand((unsigned)time(NULL));
		unsigned end = (unsigned)items.size();
		for (unsigned i = begin; i < end; i++)
		{
			CamoItem &item = *items[i];
			write(fd, _comments[item.type].type, sizeof(_comments[0].type) - 1);
			
			CamoItem &newItem = *NewItem(item);
			ProduceItem(fd, item, newItem _ALIGN_ARG);
			
			if (item.type == CamoItemProperty)
			{
				count += 2;
				ProduceSetItem(fd, item, newItem _ALIGN_ARG);
				
			}
			else if (item.type != CamoItemIgnore)
			{
				count++;
			}
		}
		
		printf("PRODUCE: %u valid symbols\n\n", count);
		if (!stdo) close(fd);
		
		return count;
	}
	
private:
	//
	inline void ProduceItem(int fd, CamoItem &item, CamoItem &newItem _ALIGN_PARAM)
	{
		write(fd, "#define ", sizeof("#define ") - 1);
		write(fd, item.symbol, item.length);
		
		_ALIGN_LOOP(item.length);

		write(fd, newItem.symbol, newItem.length);
		
		write(fd, "\n", 1);
	}
	
	//
	inline void ProduceSetItem(int fd, CamoItem &item, CamoItem &newItem _ALIGN_PARAM)
	{
		write(fd, "/* AUTP */ ", sizeof("/* AUTP */ ") - 1);
		write(fd, "#define ", sizeof("#define ") - 1);
		write(fd, "set", sizeof("set") - 1);
		char ch1 = toupper(item.symbol[0]);
		write(fd, &ch1, 1);
		write(fd, item.symbol + 1, item.length - 1);
		
		_ALIGN_LOOP(item.length + 3);
		
		write(fd, "set", sizeof("set") - 1);
		char ch2 = toupper(newItem.symbol[0]);
		write(fd, &ch2, 1);
		write(fd, newItem.symbol + 1, newItem.length - 1);
		
		write(fd, "\n", 1);
		
		//
		write(fd, "/* AUTV */ ", sizeof("/* AUTV */ ") - 1);
		write(fd, "#define ", sizeof("#define ") - 1);
		write(fd, "_", 1);
		write(fd, item.symbol, item.length);
		
		_ALIGN_LOOP(item.length + 3);
		
		write(fd, "_", 1);
		write(fd, newItem.symbol, newItem.length);
		
		write(fd, "\n", 1);
	}
	
	//
	CamoItem *NewItem(CamoItem &item)
	{
		char buffer[256];
		unsigned length = 8 + rand() % 16;
		const static char newSymbolChars[] = "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		while (true)
		{
			unsigned i = 0;
			if (!memcmp(item.symbol, "init", 4))
			{
				i = 5;
				memcpy(buffer, "initW", 5);
			}

			buffer[i++] = newSymbolChars[rand() % 26];	// To ensure lower case for property
			buffer[i++] = '0' + rand() % 10;	// To ensure digit char
			
			for (; i < length; i++)
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
