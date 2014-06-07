
const int Node::ROOT_NODE_INDEX = -1;
const int Node::NEW_NODE_INDEX = -2;

Node::Node(Node * parent, char const * name) : parent(parent), sname(name), is_dir(false), open_count(0), state(CLOSED) {
	this->name = sname.c_str();
}

Node::~Node() {
	delete buffer;

	for (nodelist_t::iterator i = childs.begin(); i != childs.end(); i++) {
		delete *i;
	}
}

Node * Node::insert(char * path) {
	char * path2 = path;
	bool dir = false;
	do {
		if (*path2 == '/') {
			dir = true;
			*path2 = '\0';
		}
	} while(*path2++);

	if (dir) {
		for (nodelist_t::iterator i = childs.begin(); i != childs.end(); i++) {
			Node * child = *i;
			if (child->sname == path) {
				return child->insert(path2);
			}
		}
		// not found
		Node * child = new Node(this, path);
		childs.push_back(child);
		return child->insert(path2);
	}
	else {
		for (nodelist_t::iterator i = childs.begin(); i != childs.end(); i++) {
			Node * child = *i;
			if (child->sname == path) {
				stringstream ss;
				ss << "File " << path << " already exists !";
				throw runtime_error(ss.str());
			}
		}
		Node * child = new Node(this, path);
		return child;
	}
}

std::string Node::fullname() const {
	if (parent != NULL) {
		return sname;
	}
	return parent->fullname() + "/" + sname;
}

Node * Node::find(char const * path) const {
	string s(path);
	char const * path2 = path;
	bool sub = false;
	do {
		if (*path2 == '/') {
			sub = true;
		}
	} while(*path2++);

	if (sub) {
		for (nodelist_t::const_iterator i = childs.begin(); i != childs.end(); i++) {
			Node * child = *i;
			if (s.find(child->sname) == 0) {
				return child->find(path2);
			}
		}
		stringstream ss;
		ss << "Expected " << path << " to be a child of " << fullname() << " but it is not";
		throw runtime_error(ss.str());
	}
	else {
		for (nodelist_t::const_iterator i = childs.begin(); i != childs.end(); i++) {
			Node * child = *i;
			if (child->sname == path) {
				return child;
			}
		}
		stringstream ss;
		ss << "Child " << path << " not found in " << fullname() << " but it is not";
		throw runtime_error(ss.str());
	}
}

#if 0
int Node::open() {
#if defined(FUSE7Z_WRITE)
	if (state == NEW) {
		return 0;
	}
	if (state == OPENED) {
		if (open_count == INT_MAX) {
			return -EMFILE;
		} else {
			++open_count;
		}
	}
	if (state == CLOSED) {
		open_count = 1;
#endif
		try {
			buffer = new BigBuffer(data, id, stat.st_size);
#if defined(FUSE7Z_WRITE)
			state = OPENED;
#endif
		}
		catch (std::bad_alloc&) {
			return -ENOMEM;
		}
		catch (std::runtime_error&) {
			return -EIO;
		}
#if defined(FUSE7Z_WRITE)
	}
#endif
	return 0;
}

void Node::rename(char *fname) {
	throw runtime_error("not implemented");
	parse_name(fname);
	parent->childs.remove(this);
}

void Node::rename_wo_reparenting(char *new_name) {
	//parse_name(new_name);
}

int Node::read(char *buf, size_t size, offset_t offset) const {
	return buffer->read(buf, size, offset);
}

#if defined(FUSE7Z_WRITE)
int Node::write(const char *buf, size_t size, offset_t offset) {
	if (state == OPENED) {
		state = CHANGED;
	}
	return buffer->write(buf, size, offset);
}
#endif

int Node::close() {
	stat.st_size = buffer->len;
	if (state == OPENED && --open_count == 0) {
		delete buffer;
		buffer = NULL;
		state = CLOSED;
	}
	if (state == CHANGED) {
		stat.st_mtime = time(NULL);
	}
	return 0;
}

#if 0
int Node::save() {

	return buffer->saveToZip(stat.st_mtime, fullname, state == NEW, id);
}
#endif

#if 0
int Node::truncate(offset_t offset) {
	if (state != CLOSED) {
		if (state != NEW) {
			state = CHANGED;
		}
		try {
			buffer->truncate(offset);
			return 0;
		}
		catch (const std::bad_alloc &) {
			return EIO;
		}
	} else {
		return EBADF;
	}
}
#endif

offset_t Node::size() const {
	if (state == NEW || state == OPENED || state == CHANGED) {
		return buffer->len;
	} else {
		return stat.st_size;
	}
}

#endif
