/*
 * This file is part of NGWorld.
 * (C) Copyright 2016 DLaboratory
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LOGGER_H_
#define _LOGGER_H_

enum LOG_LEVEL
{
    LOG_LEVEL_VERBOSE,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    
    LOG_LEVLE_COUNT
};

#include <vector>
#include <iostream>

class Logger
{
private:
    std::vector<std::string> m_logs;
    std::string m_log_fname;
    bool m_write_to_os, m_write_to_file;

public:
    Logger();
    ~Logger();
    
    void log(const std::string &str, LOG_LEVEL level = LOG_LEVEL_VERBOSE);
};

extern std::ostream verbosestream;
extern std::ostream infostream;
extern std::ostream warningstream;
extern std::ostream errorstream;

#endif