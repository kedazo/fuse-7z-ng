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
#include "config.h"
#include "logger.h"
#include <syslog.h>
#include <iostream>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
volatile bool win32SyslogInitialized = false;
#endif

Logger::Logger() :
        m_syslog (false)
{
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
    if(!win32SyslogInitialized){
        init_syslog(nullptr);
        win32SyslogInitialized=true;
    }
    #endif
	m_stream.str("");
	openlog(const_cast<char*>(PACKAGE), LOG_PID, LOG_USER);
}

Logger::~Logger()
{
	closelog();
}

Logger &
Logger::instance ()
{
    static Logger logger;
    return logger;
}

void
Logger::enableSyslog (bool enable)
{
    m_syslog = enable;
}

void
Logger::logger(std::string const & text)
{
	if (m_syslog) {
		syslog(LOG_INFO, "%s", text.c_str());
	}
	else {
		std::cerr << "fuse-7z: " << text << std::endl;
	}
	m_stream.str("");
}

void
Logger::err(std::string const & text)
{
	if (m_syslog) {
		syslog(LOG_ERR, "%s", text.c_str());
	}
	else {
		std::cerr << text << std::endl;
	}
}

Logger &
Logger::endl (Logger &f)
{
	f.logger(f.m_stream.str());
	return f;
}

