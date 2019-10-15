#include "compare.h"

int 
compareSizeDescending(const FTSENT **fileOnePointer, const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;

    if(fileOne->fts_statp->st_size > fileTwo->fts_statp->st_size) {
        return -1;
    } else if(fileOne->fts_statp->st_size == fileTwo->fts_statp->st_size) {
        return compareLexographically(fileOnePointer, fileTwoPointer);
    } else {
        return 1;
    }
}

int 
compareSizeAscending(const FTSENT **fileOnePointer, const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;

    if(fileOne->fts_statp->st_size < fileTwo->fts_statp->st_size) {
        return -1;
    } else if(fileOne->fts_statp->st_size == fileTwo->fts_statp->st_size) {
        return compareLexographically(fileOnePointer, fileTwoPointer);
    } else {
        return 1;
    }
}

int 
compareLastModified(const FTSENT **fileOnePointer, 
                    const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;
    return compareTime(fileOne->fts_statp->st_mtimespec.tv_sec, 
                       fileTwo->fts_statp->st_mtimespec.tv_sec,
                       fileOne->fts_statp->st_mtimespec.tv_nsec,
                       fileTwo->fts_statp->st_mtimespec.tv_nsec,
                       fileOnePointer,
                       fileTwoPointer);
}

int 
compareLastModifiedReverse(const FTSENT **fileOnePointer, 
                           const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;
    return compareTime(fileTwo->fts_statp->st_mtimespec.tv_sec, 
                       fileOne->fts_statp->st_mtimespec.tv_sec,
                       fileTwo->fts_statp->st_mtimespec.tv_nsec,
                       fileOne->fts_statp->st_mtimespec.tv_nsec,
                       fileTwoPointer,
                       fileOnePointer);
}

int 
compareLastFileStatusChange(const FTSENT **fileOnePointer, 
                            const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;
    return compareTime(fileOne->fts_statp->st_ctimespec.tv_sec, 
                       fileTwo->fts_statp->st_ctimespec.tv_sec,
                       fileOne->fts_statp->st_ctimespec.tv_nsec, 
                       fileTwo->fts_statp->st_ctimespec.tv_nsec,
                       fileOnePointer,
                       fileTwoPointer);
}

int 
compareLastFileStatusChangeReverse(const FTSENT **fileOnePointer, 
                                   const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;
    return compareTime(fileTwo->fts_statp->st_ctimespec.tv_sec, 
                       fileOne->fts_statp->st_ctimespec.tv_sec,
                       fileTwo->fts_statp->st_ctimespec.tv_nsec, 
                       fileOne->fts_statp->st_ctimespec.tv_nsec,
                       fileTwoPointer,
                       fileOnePointer);
}

int 
compareAccessTime(const FTSENT **fileOnePointer, 
                  const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;
    return compareTime(fileOne->fts_statp->st_atimespec.tv_sec, 
                       fileTwo->fts_statp->st_atimespec.tv_sec,
                       fileOne->fts_statp->st_atimespec.tv_nsec, 
                       fileTwo->fts_statp->st_atimespec.tv_nsec,
                       fileOnePointer,
                       fileTwoPointer);
}

int 
compareAccessTimeReverse(const FTSENT **fileOnePointer, 
                         const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;
    return compareTime(fileTwo->fts_statp->st_atimespec.tv_sec, 
                       fileOne->fts_statp->st_atimespec.tv_sec,
                       fileTwo->fts_statp->st_atimespec.tv_nsec, 
                       fileOne->fts_statp->st_atimespec.tv_nsec,
                       fileTwoPointer,
                       fileOnePointer);
}

int 
compareLexographically(const FTSENT **fileOnePointer, 
                       const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;
    
    return strcmp(fileOne->fts_name, fileTwo->fts_name);
}

int 
compareLexographicallyReverese(const FTSENT **fileOnePointer, 
                               const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;
    
    return strcmp(fileTwo->fts_name, fileOne->fts_name);
}

int
compareTime(time_t timeOne, 
            time_t timeTwo,
            time_t timeOneNano,
            time_t timeTwoNano, 
            const FTSENT **fileOnePointer, 
            const FTSENT **fileTwoPointer) {
    if (timeOne > timeTwo) {
        return -1;
    } else if (timeOne == timeTwo) {
        if (timeOneNano > timeTwoNano) {
            return -1;
        } else if (timeOneNano == timeTwoNano) {
             return compareLexographically(fileOnePointer, fileTwoPointer);
        } else {
            return 1;
        }
    } else {
        return 1;
    }
}

sort
getSortFunctionalPointer(sorting_type option) {
    int (*sortFunction)(const FTSENT **, const FTSENT **);

    switch (option)
    {
    case ASCENDING_SIZE:
        sortFunction = &compareSizeAscending;
        break;
    case DESCENDING_SIZE:
        sortFunction = &compareSizeDescending;
        break;
    case BY_LAST_MODIFIED:
        sortFunction = &compareLastModified;
        break;
    case BY_LAST_MODIFIED_REV:
        sortFunction = &compareLastModifiedReverse;
        break;
    case BY_FILE_STATUS_CHANGE:
        sortFunction = &compareLastFileStatusChange;
        break;
    case BY_FILE_STATUS_CHANGE_REV:
        sortFunction = &compareLastFileStatusChangeReverse;
        break;
    case BY_FILE_ACCESS_TIME:
        sortFunction = &compareAccessTime;
        break;
    case BY_FILE_ACCESS_TIME_REV:
        sortFunction = &compareAccessTimeReverse;
        break;
    case LEXOGRAHICALLY:
        sortFunction = &compareLexographically;
        break;
    case LEXOGRAHICALLY_REV:
        sortFunction = &compareLexographicallyReverese;
        break;
    default:
        break;
    }

    return sortFunction;
}