

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <dirent.h>

//
class SourceParser
{
public:
	//
	void ParseDir(const char *dir)
	{
		DIR *dp = opendir (dir);
		if (dp)
		{
			char path[2048];
			strcpy(path, dir);
			char *subpath = path + strlen(path);
			*subpath++ = '/';

			fprintf(stderr, "INFO Dir: %s\n", dir);
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
					else if (!memcmp(endpath - 2, ".m", 2) || !memcmp(endpath - 3, ".mm", 3))
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
		FILE *fp = fopen(file, "rb");
		if (fp)
		{
			fprintf(stderr, "INFO File: %s\n", file);
			
			fseek(fp, 0, SEEK_END);
			long size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			
			char *source = (char *)malloc(size + 2);
			if (source)
			{
				fread(source, size, 1, fp);
				source[size] = 0;
				source[size + 1] = 0;	//
				ParseSource(source);
				free(source);
			}
			else
			{
				fprintf(stderr, "ERROR Memory: %s\n", file);
			}
			fclose(fp);
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
		while (true)
		{
			if (*p == 0)
			{
				break;
			}
			else if (*p == '#')
			{
				p = ParsePreprocessor(p);
			}
			else if (*p == '\"')
			{
				p = ParseString(p);
			}
			else if (*p == '/')
			{
				if (p[1] == '/')
				{
					p = ParseComment(p);
				}
				else if (p[1] == '*')
				{
					p = ParseComments(p);
				}
				else
				{
					p++;
				}
			}
			else if (strcmp(p, "@interface") == 0)
			{
				p = ParseInterface(p);
			}
			else if (strcmp(p, "@protocol") == 0)
			{
				p = ParseInterface(p);
			}
			else if (strcmp(p, "@implementation") == 0)
			{
				p = ParseInterface(p);
			}
			else
			{
				p++;
			}
		}
	}
	
private:
	char *ParsePreprocessor(char *source)
	{
		char *p = source + 1;
		for (; *p; p++)
		{
			if ((*p == '\r' || *p == '\n') && (p[-1] != '\\'))
			{
				//puts("Preprocessor:"); fwrite(source, p + 1 - source, 1, stdout); puts("\n");
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
				//puts("String:"); fwrite(source, p + 1 - source, 1, stdout); puts("\n");
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
				//puts("Comment:"); fwrite(source, p + 1 - source, 1, stdout); puts("\n");
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
			if (*p == '*' || p[1] != '/')
			{
				//puts("Comments:"); fwrite(source, p + 2 - source, 1, stdout); puts("\n");
				return p + 2;
			}
		}
		fprintf(stderr, "BROKEN Comments: %s\n", source);
		return p;
	}
	
	//
	char *ParseInterface(char *s)
	{
		return s + 1;
	}
};

//
int main(int argc, char * argv[])
{
	SourceParser parser;
	parser.ParseDir("/Users/Yonsm/Documents/GitHub/Sample");
    return 0;
}

