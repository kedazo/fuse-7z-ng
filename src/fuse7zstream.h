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

#include "logger.h"
#include <lib7zip.h>
#include <cstdio>
#include <cstring>
#include <vector>

class Fuse7zOutStream : public C7ZipOutStream, public NodeBuffer
{
	private:
	unsigned long long int position;

	public:
	std::vector<char> buffer;

	Fuse7zOutStream() : position(0) {
	}

	virtual ~Fuse7zOutStream() {
	}

	virtual int Write(const void *data, unsigned int size, unsigned int *processedSize) {
        Logger &logger = Logger::instance ();
		logger << "Write " << data << " size=" << size << " processed " << processedSize << Logger::endl;
		memcpy(&buffer[position], data, size);
		*processedSize = size;
		position += size;
		return 0;
	}

	virtual int Seek(long long int offset, unsigned int seekOrigin, unsigned long long int *newPosition) {
        Logger &logger = Logger::instance ();
		logger << "Seek " << offset << " " << seekOrigin << Logger::endl;
		position = seekOrigin;
		return 0;
	}

	virtual int SetSize(unsigned long long int size) {
        Logger &logger = Logger::instance ();
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
			size_t pos = m_strFileName.find_last_of(".");
			if (pos != m_strFileName.npos) {
				m_strFileExt.resize(fileName.length() - pos);
				for (unsigned i = 0; i < m_strFileExt.length(); i++) {
					m_strFileExt[i] = m_strFileName[pos+1+i];
				}
			}
		}
		else {
			std::stringstream ss;
			ss << "Can't open " << fileName;
			throw std::runtime_error(ss.str());
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


