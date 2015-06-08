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

typedef struct rule_t {
	
} Rule;