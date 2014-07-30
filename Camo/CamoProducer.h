
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
		
		//
		signed count = 0;
		srand((unsigned)time(NULL));
		unsigned end = (unsigned)items.size();
		for (unsigned i = begin; i < end; i++)
		{
			CamoItem &item = *items[i];
			write(fd, _comments[item.type].type, sizeof(_comments[0].type) - 1);
			
			write(fd, "#define ", sizeof("#define ") - 1);
			write(fd, item.symbol, item.length);
			
#ifdef _SUPPORT_ALIGN
			if (!stdo)
			{
				for (unsigned j = item.length; j <= items.maxLength; j++)
				{
					write(fd, " ", 1);
				}
			}
			else
#endif
			{
				write(fd, " ", 1);
			}
			
			CamoItem &newItem = *NewItem();
			write(fd, newItem.symbol, newItem.length);
			
			write(fd, "\n", 1);
			
			if (item.type == CamoItemProperty)
			{
				count += 2;
				
				write(fd, "/* AUTP */ ", sizeof("/* AUTP */ ") - 1);
				write(fd, "#define ", sizeof("#define ") - 1);
				write(fd, "set", sizeof("set") - 1);
				char ch1 = toupper(item.symbol[0]);
				write(fd, &ch1, 1);
				write(fd, item.symbol + 1, item.length - 1);
				
#ifdef _SUPPORT_ALIGN
				if (!stdo)
				{
					for (unsigned j = item.length + 3; j <= items.maxLength; j++)
					{
						write(fd, " ", 1);
					}
				}
				else
#endif
				{
					write(fd, " ", 1);
				}
				
				write(fd, "set", sizeof("set") - 1);
				char ch2 = toupper(newItem.symbol[0]);
				write(fd, &ch2, 1);
				write(fd, newItem.symbol + 1, newItem.length - 1);
				
				write(fd, "\n", 1);
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
	CamoItem *NewItem()
	{
		char buffer[256];
		unsigned length = 10 + rand() % 15;
		const static char newSymbolChars[] = "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		while (true)
		{
			buffer[0] = newSymbolChars[rand() % 26];	// To ensure lower case for property
			buffer[1] = '0' + rand() % 10;	// To ensure digit char
			for (unsigned i = 2; i < length; i++)
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
