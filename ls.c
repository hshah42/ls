#include "ls.h"

int
main(int argc, char **argv) {
    (void) setprogname(argv[0]);
    int initialErrorIndex=0, initialFileIndex=0, initialDirectoryIndex = 0;
    
    FTS *fts;
    FTSENT *ftsent;
    
    struct OPT *options = malloc(sizeof(struct OPT));
    if(options == NULL) {
        printError(strerror(errno));
        return 1;
    }

    setOptions(argc, argv, options);

    int maxSize = argc - optind; 
    
    char *errors[maxSize]; 
    int  *errorIndex = &initialErrorIndex;

    char *files[maxSize];
    int  *fileIndex = &initialFileIndex;

    char *directories[maxSize];
    int  *directoryIndex = &initialDirectoryIndex;

    for (int i=optind ; i<argc; i++) {
        allocateFileType(argv[i], errors, errorIndex, files, fileIndex, directories, directoryIndex);
    }

    if(optind == argc) {
        char *dir[1];
        dir[0] = ".";
        readDir(dir, options, 0);
    }
    else
    {
        char **files = malloc(argc * sizeof(char*));
        if (files == NULL) {
            printError(strerror(errno));
            return 1;
        }

        int index = 0;

        for(int i = optind; i < argc; i++) {
            files[index] = strdup(argv[i]);
            index++;
        }

        printErrors(errors, errorIndex);
        printFile(files, fileIndex);
        
        if((argc - optind) == 1) {
            readDir(files, options, 0);
        } else {
            readDir(files, options, 1);
        }

        free(files);
    }

    free(options);

    return 0;
}

void
setOptions(int argc, char **argv, struct OPT *options) {
    int opt;

    while((opt = getopt(argc, argv, "Aail")) != -1) {
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
        fprintf(stderr, "error: %s \n", strerror(errno));
        return 1;
    }

    while ((ftsent = fts_read(fts)) != NULL)
    {
        struct elements el = getDefaultStruct();

        if(ftsent->fts_level == 0) {
            if(ftsent->fts_info == FTS_DP)
                continue;
            printNewLine();
            printDirectory(ftsent->fts_path);
            continue;
        }

        if(ftsent->fts_info == FTS_DP || ftsent->fts_level > 1){
            fts_set(fts, ftsent, FTS_SKIP);
            continue;
        }

        int shouldPrint = 0;

        if(options->listAllFlag) {
            shouldPrint = 1;
        } else {
            if(strncmp(ftsent->fts_name, ".", 1)) {
                shouldPrint = 1;
            } else {
                if(options->includeHiddenFiles) {
                    shouldPrint = 1;
                }
            }
        }
        
        if(shouldPrint) {
            el.name = ftsent->fts_name;
            generateElement(ftsent->fts_path, &el, options, ftsent);
            printLine(el);
        }
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
        el->strmode = permission;
    }

    free(permission);
}

void
allocateFileType(char *pathname, char *errors[], int *errorIndex, char *files[], 
                int *fileIndex, char *directories[], int *directoryIndex) {
    struct stat stats;

    if(stat(pathname, &stats) != 0) {
        char *error = malloc(strlen(pathname) + strlen(strerror(errno)) + 3);
        error[0] = '\0';
        strcat(error, pathname);
        strcat(error, ": ");
        strcat(error, strerror(errno));
        errors[*errorIndex] = error;
        (*errorIndex)++;
        free(error);
        return;
    }

    if(S_ISDIR(stats.st_mode)) {
        directories[*directoryIndex] = pathname;
        (*directoryIndex)++;
    } else {
        files[*fileIndex] = pathname;
        (*fileIndex)++;
    }
}