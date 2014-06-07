#include "fuse-7z-node.h"
#include "fuse-7z.h"

#include <cstring>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <iostream>

using namespace std;

const int Node::ROOT_NODE_INDEX = -1;
const int Node::NEW_NODE_INDEX = -2;
int Node::max_inode = 0;

Node::Node(Node * parent, char const * name) : parent(parent), sname(name), is_dir(false), open_count(0), state(CLOSED) {
	buffer = NULL;
	this->name = sname.c_str();
	stat.st_ino = max_inode++;
}

Node::~Node() {
	delete buffer;

	for (nodelist_t::iterator i = childs.begin(); i != childs.end(); i++) {
		delete i->second;
	}
}

Node * Node::insert(char * path) {
	//logger << "Inserting " << path << " in " << fullname() << "..." << Logger::endl;
	char * path2 = path;
	bool dir = false;
	do {
		if (*path2 == '/') {
			dir = true;
			*path2 = '\0';
		}
	} while(*path2++);

	if (dir) {
		nodelist_t::iterator i = childs.find(path);
		if (i != childs.end())
			return i->second->insert(path2);
		// not found
		//logger << "new subdir" << path << Logger::endl;
		Node * child = new Node(this, path);
		childs[child->name] = child;
		child->is_dir = true;
		return child->insert(path2);
	}
	else {
		nodelist_t::iterator i = childs.find(path);
		if (i != childs.end()) {
			Node * child = i->second;
			//logger << "When inserting " << path << " in " << fullname() << ", found that it already exists !" << Logger::endl;
			return child;
		}
		//logger << "leaf " << path << Logger::endl;
		Node * child = new Node(this, path);
		childs[child->name] = child;
		return child;
	}
}

std::string Node::fullname() const {
	if (parent == NULL) {
		return sname;
	}
	if (parent->parent == NULL) {
		return sname;
	}
	return parent->fullname() + "/" + sname;
}

Node * Node::find(char const * path) {
	//logger << "Finding " << path << " from " << fullname() << Logger::endl;
	if (*path == '\0') {
		return this;
	}

	char const * path2 = path;
	bool sub = false;
	do {
		if (*path2 == '/') {
			sub = true;
			break;
		}
	} while(*path2++);

	if (sub) {
		char pouet[255];
		strncpy(pouet, path, path2-path);
		pouet[path2-path] = '\0';
		//cout << "Checking for " << pouet << " in subdirs..." << flush;
		nodelist_t::iterator i = childs.find(pouet);
		if (i != childs.end()) {
			Node * child = i->second;
			path2++;
			//cout << " ok, looking for " << path2 << " in subdir " << child->name << endl;
			return child->find(path2);
		}
		//cout << "no" << endl;
		return NULL;
	}
	else {
		nodelist_t::iterator i = childs.find(path);
		if (i != childs.end()) {
			Node * child = i->second;
			return child;
		}
		//cerr << "Child " << path << " not found in " << fullname() << " but it is not" << endl;
		return NULL;
	}
}
