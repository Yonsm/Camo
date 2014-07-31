
#import "CamoVector.h"
#import <string.h>
#import <fcntl.h>
#import <dirent.h>
#import <unistd.h>
#import <sys/stat.h>

#ifdef _SUPPORT_UTF16
#import <string>
#import <codecvt>
#include <fstream>
#endif

//
class CamoParser
{
public:
	CamoVector symbols;
	
public:
	//
	void Parse(const char *arg)
	{
		if (*arg == '@')
		{
			ParseCode(arg);
		}
		else
		{
			struct stat st;
			stat(arg, &st);
			if (S_ISDIR(st.st_mode))
			{
				ParseDir(arg);
			}
			else
			{
				ParseFile(arg);
			}
		}
	}
	
private:
	//
	void ParseDir(const char *dir)
	{
		DIR *dp = opendir(dir);
		if (dp)
		{
			char path[2048];
			strcpy(path, dir);
			char *subpath = path + strlen(path);
			*subpath++ = '/';
			
			//printf("PARSE Dir: %s\n", dir);
			struct dirent *ent;
			while ((ent = readdir(dp)))
			{
				if (ent->d_name[0] != '.')
				{
					strcpy(subpath, ent->d_name);
					char *endpath = subpath + strlen(subpath);
					if (ent->d_type == DT_DIR)
					{
						ParseDir(path);
					}
					else if (!memcmp(endpath - 2, ".h", 2) || !memcmp(endpath - 2, ".m", 2) || !memcmp(endpath - 3, ".mm", 3))
					{
						ParseFile(path);
					}
				}
			}
			closedir(dp);
		}
		else
		{
			printf("ERROR: Could not open dir %s\n", dir);
		}
	}
	
	//
	void ParseFile(const char *file)
	{
		int fd = open(file, O_RDONLY);
		if (fd != -1)
		{
			printf("PARSING: %s\n", file);
			struct stat stat;
			fstat(fd, &stat);
			
			char *code = (char *)malloc(stat.st_size + 16 /*Extra Buffer for Fault Tolerant*/);
			if (code)
			{
				memset(code + stat.st_size, 0, 16);
				read(fd, code, stat.st_size);
#ifdef _SUPPORT_UTF16
				if ((code[0] == '\xFF' && code[1] == '\xFE') || (stat.st_size > 2 && code[1] == 0))
				{
					std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> convert;
					std::u16string u16str = (char16_t *)code;
					std::string u8str = convert.to_bytes(u16str);
					ParseCode(u8str.c_str());
				}
				else if ((code[0] == '\xFE' && code[1] == '\xFF') || (stat.st_size > 2 && code[0] == 0))
				{
					//TODO: Shit! How to convert UTF-16 BE to UTF-8?
				}
				else
#endif
				{
					ParseCode(code);
				}
				free(code);
			}
			else
			{
				printf("ERROR: Could not allocate memory for %s\n", file);
			}
			close(fd);
		}
		else
		{
			printf("ERROR: Clould not open file %s\n", file);
		}
	}
	
	//
	void ParseCode(const char *code)
	{
		const char *p = code;
		while (*p)
		{
			if (const char *q = ParseCommon(p))
			{
				p = q;
			}
			else if (*p == '{')
			{
				p = ParseBlock(p);
			}
			else if (!memcmp(p, "@interface", sizeof("@interface") - 1))
			{
				p = ParseObject(p, CamoItemInterface);
			}
			else if (!memcmp(p, "@protocol", sizeof("@protocol") - 1))
			{
				p = ParseObject(p, CamoItemProtocol);
			}
			else if (!memcmp(p, "@implementation", sizeof("@implementation") - 1))
			{
				p = ParseObject(p, CamoItemImplementation);
			}
			else
			{
				p++;
			}
		}
	}
	
	// @interface @implementation @protocol
	const char *ParseObject(const char *code, CamoItemType type)
	{
		const char *p = code;
		p = ParseSolid(p);
		p = ParseBlank(p);
		p = ParseSymbol(p, type);
		PrintOut("OBJECT:", code, p);
		
		while (*p)
		{
			if (const char *q = ParseCommon(p))
			{
				p = q;
			}
			else if (*p == '{')
			{
				p = ParseBlock(p);
			}
			else if (*p == '-' || *p == '+')
			{
				p = ParseMethod(p);
			}
			else if (!memcmp(p, "@property", sizeof("@property") - 1))
			{
				p = ParseProperty(p);
			}
			else if (!memcmp(p, "@end", sizeof("@end") - 1))
			{
				return p + 4;
			}
			else
			{
				p++;
			}
		}
		printf("BROKEN: Object %s\n", code);
		return p;
	}
	
	// +/-
	const char *ParseMethod(const char *code)
	{
		const char *p = code + 1;
		p = ParseUntil(p, '\(');
		p = ParseBlock<'('>(p);
		p = ParseBlank(p);
		const char *symbol = p;
		const char *first = symbol;
		while (*p)
		{
			if (const char *q = ParseCommon(p))
			{
				p = q;
			}
			else switch (*p)
			{
				case ';':
					if (first == symbol) ParseMethodSymbol(symbol);
					PrintOut("DECLARATION:", code, p);
					return p + 1;
					
				case '{':
					if (first == symbol) ParseMethodSymbol(symbol);
					PrintOut("IMPLEMENTATION:", code, p);
					return ParseBlock(p);
					
				case ':':
					ParseMethodSymbol(symbol, first == symbol);
					p = ParseBlank(p + 1);
					if (*p == '(')
					{
						p = ParseBlock<'('>(p);
						p = ParseBlank(p);
					}
					p = ParseSolid(p);
					p = ParseBlank(p);
					symbol = p;
					break;
					
				default:
					p++;
					break;
			}
		}
		printf("BROKEN: Method %s\n", code);
		return p;
	}
	
	// @property (...) BOOL ** aa;
	const char *ParseProperty(const char *code)
	{
		CamoItemType type = CamoItemProperty;
		const char *p = code + sizeof("@property") - 1;
		p = ParseBlank(p);
		if (*p == '(')
		{
			p++;
			while (*p)
			{
				if (*p == ')')
				{
					p = ParseBlank(p + 1);
					break;
				}
				else if (!memcmp(p, "getter", sizeof("getter") - 1) || !memcmp(p, "setter", sizeof("setter") - 1))
				{
					CamoItemType type = (*p == 'g') ? CamoItemGetter : CamoItemSetter;
					p += sizeof("getter") - 1;
					p = ParseUntil(p, '=');
					p = ParseBlank(p + 1);
					p = ParseSymbol(p, type);
				}
				else
				{
					if (!memcmp(p, "readonly", sizeof("readonly") - 1))
					{
						type = CamoItemReadOnly;
					}
					p++;
				}
			}
		}
		
		p = ParseSolid(p);
		while (*p)
		{
			if (const char *q = ParseCommon(p))
			{
				p = q;
			}
			else switch (*p)
			{
				case '(':
					p = ParseBlock<'('>(p);
					break;

				case '<':
					p = ParseBlock<'<'>(p);
					break;

				case '{':
					p = ParseBlock(p);
					break;
					
				case ' ':
				case '\t':
				case '\r':
				case '\n':
				case '*':
				case '&':
					p++;
					break;
					
				default:
					p = ParseSymbol(p, type);
					while (*p && *p++ != ';');
					return p;
			}
		}
		return p;
	}
	
	//
	inline const char *ParseMethodSymbol(const char *code, bool checkSetter = true)
	{
		if (checkSetter && !memcmp(code, "set", 3))
		{
			char ch = code[3];
			if (ch >= 'A' && ch <= 'Z')
			{
				((char *)code)[3] = tolower(ch);
				const char *p = ParseSymbol(code + 3, CamoItemProperty);
				((char *)code)[3] = ch;
				return p;
			}
		}
		return ParseSymbol(code, CamoItemMethod);
	}
	
	//
	const char *ParseSymbol(const char *code, CamoItemType type/* = CamoItemNormal*/)
	{
		const char *p = code;
		while ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p == '_') || (*p == '$')) p++;
		symbols.PushSymbol(code, unsigned(p - code), type);
		return p;
	}
	
private:
	//
	inline const char *ParseCommon(const char *code)
	{
		if (*code == '#')
		{
			return ParsePreprocessor(code);
		}
		else if (*code == '\'' || *code == '\"')
		{
			return ParseString(code);
		}
		else if (*code == '/' && code[1] == '/')
		{
			return ParseComment(code);
		}
		else if (*code == '/' && code[1] == '*')
		{
			return ParseComments(code);
		}
		else if (*code == '\\')
		{
			return code + 2;
		}
		return NULL;
	}
	
	//
	const char *ParsePreprocessor(const char *code)
	{
		const char *p = code + 1;
		for (; *p; p++)
		{
			if ((*p == '\r' || *p == '\n') && (p[-1] != '\\'))
			{
				PrintOut("PREPROCESSOR:", code, p + 1);
				return p + 1;
			}
		}
		return p;
	}
	
	// ' or " block
	const char *ParseString(const char *code)
	{
		char ch = *code;
		const char *p = code + 1;
		for (; *p; p++)
		{
			if (*p == '\\')
			{
				p++;
			}
			else if (*p == ch)
			{
				PrintOut("STRING:", code, p + 1);
				return p + 1;
			}
		}
		printf("BROKEN: String %s\n", code);
		return p;
	}
	
	//
	const char *ParseComment(const char *code)
	{
		const char *p = code + 2;
		for (; *p; p++)
		{
			if (*p == '\r' || *p == '\n')
			{
				PrintOut("COMMENT:", code, p + 1);
				return p + 1;
			}
		}
		//printf("BROKEN: Comment %s\n", code);
		return p;
	}
	
	//
	const char *ParseComments(const char *code)
	{
		const char *p = code + 2;
		for (; *p; p++)
		{
			if (*p == '*' && p[1] == '/')
			{
				PrintOut("COMMENTS:", code, p + 2);
				return p + 2;
			}
		}
		printf("BROKEN: Comments %s\n", code);
		return p;
	}
	
	//  Nest block () [] {} <>
	template <char STARTCHAR = '{'> const char *ParseBlock(const char *code)
	{
		//		char STARTCHAR = *code;
		char ENDCHAR = (STARTCHAR == '(') ? ')' : (STARTCHAR + 2);
		const char *p = code + 1;
		while (*p)
		{
			if (const char *q = ParseCommon(p))
			{
				p = q;
			}
			else if (*p == ENDCHAR)
			{
				return p + 1;
			}
			else if (*p == STARTCHAR)
			{
				p = ParseBlock<STARTCHAR>(p);
			}
			else
			{
				if (STARTCHAR == '{')	// Should be omitted by compiler optimization
				{
					p = ParseIgnore(p);
				}
				p++;
			}
		}
		printf("BROKEN: Block %s\n", code);
		return p;
	}
	
	//
	const char *ParseIgnore(const char *code)
	{
		const char *p = code;
		if (!memcmp(p, "NSClassFromString", sizeof("NSClassFromString") - 1) ||
			!memcmp(p, "NSSelectorFromString", sizeof("NSSelectorFromString") - 1))
		{
			p += ((p[2] == 'C') ? (sizeof("NSClassFromString") - 1) : (sizeof("NSSelectorFromString") - 1));
			while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' || *p == '(' || *p == '@') p++;
			if (*p == '"')
			{
				for (const char *symbol = ++p; *p; p++)	// Multiple arguments selector
				{
					if (*p == ':')
					{
						if (!memcmp(symbol, "set", 3))	// Ignore setter
						{
							char ch = symbol[3];
							if (ch >= 'A' && ch <= 'Z')
							{
								((char *)symbol)[3] = tolower(ch);
								symbols.PushSymbol(symbol + 3, unsigned(p - symbol - 3), CamoItemIgnore);
								((char *)symbol)[3] = ch;
							}
						}
						symbols.PushSymbol(symbol, unsigned(p - symbol), CamoItemIgnore);
						symbol = p + 1;
					}
					else if (*p == '"')
					{
						symbols.PushSymbol(symbol, unsigned(p - symbol), CamoItemIgnore);
						p++;
						break;
					}
				}
			}
		}
		return p;
	}
	
private:
	//
	inline const char *ParseUntil(const char *p, char c)
	{
		while (*p != c)
			if (*p)
				p++;
			else
				return p;
		return p;
	}
	
	//
	inline const char *ParseBlank(const char *p)
	{
		while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
		return p;
	}
	
	//
	inline const char *ParseSolid(const char *p)
	{
		while (*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' && *p != ';' && *p != '{') if (*p) p++; else return p;
		return p;
	}
	
private:
	//
	inline void PrintOut(const char *type, const char *code, const char *end)
	{
		//puts(type); fwrite(code, end - code, 1, stdout); puts("\n");
	}
};
