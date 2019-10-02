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

        allocateFile(maxSize, argc, argv, directories);
        
        if((argc - optind) == 1) {
            readDir(directories, options, 0);
        } else {
            readDir(directories, options, 1);
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

    while ((ftsent = fts_read(fts)) != NULL)
    {
        int shouldPrintLine = 0;

        FTSENT* node = fts_children(fts, 0);

        if(node == NULL)
            continue;

        if(node->fts_level > 1 && !(options->recurse))
            continue;
        
        printDirectory(node->fts_parent->fts_path);

        int shouldPrint = 0;

        while (node != NULL)
        {
            shouldPrintLine = 1;

            if(options->listAllFlag) {
                shouldPrint = 1;
            } else {
                if(strncmp(node->fts_name, ".", 1)) {
                    shouldPrint = 1;
                } else {
                    if(options->includeHiddenFiles) {
                        shouldPrint = 1;
                    }
                }
            }

            if(shouldPrint) {
                struct elements el = getDefaultStruct();
                el.name = node->fts_name;                
                generateElement(node->fts_path, &el, options, node);
                printLine(el);
                shouldPrintLine = 1;
            }

            node = node->fts_link;
        }

        if(shouldPrintLine)
            fprintf(stdout, "\n");
    }
    
    
    fts_close(fts);

    return 0;
}

int
generateElement(char *path, struct elements *el, struct OPT *options, FTSENT *ftsent) {
    struct stat stats;
    struct passwd *userInfo;
    struct group *groupInfo;
    char *permission = malloc(10);

    if(options->printInode) {
        el->inode = &(ftsent->fts_ino);
    }

    if(options->printStat) {
        if(lstat(path, &stats) != 0) {
            printError(strerror(errno));
            return 1;
        }
        
        el->hardlinks = &stats.st_nlink;
        userInfo = getpwuid(stats.st_uid);
        el->owner = userInfo->pw_name;
        groupInfo = getgrgid(stats.st_gid);
        el->group = groupInfo->gr_name;
        el->size = &stats.st_size;
        int hasSize = 1;
        el->hasSize = &hasSize;
        el->time = &stats.st_atime;
        strmode(stats.st_mode, permission);
        el->strmode = strdup(permission);
    }

    free(permission);
}

void
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
}