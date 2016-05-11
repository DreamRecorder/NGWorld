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
#include <iostream>
#include <sstream>
using namespace std;

void LoggerForwarderConsole::forward_log(const std::string &str)
{
    // 直接向终端吐出
    cout << str;
}

LoggerForwarderFile::LoggerForwarderFile() : m_file_name("ngworld.log")
{
    m_fout.open(m_file_name.c_str(), ios::out | ios::app);
    m_fout << endl << "NGWorld logger started" << endl;
}

LoggerForwarderFile::LoggerForwarderFile(const string &file_name) : m_file_name(file_name)
{
    m_fout.open(m_file_name.c_str(), ios::out | ios::app);
    m_fout << endl << "NGWorld logger started" << endl;
}

LoggerForwarderFile::~LoggerForwarderFile()
{
    m_fout.close();
}

void LoggerForwarderFile::forward_log(const std::string &str)
{
    m_fout << str;
}

// 使用默认配置，一个终端转发器，一个文件转发器
Logger::Logger()
{
    m_log_forwarders.push_back(make_pair(new LoggerForwarderConsole(), true));
    m_log_forwarders.push_back(make_pair(new LoggerForwarderFile(), false));
    m_last_write_position = -1;
}

Logger::~Logger()
{
    int i, size = (int)m_logs.size();
    for (vector<pair<LoggerForwarder*, bool> >::iterator it = m_log_forwarders.begin(); it != m_log_forwarders.end(); ++it)
    {
        if (it->second == false)
        {
            for (i = m_last_write_position+1; i < size; ++i)
            {
                it->first->forward_log(m_logs[i]);
            }
        }
        delete it->first;

    }
}

void Logger::log(const string &str, LOG_LEVEL level)
{
    // 制作消息头
    stringstream ss;
    ss.precision(5);
    string result;
    ss << '[' << clock() * 1.0 / CLOCKS_PER_SEC << "] ";
    switch (level)
    {
        case LOG_LEVEL_VERBOSE:
            ss << "(VV) ";
            break;
        case LOG_LEVEL_INFO:
            ss << "(II) ";
            break;
        case LOG_LEVEL_WARNING:
            ss << "(WW) ";
            break;
        case LOG_LEVEL_ERROR:
            ss << "(EE) ";
            break;
        default:
            ss << "(??) ";
    }
    ss << str << endl;
    result = ss.str();
    m_logs.push_back(result);
    
    // 转发消息
    int i, size = (int)m_logs.size();
    for (vector<pair<LoggerForwarder*, bool> >::iterator it = m_log_forwarders.begin(); it != m_log_forwarders.end(); ++it)
    {
        if (it->second) // 实时输出
        {
            it->first->forward_log(result);
        }
        else if (size - m_last_write_position > logger_forwarder_max_buffer) // 超过缓存大小之后输出
        {
            for (i = m_last_write_position+1; i < size; ++i)
            {
                it->first->forward_log(m_logs[i]);
            }
        }
    }
    if (size - m_last_write_position > logger_forwarder_max_buffer)
        m_last_write_position = size-1;
}
