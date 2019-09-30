#include "print.h"

const char *monthNames[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void
printLine(struct elements el) {
    struct tm *temp;

    if(el.inode > 0) {
        fprintf(stdout, "%llu ", *el.inode);
    }
    
    if(el.strmode != NULL) {
        fprintf(stdout, "%s ", el.strmode);
    }

    if(el.hardlinks > 0) {
        fprintf(stdout, "%ld ", *el.hardlinks);
    }

    if(el.owner != NULL) {
        fprintf(stdout, "%s ", el.owner);
    }

    if(el.group != NULL) {
        fprintf(stdout, "%s ", el.group);
    }

    if(el.size >= 0 && el.hasSize == 0) {
        fprintf(stdout, "%ld ", *el.size);
    }

    if(el.time > 0) {
        temp = localtime(el.time);
        fprintf(stdout, "%s %i %i:%i ", monthNames[temp->tm_mon], temp->tm_mday, temp->tm_hour, temp->tm_min);
    }

    if(el.name != NULL) {
        fprintf(stdout, "%s ", el.name);
    }

    fprintf(stdout, "\n");
}

void
printError(char *error) {
    fprintf(stderr, "%s\n", error);
}

struct elements
getDefaultStruct() {
    struct elements el;
    int false = 1;

    el.name = NULL;
    el.strmode = NULL;
    el.owner = NULL;
    el.group = NULL;
    el.symbolicLink = NULL;
    el.time = 0;
    el.hardlinks = 0;
    el.size = 0;
    el.inode = 0;
    el.hasSize = &false;

    return el;
}

void
printFile(char **files, int *fileCount) {
    int addLine = 0;
    for(int i = 0; i < *fileCount; i++) {
        fprintf(stdout, "%s \t", files[i]);
        addLine = 1;
    }
    if(addLine)
        printf("\n\n");
}

void
printDirectory(char *directoryName) {
    fprintf(stdout, "%s: \n", directoryName);
}

void 
printErrors(char **errors, int *errorCount) {
    for(int i = 0; i < *errorCount; i++) {
        fprintf(stdout, "%s \n", errors[i]);
    }
}

void 
print(char *directoryName, struct elements el[], int elementsCount) {
    fprintf(stdout, "%s: \n", directoryName);
 
    for(int i = 0; i < elementsCount; i++) {
        printLine(el[i]);
    }
}