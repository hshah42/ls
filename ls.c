#include "ls.h"

int
main(int argc, char **argv) {
    (void) setprogname(argv[0]);
    
    FTS *fts;
    FTSENT *ftsent;
    
    struct OPT *options = malloc(sizeof(struct OPT));
    if(options == NULL) {
        printError(strerror(errno));
        return 1;
    }

    setOptions(argc, argv, options);

    int maxSize = argc - optind; 

    if(optind == argc) {
        char *dir[1];
        dir[0] = ".";
        readDir(dir, options, 0);
    }
    else
    {
        char **directories = malloc(argc * sizeof(char*));
        
        if (directories == NULL) {
            printError(strerror(errno));
            return 1;
        }

        int dirSize = allocateFile(maxSize, argc, argv, directories);
        
        if (dirSize > 0) {
            if((argc - optind) == 1) {
                readDir(directories, options, 0);
            } else {
                readDir(directories, options, 1);
            }
        }

        free(directories);
    }

    free(options);

    return 0;
}

void
setOptions(int argc, char **argv, struct OPT *options) {
    int opt;

    while((opt = getopt(argc, argv, "AailR")) != -1) {
        switch (opt) {
        case 'A':
            options->includeHiddenFiles = 1;
            break;
        case 'a':
            options->listAllFlag = 1;
            break;
        case 'i':
            options->printInode = 1;
            break;
        case 'l':
            options->printStat = 1;
            break;
        case 'R':
            options->recurse = 1;
            break;
        case '?':
            fprintf(stderr, "Invalid Parameter");
            break;
        default:
            break;
        }
    }
}

int
readDir(char **files, struct OPT *options, int isDirnameRequired) {
    FTS *fts;
    FTSENT *ftsent;
    int flags = FTS_NOCHDIR;

    if(options->listAllFlag) {
        flags = flags | FTS_SEEDOT;
    }
    
    if((fts = fts_open(files, flags, NULL)) == NULL) {
        printError(strerror(errno));
        return 1;
    }

    performLs(fts, ftsent, options);    
    
    fts_close(fts);

    return 0;
}

int
performLs(FTS *fts, FTSENT *ftsent, struct OPT *options) {
    int falseInit = 0;
    int *shouldPrintContent = &falseInit;
    struct maxsize max;

    while ((ftsent = fts_read(fts)) != NULL) {
        int shouldPrintLine = 0;
        FTSENT* node = fts_children(fts, 0);

        if (node == NULL) {
            continue;
        }
            
        if (node->fts_level > 1 && !(options->recurse)) {
            break;
        }
        
        if (*shouldPrintContent)
            printDirectory(node->fts_parent->fts_path);
        else
            max = getDefaultMaxSizeStruct();
        
        FTSENT* directory = node->fts_parent;

        while (node != NULL)
        {
            shouldPrintLine = 1;

            if (*shouldPrintContent) {
                if(shouldPrint(options, node)) {
                    struct elements el = getDefaultStruct();
                    el.name = node->fts_name;                
                    generateElement(node->fts_path, &el, options, node);
                    printLine(el);
                    shouldPrintLine = 1;
                } else {
                    fts_set(fts, directory, FTS_SKIP);
                }
            } else {
                if (shouldPrint(options, node)) {
                   max = generateMaxSizeStruct(node, max);
                }
            }

            node = node->fts_link;
        }

        postChildTraversal(shouldPrintContent, shouldPrintLine, fts, directory);
        
    }

    return 0;
}

struct maxsize
generateMaxSizeStruct(FTSENT *node, struct maxsize max) {
    struct passwd *userInfo;
    struct group *groupInfo;

    if (node->fts_namelen > max.name) {
        max.name = node->fts_namelen;
    }

    if((userInfo = getpwuid(node->fts_statp->st_uid)) == NULL) {
        printError(strerror(errno));
    }

    if((groupInfo = getgrgid(node->fts_statp->st_gid)) == NULL) {
        printError(strerror(errno));
    }

    if(strlen(userInfo->pw_name) > max.owner) {
        max.owner = strlen(userInfo->pw_name);
    }

    if(strlen(groupInfo->gr_name) > max.group) {
        max.group = strlen(groupInfo->gr_name);
    }

    long inode = node->fts_statp->st_ino;
    long size = node->fts_statp->st_size;
    long hardlinks = node->fts_statp->st_nlink;

    if (getNumberOfDigits(inode) > max.inode) {
        max.inode = getNumberOfDigits(inode);
    }

    if (getNumberOfDigits(size) > max.size) {
        max.size = getNumberOfDigits(size);
    }

    if (getNumberOfDigits(hardlinks) > max.hardlinks) {
        max.hardlinks = getNumberOfDigits(hardlinks);
    }

    return max;
}

long
getNumberOfDigits(long number) {
    long count = 0;

    while(number != 0) {
        number = number / 10;
        count++;
    }

    return count;
}

int
postChildTraversal(int *shouldPrintContent, int shouldPrintLine, FTS *fts, FTSENT *directory) {
    if (!(*shouldPrintContent))
        fts_set(fts, directory, FTS_AGAIN);

    if (*shouldPrintContent) {
        *shouldPrintContent = 0;
    } else {
        *shouldPrintContent = 1;
    }
    
    if (shouldPrintLine)
        fprintf(stdout, "\n");
}

int
shouldPrint(struct OPT *options, FTSENT *node) {
    if(options->listAllFlag) {
        return 1;
    } else {
        if(strncmp(node->fts_name, ".", 1)) {
            return 1;
        } else {
            if(options->includeHiddenFiles) {
                return 1;
            }
        }
    }

    return 0;
}

int
generateElement(char *path, struct elements *el, struct OPT *options, FTSENT *ftsent) {
    struct stat stats;
    struct passwd *userInfo;
    struct group *groupInfo;
    char *permission = malloc(10);

    if(options->printInode) {
        el->inode = &(ftsent->fts_statp->st_ino);
    }

    if(options->printStat) {
        if(lstat(path, &stats) != 0) {
            printError(strerror(errno));
            return 1;
        }
        
        el->hardlinks = &(ftsent->fts_statp->st_nlink);
        userInfo = getpwuid(ftsent->fts_statp->st_uid);
        el->owner = userInfo->pw_name;
        groupInfo = getgrgid(ftsent->fts_statp->st_gid);
        el->group = groupInfo->gr_name;
        el->size = &(ftsent->fts_statp->st_size);
        int hasSize = 1;
        el->hasSize = &hasSize;
        el->time = &(ftsent->fts_statp->st_atime);
        strmode(ftsent->fts_statp->st_mode, permission);
        el->strmode = strdup(permission);
    }

    free(permission);
}

int
allocateFile(int maxSize, int argc, char **argv, char **directories) {
    int initialErrorIndex=0, initialFileIndex=0, initialDirectoryIndex = 0;
    
    char *errors[maxSize]; 
    int  *errorIndex = &initialErrorIndex;

    char *files[maxSize];
    int  *fileIndex = &initialFileIndex;

    struct stat stats;
    int index = 0;

    for (int i=optind ; i < argc; i++) {
        if(stat(argv[i], &stats) != 0) {
            char *error = malloc(strlen(argv[i]) + strlen(strerror(errno)) + 3);
            error[0] = '\0';
            strcat(error, argv[i]);
            strcat(error, ": ");
            strcat(error, strerror(errno));
            errors[*errorIndex] = strdup(error);
            (*errorIndex)++;
            free(error);
            continue;
        }

        if(S_ISDIR(stats.st_mode)) {
            directories[index] = argv[i];
            index++;
        } else {
            files[*fileIndex] = argv[i];
            (*fileIndex)++;
        }
    }

    printErrors(errors, errorIndex);
    printFile(files, fileIndex);

    return index;
}