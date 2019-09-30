#include "ls.h"

int
main(int argc, char **argv) {
    setprogname(argv[0]);
    int initialErrorIndex=0, initialFileIndex=0, initialDirectoryIndex = 0;
    
    struct OPT *options = malloc(sizeof(struct OPT));

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
        readDir(".", options, 0);
    }
    else
    {
        printErrors(errors, errorIndex);
        printFile(files, fileIndex);
        
        for(int i = 0; i < *directoryIndex; i++) {
            if((argc - optind) == 1) {
                readDir(directories[i], options, 0);
            } else {
                readDir(directories[i], options, 1);
            }   
        }
    }
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
readDir(char *pathname, struct OPT *options, int isDirnameRequired) {
    DIR *dir;
    struct dirent *content;
    struct elements el = getDefaultStruct();

    if((dir = opendir(pathname)) == NULL) {
        return -1; 
    }

    if(isDirnameRequired)
        printDirectory(pathname);

    while(content = readdir(dir)) {
        if(strncmp(content->d_name, ".", 1)) {
            el.name = content->d_name;
            generateElement(pathname, &el, options);
            printLine(el);
        }
        else {
            if(strcmp(content->d_name, ".") == 0 || strcmp(content->d_name, "..") == 0) {
                if(options->listAllFlag) {
                    el.name = content->d_name;
                    generateElement(pathname, &el, options);
                    printLine(el);
                }
            }
            else if(options->listAllFlag || options->includeHiddenFiles) {
                el.name = content->d_name;
                generateElement(pathname, &el, options);
                printLine(el);
            }
        }
    }

    printNewLine();

    (void) closedir(dir);
     
    return 0;
}

int
generateElement(char *pathname, struct elements *el, struct OPT *options) {
    struct stat stats;
    struct passwd *userInfo;
    struct group *groupInfo;
    int mutiplier = 1;
    char *oldDirectory = NULL;

    char *fullPath = malloc(strlen(pathname) + strlen(el->name) + 2);
    fullPath[0] = '\0';
    strcat(fullPath, pathname);
    strcat(fullPath, "/");
    strcat(fullPath, el->name);

    if(lstat(fullPath, &stats) != 0) {
        printf("what: %s \n", el->name);
        printError(strerror(errno));
        return 1;
    }

    if(options->printInode) {
        el->inode = &stats.st_ino;
    }

    if(options->printStat) {
        el->hardlinks = &stats.st_nlink;
        userInfo = getpwuid(stats.st_uid);
        el->owner = userInfo->pw_name;
        groupInfo = getgrgid(stats.st_gid);
        el->group = groupInfo->gr_name;
        el->size = &stats.st_size;
        int hasSize = 1;
        el->hasSize = &hasSize;
        el->time = &stats.st_atime;
    }
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