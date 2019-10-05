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
compareLastModified(const FTSENT **fileOnePointer, const FTSENT **fileTwoPointer) {
    const FTSENT *fileOne = *fileOnePointer;
    const FTSENT *fileTwo = *fileTwoPointer;

    if(fileOne->fts_statp->st_mtimespec.tv_sec > fileTwo->fts_statp->st_mtimespec.tv_sec) {
        return -1;
    } else if (fileOne->fts_statp->st_mtimespec.tv_sec == fileTwo->fts_statp->st_mtimespec.tv_sec) {
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
    default:
        break;
    }

    return sort;
}