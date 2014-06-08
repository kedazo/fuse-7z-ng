#include "fuse-7z.h"
#include "fuse-7z-node.h"

#include <cerrno>
#include <climits>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <queue>
#include <iostream>
#include <cassert>
#include <sstream>
#include <stdexcept>

#include <unistd.h>
#include <limits.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <linux/limits.h>

#define STANDARD_BLOCK_SIZE (512)
#define ERROR_STR_BUF_LEN 0x100

using namespace std;

struct Logger logger;

extern "C" int param;

Logger::Logger() {
	ss.str("");
	openlog("fuse-7z-ng", LOG_PID, LOG_USER);
}

Logger::~Logger() {
	closelog();
}

void Logger::logger(std::string const & text) {
	if (param) {
		syslog(LOG_INFO, "%s", text.c_str());
	}
	else {
		cout << "fuse-7z: " << text << std::endl;
	}
	ss.str("");
}
void Logger::err(std::string const & text) {
	if (param) {
		syslog(LOG_ERR, "%s", text.c_str());
	}
	else {
		cerr << text << std::endl;
	}
}

Logger & Logger::endl(Logger &f) {
	f.logger(f.ss.str());
	return f;
}

inline Fuse7z *get_data() {
	return (Fuse7z*)fuse_get_context()->private_data;
}

Fuse7z::Fuse7z(std::string const & archiveName, std::string const & cwd) :
 archive_fn(archiveName), cwd(cwd) {
	root_node = new Node(NULL, "");
	root_node->is_dir = true;
}

Fuse7z::~Fuse7z() {
	delete root_node;
}

void *fuse7z_init(struct fuse_conn_info *conn) {
	   (void) conn;
	   Fuse7z *data = get_data();
	   return data;
}

void fuse7z_destroy(void *_data) {
	   Fuse7z *data = (Fuse7z *)_data;
	   delete data;
	   logger << "File system unmounted" << Logger::endl;
}

int fuse7z_getattr(const char *path, struct stat *stbuf) {
	Fuse7z *data = get_data();

	if (*path == '\0') {
		return -ENOENT;
	}

	Node * node = data->root_node->find(path + 1);
	if (node == NULL) {
		return -ENOENT;
	}

	//logger << "Getattr " << node->fullname() << Logger::endl;

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

int fuse7z_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	Fuse7z *data = get_data();

	if (*path == '\0') {
		return -ENOENT;
	}

	//logger << "Reading directory[" << path << "]" << Logger::endl;

	Node * node = data->root_node->find(path + 1);
	if (node == NULL) {
		return -ENOENT;
	}

	//logger.logger(node->fullname());

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	for (nodelist_t::const_iterator i = node->childs.begin(); i != node->childs.end(); ++i) {
		Node * node = i->second;
		filler(buf, node->name, NULL, 0);
	}

	return 0;
}

int fuse7z_statfs(const char *path, struct statvfs *buf) {
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

int fuse7z_open(const char *path, struct fuse_file_info *fi) {
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

int fuse7z_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	return -ENOTSUP;

}

int fuse7z_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	Fuse7z *data = get_data();
	return data->read(path, (Node*)fi->fh, buf, size, offset);
}

int fuse7z_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
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

int fuse7z_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi) {
	return -ENOTSUP;
}

int fuse7z_truncate(const char *path, off_t offset) {
	return -ENOTSUP;
}

int fuse7z_unlink(const char *path) {
	return -ENOTSUP;
}

int fuse7z_rmdir(const char *path) {
	return -ENOTSUP;
}

int fuse7z_mkdir(const char *path, mode_t mode) {
	return -ENOTSUP;
}

int fuse7z_rename(const char *path, const char *new_path) {
	return -ENOTSUP;
}

int fuse7z_utimens(const char *path, const struct timespec tv[2]) {
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

#if ( __FreeBSD__ >= 10 )
int fuse7z_setxattr(const char *, const char *, const char *, size_t, int, uint32_t)
#else
int fuse7z_setxattr(const char *, const char *, const char *, size_t, int)
#endif
{
	return -ENOTSUP;
}

#if ( __FreeBSD__ >= 10 )
int fuse7z_getxattr(const char *, const char *, char *, size_t, uint32_t)
#else
int fuse7z_getxattr(const char *, const char *, char *, size_t)
#endif
{
	return -ENOTSUP;
}

int fuse7z_listxattr(const char *, char *, size_t) {
	return -ENOTSUP;
}

int fuse7z_removexattr(const char *, const char *) {
	return -ENOTSUP;
}

int fuse7z_chmod(const char *, mode_t) {
	return -ENOTSUP;
}

int fuse7z_chown(const char *, uid_t, gid_t) {
	return -ENOTSUP;
}

int fuse7z_flush(const char *, struct fuse_file_info *) {
	return 0;
}

int fuse7z_fsync(const char *, int, struct fuse_file_info *) {
	return 0;
}

int fuse7z_fsyncdir(const char *, int, struct fuse_file_info *) {
	return 0;
}

int fuse7z_opendir(const char *, struct fuse_file_info *) {
	return 0;
}

int fuse7z_releasedir(const char *, struct fuse_file_info *) {
	return 0;
}

int fuse7z_access(const char *, int) {
	return 0;
}

