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

#include <vector>
#include <string>
#include <fstream>

enum LOG_LEVEL
{
    LOG_LEVEL_VERBOSE,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,

    LOG_LEVLE_COUNT
};

static const char log_level_string[][5] =
{
    "(VB)", // LOG_LEVEL_VERBOSE
    "(II)", // LOG_LEVEL_INFO
    "(WW)", // LOG_LEVEL_WARNING
    "(EE)", // LOG_LEVEL_ERROR
};

class LoggerForwarder
{
public:
    LoggerForwarder() {}
    virtual ~LoggerForwarder() {}
    virtual void forward_log(const std::string &str) = 0;
};

class LoggerForwarderConsole : public LoggerForwarder
{
public:
    void forward_log(const std::string &str);
};

class LoggerForwarderFile : public LoggerForwarder
{
private:
    const std::string m_file_name;
    std::ofstream m_fout;
public:
    LoggerForwarderFile();
    LoggerForwarderFile(const std::string &file_name);
    ~LoggerForwarderFile();
    void forward_log(const std::string &str);
};

class Logger
{
private:
    static const int forwarder_buffer_size = 8;
    std::string m_forwarder_buffer[forwarder_buffer_size];
    int m_forward_buf_position;
    std::vector<std::pair<LoggerForwarder*, bool> > m_forwarders;
    LOG_LEVEL m_notice_level;

    // 在调制消息头的过程中，防止频繁地分配和释放内存，
    // 提前一次性分配好一个长度为128字节的缓存空间。
    // 在不够用的情况下，再临时分配和释放所需的内存。
    char m_message_buffer[128];

public:
    Logger(LOG_LEVEL least_notice_level = LOG_LEVEL_INFO);
    ~Logger();

    void log(const std::string &str, LOG_LEVEL level = LOG_LEVEL_VERBOSE);
};

#endif
