
#include "CamoStore.h"
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

//
class CamoParser
{
public:
	CamoStore _store;
	
public:
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
			
			//fprintf(stderr, "INFO Dir: %s\n", dir);
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
			fprintf(stderr, "ERROR Dir: %s\n", dir);
		}
	}
	
	//
	void ParseFile(const char *file)
	{
		int fd = open(file, O_RDONLY);
		if (fd)
		{
			fprintf(stderr, "INFO File: %s\n", file);
			struct stat stat;
			fstat(fd, &stat);
			
			char *code = (char *)malloc(stat.st_size + 2);
			if (code)
			{
				read(fd, code, stat.st_size);
				code[stat.st_size] = 0;
				code[stat.st_size + 1] = 0;	//
				ParseCode(code);
				free(code);
			}
			else
			{
				fprintf(stderr, "ERROR Memory: %s\n", file);
			}
			close(fd);
		}
		else
		{
			fprintf(stderr, "ERROR File: %s\n", file);
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
			else if (!memcmp(p, "@interface", sizeof("@interface") - 1))
			{
				p = ParseObject(p);
			}
			else if (!memcmp(p, "@protocol", sizeof("@protocol") - 1))
			{
				p = ParseObject(p);
			}
			else if (!memcmp(p, "@implementation", sizeof("@implementation") - 1))
			{
				p = ParseObject(p);
			}
			else
			{
				p++;
			}
		}
	}
	
private:
	//
	const char *ParseObject(const char *code)
	{
		const char *p = code;
		p = ParseSolid(p);
		p = ParseSpace(p);
		p = ParseSymbol(p);
		PrintOut("Object:", code, p - code);
		
		while (*p)
		{
			if (const char *q = ParseCommon(p))
			{
				p = q;
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
		
		return p;
	}
	
	//
	const char *ParseMethod(const char *code)
	{
		const char *p = code + 1;
		p = ParseUntil(p, '(');
		p = ParseBlock(p);
		p = ParseSpace(p);
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
					if (first == symbol) ParseSymbol(symbol);
					PrintOut("Declaration:", code, p - code);
					return p + 1;
					
				case '{':
					if (first == symbol) ParseSymbol(symbol);
					PrintOut("Implentation:", code, p - code);
					return ParseBlock(p);
					
				case ':':
					ParseSymbol(symbol);
					p = ParseUntil(p, '(');
					p = ParseBlock(p);
					p = ParseSpace(p);
					p = ParseSolid(p);
					p = ParseSpace(p);
					symbol = p;
					break;
					
				default:
					p++;
					break;
			}
		}
		
		return p;
	}
	
	//TODO:
	const char *ParseProperty(const char *code)
	{
		const char *p = code + 1;
		p = ParseSpace(p);
		while (*p)
		{
			if (*p == ';')
			{
				PrintOut("Property:", code, p - code);
				return p + 1;
			}
			else
			{
				p++;
			}
		}
		return p;
	}
	
	//
	const char *ParseSymbol(const char *code)
	{
		const char *p = code;
		while ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p == '_') || (*p == '$')) p++;
		//while (*p != 0 && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' && *p != '<' && *p != '{' && *p != ':' && *p != ';') p++;
		if (p > code) _store.PushSymbol(code, p - code);
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
				//PrintOut("Preprocessor:", code, p + 1 - code);
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
				//PrintOut("String:", code, p + 1 - code);
				return p + 1;
			}
		}
		fprintf(stderr, "BROKEN String: %s\n", code);
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
				//PrintOut("Comment:", code, p + 1 - code);
				return p + 1;
			}
		}
		fprintf(stderr, "BROKEN Comment: %s\n", code);
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
				//PrintOut("Comments:", code, p + 2 - code);
				return p + 2;
			}
		}
		fprintf(stderr, "BROKEN Comments: %s\n", code);
		return p;
	}
	
	//  Nest block () [] {}
	const char *ParseBlock(const char *code)
	{
		char start = *code;
		char end = (start == '(') ? ')' : (start + 1);
		const char *p = code + 1;
		while (*p)
		{
			if (const char *q = ParseCommon(p))
			{
				p = q;
			}
			else if (*p == start)
			{
				p = ParseBlock(p);
			}
			else if (*p == end)
			{
				return p + 1;
			}
			else
			{
				p++;
			}
		}
		return p;
	}
	
private:
	//
	inline const char *ParseUntil(const char *p, char c)
	{
		while (*p != c) if (*p) p++; else return p;
		return p;
	}
	
	//
	inline const char *ParseSpace(const char *p)
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
	inline void PrintOut(const char *type, const char *code, size_t size)
	{
		//puts(type);
		//fwrite(code, size, 1, stdout);
		//puts("\n");
	}
};
