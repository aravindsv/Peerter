//General Idea is to have rules that specify which peers can access which files
//Things that need to be defined in rules: filename
//										   include or exclude
//										   network addresses
//										   alias
//										   port numbers

// for printf/file io?
#include <stdio.h>
// for symlink stuff: 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
// for strtok
#include <stdlib.h>
#include <string.h>
#include "acessSys.h"

//#filename
//#includeIP
//#excludeIP
//#includeAlias
//#excludeAlias
//#includePort
//#excludePort
//
// typedef int bool;
// enum { false, true };

#define INCLUDE true
#define EXCLUDE false
#define FILENAMESIZ 256
#define PEERLISTSIZ 1024



int rulebook_max_size;
int rulebook_size;

void parseFile(char *rule_file);

char* rule_file = ".ajaccess";

void parseFile(char* rule_file);

typedef enum peer_definition {		// Which type of connection is this?
	ALIAS,
	NETWORKADDR,
	PORT
} peerDef;

typedef struct rule_t {
	char *filename;
	peerDef type;
	bool include;
	char *peerList;
	char **peerArr; // TODO why double pointer? 
	int num_peers;
} Rule;

Rule** rulebook;

// use this when the p2p program starts, as this creates the rulebook. 
void init()
{
	rulebook_size = 0;
	rulebook_max_size = 25;
	rulebook = (Rule**)malloc(rulebook_max_size * sizeof(Rule*));
	for (int i = rulebook_size; i < rulebook_max_size; i++)
		rulebook[i] = NULL;

	parseFile(rule_file); // TODO insert the filename of the rule file here. (or make it inputted, idk lol). 
}

void rule_free()
{
	for (int i = 0; i < rulebook_size; i++)
	{
		free(rulebook[i]->filename);
		free(rulebook[i]->peerArr);
		free(rulebook[i]->peerList);
		free(rulebook[i]);
	}
	free(rulebook);
}

//General Idea for code to check if a peer should download or not
bool peerWillAccess(Rule myRule, char* peer) {
	int i;
	for (int i = 0; i < myRule.num_peers; i++)
	{
		if (myRule.peerArr[i] == NULL)
		{
			// this shouldn't happen, but since it did, we will just skip this. 
			continue;
		}
		if (strncmp(peer, myRule.peerArr[i], PEERLISTSIZ) == 0)
		{
			return myRule.include;
		}
	}
	return !myRule.include;
}

bool peerShouldAccess(char* peer, char *file) {
	int n = 0;
	for (n = 0; n < rulebook_size; n++) {
		if (rulebook[n] == NULL) {
			// error, this shouldn't be the case. 
			break;
		}
		if (strncmp(rulebook[n]->filename, file, FILENAMESIZ) == 0) {
			return peerWillAccess(*rulebook[n], peer);
		}
	}
	return true;
}

//General Idea for code to parse access file into rules
void addRule(char *fname, peerDef ty, bool incl, char* list, char** arr, size_t size)
{
	if (rulebook_size >= rulebook_max_size)
	{
		rulebook_max_size *= 2;
		rulebook = (Rule**)realloc(rulebook, rulebook_max_size * sizeof(Rule*));
		for (int i = rulebook_size; i < rulebook_max_size; i++)
			rulebook[i] = NULL;
	}
	rulebook[rulebook_size] = (Rule*)malloc(sizeof(Rule));
	rulebook[rulebook_size]->filename = fname;
	rulebook[rulebook_size]->type = ty;
	rulebook[rulebook_size]->include = incl;
	rulebook[rulebook_size]->peerList = list;
	rulebook[rulebook_size]->peerArr = arr;
	rulebook[rulebook_size]->num_peers = size;
	rulebook_size++;
}

// helper function which may or may not be useful (to be determined)
bool isSymlink(char *filename)
{
	struct stat *buf = NULL;
	int ret = lstat(filename, buf);

	return S_ISLNK(buf->st_mode);
}

size_t split(char* str, const char delim, char** result)
{
	size_t num_tokens = 0;
	
	int i;
	for (i = 0; str[i] != '\n' && str[i] != '\0'; i++)
	{
		if (delim == str[i])
		{
			num_tokens++;
		}
	}
	// allows a trailing , at the end. 
	//if (str[i-1] != delim)
	num_tokens++;

	result = (char**)malloc(num_tokens*sizeof(char*));
	int count = 0;

	result[count] = str;
	count++;
	for (i = 0; str[i] != '\n' && str[i] != '\0'; i++)
	{
		if (delim == str[i])
		{
			str[i] = '\0';
			result[count] = str + i + 1;
			count++;
		}
	}

	//for (i = 0; i < num_tokens; i++)
	//{
	//	char* token = strsep(&str, ',');
	//	if (token != NULL)
	//		result[i] = token;
	//	else
	//	{
	//		i--;
	//		num_tokens--;
	//	}
	//}

	return num_tokens;
}

void parseFile(char *rule_file)
{
	FILE *file; 		// file object. 
	char *line = NULL;	// eventually contains the line. (delimiter, if found + nullbyte included)
	size_t size = 0;	// when line is NULL, this can be any value, and will be changed to line's length. 
	ssize_t read; 		// return value is (num characters read, if read) or -1 (if EOF or error). 

	char *fname = (char*) malloc(FILENAMESIZ*sizeof(char));
	peerDef ty = NETWORKADDR;
	bool incl = false;
	char* peerList = (char*) malloc(PEERLISTSIZ*sizeof(char));
	char** peerArray = NULL;
	size_t num_peers = 0;

	file = fopen(rule_file, "r"); // opens filename with read-only permissions. 

	if (file == NULL)
	{
		printf("file not found. uh oh. :(");
		return;
	}


	read = getline(&line, &size, file); // stores line (includes null pointer) in line. 
	// allocates line for us. 

	int i = 0; 
	while (read != -1)
	{
		if (line[0] != '\n') // ignore newlines. 
		{
			if (strncmp(line, "#file", 5) != 0)
			{
				printf("%s is not a recognized rule delimiter\n", line); 
				break;
			}
			i += 5;
			if (line[i] != ' ' && line[i] != '\t')
			{
				printf("%s is not a recognized rule delimiter\n", line);
				break;
			}
			while (line[i] == ' ' || line[i] == '\t')
				i++;
			strncpy(fname, line + i, FILENAMESIZ);

			// get next non-empty line. 
			read = getline(&line, &size, file);
			printf("%s", line);
			if (read == -1)
			{
				// error. (no peers to add to the rule). (file ended before rule is completed. 
				break; // current solution, just ignore the file. (no rule added for it). 
			}

			while (line[0] == '\n') // ignore newlines. 
			{
				read = getline(&line, &size, file);
				if (read == -1)
				{
					// error. (no peers to add to the rule).
					break;
				}
			}
			if (read == -1)
			{
				// error. (no peers to add to the rule). (file ended before rule is completed. 
				break; // current solution, just ignore the file. (no rule added for it). 
			}

			i = 0;
			if (strncmp(line, "#include", 8) == 0)
			{
				i += 8;
				incl = true;
				if (line[i] == ' ' || line[i] == '\t')
				{
					// default to IP address. 
					peerDef ty = NETWORKADDR;
				}
				else if (strncmp(line + i, "IP", 2) == 0)
				{
					i += 2;
					peerDef ty = NETWORKADDR;
					if (line[i] != ' ' && line[i] != '\t')
					{
						printf("%s is not a recognized rule delimiter\n", line);
						break;
					}
				}
				else if (strncmp(line + i, "Alias", 5) == 0)
				{
					i += 5;
					peerDef ty = ALIAS;
					if (line[i] != ' ' && line[i] != '\t')
					{
						printf("%s is not a recognized rule delimiter\n", line);
						break;
					}
				}
				else if (strncmp(line + i, "Port", 4) == 0)
				{
					i += 4;
					peerDef ty = PORT;
					if (line[i] != ' ' && line[i] != '\t')
					{
						printf("%s is not a recognized rule delimiter\n", line);
						break;
					}
				}
				else
				{
					printf("%s is not a recognized rule delimiter\n", line);
					break;
				}
			}
			else if (strncmp(line, "#exclude", 8) == 0)
			{
				i += 8;
				incl = false;
				if (line[i] == ' ' || line[i] == '\t')
				{
					// default to IP address. 
					peerDef ty = NETWORKADDR;
				}
				else if (strncmp(line + i, "IP", 2) == 0)
				{
					i += 2;
					peerDef ty = NETWORKADDR;
					if (line[i] != ' ' && line[i] != '\t')
					{
						printf("%s is not a recognized rule delimiter\n", line);
						break;
					}
				}
				else if (strncmp(line + i, "Alias", 5) == 0)
				{
					i += 5;
					peerDef ty = ALIAS;
					if (line[i] != ' ' && line[i] != '\t')
					{
						printf("%s is not a recognized rule delimiter\n", line);
						break;
					}
				}
				else if (strncmp(line + i, "Port", 4) == 0)
				{
					i += 4;
					peerDef ty = PORT;
					if (line[i] != ' ' && line[i] != '\t')
					{
						printf("%s is not a recognized rule delimiter\n", line);
						break;
					}
				}
				else
				{
					printf("%s is not a recognized rule delimiter\n", line);
					break;
				}
			}
			else
			{
				printf("%s is not a recognized rule delimiter\n", line);
				break;
			}
			while (line[i] == ' ' || line[i] == '\t')
				i++;
			strncpy(peerList, line + i, PEERLISTSIZ);
			num_peers = split(peerList, ',', peerArray);
			// parse peers/include/exclude. (same syntax parsing stuff). 
				// figure out peerDef. 
				// set include = INCLUDE or EXCLUDE. 
				// add peers into a peer array. 
			addRule(fname, ty, incl, peerList, peerArray, num_peers);
		}

		read = getline(&line, &size, file); // stores line (includes null pointer) in line. 
	}

	free(line);
}


