#include "print.h"

const char *monthNames[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void
printLine(struct elements el, struct maxsize max) {
    struct tm *temp;

    if (el.inode > 0) {
        long whitespaces = max.inode - getNumberOfDigits(el.inode);
        addWhiteSpaces(whitespaces);
        fprintf(stdout, "%lu ", el.inode);
    }

    if (el.blockSize >= 0 && el.showBlockSize) {
        if (el.useHumanReadable) {
            char *print = convertByteToHumanReadable(el.rawBlockSize);
            long whitespaces = 4 - strlen(print);
            addWhiteSpaces(whitespaces);
            fprintf(stdout, "%s ", print);
        } else {
            long whitespaces = max.blocksize - getNumberOfDigits(el.blockSize);
            addWhiteSpaces(whitespaces);
            fprintf(stdout, "%li ", el.blockSize);
        }
    }
    
    if (el.strmode != NULL) {
        fprintf(stdout, "%s ", el.strmode);
    }

    if (el.hardlinks > 0) {
        long whitespaces = max.hardlinks - getNumberOfDigits(el.hardlinks);
        addWhiteSpaces(whitespaces);
        fprintf(stdout, "%hu ", el.hardlinks);
    }

    if (el.owner != NULL) {
        long whitespaces = max.owner - strlen(el.owner);
        fprintf(stdout, "%s  ", el.owner);
        addWhiteSpaces(whitespaces);
    }

    if (el.group != NULL) {
        long whitespaces = max.group - strlen(el.group);
        fprintf(stdout, "%s  ", el.group);
        addWhiteSpaces(whitespaces);
    }

    if (el.size >= 0 && (el.hasSize)) {
        if (el.major >= 0 && el.minor >=0) {
            long totalLenght = 2 + getNumberOfDigits(el.major) + max.minor;
            long whitespaces = max.size - totalLenght;
            addWhiteSpaces(whitespaces);
            fprintf(stdout, "%i, ", el.major);
            long minorWhitspaces = max.minor - getNumberOfDigits(el.minor);
            addWhiteSpaces(minorWhitspaces);
            fprintf(stdout, "%i ", el.minor);
        } else {
            if (el.useHumanReadable) {
                char *print = convertByteToHumanReadable(el.size);
                long whitespaces = max.size - strlen(print);
                addWhiteSpaces(whitespaces);
                fprintf(stdout, "%s ", print);
            } else {
                long whitespaces = max.size - getNumberOfDigits(el.size);
                addWhiteSpaces(whitespaces);
                fprintf(stdout, "%ld ", el.size);
            }
        }
    }

    if (el.time > 0) {
        temp = localtime(&(el.time));
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

    if (el.name != NULL) {
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
    fprintf(stderr, "%s: %s\n", getprogname(), error);
}

struct elements
getDefaultStruct() {
    struct elements el;
    int defaultVal = 0;
    long defaultLong = 0;

    time_t defaultTime = 0;
    ino_t defaultInode = 0;
    nlink_t defaultHardlink = 0;
    off_t defaultSize = 0;
    blkcnt_t defaultRawBlockSize = 0;

    el.name = NULL;
    el.strmode = NULL;
    el.owner = NULL;
    el.group = NULL;
    el.symbolicLink = NULL;
    el.time = defaultTime;
    el.hardlinks = defaultHardlink;
    el.size = defaultSize;
    el.inode = defaultInode;
    el.hasSize = defaultVal;
    el.useHumanReadable = defaultVal;
    el.showBlockSize = defaultVal;
    el.blockSize = defaultLong;
    el.rawBlockSize = defaultRawBlockSize;
    el.major = -1;
    el.minor = -1;

    return el;
}

struct maxsize
getDefaultMaxSizeStruct() {
    struct maxsize max;
    blkcnt_t defaultRawBlockSize = 0;

    max.name = 0;
    max.inode = 0;
    max.hardlinks = 0;
    max.owner = 0;
    max.group = 0;
    max.size = 0;
    max.blocksize = 0;
    max.minor = 0;
    max.totalBlockSize = defaultRawBlockSize;
    
    return max;
}

void
printFile(char **files, int *fileCount) {
    int addLine = 0;
    for (int i = 0; i < *fileCount; i++) {
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
    for (int i = 0; i < *errorCount; i++) {
        printError(errors[i]);
    }
}

void
printNewLine() {
    fprintf(stdout, "\n");
}

void
addWhiteSpaces(long number) {
    for (int i = 0; i < number; i++) {
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

char *
convertByteToHumanReadable(size_t bytes) {
    char size[] = {'B', 'K', 'M', 'T', 'P'};
    char *humanReadableSize = malloc(getNumberOfDigits(bytes) + 2);
    
    if(humanReadableSize == NULL) {
        return NULL;
    }

    humanReadableSize[0] = '\0';
    float result = bytes;
    char postFix = 'B';

    if (bytes >= 1024) {
        int power = (int) (log(bytes) / log(1024));
        result = bytes / pow(1024, power);
        postFix = size[power];
    } else if (bytes < 1024 && bytes >= 1000) {
        int power = (int) (log(bytes) / log(1000));
        result = bytes / pow(1000, power);
        postFix = size[power];
    }

    if (getNumberOfDigits(result) == 1 && postFix != 'B') {
        sprintf(humanReadableSize, "%.1f%c", result, postFix);
    } else {
        sprintf(humanReadableSize, "%.0f%c", result, postFix);
    }

    char *resultString = strdup(humanReadableSize);
    free(humanReadableSize);
    
    return resultString;
}

long
getBlockSize() {
    char *environmentValue = getenv("BLOCKSIZE");
    
    if (environmentValue == NULL) {
        return 512;
    }

    long blockSize = 0;
    long multiplier = 1024;

    for (unsigned int i = 0; i < strlen(environmentValue); i++) {
        char value = environmentValue[i];
        if (isdigit(value)) {
            blockSize = (blockSize * 10) + (long)(value - '0');
        } else if (isalpha(value)) {
            if (value == 'k' || value == 'K') {
                blockSize *= multiplier;
            } else if (value == 'm' || value == 'M') {
                blockSize *= multiplier * multiplier;
            } else if (value == 'g' || value == 'G') {
                blockSize *= multiplier * multiplier * multiplier;
            } else {
                return -1;
            }

            if ((i + 1) != strlen(environmentValue)) {
                return -1;
            }
        } else {
            return -1;
        }
    }

    if (blockSize < 512) {
        return -2;
    } else if (blockSize > (multiplier * multiplier * multiplier)) {
        return -3;
    }

    return blockSize;
}

long
convertToEnvironmentBlocksize(blkcnt_t blocks, long environmentBlocksize) {
    float divisor = environmentBlocksize / 512;
    long updatedBlocksize = (long) ceil(blocks / divisor);
    return updatedBlocksize;
}