#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <stdio.h>
#include <err.h>
#include <string.h>     
#include <errno.h>     
#include <stdlib.h>

int compareSize(const FTSENT **fileOne, const FTSENT **fileTwo);
void file_ls(FTS* file_system, int* flags, int *shouldRepeat);

int
main(int argc, char **argv) {
    char **files = malloc(argc * sizeof(char*));
    FTS *fts;
    FTSENT *ftsent;
    int (*compar)(const FTSENT **, const FTSENT **) = &compareSize;

    int flags = FTS_NOCHDIR;
    
    for(int i = 1; i < argc; i++) {
        files[i - 1] = strdup(argv[i]);
    }
    
    if((fts = fts_open(files, flags, compar)) == NULL) {
        fprintf(stderr, "error: %s \n", strerror(errno));
        return 1;
    }

    int false = 0;
    int *shouldPrint = &false;

    while ((ftsent = fts_read(fts)) != NULL)
    {
        // if(ftsent->fts_level == 0) {
        //     if(ftsent->fts_info == FTS_DP)
        //         continue;
        //     printf("\n");
        //     fprintf(stdout, "%s:\n", ftsent->fts_path);
        //     continue;
        // }

        // if(ftsent->fts_info == FTS_DP){
        //     fts_set(fts, ftsent, FTS_SKIP);
        //     continue;
        // }
        //printf("file %s \n", ftsent->fts_name);
        if (ftsent->fts_level > 1) {
            fts_set(fts, ftsent, FTS_SKIP);
            continue;
        }
        file_ls(fts, 0, shouldPrint);

        //fprintf(stdout, "innnnnn %lld %s\n", ftsent->fts_statp->st_size,ftsent->fts_name);
    }
    
    
    fts_close(fts);

    return 0;
}

void file_ls(FTS* file_system, int* flags, int *shouldPrint)
{
    FTSENT* node = fts_children(file_system, 0);

    if (errno != 0) {
         perror("fts_children");
    }
       
    int shouldPrintLine = 0;

    if(node == NULL)
        return;

    // if(node != NULL && *shouldPrint) {
    //     fprintf(stdout, "%s: \n", node->fts_parent->fts_path);
    // }
    
    if (node->fts_level > 1) {
        return;
    }

    FTSENT* directory = node->fts_parent;
    
    while (node != NULL)
    {
        // TODO use file_name and flags
        //printf("%d %d \n", node->fts_info, FTS_D);
        // long a = node->fts_statp->st_size;
        // printf("%d \n", a);
        if(*shouldPrint)
            printf("%s %d\n", node->fts_name, node->fts_statp->st_blocks);
        
        node = node->fts_link;
        shouldPrintLine = 1;
    }

    if(!(*shouldPrint))
        fts_set(file_system, directory, FTS_AGAIN);

    if(*shouldPrint) {
        *shouldPrint = 0;
    } else {
        *shouldPrint = 1;
    }

    if(shouldPrintLine)
        fprintf(stdout, "\n");

}

int 
compareSize(const FTSENT **fileOne, const FTSENT **fileTwo) {
    const FTSENT *file = *fileOne;
    const FTSENT *fileTest = *fileTwo;

    if(file->fts_statp->st_size > fileTest->fts_statp->st_size) {
        return -1;
    }
    return 1;
}