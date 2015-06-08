//General Idea is to have rules that specify which peers can access which files
//Things that need to be defined in rules: filename
//										   include or exclude
//										   network addresses
//										   alias
//										   port numbers

#filename
#includeIP
#excludeIP
#includeAlias
#excludeAlias
#includePort
#excludePort

#define INCLUDE true
#define EXCLUDE false

typedef enum peer_definition {		// Which type of connection is this?
	ALIAS,
	NETWORKADDR,
	PORT
} peerDef;

typedef struct rule_t {
	char *filename
	peerDef type;
	bool include;
	int **peerArr;
} Rule;


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

bool peerShouldAccess(Rule *rulebook, int peer, char *file) {
	int n = 0;
	for (n = 0; n < MAX_RULES; n++) {
		if (rulebook[n] == NULL) {
			break;
		}
		if (strncmp(rulebook[n].filename, file, FILENAMESIZ) == 0) {
			return peerShouldAccess(rulebook[n], peer);
		}
	}
	return true;
}

//General Idea for code to parse access file into rules
