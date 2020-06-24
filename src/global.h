/*  bbfpromix
 *
 *  Copyright 2020 Thomas Ebeling <penguins@bollie.de>
 *
 *  This file is part of bbfpromix.
 *
 *  bbfpromix is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  bbfpromix is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with bbfpromix.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define BBF_NOF_INPUTS 12
#define BBF_NOF_OUTPUTS 6
#define BBF_VOL_MAX 65536
#define BBF_VOL_MIN 0
#define BBF_VOL_SLIDER_MAX 120
#define BBF_VOL_SLIDER_MIN 0
#define BBF_VOL_SLIDER_ZERO_DB 100
#define BBF_VOL_ZERO_DB (BBF_VOL_MAX/2.f)

typedef enum __bool {
    false = 0,
    true
} bool;

static const char * const BBF_INPUTS[BBF_NOF_INPUTS] = {
    "AN1", "AN2", "IN3", "IN4", "AS1", "AS2", "ADAT3", "ADAT4", "ADAT5",
    "ADAT6", "ADAT7", "ADAT8"
};

static const char * const BBF_OUTPUTS[BBF_NOF_OUTPUTS][2] = {
    { "AN1", "AN2" },
    { "PH3", "PH4" },
    { "AS1", "AS2" },
    { "ADAT3", "ADAT4" },
    { "ADAT5", "ADAT6" },
    { "ADAT7", "ADAT8" }
};


#endif
