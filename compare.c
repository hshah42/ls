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
compareTime(time_t timeOne, 
            time_t timeTwo, 
            const FTSENT **fileOnePointer, 
            const FTSENT **fileTwoPointer) {
    if (timeOne > timeTwo) {
        return -1;
    } else if (timeOne == timeTwo) {
        return compareLexographically(fileOnePointer, fileTwoPointer);
    } else {
        return 1;
    }
}

sort
getSortFunctionalPointer(sorting_type option) {
    int (*sort)(const FTSENT **, const FTSENT **);

    switch (option)
    {
    case ASCENDING_SIZE:
        sort = &compareSizeAscending;
        break;
    case DESCENDING_SIZE:
        sort = &compareSizeDescending;
        break;
    case BY_LAST_MODIFIED:
        sort = &compareLastModified;
        break;
    case BY_LAST_MODIFIED_REV:
        sort = &compareLastModifiedReverse;
        break;
    case BY_FILE_STATUS_CHANGE:
        sort = &compareLastFileStatusChange;
        break;
    case BY_FILE_STATUS_CHANGE_REV:
        sort = &compareLastFileStatusChangeReverse;
        break;
    case BY_FILE_ACCESS_TIME:
        sort = &compareAccessTime;
        break;
    case BY_FILE_ACCESS_TIME_REV:
        sort = &compareAccessTimeReverse;
        break;
    case LEXOGRAHICALLY:
        sort = &compareLexographically;
        break;
    default:
        break;
    }

    return sort;
}