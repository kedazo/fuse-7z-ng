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
#include <sstream>

class Logger
{
    public:
        static Logger & instance ();
        ~Logger();

        void enableSyslog (bool enable = true);

        void logger(std::string const & text);
        void err(std::string const & text);
        template<typename T>
            inline Logger & operator<<(T const & data) {
                m_stream << data;
                return *this;
            }
        static Logger &endl(Logger &f);

        typedef Logger& (*LoggerManipulator)(Logger&);

        Logger& operator<<(LoggerManipulator manip) {
            return manip(*this);
        }

    private:
        Logger();

        bool                m_syslog;
        std::stringstream   m_stream;
};


