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
#include "fuse_functions.h"
#include "fuse7z.h"
#include "fuse-7z-node.h"
#include "logger.h"

#include <unistd.h>
#include <sys/types.h>

#define STANDARD_BLOCK_SIZE 512

inline Fuse7z *get_data ()
{
    return (Fuse7z*) fuse_get_context ()->private_data;
}

void *
fuse7z_initlib (char const * archive, char const * cwd)
{
    void * lib = new Fuse7z(archive, cwd);
    return lib;
}

void *
fuse7z_init (struct fuse_conn_info *conn)
{
    (void) conn;
    Fuse7z *data = get_data();
    return data;
}

void
fuse7z_destroy (void *_data)
{
    Fuse7z *data = (Fuse7z *)_data;
    delete data;
    Logger::instance() << "File system unmounted" << Logger::endl;
}

int
fuse7z_getattr (const char *path, struct stat *stbuf)
{
    Fuse7z *data = get_data();

    if (*path == '\0') {
        return -ENOENT;
    }

    Node * node = data->root_node->find(path + 1);
    if (node == NULL) {
        return -ENOENT;
    }

    //Logger::instance() << "Getattr " << node->fullname() << Logger::endl;

    memcpy(stbuf, &node->stat, sizeof(struct stat));

    if (node->is_dir) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2 + node->childs.size();
        stbuf->st_size = node->childs.size();
    } else {
        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_nlink = 1;
    }

    stbuf->st_blksize = STANDARD_BLOCK_SIZE;
    stbuf->st_blocks = (stbuf->st_size + STANDARD_BLOCK_SIZE - 1) / STANDARD_BLOCK_SIZE;
    stbuf->st_uid = geteuid();
    stbuf->st_gid = getegid();

    return 0;
}

int
fuse7z_readdir (
        const char              *path,
        void                    *buf,
        fuse_fill_dir_t          filler,
        off_t                    offset,
        struct fuse_file_info   *fi)
{
    Fuse7z *data = get_data();

    if (*path == '\0') {
        return -ENOENT;
    }

    //Logger::instance() << "Reading directory[" << path << "]" << Logger::endl;

    Node * node = data->root_node->find(path + 1);
    if (node == NULL) {
        return -ENOENT;
    }

    //Logger::instance().logger(node->fullname());

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    for (nodelist_t::const_iterator i = node->childs.begin(); i != node->childs.end(); ++i) {
        Node * node = i->second;
        filler(buf, node->name, NULL, 0);
    }

    return 0;
}

int
fuse7z_statfs (
        const char      *path,
        struct statvfs  *buf)
{
    (void) path;

    struct statvfs st;
    int err;
    if ((err = statvfs(get_data()->cwd.c_str(), &st)) != 0) {
        return -err;
    }
    buf->f_bavail = buf->f_bfree = st.f_frsize * st.f_bavail;

    buf->f_bsize = 1;
    buf->f_blocks = buf->f_bavail + 0;

    buf->f_ffree = 0;
    buf->f_favail = 0;

    buf->f_files = -1; // TODO get_data()->files.size() - 1;
    buf->f_namemax = 255;

    return 0;
}

int
fuse7z_open (
        const char              *path,
        struct fuse_file_info   *fi)
{
    Fuse7z *data = get_data();
    if (*path == '\0') {
        return -ENOENT;
    }
    Node *node = data->root_node->find(path + 1);
    if (node == NULL) {
        return -ENOENT;
    }
    if (node->is_dir) {
        return -EISDIR;
    }
    fi->fh = (uint64_t)node;

    try {
        data->open(path, node);
        return 0;
    }
    catch (std::bad_alloc&) {
        return -ENOMEM;
    }
    catch (std::runtime_error&) {
        return -EIO;
    }
}

int
fuse7z_create (
        const char             *path,
        mode_t                  mode,
        struct fuse_file_info  *fi)
{
    return -ENOTSUP;

}

int
fuse7z_read (
        const char              *path,
        char                    *buf,
        size_t                   size,
        off_t                    offset,
        struct fuse_file_info   *fi)
{
    Fuse7z *data = get_data();
    return data->read(path, (Node*)fi->fh, buf, size, offset);
}

int
fuse7z_write (
        const char              *path,
        const char              *buf,
        size_t                   size,
        off_t                    offset,
        struct fuse_file_info   *fi)
{
    return -ENOTSUP;
}

int fuse7z_release (const char *path, struct fuse_file_info *fi) {
    Fuse7z *data = get_data();
    try {
        data->close(path, (Node*)fi->fh);
        return 0;
    }
    catch(...) {
        return -ENOMEM;
    }
}

int
fuse7z_ftruncate (
        const char *path, off_t offset, struct fuse_file_info *fi)
{
    return -ENOTSUP;
}

int
fuse7z_truncate (const char *path, off_t offset)
{
    return -ENOTSUP;
}

int
fuse7z_unlink (const char *path)
{
    return -ENOTSUP;
}

int
fuse7z_rmdir (const char *path)
{
    return -ENOTSUP;
}

int
fuse7z_mkdir (const char *path, mode_t mode)
{
    return -ENOTSUP;
}

int
fuse7z_rename (const char *path, const char *new_path)
{
    return -ENOTSUP;
}

int
fuse7z_utimens (const char *path, const struct timespec tv[2])
{
    Fuse7z *data = get_data();
    if (*path == '\0') {
        return -ENOENT;
    }
    Node *node = data->root_node->find(path + 1);
    if (node == NULL) {
        return -ENOENT;
    }
    node->stat.st_mtime = tv[1].tv_sec;
    return 0;
}

int
#if ( __FreeBSD__ >= 10 )
fuse7z_setxattr (
        const char *, const char *, const char *, size_t, int, uint32_t)
#else
fuse7z_setxattr (
        const char *, const char *, const char *, size_t, int)
#endif
{
    return -ENOTSUP;
}

int
#if ( __FreeBSD__ >= 10 )
fuse7z_getxattr (
        const char *, const char *, char *, size_t, uint32_t)
#else
fuse7z_getxattr (
        const char *, const char *, char *, size_t)
#endif
{
    return -ENOTSUP;
}

int
fuse7z_listxattr (const char *, char *, size_t)
{
    return -ENOTSUP;
}

int
fuse7z_removexattr (const char *, const char *)
{
    return -ENOTSUP;
}

int
fuse7z_chmod (const char *, mode_t)
{
    return -ENOTSUP;
}

int
fuse7z_chown (const char *, uid_t, gid_t)
{
    return -ENOTSUP;
}

int
fuse7z_flush (const char *, struct fuse_file_info *)
{
    return 0;
}

int
fuse7z_fsync (const char *, int, struct fuse_file_info *)
{
    return 0;
}

int
fuse7z_fsyncdir (const char *, int, struct fuse_file_info *)
{
    return 0;
}

int
fuse7z_opendir (const char *, struct fuse_file_info *)
{
    return 0;
}

int
fuse7z_releasedir (const char *, struct fuse_file_info *)
{
    return 0;
}

int
fuse7z_access(const char *, int)
{
    return 0;
}

