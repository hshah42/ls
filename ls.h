#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>
#include "print.h"
#include "common.h"
#include <ctype.h>
#include "compare.h"

struct OPT
{
    int listAllFlag;
    int includeHiddenFiles;
    int printInode;
    int printStat;
    int printOwnerAndGroupNames;
    int printOwnerAndGroupID;
    int recurse;
    int listDirectories;
    int sortBySizeDescending;
    int sortByLastModified;
    int useFileStatusChangeTime;
    int useLastAccessTime;
    int keepUnsorted;
    int appendFileType;
    int reverseOrder;
    int replaceNonPrintables;
    int printNonPrintables;
    int isHumanReadableSize;
    long blocksize;
    int printBlockSize;
    int printBlockSizeInK;
};

int readDir(char **files, struct OPT *options, int isDirnameRequired, int onFiles);
int setOptions(int argc, char **argv, struct OPT *options);
void initOptions(struct OPT *options);
int allocateFile(int maxSize, int argc, char **argv, struct OPT *options, char **files);
int generateElement(struct elements *el, struct OPT *options, FTSENT *ftsent);
int performLs(FTS *fts, struct OPT *options, int isDirnameRequired);
int shouldPrint(struct OPT *options, FTSENT *node);
void postChildTraversal(int *shouldPrintContent);
struct maxsize generateMaxSizeStruct(FTSENT *node, struct OPT *options, struct maxsize max);
int addLinkName(FTSENT *node, struct elements *el);
void resetSortOptions(struct OPT *options);
sort getSortType(struct OPT *options);
int printInformation(struct OPT *options, FTSENT *node, struct maxsize max);
void appendType(FTSENT *node, struct elements *el);
void checkPrintableCharacters(struct elements *el);
int preformLsOnfiles (FTS *fts, struct OPT *options);
FTSENT deepCopyFTSENT(FTSENT *fts, int includeParent);
void checkBlockSize(struct OPT *options);
int printErrorIfAny(FTSENT *ftsent);
struct maxsize traverseChildren(FTS *fts, FTSENT *node, int shouldPrintContent, struct OPT *options, struct maxsize max);