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
    uid_t eid = geteuid();
    int maxSize;
    int dirSize;
    
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

    if ( eid == 0 ) {
        options->includeHiddenFiles = 1;
    }

    if (setOptions(argc, argv, options) != 0) {
        return 1;
    }

    (void) checkBlockSize(options);

    maxSize = argc - optind; 

    if(optind == argc) {
        char **dir = malloc(2);
        if (dir == NULL) {
            return 1;
        }

        dir[0] = "./";
        if (options->listDirectories) {
            readDir(dir, options, 0, 1);
        } else {
            readDir(dir, options, 0, 0);
        }
        
       (void) free(dir);
    }
    else
    {
        char **directories = malloc(argc * sizeof(char*));
        
        if (directories == NULL) {
            (void) printError(strerror(errno));
            return 1;
        }

        dirSize = allocateFile(maxSize, argc, argv, options, directories);

        if (dirSize > 0 && options->listDirectories) {
            (void) readDir(directories, options, 0, 1);
        } else {
            if (dirSize > 0) {
                if((argc - optind) == 1) {
                    (void) readDir(directories, options, 0, 0);
                } else {
                    (void) readDir(directories, options, 1, 0);
                }
            }
        }

        (void) free(directories);
    }

    (void) free(options);

    return 0;
}

/**
 * setOptions takes in the arguments passed an gets the
 * optional parameters set by the user to enable flags that indicate
 * the flow of the program.
 * 
 **/
int
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
            fprintf(stderr, "usage: %s [-AailRdStcnufFrqwhsk] [file ...] \n", 
                    getprogname());
            return 1;
            break;
        default:
            break;
        }
    }

    return 0;
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
    sort sortFunction;
    int flags = FTS_PHYSICAL | FTS_NOCHDIR;

    if(options->listAllFlag) {
        flags = flags | FTS_SEEDOT;
    }
    
    sortFunction = getSortType(options);

    if((fts = fts_open(files, flags, sortFunction)) == NULL) {
        (void) printError(strerror(errno));
        return 1;
    }

    if (onFiles) {
        (void) preformLsOnfiles(fts, options);
    } else {
        (void) performLs(fts, options, isDirnameRequired);
    }
    
    //(void) fts_close(fts);

    return 0;
}

/**
 * Determine the type of sorting required based on the options
 * 
 **/
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
    /* This variable determines when to print the contents and
     when to get the max values for printing */
    int *shouldPrintContent = &falseInit;
    struct maxsize max = getDefaultMaxSizeStruct();
    FTSENT *ftsent;
    FTSENT* node;

    while ((ftsent = fts_read(fts)) != NULL) {
        /* Ignore the dirs which should not be printed 
         and are not the root directory */
        if (!shouldPrint(options, ftsent) && ftsent->fts_level != 0) {
            continue;
        }

        /* If option is not recurse, ignore the files with level > 1 */
        if (ftsent->fts_level > 1 && !(options->recurse)) {
            (void) fts_set(fts, ftsent, FTS_SKIP);
            continue;
        }

        node = fts_children(fts, 0);
        /* If ftsent has no children then continue with traversal */
        if (node == NULL || (node->fts_level > 1 && !(options->recurse))) {
            if ((options->recurse || (isDirnameRequired && ftsent->fts_level == 0)) 
                    && ftsent->fts_info == FTS_D) {
                (void) printNewLine();
                (void) printDirectory(ftsent->fts_path);
            }
            /* Since if there is an error and it will not get any children
               we need to print the reason we encountered the error */
            if ((ftsent->fts_level < 1 || options->recurse)) {
                (void) printErrorIfAny(ftsent);
            }

            continue;
        }
        
        if (isDirnameRequired) {
            (void) printNewLine();
            (void) printDirectory(node->fts_parent->fts_path);
        } else if (options->recurse) {
            isDirnameRequired = 1;
        }

        max = getDefaultMaxSizeStruct();

        /* Traverse linked list to get the max width for each field */
        max = traverseChildren(fts, node, 0, options, max);

        if (options->printStat || options->printBlockSize) {
            if (options->isHumanReadableSize) {
                char *output = convertByteToHumanReadable(max.totalBlockSize * 512);
                fprintf(stdout, "total: %s \n", output);
            } else {
                long totalBlocks = convertToEnvironmentBlocksize(max.totalBlockSize, 
                                                                     options->blocksize);
                fprintf(stdout, "total: %lu \n", totalBlocks);
            }
        }

        // /* Traverse the linked list again to print the values using 
        //    the max generated in previous traversal */
        (void) traverseChildren(fts, node, 1, options, max);

        (void) postChildTraversal(shouldPrintContent);
    }

    return 0;
}

struct maxsize
traverseChildren(FTS *fts, FTSENT *node, int shouldPrintContent, 
                 struct OPT *options, struct maxsize max) {
    while (node != NULL)
    {
        if (printErrorIfAny(node) != 0) {
            node = node->fts_link;
            continue;
        }
        if (shouldPrintContent) {
            if (shouldPrint(options, node)) {
                if (printInformation(options, node, max) != 0) {
                    node = node->fts_link;
                    continue;
                }
            }
        } else {
            if (shouldPrint(options, node)) {
                max = generateMaxSizeStruct(node, options, max); 
            }
        }
        node = node->fts_link;
    }

    return max;
}

/**
 * Perform ls on files itself that are passed as parameters rather than
 * children of the files.
 **/
int
preformLsOnfiles(FTS *fts, struct OPT *options) {
    struct maxsize max = getDefaultMaxSizeStruct();
    FTSENT *ftsent;

    while ((ftsent = fts_read(fts)) != NULL) {
        if (ftsent->fts_level > 0) {
            (void) fts_set(fts, ftsent, FTS_SKIP);
            continue;
        } else if (ftsent->fts_info == FTS_DP || ftsent->fts_info == FTS_DNR) {
            continue;
        }
        if (printErrorIfAny(ftsent) != 0) {
            continue;
        }
        (void) printInformation(options, ftsent, max);
    }

    return 0;
}

/**
 * Print information related to the file that is being
 * traversed. It calls generate element, which will 
 * encapsulate all the information related to the printing of line.
 **/
int
printInformation(struct OPT *options, FTSENT *node, struct maxsize max) {
    struct elements el = getDefaultStruct();

    /* If directory and if the node is of level 0 which means that
       the directory was passed in the argumets, then we need to print the path
       and in other cases we print the file name */
     if (options->listDirectories || node->fts_level == 0) {
             el.name = node->fts_path;
    } else {
             el.name = node->fts_name;
    }

    if (options->appendFileType) {
        (void) appendType(node, &el);
    }
                    
    if (S_ISLNK(node->fts_statp->st_mode) && options->printStat) {
        if (addLinkName(node, &el) != 0) {
            return 1;
        }
    }

    if (options->replaceNonPrintables)
        (void) checkPrintableCharacters(&el);
    
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
    char newName[PATH_MAX+ 2];
    newName[0]='\0';
    (void) strcat(newName, el->name);

    if (S_ISDIR(node->fts_statp->st_mode)) {
        (void) strcat(newName, "/\0");
    } else if (S_ISLNK(node->fts_statp->st_mode)) {
        (void) strcat(newName, "@\0");
    } else if (node->fts_info == FTS_W) {
        (void) strcat(newName, "%\0");
    } else if (S_ISSOCK(node->fts_statp->st_mode)) {
        (void) strcat(newName, "=\0");
    } else if (S_ISFIFO(node->fts_statp->st_mode)) {
        (void) strcat(newName, "|\0");
    } else if (node->fts_statp->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
        (void) strcat(newName, "*\0");
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
    char pathname[PATH_MAX + 1];
    pathname[0]='\0';
    /* Using path because readlink requires full path or else
       readlink will not be able to find the file */
    (void) strcat(pathname, node->fts_accpath);
    
    if (S_ISDIR(node->fts_parent->fts_statp->st_mode)) {
        (void) strcat(pathname, node->fts_name);  
    }
    
    ssize_t len;
    if ((len = readlink(pathname, linkname, (sizeof(linkname) - 1) )) == -1) {
        (void) printError(strerror(errno));
        return 1;
    }
    linkname[len] = '\0';
    char fullLink[PATH_MAX + 1];
    fullLink[0] = '\0';
    (void) strcat(fullLink, el->name);
    (void) strcat(fullLink, " -> ");
    (void) strcat(fullLink, linkname);
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

    if (options->printOwnerAndGroupID || 
        (userInfo = getpwuid(node->fts_statp->st_uid)) == NULL) {
        if (getNumberOfDigits(node->fts_statp->st_uid) > max.owner) {
            max.owner = getNumberOfDigits(node->fts_statp->st_uid);
        }
    } else {
        if(strlen(userInfo->pw_name) > max.owner) {
            max.owner = strlen(userInfo->pw_name);
        }
    }

    if (options->printOwnerAndGroupID || 
        (groupInfo = getgrgid(node->fts_statp->st_gid)) == NULL) {
        if (getNumberOfDigits(node->fts_statp->st_gid) > max.group) {
            max.group = getNumberOfDigits(node->fts_statp->st_gid);
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

    if (options->isHumanReadableSize) {
        if (max.size < 4) {
            max.size = 4;
        }
    } else {
        if (getNumberOfDigits(size) > max.size) {
            max.size = getNumberOfDigits(size);
        }
    }

    if (getNumberOfDigits(hardlinks) > max.hardlinks) {
        max.hardlinks = getNumberOfDigits(hardlinks);
    }

    long blocksize = convertToEnvironmentBlocksize(node->fts_statp->st_blocks, 
                                                      options->blocksize);
    if (getNumberOfDigits(blocksize) > max.blocksize) {
        max.blocksize = getNumberOfDigits(blocksize);
    }

     if (S_ISBLK(node->fts_statp->st_mode) || 
         S_ISCHR(node->fts_statp->st_mode)) {
        int major = major(node->fts_statp->st_rdev);
        int minor = minor(node->fts_statp->st_rdev);
        unsigned int maxMajorMinor = 2 + getNumberOfDigits(major) 
                                       + getNumberOfDigits(minor);
        if (max.size < maxMajorMinor) {
            max.size = maxMajorMinor;
        }

        if (max.minor < getNumberOfDigits(minor)) {
            max.minor = getNumberOfDigits(minor);
        }
    }

    if (options->isHumanReadableSize) {
        max.totalBlockSize = max.totalBlockSize + (size / 512);
    } else {
        max.totalBlockSize = max.totalBlockSize + node->fts_statp->st_blocks;
    }

    return max;
}

void
postChildTraversal(int *shouldPrintContent) {
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
 * If level is 0 which means it was passed in the argument and hence
 * it needs to be printed
 **/
int
shouldPrint(struct OPT *options, FTSENT *node) {
    if (node->fts_level == 0) {
        return 0;
    }

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
    
    if (permission == NULL) {
        return 1;
    }

    if (options->printBlockSize) {
        el->blockSize = convertToEnvironmentBlocksize(ftsent->fts_statp->st_blocks, 
                                                      options->blocksize);
        el->rawBlockSize = ftsent->fts_statp->st_blocks * 512;
        el->showBlockSize = 1;
    }

    if (options->printInode) {
        el->inode = ftsent->fts_statp->st_ino;
    }


    if (options->isHumanReadableSize) {
        el->useHumanReadable = 1;
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
            (void) free(group);
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
        el->hasTime = 1;

        (void) strmode(ftsent->fts_statp->st_mode, permission);
        el->strmode = strdup(permission);

        if (S_ISBLK(ftsent->fts_statp->st_mode) || 
            S_ISCHR(ftsent->fts_statp->st_mode)) {
            el->major = major(ftsent->fts_statp->st_rdev);
            el->minor = minor(ftsent->fts_statp->st_rdev);
        }
    }

    free(permission);

    return 0;
}

int
allocateFile(int maxSize, int argc, char **argv, 
                struct OPT *options, char **directories) {
    int initialErrorIndex=0, initialFileIndex=0;
    
    char *errors[maxSize]; 
    int  *errorIndex = &initialErrorIndex;

    char *files[maxSize];
    int  *fileIndex = &initialFileIndex;

    struct stat stats;
    int index = 0;

    for (int i=optind ; i < argc; i++) {
        if(lstat(argv[i], &stats) != 0) {
            char *error = malloc(strlen(argv[i]) + strlen(strerror(errno)) + 3);
            if (error == NULL) {
                continue;
            }
            error[0] = '\0';
            (void) strcat(error, argv[i]);
            (void) strcat(error, ": ");
            (void) strcat(error, strerror(errno));
            errors[*errorIndex] = strdup(error);
            (*errorIndex)++;
            (void) free(error);
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

    (void) printErrors(errors, errorIndex);
    
    if (*fileIndex > 0) {
        files[*fileIndex] = '\0';
        (void) readDir(files, options, 0, 1);
    }

    return index;
}

/**
 * Validate the blocksize set in the environment
 **/
void
checkBlockSize(struct OPT *options) {
    if (options->printBlockSizeInK) {
        options->blocksize = 1024;
        return;
    }

    long blockSize = getBlockSize();

    if (blockSize < 0) {
        char *env = getenv("BLOCKSIZE");
        const char *programName = getprogname();

        if (blockSize == -1) {
            options->blocksize = 512;
            if (options->printStat || options->printBlockSize) {
                fprintf(stderr, "%s: %s: unknown blocksize \n", programName, env);
                fprintf(stderr, "%s: maximum blocksize is 1G \n", programName);
                fprintf(stderr, "%s: %s: minimum blocksize is 512 \n", programName, env);
            }
        } else if (blockSize == -2) {
            options->blocksize = 512;
            if (options->printStat || options->printBlockSize) {
                fprintf(stderr, "%s: %s: minimum blocksize is 512 \n", programName, env);
            }
        } else if (blockSize == -3) {
            options->blocksize = 1024 * 1024 *1024;
            if (options->printStat || options->printBlockSize) {
                fprintf(stderr, "%s: maximum blocksize is 1G \n", programName);
            }
        } else {
             options->blocksize = 512;
        }

    } else {
        options->blocksize = blockSize;
    }
    
}