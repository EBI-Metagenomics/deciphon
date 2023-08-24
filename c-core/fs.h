#ifndef DECIPHON_FS_H
#define DECIPHON_FS_H

#include <stdbool.h>
#include <stdio.h>

int dcp_fs_tell(FILE *restrict, long *offset);
int dcp_fs_seek(FILE *restrict, long offset, int whence);
int dcp_fs_copy(FILE *restrict dst, FILE *restrict src);
int dcp_fs_refopen(FILE *restrict, char const *mode, FILE **out);
int dcp_fs_getpath(FILE *restrict, unsigned size, char *filepath);
int dcp_fs_close(FILE *restrict);
int dcp_fs_readall(char const *filepath, long *size, unsigned char **data);
int dcp_fs_tmpfile(FILE **out);
int dcp_fs_copyp(FILE *restrict dst, FILE *restrict src);
int dcp_fs_cksum(char const *filepath, long *chk);
int dcp_fs_mkdir(char const *dirpath, bool exist_ok);
int dcp_fs_rmdir(char const *dirpath);
int dcp_fs_rmfile(char const *filepath);
int dcp_fs_touch(char const *filepath);
int dcp_fs_rmtree(char const *dirpath);
int dcp_fs_size(char const *filepath, long *size);
int dcp_fs_mkstemp(FILE **fp, char const *template);

#endif
