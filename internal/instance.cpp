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

#include "instance.h"

RandGen *rng;
Logger *logger;

void init_global_variables()
{
#if (defined __GNUC__) && (defined __RDRND__)
    rng = new IntelRandGen();
#else
    rng = new MersenneRandGen();
#endif

    logger = new Logger();
}

void free_global_variables()
{
    delete rng;
    delete logger;
}

