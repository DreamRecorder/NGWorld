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
    m_forwarders.push_back(make_pair(new LoggerForwarderConsole(), true));
    m_forwarders.push_back(make_pair(new LoggerForwarderFile(), false));
    m_forward_buf_position = 0;
}

Logger::~Logger()
{
    vector<pair<LoggerForwarder*, bool> >::iterator it;
    int i;
    for (it = m_forwarders.begin(); it != m_forwarders.end(); ++it)
    {
        if (it->second == false)
        {
            for (i = 0; i < m_forward_buf_position; i++)
                it->first->forward_log(m_forwarder_buffer[i]);
        }
        delete it->first;
    }
}

void Logger::log(const string &str, LOG_LEVEL level)
{
    // 制作消息头
    char *p = m_message_buffer;
    if(str.size() > 100) // 对于长消息，临时分配缓存
        p = new char[str.size() + 20];
    sprintf(p, "[%.3lf] %s %s\n", clock() * 1.0 / CLOCKS_PER_SEC, log_level_string[level], str.c_str());
    m_forwarder_buffer[m_forward_buf_position++] = (string)p;
    
    // 转发消息
    int i;
    for (vector<pair<LoggerForwarder*, bool> >::iterator it = m_forwarders.begin(); it != m_forwarders.end(); ++it)
    {
        if (it->second) // 实时输出
        {
            it->first->forward_log(p);
        }
        else if (m_forward_buf_position == forwarder_buffer_size) // dump缓存
        {
            for (i = 0; i < forwarder_buffer_size; i++)
                it->first->forward_log(m_forwarder_buffer[i]);
        }
    }
    
    if (m_forward_buf_position == forwarder_buffer_size)
        m_forward_buf_position = 0;
    
    if(str.size() > 100)
        delete p;
}