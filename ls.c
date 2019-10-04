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
        char **dir = malloc(2);
        dir[0] = "./";
        readDir(dir, options, 0);
        free(dir);
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

/**
 * setOptions takes in the arguments passed an gets the
 * optional parameters set by the user to enable flags that indicate
 * the flow of the program.
 * 
 **/
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

/**
 * Reading the directory specified in the arguments
 * This method will call the subsiquent methods that print the
 * directories depending on the optional parameters passed by the
 * user.
 * 
 **/
int
readDir(char **files, struct OPT *options, int isDirnameRequired) {
    FTS *fts;
    FTSENT *ftsent;
    int flags = FTS_NOCHDIR | FTS_PHYSICAL;

    if(options->listAllFlag) {
        flags = flags | FTS_SEEDOT;
    }
    
    if((fts = fts_open(files, flags, NULL)) == NULL) {
        printError(strerror(errno));
        return 1;
    }

    performLs(fts, ftsent, options, isDirnameRequired);    
    
    fts_close(fts);

    return 0;
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
performLs(FTS *fts, FTSENT *ftsent, struct OPT *options, int isDirnameRequired) {
    int falseInit = 0;
    // This variable determines when to print the contents and
    // when to get the max values for printing.
    int *shouldPrintContent = &falseInit;
    struct maxsize max;

    while ((ftsent = fts_read(fts)) != NULL) {
        FTSENT* node = fts_children(fts, 0);

        if (ftsent->fts_level > 1 && !(options->recurse)) {
            fts_set(fts, ftsent, FTS_SKIP);
            continue;
        }

        if (node == NULL) {
            continue;
        }
            
        if (node->fts_level > 1 && !(options->recurse)) {
            continue;
        }
        
        if (*shouldPrintContent) {
            if(isDirnameRequired || options->recurse) {
                printNewLine();
                printDirectory(node->fts_parent->fts_path);
            }
        }
        else
            max = getDefaultMaxSizeStruct();
        
        FTSENT* directory = node->fts_parent;

        while (node != NULL)
        {
            if (*shouldPrintContent) {
                if(shouldPrint(options, node)) {
                    struct elements el = getDefaultStruct();
                    
                    if(S_ISLNK(node->fts_statp->st_mode) && options->printStat) {
                        if (addLinkName(node, &el) != 0) {
                            printError(strerror(errno));
                            break;
                        }
                    } else {
                        el.name = node->fts_name; 
                    }               
                    
                    generateElement(node->fts_path, &el, options, node);
                    printLine(el, max);
                }
            } else {
                if (shouldPrint(options, node)) {
                   max = generateMaxSizeStruct(node, max); 
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

/**
 * Creating a symbolic link name when encountered and adding it to
 * element struct.
 * 
 **/
int
addLinkName(FTSENT *node, struct elements *el) {
    char linkname[PATH_MAX];
    char pathname[node->fts_namelen + node->fts_pathlen + 1];
    strcat(pathname, node->fts_path);
    strcat(pathname, node->fts_name);
    ssize_t len;

    if ((len = readlink(pathname, linkname, sizeof(linkname) - 1)) == -1) {
        printError(strerror(errno));
        return 1;
    }

    linkname[len] = '\0';
    char fullLink[strlen(linkname) + strlen(node->fts_name) + 1];
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
generateMaxSizeStruct(FTSENT *node, struct maxsize max) {
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

    return max;
}

int
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

    if (options->printInode) {
        el->inode = &(ftsent->fts_statp->st_ino);
    }

    if (options->printStat) {
        if(lstat(path, &stats) != 0) {
            printError(strerror(errno));
            return 1;
        }
        
        el->hardlinks = &(ftsent->fts_statp->st_nlink);
        if ((userInfo = getpwuid(ftsent->fts_statp->st_uid)) == NULL) {
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
        
        if ((groupInfo = getgrgid(ftsent->fts_statp->st_gid)) == NULL) {
            int size = getNumberOfDigits(ftsent->fts_statp->st_gid);
            char *group = malloc(size + 1);
            if(group == NULL) {
                printError(strerror(errno));
                return 1;
            }
            sprintf(group, "%d", ftsent->fts_statp->st_gid);
            el->group = strdup(group);
            free(group);
        } else {
            el->group = groupInfo->gr_name;
        }
        el->size = &(ftsent->fts_statp->st_size);
        int hasSize = 1;
        el->hasSize = &hasSize;
        el->time = &(ftsent->fts_statp->st_mtimespec.tv_sec);
        strmode(ftsent->fts_statp->st_mode, permission);
        el->strmode = strdup(permission);
    }

    free(permission);

    return 0;
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