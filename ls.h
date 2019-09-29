#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <fts.h>
#include "print.h"

struct OPT
{
    int listAllFlag;
    int printInode;
};

struct output
{
    char **error;
    struct elements el[];
};

int  readDir(char *pathname, struct OPT *options);
void setOptions(int argc, char **argv, struct OPT *options);
void initOptions(struct OPT *options);