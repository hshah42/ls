#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>
#include "print.h"
#include "common.h"
#include "compare.h"

struct OPT
{
    int listAllFlag;
    int includeHiddenFiles;
    int printInode;
    int printStat;
    int recurse;
    int listDirectories;
    int sortBySizeDescending;
    int sortByLastModified;
};

struct output
{
    char **error;
    struct elements el[];
};

int readDir(char **files, struct OPT *options, int isDirnameRequired);
void setOptions(int argc, char **argv, struct OPT *options);
void initOptions(struct OPT *options);
int allocateFile(int maxSize, int argc, char **argv, char **files);
int generateElement(char *pathname, struct elements *el, struct OPT *options, FTSENT *ftsent);
int changeDirectory(char *pathname);
int performLs(FTS *fts, FTSENT *ftsent, struct OPT *options, int isDirnameRequired);
int shouldPrint(struct OPT *options, FTSENT *node);
int postChildTraversal(int *shouldPrintContent, FTS *fts, FTSENT *directory);
struct maxsize generateMaxSizeStruct(FTSENT *node, struct maxsize max);
int addLinkName(FTSENT *node, struct elements *el);
sort getSortType(struct OPT *options);