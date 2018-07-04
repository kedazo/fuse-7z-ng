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
#include "node.h"
#include "logger.h"
#include "fuse7zstream.h"

#include <string>
#include <lib7zip.h>

class Fuse7z
{
	C7ZipLibrary lib;
	C7ZipArchive * archive;
	Fuse7zInStream stream;

	public:
	Fuse7z(std::string const & filename, std::string const & cwd);

	virtual ~Fuse7z();

	virtual void open(char const * path, Node * node);

	virtual void close(char const * path, Node * node);

	virtual int read(char const * path, Node * node, char * buf, size_t size, off_t offset);

    // FIXME: these must be private
    public:
	std::string const archive_fn;
	std::string const cwd;
	Node * root_node;
};

