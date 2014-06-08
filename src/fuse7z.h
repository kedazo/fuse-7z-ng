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
#include "fuse-7z-node.h"
#include "fuse7zstream.h"
#include "logger.h"

#include <string>
#include <lib7zip.h>

class Fuse7z
{
	C7ZipLibrary lib;
	C7ZipArchive * archive;
	Fuse7zInStream stream;

	public:
	Fuse7z(std::string const & filename, std::string const & cwd) :
         stream (filename),
         archive_fn (filename),
         cwd (cwd)
    {
	    root_node = new Node(NULL, "");
    	root_node->is_dir = true;

        Logger &logger = Logger::instance ();
		logger << "Initialization of fuse-7z with archive " << filename << Logger::endl;

		if (!lib.Initialize()) {
			throw std::runtime_error("7z library initialization failed. Is the 7z.so folder in LD_LIBRARY_PATH?");
		}

		WStringArray exts;

		if (!lib.GetSupportedExts(exts)) {
			std::stringstream ss;
			ss << "Could not get list of 7z-supported file extensions";
			throw std::runtime_error(ss.str());
		}


		logger << "Supported extensions : ";
		size_t size = exts.size();
		for(size_t i = 0; i < size; i++) {
			std::wstring ext = exts[i];


			for(size_t j = 0; j < ext.size(); j++) {
				logger << (char)(ext[j] &0xFF);
			}
			logger << " ";

		}
		logger << Logger::endl;


		if (lib.OpenArchive(&stream, &archive)) {
			unsigned int numItems = 0;

			archive->GetItemCount(&numItems);

			logger << "Archive contains " << numItems << " entries" << Logger::endl;
			Node * node = 0;
			for(unsigned int i = 0;i < numItems;i++) {
				C7ZipArchiveItem * pArchiveItem = NULL;
				if (archive->GetItemInfo(i, &pArchiveItem)) {
					std::wstring wpath(pArchiveItem->GetFullPath());
					std::string path;
					path.resize(wpath.length());
					std::copy(wpath.begin(), wpath.end(), path.begin());

					node = root_node->insert(const_cast<char*>(path.c_str()));
					node->id = i;

					node->is_dir = pArchiveItem->IsDir();

					{
						unsigned long long size;
						pArchiveItem->GetUInt64Property(lib7zip::kpidSize, size);
						node->stat.st_size = size;
					}

					{
						unsigned long long secpy, time, bias, gain;
						secpy = 31536000;
						gain = 10000000ULL;
						bias = secpy * gain * 369 + secpy * 2438356ULL + 5184000ULL;
						pArchiveItem->GetFileTimeProperty(lib7zip::kpidATime, time);
						node->stat.st_atim.tv_sec = (time - bias)/gain;
						node->stat.st_atim.tv_nsec = ((time - bias) % gain) * 100;
						pArchiveItem->GetFileTimeProperty(lib7zip::kpidCTime, time);
						node->stat.st_ctim.tv_sec = (time - bias)/gain;
						node->stat.st_ctim.tv_nsec = ((time - bias) % gain) * 100;
						pArchiveItem->GetFileTimeProperty(lib7zip::kpidMTime, time);
						node->stat.st_mtim.tv_sec = (time - bias)/gain;
						node->stat.st_mtim.tv_nsec = ((time - bias) % gain) * 100;
					}
				}

				if (node && ((i+1) % 10000 == 0)) {
					logger << "Indexed " << (i+1) << "th file : " << node->fullname() << Logger::endl;
				}
			}
		}
		else {
			std::stringstream ss;
			ss << "open archive " << archive_fn  << " failed" << std::endl;
			throw std::runtime_error(ss.str());
		}
	}

	virtual ~Fuse7z() {
		delete archive;
		lib.Deinitialize();

        delete root_node;
	}

	virtual void open(char const * path, Node * node) {
        Logger &logger = Logger::instance ();
		logger << "Opening file " << path << "(" << node->fullname() << ")" << Logger::endl;
		Fuse7zOutStream * stream = new Fuse7zOutStream;
		node->buffer = stream;
		stream->SetSize(node->stat.st_size);
		int id = node->id;
		archive->Extract(id, stream);
	}

	virtual void close(char const * path, Node * node) {
        Logger &logger = Logger::instance ();
		logger << "Closing file " << path << "(" << node->fullname() << ")" << Logger::endl;
		Fuse7zOutStream * stream = dynamic_cast<Fuse7zOutStream*>(node->buffer);
		delete stream;
		node->buffer = NULL;
	}

	virtual int read(char const * path, Node * node, char * buf, size_t size, off_t offset) {
        Logger &logger = Logger::instance ();
		logger << "Reading file " << path << "(" << node->fullname() << ") for " << size << " at " << offset << ", arch_id=" << node->id << Logger::endl;
		Fuse7zOutStream * stream = dynamic_cast<Fuse7zOutStream*>(node->buffer);
		memcpy(buf, &stream->buffer[offset], size);
		return size;
	}

    // FIXME: these must be private
    public:
	std::string const archive_fn;
	std::string const cwd;
	Node * root_node;
};

