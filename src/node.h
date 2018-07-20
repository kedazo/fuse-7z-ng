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

#include <string>
#include <cstring>

#include <list>
#include <sys/stat.h>
#include <map>

class NodeBuffer
{
    public:
        NodeBuffer() {}
        virtual ~NodeBuffer() {}
};

struct ltstr
{
    bool operator() (const char* s1, const char* s2) const
    {
        return strcmp(s1, s2) < 0;
    }
};

class Node;
typedef std::map <const char *, Node*, ltstr> nodelist_t;

class Node
{
    public:
        Node(Node * parent, char const * name);
        ~Node();

        static const int ROOT_NODE_INDEX, NEW_NODE_INDEX;

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

        Node * find(char const *);
        Node * insert(char * leaf);

    private:
        static unsigned int max_inode;

        enum nodeState {
            CLOSED,
            OPENED,
            CHANGED,
            NEW
        };

        int open_count;
        nodeState state;
};

