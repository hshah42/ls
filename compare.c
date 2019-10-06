#include "compare.h"

int 
compareSizeDescending(const FTSENT **fileOnePointer, const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;

    if(fileOne->fts_statp->st_size > fileTwo->fts_statp->st_size) {
        return -1;
    } else if(fileOne->fts_statp->st_size == fileTwo->fts_statp->st_size) {
        return 0;
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
        return 0;
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
                       fileTwo->fts_statp->st_mtimespec.tv_sec);
}

int 
compareLastModifiedReverse(const FTSENT **fileOnePointer, 
                           const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;
    return compareTime(fileTwo->fts_statp->st_mtimespec.tv_sec, 
                       fileOne->fts_statp->st_mtimespec.tv_sec);
}

int 
compareLastFileStatusChange(const FTSENT **fileOnePointer, 
                            const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;
    return compareTime(fileOne->fts_statp->st_ctimespec.tv_sec, 
                       fileTwo->fts_statp->st_ctimespec.tv_sec);
}

int 
compareLastFileStatusChangeReverse(const FTSENT **fileOnePointer, 
                                   const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;
    return compareTime(fileTwo->fts_statp->st_ctimespec.tv_sec, 
                       fileOne->fts_statp->st_ctimespec.tv_sec);
}

int 
compareAccessTime(const FTSENT **fileOnePointer, 
                  const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;
    return compareTime(fileOne->fts_statp->st_atimespec.tv_sec, 
                       fileTwo->fts_statp->st_atimespec.tv_sec);
}

int 
compareAccessTimeReverse(const FTSENT **fileOnePointer, 
                         const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;
    return compareTime(fileTwo->fts_statp->st_atimespec.tv_sec, 
                       fileOne->fts_statp->st_atimespec.tv_sec);
}

int
compareTime(time_t timeOne, time_t timeTwo) {
    if(timeOne > timeTwo) {
        return -1;
    } else if (timeOne == timeTwo) {
        return 0;
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
    default:
        break;
    }

    return sort;
}