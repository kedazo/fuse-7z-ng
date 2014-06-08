#include "fuse-7z.h"
#include "fuse-7z-node.h"

#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <sstream>

#include "lib7zip.h"

using namespace std;


class Fuse7zOutStream : public C7ZipOutStream, public NodeBuffer
{
	private:
	unsigned long long int position;

	public:
	vector<char> buffer;

	Fuse7zOutStream() : position(0) {
	}

	virtual ~Fuse7zOutStream() {
	}

	virtual int Write(const void *data, unsigned int size, unsigned int *processedSize) {
		logger << "Write " << data << " size=" << size << " processed " << processedSize << Logger::endl;
		memcpy(&buffer[position], data, size);
		*processedSize = size;
		position += size;
		return 0;
	}

	virtual int Seek(long long int offset, unsigned int seekOrigin, unsigned long long int *newPosition) {
		logger << "Seek " << offset << " " << seekOrigin << Logger::endl;
		position = seekOrigin;
		return 0;
	}

	virtual int SetSize(unsigned long long int size) {
		logger << "SetSize " << size << Logger::endl;
		buffer.resize(size);
		return 0;
	}
};

class Fuse7zInStream : public C7ZipInStream
{
private:
	FILE * m_pFile;
	std::string m_strFileName;
	std::wstring m_strFileExt;
	int m_nFileSize;
public:
	Fuse7zInStream(std::string const & fileName) : m_strFileName(fileName), m_strFileExt(L"7z") {

		m_pFile = fopen(fileName.c_str(), "rb");
		if (m_pFile) {
			fseek(m_pFile, 0, SEEK_END);
			m_nFileSize = ftell(m_pFile);
			fseek(m_pFile, 0, SEEK_SET);
			int pos = m_strFileName.find_last_of(".");
			if (pos != m_strFileName.npos) {
				m_strFileExt.resize(fileName.length() - pos);
				for (int i(0); i < m_strFileExt.length(); i++) {
					m_strFileExt[i] = m_strFileName[pos+1+i];
				}
			}
		}
		else {
			stringstream ss;
			ss << "Can't open " << fileName;
			throw runtime_error(ss.str());
		}
	}

	virtual ~Fuse7zInStream()
	{
		fclose(m_pFile);
	}

public:
	virtual std::wstring GetExt() const
	{
		return m_strFileExt;
	}

	virtual int Read(void *data, unsigned int size, unsigned int *processedSize)
	{
		if (!m_pFile)
			return 1;

		int count = fread(data, 1, size, m_pFile);
		if (count >= 0) {
			if (processedSize != NULL)
				*processedSize = count;

			return 0;
		}

		return 1;
	}

	virtual int Seek(long long int offset, unsigned int seekOrigin, unsigned long long int *newPosition)
	{
		if (!m_pFile)
			return 1;

		int result = fseek(m_pFile, (long)offset, seekOrigin);

		if (!result) {
			if (newPosition)
				*newPosition = ftell(m_pFile);

			return 0;
		}

		return result;
	}

	virtual int GetSize(unsigned long long int * size)
	{
		if (size)
			*size = m_nFileSize;
		return 0;
	}
};
const wchar_t * index_names[] = {
		L"kpidPackSize", //(Packed Size)
		L"kpidAttrib", //(Attributes)
		L"kpidCTime", //(Created)
		L"kpidATime", //(Accessed)
		L"kpidMTime", //(Modified)
		L"kpidSolid", //(Solid)
		L"kpidEncrypted", //(Encrypted)
		L"kpidUser", //(User)
		L"kpidGroup", //(Group)
		L"kpidComment", //(Comment)
		L"kpidPhySize", //(Physical Size)
		L"kpidHeadersSize", //(Headers Size)
		L"kpidChecksum", //(Checksum)
		L"kpidCharacts", //(Characteristics)
		L"kpidCreatorApp", //(Creator Application)
		L"kpidTotalSize", //(Total Size)
		L"kpidFreeSpace", //(Free Space)
		L"kpidClusterSize", //(Cluster Size)
		L"kpidVolumeName", //(Label)
		L"kpidPath", //(FullPath)
		L"kpidIsDir", //(IsDir)
};


class Fuse7z_lib7zip : public Fuse7z {
	C7ZipLibrary lib;
	C7ZipArchive * archive;
	Fuse7zInStream stream;
	public:

	Fuse7z_lib7zip(std::string const & filename, std::string const & cwd) : Fuse7z(filename, cwd), stream(filename) {
		logger << "Initialization of fuse-7z with archive " << filename << Logger::endl;

		if (!lib.Initialize()) {
			throw runtime_error("7z library initialization failed. Is the 7z.so folder in LD_LIBRARY_PATH?");
		}

		WStringArray exts;

		if (!lib.GetSupportedExts(exts)) {
			stringstream ss;
			ss << "Could not get list of 7z-supported file extensions";
			throw runtime_error(ss.str());
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
			Node * node;
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
				if ((i+1) % 10000 == 0) {
					logger << "Indexed " << (i+1) << "th file : " << node->fullname() << Logger::endl;
				}
			}
		}
		else {
			stringstream ss;
			ss << "open archive " << archive_fn  << " failed" << endl;
			throw runtime_error(ss.str());
		}

	}
	virtual ~Fuse7z_lib7zip() {
		delete archive;
		lib.Deinitialize();
	}

	virtual void open(char const * path, Node * node) {
		logger << "Opening file " << path << "(" << node->fullname() << ")" << Logger::endl;
		Fuse7zOutStream * stream = new Fuse7zOutStream;
		node->buffer = stream;
		stream->SetSize(node->stat.st_size);
		int id = node->id;
		archive->Extract(id, stream);
	}
	virtual void close(char const * path, Node * node) {
		logger << "Closing file " << path << "(" << node->fullname() << ")" << Logger::endl;
		Fuse7zOutStream * stream = dynamic_cast<Fuse7zOutStream*>(node->buffer);
		delete stream;
		node->buffer = NULL;
	}
	virtual int read(char const * path, Node * node, char * buf, size_t size, off_t offset) {
		logger << "Reading file " << path << "(" << node->fullname() << ") for " << size << " at " << offset << ", arch_id=" << node->id << Logger::endl;
		Fuse7zOutStream * stream = dynamic_cast<Fuse7zOutStream*>(node->buffer);
		memcpy(buf, &stream->buffer[offset], size);
		return size;
	}

};

void * fuse7z_initlib(char const * archive, char const * cwd) {
	void * lib = new Fuse7z_lib7zip(archive, cwd);
	return lib;
}

