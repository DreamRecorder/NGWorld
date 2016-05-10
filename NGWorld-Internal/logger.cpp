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

#include "logger.h"
#include <fstream>
#include <sstream>
using namespace std;

Logger::Logger()
{
    m_log_fname = "ngworld.log";
    m_write_to_file = true;
}

Logger::~Logger()
{
    if (m_write_to_file)
    {
        ofstream fout(m_log_fname, ios::app);
        for(vector<string>::iterator it = m_logs.begin(); it != m_logs.end(); ++it)
            fout << *it; // 消息内已有endl
        fout.close();
    }
}

void Logger::log(const string &str, LOG_LEVEL level)
{
    stringstream ss;
    ss << '[' << clock() * 1.0 / CLOCKS_PER_SEC << "] ";
    ostream *os;
    switch(level)
    {
        case LOG_LEVEL_VERBOSE:
            ss << "(VV) ";
            os = &verbosestream;
            break;
        case LOG_LEVEL_INFO:
            ss << "(II) ";
            os = &infostream;
            break;
        case LOG_LEVEL_WARNING:
            ss << "(WW) ";
            os = &warningstream;
            break;
        case LOG_LEVEL_ERROR:
            ss << "(EE) ";
            os = &errorstream;
            break;
        default:
            ss << "(??) ";
            os = &errorstream;
    }
    ss << str << endl;
    m_logs.push_back(ss.str());
    (*os) << ss.str();
}