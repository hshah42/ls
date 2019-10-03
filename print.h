#include <sys/stat.h>
#include <stdio.h>
#include <time.h>

struct elements {
    char     *name;
    char     *strmode;
    char     *owner;
    char     *group;
    char     *symbolicLink;
    time_t   *time;
    nlink_t  *hardlinks;
    off_t    *size;
    ino_t    *inode;
    int      *hasSize;
};

struct maxsize
{
    long name;
    long owner;
    long group;
    long hardlinks;
    long size;
    long inode;
};


void printLine(struct elements el);
struct elements getDefaultStruct();
void print(char *directoryName, struct elements el[], int elementsCount);
void printFile(char **files, int *fileCount);
void printErrors(char **errors, int *errorCount);
void printError(char *error);
void printDirectory(char *directoryName);
void printNewLine();
void addToLine(struct elements el);
struct maxsize getDefaultMaxSizeStruct();