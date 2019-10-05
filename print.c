#include "print.h"

const char *monthNames[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void
printLine(struct elements el, struct maxsize max) {
    struct tm *temp;

    if(*el.inode > 0) {
        long whitespaces = max.inode - getNumberOfDigits(*el.inode);
        addWhiteSpaces(whitespaces);
        fprintf(stdout, "%lu ", *el.inode);
    }
    
    if(el.strmode != NULL) {
        fprintf(stdout, "%s ", el.strmode);
    }

    if(*el.hardlinks > 0) {
        long whitespaces = max.hardlinks - getNumberOfDigits(*el.hardlinks);
        addWhiteSpaces(whitespaces);
        fprintf(stdout, "%hu ", *el.hardlinks);
    }

    if(el.owner != NULL) {
        long whitespaces = max.owner - strlen(el.owner);
        fprintf(stdout, "%s  ", el.owner);
        addWhiteSpaces(whitespaces);
    }

    if(el.group != NULL) {
        long whitespaces = max.owner - strlen(el.group);
        fprintf(stdout, "%s  ", el.group);
        addWhiteSpaces(whitespaces);
    }

    if(*el.size >= 0 && *(el.hasSize)) {
        long whitespaces = max.size - getNumberOfDigits(*el.size);
        addWhiteSpaces(whitespaces);
        fprintf(stdout, "%ld ", *el.size);
    }

    if(*el.time > 0) {
        temp = localtime(el.time);
        fprintf(stdout, "%s ", monthNames[temp->tm_mon]);
        long whitespaces = 2 - getNumberOfDigits(temp->tm_mday);
        addWhiteSpaces(whitespaces);
        fprintf(stdout, "%d ", temp->tm_mday);
        whitespaces = 2 - getNumberOfDigits(temp->tm_hour);
        if (getNumberOfDigits(temp->tm_hour) > 1) {
            addWhiteSpaces(whitespaces);
            fprintf(stdout, "%d:", temp->tm_hour);
        } else {
            addWhiteSpaces(whitespaces - 1);
            fprintf(stdout, "0%d:", temp->tm_hour);
        }
        if (getNumberOfDigits(temp->tm_min) > 1) {
            fprintf(stdout, "%d ", temp->tm_min);
        } else {
            fprintf(stdout, "0%d ", temp->tm_min);
        }
    }

    if(el.name != NULL) {
        int whitespaces = max.name - strlen(el.name);
        fprintf(stdout, "%s ", el.name);
        addWhiteSpaces(whitespaces);
    }

    fprintf(stdout, "\n");
}

void
addToLine(struct elements el) {
    fprintf(stdout, "%s \t", el.name);
}

void
printError(char *error) {
    fprintf(stderr, "%s\n", error);
}

struct elements
getDefaultStruct() {
    struct elements el;
    int defaultVal = 0;
    time_t defaultTime = 0;
    ino_t defaultInode = 0;
    nlink_t defaultHardlink = 0;
    off_t defaultSize = 0;

    el.name = NULL;
    el.strmode = NULL;
    el.owner = NULL;
    el.group = NULL;
    el.symbolicLink = NULL;
    el.time = &defaultTime;
    el.hardlinks = &defaultHardlink;
    el.size = &defaultSize;
    el.inode = &defaultInode;
    el.hasSize = &defaultVal;

    return el;
}

struct maxsize
getDefaultMaxSizeStruct() {
    struct maxsize max;

    max.name = 0;
    max.inode = 0;
    max.hardlinks = 0;
    max.owner = 0;
    max.group = 0;
    max.size = 0;
    
    return max;
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
        printError(errors[i]);
    }
}

void
printNewLine() {
    fprintf(stdout, "\n");
}

void
addWhiteSpaces(long number) {
    for(int i = 0; i < number; i++) {
        fprintf(stdout, " ");
    }
}

unsigned long
getNumberOfDigits(long number) {
    if(number == 0) {
        return 1;
    }

    unsigned long count = 0;

    while(number != 0) {
        number = number / 10;
        count++;
    }

    return count;
}