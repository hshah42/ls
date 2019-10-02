#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <stdio.h>
#include <err.h>
#include <string.h>     
#include <errno.h>     
#include <stdlib.h>

int compareSize(const FTSENT **fileOne, const FTSENT **fileTwo);

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

    while ((ftsent = fts_read(fts)) != NULL)
    {
        if(ftsent->fts_level == 0) {
            if(ftsent->fts_info == FTS_DP)
                continue;
            printf("\n");
            fprintf(stdout, "%s:\n", ftsent->fts_path);
            continue;
        }

        if(ftsent->fts_info == FTS_DP || ftsent->fts_level > 1){
            fts_set(fts, ftsent, FTS_SKIP);
            continue;
        }
        
        fprintf(stdout, "%lld %s %s \n", ftsent->fts_statp->st_size,ftsent->fts_path, ftsent->fts_name);
    }
    
    
    fts_close(fts);

    return 0;
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