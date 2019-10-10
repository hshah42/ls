#include "ls.h"

/**
 * Entry point of the program. Takes 2 arguments
 * 
 * argc: number of args passed in argv
 * argv: arguments passed to the program 
 **/
int
main(int argc, char **argv) {
    (void) setprogname(argv[0]);
    
    struct OPT *options = malloc(sizeof(struct OPT));
    if(options == NULL) {
        printError(strerror(errno));
        return 1;
    }

    if (isatty(STDOUT_FILENO)) {
        options->replaceNonPrintables = 1;
    } else {
        options->printNonPrintables = 1;
    }

    setOptions(argc, argv, options);

    int maxSize = argc - optind; 

    if(optind == argc) {
        char **dir = malloc(2);
        dir[0] = "./";
        readDir(dir, options, 0, 0);
        free(dir);
    }
    else
    {
        char **directories = malloc(argc * sizeof(char*));
        
        if (directories == NULL) {
            printError(strerror(errno));
            return 1;
        }

        int dirSize = allocateFile(maxSize, argc, argv, options, directories);

        if (dirSize > 0 && options->listDirectories) {
            readDir(directories, options, 0, 1);
        } else {
            if (dirSize > 0) {
                if((argc - optind) == 1) {
                    readDir(directories, options, 0, 0);
                } else {
                    readDir(directories, options, 1, 0);
                }
            }
        }

        free(directories);
    }

    free(options);

    return 0;
}

/**
 * setOptions takes in the arguments passed an gets the
 * optional parameters set by the user to enable flags that indicate
 * the flow of the program.
 * 
 **/
void
setOptions(int argc, char **argv, struct OPT *options) {
    int opt;

    while((opt = getopt(argc, argv, "AailRdStcnufFrqwhsk")) != -1) {
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
            options->printOwnerAndGroupNames = 1;
            options->printOwnerAndGroupID = 0;
            break;
        case 'R':
            options->recurse = 1;
            break;
        case 'd':
            options->listDirectories = 1;
            break;
        case 'S':
            resetSortOptions(options);
            options->sortBySizeDescending = 1;
            break;
        case 't':
            resetSortOptions(options);
            options->sortByLastModified = 1;
            break;
        case 'c':
            options->useFileStatusChangeTime = 1;
            options->useLastAccessTime = 0;
            break;
        case 'n':
            options->printStat = 1;
            options->printOwnerAndGroupNames = 0;
            options->printOwnerAndGroupID = 1;
            break;
        case 'u':
            options->useFileStatusChangeTime = 0;
            options->useLastAccessTime = 1;
            break;
        case 'f':
            options->keepUnsorted = 1;
            break;
        case 'F':
            options->appendFileType = 1;
            break;
        case 'r':
            options->reverseOrder = 1;
            break;
        case 'q':
            options->printNonPrintables = 0;
            options->replaceNonPrintables = 1;
            break;
        case 'w':
            options->replaceNonPrintables = 0;
            options->printNonPrintables = 1;
            break;
        case 'h':
            options->isHumanReadableSize = 1;
            options->printBlockSizeInK = 0;
            break;
        case 's':
            options->printBlockSize = 1;
            break;
        case 'k':
            options->printBlockSizeInK = 1;
            options->isHumanReadableSize = 0;
            break;
        case '?':
            fprintf(stderr, "Invalid Parameter");
            break;
        default:
            break;
        }
    }
}

/**
 * This method resets the sorting options since
 * the last specified sorting option is used
 * 
 **/
void
resetSortOptions(struct OPT *options) {
    options->sortByLastModified = 0;
    options->sortBySizeDescending = 0;
}

/**
 * Reading the directory specified in the arguments
 * This method will call the subsiquent methods that print the
 * directories depending on the optional parameters passed by the
 * user.
 * 
 **/
int
readDir(char **files, struct OPT *options, 
        int isDirnameRequired, int onFiles) {
    FTS *fts;
    int flags = FTS_PHYSICAL | FTS_NOCHDIR;

    if(options->listAllFlag) {
        flags = flags | FTS_SEEDOT;
    }
    
    sort sortFunction = getSortType(options);

    if((fts = fts_open(files, flags, sortFunction)) == NULL) {
        printError(strerror(errno));
        return 1;
    }

    if (onFiles) {
        preformLsOnfiles(fts, options);
    } else {
        performLs(fts, options, isDirnameRequired);
    }
    
    (void) fts_close(fts);

    return 0;
}

sort
getSortType(struct OPT *options) {
    if (options->keepUnsorted) {
        return NULL;
    }

    if (options->sortBySizeDescending) {
        if (options->reverseOrder) {
            return getSortFunctionalPointer(ASCENDING_SIZE);
        }
        return getSortFunctionalPointer(DESCENDING_SIZE);
    }

    if (options->sortByLastModified) {
        if (options->useFileStatusChangeTime) {
            if (options->reverseOrder) {
                return getSortFunctionalPointer(BY_FILE_STATUS_CHANGE_REV);
            }
            return getSortFunctionalPointer(BY_FILE_STATUS_CHANGE);
        } else if (options->useLastAccessTime) {
            if (options->reverseOrder) {
                return getSortFunctionalPointer(BY_FILE_ACCESS_TIME_REV);
            }
            return getSortFunctionalPointer(BY_FILE_ACCESS_TIME);
        } else {
            if (options->reverseOrder) {
                return getSortFunctionalPointer(BY_LAST_MODIFIED_REV);
            }
            return getSortFunctionalPointer(BY_LAST_MODIFIED);
        }
    }

    if (options->reverseOrder) {
        return getSortFunctionalPointer(LEXOGRAHICALLY_REV);
    }

    return getSortFunctionalPointer(LEXOGRAHICALLY);
}

/**
 * This function will perform traversing of the directories that are
 * passed in the arguments.
 * 
 * It will recursively iterate through the file system in a depth first way whenever
 * it encounters a directory.
 * 
 * It will call fts_children on every directory encountered. fts_children was used
 * because fts read goes depth first and we would like to print the content of the 
 * directories before we move forward. fts_children helps us in achieving those
 * results.
 * 
 * We plus repeat iteration on the directory. During the first iteration we get all
 * max values from the directory in order to have pretty print. In the second iteration
 * we actually print the values, adding whitespaces where necessary based on the max 
 * values we gathered during the previous iteration of the same directory.
 **/
int
performLs(FTS *fts, struct OPT *options, int isDirnameRequired) {
    int falseInit = 0;
    // This variable determines when to print the contents and
    // when to get the max values for printing.
    int *shouldPrintContent = &falseInit;
    struct maxsize max = getDefaultMaxSizeStruct();
    FTSENT *ftsent;

    while ((ftsent = fts_read(fts)) != NULL) {
        if (options->listDirectories) {
            if (ftsent->fts_level > 0 || ftsent->fts_info == FTS_DP) {
                fts_set(fts, ftsent, FTS_SKIP);
                continue;
            }
            max = generateMaxSizeStruct(ftsent, options, max);
            printInformation(options, ftsent, max);
            continue;
        }

        FTSENT* node = fts_children(fts, 0);

        if (ftsent->fts_level > 1 && !(options->recurse)) {
            fts_set(fts, ftsent, FTS_SKIP);
            continue;
        }

        if (node == NULL || (node->fts_level > 1 && !(options->recurse))) {
            continue;
        }
        
        if (*shouldPrintContent) {
            if (isDirnameRequired) {
                printNewLine();
                printDirectory(node->fts_parent->fts_path);
            } else if (options->recurse) {
                isDirnameRequired = 1;
            }
        }
        else
            max = getDefaultMaxSizeStruct();
        
        FTSENT* directory = node->fts_parent;

        while (node != NULL)
        {
            if (*shouldPrintContent) {
                if (shouldPrint(options, node)) {
                    if (printInformation(options, node, max) != 0) {
                        node = node->fts_link;
                        continue;
                    }
                }
            } else {
                if (shouldPrint(options, node)) {
                   max = generateMaxSizeStruct(node, options, max); 
                } else {
                    fts_set(fts, node, FTS_SKIP);
                }
            }

            node = node->fts_link;
        }

        postChildTraversal(shouldPrintContent, fts, directory);
    }

    return 0;
}

int
preformLsOnfiles(FTS *fts, struct OPT *options) {
    struct maxsize max = getDefaultMaxSizeStruct();
    FTSENT *ftsent;

    while ((ftsent = fts_read(fts)) != NULL) {
        if (ftsent->fts_level > 0) {
            fts_set(fts, ftsent, FTS_SKIP);
            continue;
        } else if (ftsent->fts_info == FTS_DP) {
            continue;
        }
        printInformation(options, ftsent, max);
    }

    return 0;
}

int
printInformation(struct OPT *options, FTSENT *node, struct maxsize max) {
    struct elements el = getDefaultStruct();
                    
    if (S_ISLNK(node->fts_statp->st_mode) && options->printStat) {
        if (addLinkName(node, &el) != 0) {
            return 1;
        }
    } else {
        el.name = node->fts_name; 
    }

    if (options->appendFileType) {
        appendType(node, &el);
    }

    if (options->replaceNonPrintables)
        checkPrintableCharacters(&el);
    
    if (generateElement(&el, options, node) == 0) {
        printLine(el, max);
    } else {
        return 1;
    }

    return 0;
}

void
checkPrintableCharacters(struct elements *el) {
    for (unsigned int i = 0; i < strlen(el->name); i++) {
        if (!isprint(el->name[i])) {
            el->name[i] = '?';
        } 
    }
}

/**
 * If the F optional parameter is specified then this function is called.
 * The F optional parameter requires the ls function to append the file name
 * with special characters based on the type of the file
 * 
 **/
void
appendType(FTSENT *node, struct elements *el) {
    char newName[strlen(el->name) + 2];
    newName[0]='\0';
    strcat(newName, el->name);

    if (S_ISDIR(node->fts_statp->st_mode)) {
        strcat(newName, "/\0");
    } else if (S_ISLNK(node->fts_statp->st_mode)) {
        strcat(newName, "@\0");
    } else if (node->fts_info == FTS_W) {
        strcat(newName, "%\0");
    } else if (S_ISSOCK(node->fts_statp->st_mode)) {
        strcat(newName, "=\0");
    } else if (S_ISFIFO(node->fts_statp->st_mode)) {
        strcat(newName, "|\0");
    } else if (node->fts_statp->st_mode & S_IXUSR) {
        strcat(newName, "*\0");
    }

    el->name = strdup(newName);
}

/**
 * Creating a symbolic link name when encountered and adding it to
 * element struct.
 * 
 **/
int
addLinkName(FTSENT *node,  struct elements *el) {
    char linkname[PATH_MAX];
    char pathname[node->fts_namelen + node->fts_pathlen + 1];
    pathname[0]='\0';
    strcat(pathname, node->fts_accpath);
    
    if (S_ISDIR(node->fts_parent->fts_statp->st_mode)) {
        strcat(pathname, node->fts_name);  
    }
    
    ssize_t len;
    if ((len = readlink(pathname, linkname, (sizeof(linkname) - 1) )) == -1) {
        printError(strerror(errno));
        return 1;
    }
    linkname[len] = '\0';
    char fullLink[strlen(linkname) + strlen(node->fts_name) + 1];
    fullLink[0] = '\0';
    strcat(fullLink, node->fts_name);
    strcat(fullLink, " -> ");
    strcat(fullLink, linkname);
    el->name = strdup(fullLink);
    return 0;
}

/**
 * Creates a maxsize structure which is used by print.c to pretty print
 * the files by adding relevant whitespaces where needed in order to avoid
 * congestion in data representation.
 * 
 **/
struct maxsize
generateMaxSizeStruct(FTSENT *node, struct OPT *options, struct maxsize max) {
    struct passwd *userInfo;
    struct group *groupInfo;

    if (node->fts_namelen > max.name) {
        max.name = node->fts_namelen;
    }

    if ((userInfo = getpwuid(node->fts_statp->st_uid)) == NULL) {
        if (getNumberOfDigits(node->fts_statp->st_uid) > max.owner) {
            max.owner = getNumberOfDigits(node->fts_statp->st_uid);
        }
    } else {
        if(strlen(userInfo->pw_name) > max.owner) {
            max.owner = strlen(userInfo->pw_name);
        }
    }

    if ((groupInfo = getgrgid(node->fts_statp->st_gid)) == NULL) {
        if (getNumberOfDigits(node->fts_statp->st_gid) > max.group) {
            max.owner = getNumberOfDigits(node->fts_statp->st_gid);
        }
    } else {
        if (strlen(groupInfo->gr_name) > max.group) {
            max.group = strlen(groupInfo->gr_name);
        }
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

    long blocksize = convertToEnvironmentBlocksize(node->fts_statp->st_blocks, 
                                                      options->blocksize);
    if (getNumberOfDigits(blocksize) > max.blocksize) {
        max.blocksize = getNumberOfDigits(blocksize);
    }

    return max;
}

void
postChildTraversal(int *shouldPrintContent, FTS *fts, FTSENT *directory) {
    if (!(*shouldPrintContent))
        fts_set(fts, directory, FTS_AGAIN);

    if (*shouldPrintContent) {
        *shouldPrintContent = 0;
    } else {
        *shouldPrintContent = 1;
    }
}

/**
 * This method determines if the file being traversed needs to be
 * printed on not.
 * 
 * Depends on -A -a flag to determine when to include ., .. and 
 * hidden files.
 * 
 **/
int
shouldPrint(struct OPT *options, FTSENT *node) {
    if (options->listAllFlag) {
        return 1;
    } else {
        if (strncmp(node->fts_name, ".", 1)) {
            return 1;
        } else {
            if(options->includeHiddenFiles) {
                return 1;
            }
        }
    }
    return 0;
}

/**
 * Generates the element structure based on the options specified.
 * The element struct is passed onto the print functions to print the 
 * elements that have been set by this function.
 * 
 **/
int
generateElement(struct elements *el, struct OPT *options, FTSENT *ftsent) {
    struct passwd *userInfo;
    struct group *groupInfo;
    char *permission = malloc(10);

    if (options->printBlockSize) {
        if (options->printBlockSizeInK) {
            el->blockSize = convertToEnvironmentBlocksize(ftsent->fts_statp->st_blocks, 
                                                          1024);
        } else {
            el->blockSize = convertToEnvironmentBlocksize(ftsent->fts_statp->st_blocks, 
                                                      options->blocksize);
        }
        
        el->rawBlockSize = ftsent->fts_statp->st_blocks * 512;
        el->showBlockSize = 1;
    }

    if (options->printInode) {
        el->inode = ftsent->fts_statp->st_ino;
    }

    if (options->printStat) {
        el->hardlinks = ftsent->fts_statp->st_nlink;

        if (options->printOwnerAndGroupID || 
            (userInfo = getpwuid(ftsent->fts_statp->st_uid)) == NULL) {
            int size = getNumberOfDigits(ftsent->fts_statp->st_uid);
            char *owner = malloc(size + 1);
            if(owner == NULL) {
                printError(strerror(errno));
                return 1;
            }
            sprintf(owner, "%d", ftsent->fts_statp->st_uid);
            el->owner = strdup(owner);
            free(owner);
        } else {
            el->owner = userInfo->pw_name;
        }
        
        if (options->printOwnerAndGroupID 
            || ((groupInfo = getgrgid(ftsent->fts_statp->st_gid)) == NULL)) {
            int size = getNumberOfDigits(ftsent->fts_statp->st_gid);
            char *group = malloc(size + 1);
            if (group == NULL) {
                printError(strerror(errno));
                return 1;
            }
            sprintf(group, "%d", ftsent->fts_statp->st_gid);
            el->group = strdup(group);
            free(group);
        } else {
            el->group = groupInfo->gr_name;
        }

        el->size = ftsent->fts_statp->st_size;
        int hasSize = 1;
        el->hasSize = hasSize;
        
        if (options->useFileStatusChangeTime) {
            el->time = ftsent->fts_statp->st_ctimespec.tv_sec;
        } else if (options->useLastAccessTime) {
            el->time = ftsent->fts_statp->st_atimespec.tv_sec;
        } else {
            el->time = ftsent->fts_statp->st_mtimespec.tv_sec;
        }
        
        if (options->isHumanReadableSize) {
            el->useHumanReadable = 1;
        }

        strmode(ftsent->fts_statp->st_mode, permission);
        el->strmode = strdup(permission);
    }

    free(permission);

    return 0;
}

int
allocateFile(int maxSize, int argc, char **argv, 
                struct OPT *options, char **directories) {
    long blockSize = getBlockSize();

    if (blockSize < 0) {
        options->blocksize = 512;
    } else {
        options->blocksize = blockSize;
    }
    
    int initialErrorIndex=0, initialFileIndex=0;
    
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

        if (S_ISDIR(stats.st_mode)) {
            directories[index] = argv[i];
            index++;
        } else {
            if (options->listDirectories) {
                directories[index] = argv[i];
                index++;
            } else {
                files[*fileIndex] = argv[i];
                (*fileIndex)++;
            }
        }
    }

    printErrors(errors, errorIndex);
    
    if (*fileIndex > 0) {
        files[*fileIndex] = '\0';
        readDir(files, options, 0, 1);
    }

    return index;
}