

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

//
#define _ParseCommon(p)	*p == '#') p = ParsePreprocessor(p); else if (*p == '\"') p = ParseString(p); else if (*p == '/' && p[1] == '/') p = ParseComment(p); else if (*p == '/' && p[1] == '*') p = ParseComments(p
class Camo
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
			
			char *source = (char *)malloc(stat.st_size + 2);
			if (source)
			{
				read(fd, source, stat.st_size);
				source[stat.st_size] = 0;
				source[stat.st_size + 1] = 0;	//
				ParseSource(source);
				free(source);
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
	void ParseSource(char *source)
	{
		char *p = source;
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
	char *ParsePreprocessor(char *source)
	{
		char *p = source + 1;
		for (; *p; p++)
		{
			if ((*p == '\r' || *p == '\n') && (p[-1] != '\\'))
			{
				//PrintOut("Preprocessor:", source, p + 1 - source);
				return p + 1;
			}
		}
		return p;
	}
	
	//
	char *ParseString(char *source)
	{
		char *p = source + 1;
		for (; *p; p++)
		{
			if (*p == '\\')
			{
				p++;
			}
			else if (*p == '\"')
			{
				//PrintOut("String:", source, p + 1 - source);
				return p + 1;
			}
		}
		fprintf(stderr, "BROKEN String: %s\n", source);
		return p;
	}
	
	//
	char *ParseComment(char *source)
	{
		char *p = source + 2;
		for (; *p; p++)
		{
			if (*p == '\r' || *p == '\n')
			{
				//PrintOut("Comment:", source, p + 1 - source);
				return p + 1;
			}
		}
		fprintf(stderr, "BROKEN Comment: %s\n", source);
		return p;
	}
	
	//
	char *ParseComments(char *source)
	{
		char *p = source + 2;
		for (; *p; p++)
		{
			if (*p == '*' && p[1] == '/')
			{
				//PrintOut("Comments:", source, p + 2 - source);
				return p + 2;
			}
		}
		fprintf(stderr, "BROKEN Comments: %s\n", source);
		return p;
	}
	
	//
	char *ParseObject(char *source)
	{
		char *p = ParseNonSpace(source);
		char *name = ParseSpace(p);
		p = ParseNonSpace(name);
		PrintOut("Object:", source, p - source);
		
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
	char *ParseMetod(char *source)
	{
		char *p = ParseSpace(source + 1);
		while (*p)
		{
			if (*p == ';')
			{
				PrintOut("Declaration:", source, p - source);
				return p + 1;
			}
			else if (*p == '{')
			{
				PrintOut("Implentation:", source, p - source);
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
	char *ParseProperty(char *source)
	{
		char *p = ParseSpace(source + 1);
		while (*p)
		{
			if (*p == ';')
			{
				PrintOut("Property:", source, p - source);
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
	char *ParseBlock(char *source)
	{
		char *p = source + 1;
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
	inline char *ParseSpace(char *source)
	{
		while (*source == ' ' || *source == '\t' || *source == '\r' || *source == '\n') source++;
		return source;
	}
	
	//
	inline char *ParseNonSpace(char *source)
	{
		while (*source != ' ' && *source != '\t' && *source != '\r' && *source != '\n' && *source != '\0') source++;
		return source;
	}
	
	//
	inline void PrintOut(const char *type, char *source, size_t size)
	{
		puts(type);
		fwrite(source, size, 1, stdout);
		puts("\n");
	}
};

//
int main(int argc, char * argv[])
{
	Camo camo;
	//parser.ParseFile("/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS7.1.sdk/System/Library/Frameworks/Foundation.framework/Headers/NSRunLoop.h");
	camo.ParseDir("/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer");
	//parser.ParseDir("/Users/Yonsm/Documents/GitHub/Sample");
	return 0;
}

