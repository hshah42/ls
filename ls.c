#include "ls.h"

int
main(int argc, char **argv) {
    setprogname(argv[0]);
    
    struct OPT *options = malloc(sizeof(struct OPT));

    setOptions(argc, argv, options);
    
    char *errors[argc];
    int errorIndex = 0;
    for (int i=optind ; i<argc; i++) {
        readDir(argv[i], options);
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
    struct elements el;

    if((dir = opendir(pathname)) == NULL) {
        return -1; 
    }

    while(content = readdir(dir)) {
        if(strncmp(content->d_name, ".", 1)) {
            fprintf(stdout, "%s\n", content->d_name); 
        }
        else
            if(options->listAllFlag) {
                fprintf(stdout, "%s\n", content->d_name);   
            }
    }

    (void) closedir(dir);
     
    return 0;
}