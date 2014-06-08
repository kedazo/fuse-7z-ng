/*
 * This file is part of fuse-7z-ng.
 *
 * fuse-7z-ng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * fuse-7z-ng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fuse-7z-ng.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <fuse.h>

void *fuse7z_initlib(char const * archive, char const * cwd);
void *fuse7z_init(struct fuse_conn_info *conn);
void fuse7z_destroy(void *data);
int fuse7z_getattr(const char *path, struct stat *stbuf);
int fuse7z_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int fuse7z_statfs(const char *path, struct statvfs *buf);
int fuse7z_open(const char *path, struct fuse_file_info *fi);
int fuse7z_create(const char *path, mode_t mode, struct fuse_file_info *fi);
int fuse7z_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int fuse7z_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int fuse7z_release (const char *path, struct fuse_file_info *fi);
int fuse7z_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi);
int fuse7z_truncate(const char *path, off_t offset);
int fuse7z_unlink(const char *path);
int fuse7z_rmdir(const char *path);
int fuse7z_mkdir(const char *path, mode_t mode);
int fuse7z_rename(const char *path, const char *new_path);
int fuse7z_utimens(const char *path, const struct timespec tv[2]);
#if ( __FreeBSD__ >= 10 )
int fuse7z_setxattr(const char *, const char *, const char *, size_t, int, uint32_t);
int fuse7z_getxattr(const char *, const char *, char *, size_t, uint32_t);
#else
int fuse7z_setxattr(const char *, const char *, const char *, size_t, int);
int fuse7z_getxattr(const char *, const char *, char *, size_t);
#endif
int fuse7z_listxattr(const char *, char *, size_t);
int fuse7z_removexattr(const char *, const char *);
int fuse7z_chmod(const char *, mode_t);
int fuse7z_chown(const char *, uid_t, gid_t);
int fuse7z_flush(const char *, struct fuse_file_info *);
int fuse7z_fsync(const char *, int, struct fuse_file_info *);
int fuse7z_fsyncdir(const char *, int, struct fuse_file_info *);
int fuse7z_opendir(const char *, struct fuse_file_info *);
int fuse7z_releasedir(const char *, struct fuse_file_info *);
int fuse7z_access(const char *, int);

