#pragma once

#include <ctime>
#include <cstring>
#include <cstdlib>
#include <stdint.h>
#include <vector>
#include <string>
#include <list>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fuse.h>


#ifdef _LFS_LARGEFILE
typedef off64_t offset_t;
#else
typedef off_t offset_t;
#endif

class Node;
class Fuse7z;
class BigBuffer;

extern class Logger {
	public:
	std::stringstream ss;
	Logger();
	~Logger();
	void logger(std::string const & text);
	void err(std::string const & text);
	template<typename T>
	inline Logger & operator<<(T const & data) {
		ss << data;
		return *this;
	}
	static Logger &endl(Logger &f);

	typedef Logger& (*LoggerManipulator)(Logger&);

	Logger& operator<<(LoggerManipulator manip)
	{
		return manip(*this);
	}
} logger;

class Fuse7z {
	private:
	public:
	std::string const archive_fn;
	std::string const cwd;
	Node * root_node;

	public:
	Fuse7z(std::string const & filename, std::string const & cwd);
	virtual ~Fuse7z();

	virtual void open(char const * path, Node * n) = 0;
	virtual void close(char const * path, Node * n) = 0;
	virtual int read(char const * path, Node * node, char * buf, size_t size, off_t offset) = 0;

};

void * fuse7z_initlib(char const * archive, char const * cwd);
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

