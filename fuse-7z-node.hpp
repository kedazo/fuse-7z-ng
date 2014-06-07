#ifndef FUSE_7Z_NODE_H
#define FUSE_7Z_NODE_H

#include <string>
#include <vector>
#include <list>
#include <sys/stat.h>
#include <map>
#include <cstring>

class Node;

class NodeBuffer {
	public:
	NodeBuffer() {}
	virtual ~NodeBuffer() {}
};

struct ltstr {
    bool operator() (const char* s1, const char* s2) const {
        return strcmp(s1, s2) < 0;
    }
};

typedef std::map <const char *, Node*, ltstr> nodelist_t;

class Node {
	private:
	static int max_inode;

	enum nodeState {
		CLOSED,
		OPENED,
		CHANGED,
		NEW
	};

	int open_count;
	nodeState state;

	public:
	NodeBuffer *buffer;

	char const *name;
	std::string sname;
	bool is_dir;
	int id;
	nodelist_t childs;
	Node *parent;
	struct stat stat;

	std::string fullname() const;

	void parse_name(char const *fname);
	void attach();

	public:
	static const int ROOT_NODE_INDEX, NEW_NODE_INDEX;

	Node(Node * parent, char const * name);
	~Node();


	Node * find(char const *);
	Node * insert(char * leaf);

};

#endif // FUSE_7Z_NODE_H

