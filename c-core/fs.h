#ifndef FS_H
#define FS_H

#include <stdbool.h>
#include <stdio.h>

#define FS_PATH_MAX 512

int fs_open(int *fd, char const *file, int flags, int mode);
int fs_close(int);
int fs_fopen(FILE **, const char *restrict, const char *restrict);
int fs_fclose(FILE *);
int fs_seek(int fd, long offset, int whence);
int fs_copy(int dst, int src);
int fs_fcopy(FILE *restrict dst, FILE *restrict src);
int fs_reopen(int fd, int mode, int *out);
int fs_dup(int fd, int *out);
int fs_chksum(char const *filepath, long *chk);
int fs_mkdir(char const *dirpath, bool exist_ok);
int fs_rmdir(char const *dirpath);
int fs_rmfile(char const *filepath);
int fs_touch(char const *filepath);
int fs_rmtree(char const *dirpath);
int fs_size(char const *filepath, long *size);
int fs_mkstemp(int *fd, char const *template);

#endif
