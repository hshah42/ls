#include "common.h"

typedef int (*sort)(const FTSENT **, const FTSENT **);

typedef enum { 
    ASCENDING_SIZE = 0, 
    DESCENDING_SIZE = 1,  
    BY_LAST_MODIFIED = 2, 
    BY_LAST_MODIFIED_REV = 3,
    BY_FILE_STATUS_CHANGE = 4,
    BY_FILE_STATUS_CHANGE_REV = 5,
    BY_FILE_ACCESS_TIME = 6,
    BY_FILE_ACCESS_TIME_REV = 7
} sorting_type;

int compareSizeAscending(const FTSENT **fileOne, const FTSENT **fileTwo);
int compareSizeDescending(const FTSENT **fileOne, const FTSENT **fileTwo);
int compareLastModified(const FTSENT **fileOnePointer, const FTSENT **fileTwoPointer);
int compareLastModifiedReverse(const FTSENT **fileOnePointer, const FTSENT **fileTwoPointer);
int compareLastFileStatusChange(const FTSENT **fileOnePointer, const FTSENT **fileTwoPointer);
int compareLastFileStatusChangeReverse(const FTSENT **fileOnePointer, const FTSENT **fileTwoPointer);
int compareAccessTime(const FTSENT **fileOnePointer, const FTSENT **fileTwoPointer);
int compareAccessTimeReverse(const FTSENT **fileOnePointer, const FTSENT **fileTwoPointer);
sort getSortFunctionalPointer(sorting_type option);
int compareTime(time_t timeOne, time_t timeTwo);