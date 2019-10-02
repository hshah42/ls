#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <fts.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>
#include "print.h"

struct OPT
{
    int listAllFlag;
    int includeHiddenFiles;
    int printInode;
    int printStat;
    int recurse;
};

struct output
{
    char **error;
    struct elements el[];
};

int readDir(char **files, struct OPT *options, int isDirnameRequired);void setOptions(int argc, char **argv, struct OPT *options);
void initOptions(struct OPT *options);
void allocateFile(int maxSize, int argc, char **argv, char **files);
int generateElement(char *pathname, struct elements *el, struct OPT *options, FTSENT *ftsent);
int changeDirectory(char *pathname);