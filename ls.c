#include "ls.h"

int
main(int argc, char **argv) {
    setprogname(argv[0]);
    int initialErrorIndex=0, initialFileIndex=0, initialDirectoryIndex = 0;
    
    struct OPT *options = malloc(sizeof(struct OPT));

    setOptions(argc, argv, options);

    int maxSize = argc - optind; 
    
    char *errors[maxSize];
    int *errorIndex = &initialErrorIndex;

    char *files[maxSize];
    int *fileIndex = &initialFileIndex;

    char *directories[maxSize];
    int *directoryIndex = &initialDirectoryIndex;

    for (int i=optind ; i<argc; i++) {
        allocateFileType(argv[i], errors, errorIndex, files, fileIndex, directories, directoryIndex);
    }

    if(optind == argc) {

    }
    else
    {
        printErrors(errors, errorIndex);
        printFile(files, fileIndex);
        for(int i = 0; i < *directoryIndex; i++) {
            readDir(directories[i], options);
        }
    }
    
}

void
setOptions(int argc, char **argv, struct OPT *options) {
    int opt;

    while((opt = getopt(argc, argv, "ai")) != -1) {
        switch (opt) {
        case 'a':
            options->listAllFlag = 1;
            break;
        case 'i':
            options->printInode = 1;
            break;
        case '?':
            fprintf(stderr, "Invalid Parameter");
        default:
            break;
        }
    }
}

int
readDir(char *pathname, struct OPT *options) {
    DIR *dir;
    struct dirent *content;
    struct elements el = getDefaultStruct();

    if((dir = opendir(pathname)) == NULL) {
        return -1; 
    }

    printDirectory(pathname);

    while(content = readdir(dir)) {
        if(strncmp(content->d_name, ".", 1)) {
            el.name = content->d_name;
            printLine(el);
        }
        else
            if(options->listAllFlag) {
                el.name = content->d_name;
                printLine(el);
            }
    }

    printf("\n");

    (void) closedir(dir);
     
    return 0;
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