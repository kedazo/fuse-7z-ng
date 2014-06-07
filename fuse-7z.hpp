#ifndef FUSE_7ZIP_H
#define FUSE_7ZIP_H

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


#endif // FUSE_7ZIP_H
