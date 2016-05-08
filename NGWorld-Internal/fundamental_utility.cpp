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

#include "fundamental_utility.h"

#ifdef NGWORLD_OS_UNIX
// include UNIX头文件
#include <unistd.h>
#elif NGWORLD_OS_WINDOWS
// include Windows头文件
#error NGWorld support for Windows platform is not implemented yet.
#endif

void OSLayer::sleep_us(const int &us)
{
#ifdef NGWORLD_OS_UNIX
    ::usleep(us);
#elif NGWORLD_OS_WINDOWS
#error NGWorld support for Windows platform is not implemented yet.
#endif
}

void OSLayer::sleep_ms(const int &ms)
{
#ifdef NGWORLD_OS_UNIX
    ::usleep(ms * 1000);
#elif NGWORLD_OS_WINDOWS
#error NGWorld support for Windows platform is not implemented yet.
#endif
}

void OSLayer::sleep_s(const int &s)
{
#ifdef NGWORLD_OS_UNIX
    ::sleep(s);
#elif NGWORLD_OS_WINDOWS
#error NGWorld support for Windows platform is not implemented yet.
#endif
}

