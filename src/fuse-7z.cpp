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

#define ERROR_STR_BUF_LEN 0x100

using namespace std;


Fuse7z::Fuse7z(std::string const & archiveName, std::string const & cwd) :
 archive_fn(archiveName), cwd(cwd) {
	root_node = new Node(NULL, "");
	root_node->is_dir = true;
}

Fuse7z::~Fuse7z() {
	delete root_node;
}


