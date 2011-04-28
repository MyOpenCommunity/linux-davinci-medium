/*
    tvp5150.h - definition for tvp5150 inputs

    Copyright (C) 2006 Hans Verkuil (hverkuil@xs4all.nl)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _TVP5150_H_
#define _TVP5150_H_

/* Number of pixels and number of lines per frame for different standards */
#define NTSC_NUM_ACTIVE_PIXELS          (720)
#define NTSC_NUM_ACTIVE_LINES           (480)
#define PAL_NUM_ACTIVE_PIXELS           (720)
#define PAL_NUM_ACTIVE_LINES            (576)

/**
 * enum tvp515x_input - enum for different decoder input pin
 *              configuration.
 */
enum tvp5150_input {
	TVP5150_COMPOSITE0 = 0,
	TVP5150_COMPOSITE1,
	TVP5150_SVIDEO,
	TVP5150_INPUT_INVALID
};

/**
 * enum tvp5150_output - enum for different output modes
 *
 */
enum tvp5150_output {
	TVP5150_NORMAL = 0,
	TVP5150_BLACK_SCREEN,
	TVP5150_OUTPUT_INVALID
};

#endif

