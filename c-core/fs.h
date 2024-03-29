#ifndef FS_H
#define FS_H

#include <stdbool.h>
#include <stdio.h>

#define FS_PATH_MAX 512

int fs_tell(FILE *restrict, long *offset);
int fs_seek(FILE *restrict, long offset, int whence);
int fs_copy(FILE *restrict dst, FILE *restrict src);
int fs_refopen(FILE *restrict, char const *mode, FILE **out);
int fs_close(FILE *restrict);
int fs_chksum(char const *filepath, long *chk);
int fs_mkdir(char const *dirpath, bool exist_ok);
int fs_rmdir(char const *dirpath);
int fs_rmfile(char const *filepath);
int fs_touch(char const *filepath);
int fs_rmtree(char const *dirpath);
int fs_size(char const *filepath, long *size);
int fs_mkstemp(FILE **fp, char const *template);

#endif
