/*************************************************************************
 *   leotools
 *   Copyright (C) 2015 jkbenaim
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#ifndef _YAY1_H_
#define _YAY1_H_

#include <inttypes.h>

uint32_t yay1_get_size(uint8_t *s);
int yay1_decode(uint8_t *s, uint8_t *d);

#endif // _YAY1_H_
