
#import "CamoStore.h"
#import <string.h>
#import <fcntl.h>
#import <dirent.h>
#import <unistd.h>
#import <sys/stat.h>

//
class CamoParser : public CamoStore
{
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
			
			char *code = (char *)malloc(stat.st_size + 16 /*Extra Buffer for Fault Tolerant*/);
			if (code)
			{
				memset(code + stat.st_size, 0, 16);
				read(fd, code, stat.st_size);
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
	
	// @interface @implementation @protocol
	const char *ParseObject(const char *code)
	{
		const char *p = code;
		p = ParseSolid(p);
		p = ParseBlank(p);
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
		fprintf(stderr, "BROKEN Object: %s\n", code);
		return p;
	}
	
	// +/-
	const char *ParseMethod(const char *code)
	{
		const char *p = code + 1;
		p = ParseUntil(p, '(');
		p = ParseBlock(p);
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
					PrintOut("Declaration:", code, p - code);
					return p + 1;
					
				case '{':
					if (first == symbol) ParseMethodSymbol(symbol);
					PrintOut("Implentation:", code, p - code);
					return ParseBlock(p);
					
				case ':':
					ParseMethodSymbol(symbol, first == symbol);
					p = ParseUntil(p, '(');
					p = ParseBlock(p);
					p = ParseBlank(p);
					p = ParseSolid(p);
					p = ParseBlank(p);
					symbol = p;
					break;
					
				default:
					p++;
					break;
			}
		}
		fprintf(stderr, "BROKEN Method: %s\n", code);
		return p;
	}
	
	// @property (...) BOOL ** aa;
	const char *ParseProperty(const char *code)
	{
		bool readonly = false;
		const char *p = code + sizeof("@propterty") - 1;
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
					p += sizeof("getter") - 1;
					p = ParseUntil(p, '=');
					p = ParseBlank(p + 1);
					p = ParseSymbol(p);
				}
				else
				{
					if (!memcmp(p, "readonly", sizeof("readonly") - 1))
					{
						readonly = true;
					}
					p++;
				}
			}
		}
		
		p = ParseSolid(p);
		while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' || *p == '*') p++;
		p = ParsePropertySymbol(p, !readonly);
		while (*p && *p++ != ';');
		return p;
	}
	
	//
	const char *ParseMethodSymbol(const char *code, bool checkSetter = true)
	{
		const char *p = ParseSymbol(code);
		if (checkSetter && !memcmp(code, "set", 3) && (p - code > 4) && (p - code < 250))
		{
			char property[250];
			property[0] = tolower(code[3]);
			memcpy(property + 1, code + 4, p - code - 3);
			ParseSymbol(property);
		}
		return p;
	}
	
	//
	const char *ParsePropertySymbol(const char *code, bool checkProperty = true)
	{
		const char *p = ParseSymbol(code);
		if (checkProperty && (p > code) && (p - code < 250))
		{
			char setter[256];
			memcpy(setter, "set", 3);
			setter[3] = toupper(*code);
			memcpy(setter + 4, code + 1, p - code);
			ParseSymbol(setter);
		}
		return p;
	}
	
	//
	const char *ParseSymbol(const char *code)
	{
		const char *p = code;
		while ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p == '_') || (*p == '$')) p++;
		//while (*p != 0 && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' && *p != '<' && *p != '{' && *p != ':' && *p != ';' *p != ')') p++;
		if (p > code) PushSymbol(code, p - code);
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
		//fprintf(stderr, "BROKEN Comment: %s\n", code);
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
		char end = (start == '(') ? ')' : (start + 2);
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
		fprintf(stderr, "BROKEN Block: %s\n", code);
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
	inline void PrintOut(const char *type, const char *code, size_t length)
	{
		//puts(type);
		//fwrite(code, length, 1, stdout);
		//puts("\n");
	}
};
