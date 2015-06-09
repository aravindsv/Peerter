//General Idea is to have rules that specify which peers can access which files
//Things that need to be defined in rules: filename
//										   include or exclude
//										   network addresses
//										   alias
//										   port numbers

// for printf/file io?
#include <stdio>
// for symlink stuff: 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#filename
#includeIP
#excludeIP
#includeAlias
#excludeAlias
#includePort
#excludePort

#define INCLUDE true
#define EXCLUDE false

Rule** rulebook;
int rulebook_max_size;
int rulebook_size;

typedef enum peer_definition {		// Which type of connection is this?
	ALIAS,
	NETWORKADDR,
	PORT
} peerDef;

typedef struct rule_t {
	char *filename
	peerDef type;
	bool include;
	int **peerArr; // TODO why double pointer? 
} Rule;

// use this when the p2p program starts, as this creates the rulebook. 
void init()
{
	rulebook_size = 0;
	rulebook_max_size = 25;
	rulebook = (Rule**)malloc(rulebook_max_size * sizeof(Rule*));
	for (int i = rulebook_size; i < rulebook_max_size; i++)
		rulebook[i] = NULL;

	parsefile(""); // TODO insert the filename of the rule file here. (or make it inputted, idk lol). 
}

//General Idea for code to check if a peer should download or not
bool peerShouldAccess(Rule myRule, int peer) {
	int i;
	if (myRule.include == INCLUDE) {
		for (i = 0; i < MAX_PEERS; i++) {
			if (peerArr[i] == NULL) {
				break;
			}
			else if (peerArr[i] == peer) {
				return true;
			}
		}
		return false;
	}
	else if (myRule.include == EXCLUDE) {
		for (i = 0; i < MAX_PEERS; i++) {
			if (peerArr[i] == NULL) {
				break;
			}
			else if (peerArr[i] == peer) {
				return false;
			}
		}
		return true;
	}
}

bool peerShouldAccess(int peer, char *file) {
	int n = 0;
	for (n = 0; n < rulebook_size; n++) {
		if (rulebook[n] == NULL) {
			// error, this shouldn't be the case. 
			break;
		}
		if (strncmp(rulebook[n].filename, file, FILENAMESIZ) == 0) {
			return peerShouldAccess(rulebook[n], peer);
		}
	}
	return true;
}

//General Idea for code to parse access file into rules
void addRule(char *fname, peerDef ty, bool incl, int** arr)
{
	if (rulebook_size >= rulebook_max_size)
	{
		rulebook_max_size *= 2;
		rulebook = (Rule**)realloc(rulebook, rulebook_max_size * sizeof(Rule*));
		for (int i = rulebook_size; i < rulebook_max_size; i++)
			rulebook[i] = NULL;
	}
	rulebook[rulebook_size] = (Rule*)malloc(sizeof(Rule));
	rulebook[rulebook_size].filename = fname;
	rulebook[rulebook_size].type = ty;
	rulebook[rulebook_size].include = incl;
	rulebook[rulebook_size].peerArray = arr;
	rulebook_size++;
}

// helper function which may or may not be useful (to be determined)
bool isSymlink(char *filename)
{
	struct stat *buf = NULL;
	int ret = lstat(filename, buf);

	return S_ISLNK(buf->st_mode);
}

void parseFile(char *rule_file)
{
	FILE *file; 		// file object. 
	char *line = NULL;	// eventually contains the line. (delimiter, if found + nullbyte included)
	size_t size = 0;	// when line is NULL, this can be any value, and will be changed to line's length. 
	ssize_t read; 		// return value is (num characters read, if read) or -1 (if EOF or error). 

	char *fname;
	peerDef ty;
	bool incl;
	int** peerArray;

	file = fopen(rule_file, "r"); // opens filename with read-only permissions. 

	if (file == NULL)
		// insert file not found error here. 
		return;

	read = getline(&line, &size, file); // stores line (includes null pointer) in line. 
	// allocates line for us. 

	while (read != -1)
	{
		printf("%s", line); // should print line. for testing purposes. 
		if (line[0] != '\n') // ignore newlines. 
		{
			// if match syntax. 
				// parse file line. 
					// get file name.
			// else
				// return some cannot read file error.

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

			// parse peers/include/exclude. (same syntax parsing stuff). 
				// figure out peerDef. 
				// set include = INCLUDE or EXCLUDE. 
				// add peers into a peer array. 
			addRule(fname, ty, incl, peerArray);
		}

		read = getline(&line, &size, file); // stores line (includes null pointer) in line. 
	}

	free(line);
}
