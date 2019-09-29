#include <sys/stat.h>
#include <stdio.h>
#include <time.h>

struct elements {
    char   *name;
    char   *strmode;
    char   *owner;
    char   *group;
    char   *symbolicLink;
    time_t *time;
    long   *hardlinks;
    long   *size;
    ino_t  *inode;
    int    *hasSize;
};

const char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void printLine(struct elements el);
struct elements getDefaultStruct();
void print(char *directoryName, struct elements el[], int elementsCount);