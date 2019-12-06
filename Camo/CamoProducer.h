
#import "CamoVector.h"
#import <errno.h>
#define MAX_SLUGIFY 64

//
class CamoProducer
{
public:
	inline CamoProducer(const char *file = "@")
	{
		_console = (file[0] == '@');
		_fd = _console ? fileno(stdout) : creat(file, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if (_fd == -1)
		{
			printf("ERROR: Could not create %s\n\n",file);
			_fd = fileno(stdout);
			_console = true;
		}
		_prefix = (_console && file[1]) ? (file + 1) : NULL; // TODO: Refine args, Remove file output
	}
	
	inline ~CamoProducer()
	{
		if (!_console)
			close(_fd);
	}

public:
	void ProduceStrings(CamoVector &strings)
	{
		unsigned count = (unsigned)strings.size();
		for (unsigned i = 0; i < count; i++)
		{
			CamoItem &item = *strings[i];
			char name[MAX_SLUGIFY];
			unsigned length = slugify(name, item.symbol, item.length);

			write(fileno(stdout), item.type == CamoItemString ? "// " : "//@", 3);
			write(fileno(stdout), item.symbol, item.length);
			write(fileno(stdout), "\n", 1);

			write(fileno(stdout), "#define CAMO_", 13);
			write(fileno(stdout), name, length);
			write(fileno(stdout), " ", 1);
			
			if (item.type == CamoItemString)
			{
				write(fileno(stdout), "char camo_", 10);
				write(fileno(stdout), name, length);
				printf("[%d];", item.length + 1);

				write(fileno(stdout), " CamoStringDecode(camo_", 23);
				write(fileno(stdout), name, length);
				write(fileno(stdout), ", \"", 3);
			}
			else
			{
				write(fileno(stdout), " CFBridgingRelease(CamoStringDecode2(\"", 38);
			}

			for (unsigned k = 0; k < item.length; k++)
			{
				char c = item.symbol[k];
				if (c == '\\')
				{
					c = item.symbol[++k];
					if (c == 'r') c = '\r';
					else if (c == 'n') c = '\n';
					else if (c == 't') c = '\t';
				}
				c = (c ^ (unsigned char)item.length) - k;
				printf("\\x%02x", c);
			}
			printf("\", %d)", item.length);
			printf((item.type == CamoItemNSString) ? ")\n" : "\n");
				
		}
		printf("PRODUCE: %u strings\n\n", count);
	}

	//
	int ProduceSymbols(CamoVector &items, unsigned begin = 0)
	{
		//
		const static struct {const char type[12]; const char *desc;} _comments[] =
		{
			{"/* IGNO*///", "NSClassFromString/NSSelectorFromString Ignored Symbol"},
			{"/* METH */ ", "Method Symbol"},
			
			{"/* PROP */ ", "Property Symbol"},
			{"/* REAP */ ", "Readonly Property Symbol"},
			{"/* PSET */ ", "Property Setter Symbol"},
			{"/* PGET */ ", "Property Getter Symbol"},
			
			{"/* PROT */ ", "Protocol Symbol"},
			{"/* INTE */ ", "Interface Symbol"},
			{"/* IMPL */ ", "Implementation Symbol"},
		};
		
		for (unsigned i = 0; i < sizeof(_comments) / sizeof(_comments[0]); i++)
		{
			write(_fd, "//", 2);
			write(_fd, _comments[i].type + 2, 5);
			write(_fd, ": ", 2);
			write(_fd, _comments[i].desc, strlen(_comments[i].desc));
			write(_fd, "\n", 1);
		}
		write(_fd, "\n", 1);
		
#ifdef _SUPPORT_ALIGN
		unsigned align = (_console ? 0 : items.maxLength);
#define _ALIGN_ARG		, align
#define _ALIGN_PARAM	, unsigned align
#define _ALIGN_LOOP(n)	if (align) for (unsigned j = n; j <= align; j++) write(_fd, " ", 1); else write(_fd, " ", 1)
#else
#define _ALIGN_ARG
#define _ALIGN_PARAM
#define _ALIGN_LOOP(n)	write(_fd, " ", 1)
#endif

		size_t prefixLength = _prefix ? strlen(_prefix) : 0;

		//
		signed count = 0;
		srand((unsigned)time(NULL));
		unsigned end = (unsigned)items.size();
		for (unsigned i = begin; i < end; i++)
		{
			CamoItem &item = *items[i];
			write(_fd, _comments[item.type].type, sizeof(_comments[0].type) - 1);
			
			CamoItem &newItem = *NewItem(item, _prefix, prefixLength);
			ProduceItem(item, newItem _ALIGN_ARG);
			
			if (item.type == CamoItemProperty || item.type == CamoItemReadOnly)
			{
				count += 2;
				ProduceVarItem(item, newItem _ALIGN_ARG);
				if (item.type == CamoItemProperty)
				{
					count++;
					ProduceSetItem(item, newItem _ALIGN_ARG);
				}
			}
			else if (item.type != CamoItemIgnore)
			{
				count++;
			}
		}
		
		printf("PRODUCE: %u valid symbols\n\n", count);
		
		return count;
	}
	
private:
	//
	inline void ProduceItem(CamoItem &item, CamoItem &newItem _ALIGN_PARAM)
	{
		write(_fd, "#define ", sizeof("#define ") - 1);
		write(_fd, item.symbol, item.length);
		
		_ALIGN_LOOP(item.length);

		write(_fd, newItem.symbol, newItem.length);
		
		write(_fd, "\n", 1);
	}
	
	//
	inline void ProduceVarItem(CamoItem &item, CamoItem &newItem _ALIGN_PARAM)
	{
		write(_fd, "/* VARP */ ", sizeof("/* VARP */ ") - 1);
		write(_fd, "#define ", sizeof("#define ") - 1);
		write(_fd, "_", 1);
		write(_fd, item.symbol, item.length);
		
		_ALIGN_LOOP(item.length + 3);
		
		write(_fd, "_", 1);
		write(_fd, newItem.symbol, newItem.length);
		
		write(_fd, "\n", 1);
	}
	
	//
	inline void ProduceSetItem(CamoItem &item, CamoItem &newItem _ALIGN_PARAM)
	{
		write(_fd, "/* SETP */ ", sizeof("/* SETP */ ") - 1);
		write(_fd, "#define ", sizeof("#define ") - 1);
		write(_fd, "set", sizeof("set") - 1);
		char ch1 = toupper(item.symbol[0]);
		write(_fd, &ch1, 1);
		write(_fd, item.symbol + 1, item.length - 1);
		
		_ALIGN_LOOP(item.length + 3);
		
		write(_fd, "set", sizeof("set") - 1);
		char ch2 = toupper(newItem.symbol[0]);
		write(_fd, &ch2, 1);
		write(_fd, newItem.symbol + 1, newItem.length - 1);
		
		write(_fd, "\n", 1);
	}
	
	//
	CamoItem *NewItem(CamoItem &item, const char *prefix, size_t prefixLength)
	{
		char buffer[256];
		unsigned length = 8 + rand() % 16;
		const static char newSymbolChars[] = "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		while (true)
		{
			unsigned i = 0;
			if (!memcmp(item.symbol, "init", 4))
			{
				memcpy(buffer, "init", 4);
				i = 4;
			}

			if (prefix)
			{
				memcpy(buffer + i, prefix, prefixLength);
				i += prefixLength;

				memcpy(buffer + i, item.symbol, item.length);
				i += item.length;

				CamoItem *newItem = _newItems.PushItem(buffer, i);
				if (newItem)
				{
					return newItem;
				}
				prefix = NULL; // Fallback to random
			}
			else
			{
				buffer[i] = newSymbolChars[(i ? 27 : 0) + rand() % 27];	// To ensure lower case for property or upper case for init
				buffer[i + 1] = '0' + rand() % 10;	// To ensure digit char

				for (i += 2; i < length; i++)
				{
					buffer[i] = newSymbolChars[rand() % (sizeof(newSymbolChars) - 1)];
				}
			}

			CamoItem *newItem = _newItems.PushItem(buffer, length);
			if (newItem)
			{
				return newItem;
			}
		}
	}

	unsigned slugify(char *dst, const char *src, unsigned length)
	{
		signed j = 0;
		for (signed i = 0; (i < length && j < MAX_SLUGIFY - 1); i++)
		{
			char c = src[i];
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
			{
				dst[j++] = c;
			}
			else if (j && dst[j - 1] != '_')
			{
				dst[j++] = '_';
			}
		}
		if (j && dst[j - 1] == '_')
			j--;
		dst[j] = 0;
		return j;
	}
	
private:
	int _fd;
	bool _console;
	const char *_prefix;
	CamoVector _newItems;
};
