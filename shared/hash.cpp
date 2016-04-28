/*
 *  This file is part of NGWorld.
 *
 *  NGWorld is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  NGWorld is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NGWorld.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hash.h"
using namespace std;
namespace NEWorld
{

    namespace shared
    {

        unsigned int bkdr_hash(const string &str)
        {
            unsigned int h = 0;
            for(int i = 0; i < str.size(); i++)
                h = h * 131 + str[i];
            return (h & 0x7FFFFFFF);
        }

    }

}

