#include "common.h"
#include <ctype.h>

struct elements {
    char     *name;
    char     *strmode;
    char     *owner;
    char     *group;
    char     *symbolicLink;
    time_t   time;
    nlink_t  hardlinks;
    off_t    size;
    ino_t    inode;
    int      hasSize;
    int      useHumanReadable;
    int      showBlockSize;
    long     blockSize;
    blkcnt_t rawBlockSize;        
};

struct maxsize
{
    unsigned long name;
    unsigned long owner;
    unsigned long group;
    unsigned long hardlinks;
    unsigned long size;
    unsigned long inode;
    unsigned long blocksize;
};

void printLine(struct elements el, struct maxsize max);
struct elements getDefaultStruct();
void printFile(char **files, int *fileCount);
void printErrors(char **errors, int *errorCount);
void printError(char *error);
void printDirectory(char *directoryName);
void printNewLine();
void addToLine(struct elements el);
struct maxsize getDefaultMaxSizeStruct();
unsigned long getNumberOfDigits(long number);
void addWhiteSpaces(long number);
char* convertByteToHumanReadable(size_t bytes);
long convertToEnvironmentBlocksize(blkcnt_t blocks, long environmentBlocksize);
long getBlockSize();