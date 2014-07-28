
#include "CamoStore.h"
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

//
#define _ParseCommon(p)	*p == '#') p = ParsePreprocessor(p); else if (*p == '\"') p = ParseString(p); else if (*p == '/' && p[1] == '/') p = ParseComment(p); else if (*p == '/' && p[1] == '*') p = ParseComments(p
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
			//fprintf(stderr, "INFO File: %s\n", file);
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
			if (_ParseCommon(p));
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
	
	//
	const char *ParseString(const char *code)
	{
		const char *p = code + 1;
		for (; *p; p++)
		{
			if (*p == '\\')
			{
				p++;
			}
			else if (*p == '\"')
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
	
	//
	const char *ParseObject(const char *code)
	{
		const char *p = ParseNonSpace(code);
		const char *name = ParseSpace(p);
		p = ParseNonSpace(name);
		PrintOut("Object:", code, p - code);
		_store.PushSymbol(code, p - code);
		
		while (*p)
		{
			if (_ParseCommon(p));
			else if (*p == '-' || *p == '+')
			{
				p = ParseMetod(p);
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
	const char *ParseMetod(const char *code)
	{
		const char *p = ParseSpace(code + 1);
		while (*p)
		{
			if (*p == ';')
			{
				PrintOut("Declaration:", code, p - code);
				return p + 1;
			}
			else if (*p == '{')
			{
				PrintOut("Implentation:", code, p - code);
				return ParseBlock(p);
			}
			else
			{
				p++;
			}
		}
		return p;
	}
	
	//
	const char *ParseProperty(const char *code)
	{
		const char *p = ParseSpace(code + 1);
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
	const char *ParseBlock(const char *code)
	{
		const char *p = code + 1;
		while (*p)
		{
			if (_ParseCommon(p));
			else if (*p == '{')
			{
				p = ParseBlock(p);
			}
			else if (*p == '}')
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
	
	//
	inline const char *ParseSpace(const char *code)
	{
		while (*code == ' ' || *code == '\t' || *code == '\r' || *code == '\n') code++;
		return code;
	}
	
	//
	inline const char *ParseNonSpace(const char *code)
	{
		while (*code != ' ' && *code != '\t' && *code != '\r' && *code != '\n' && *code != '\0') code++;
		return code;
	}
	
	//
	inline void PrintOut(const char *type, const char *code, size_t size)
	{
		puts(type);
		fwrite(code, size, 1, stdout);
		puts("\n");
	}
};
