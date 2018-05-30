///@file
///@brief	Separate translation unit for test program
///@copyright	Copyright (C) 2014  Andre Colomb
///
/// This file is part of libdebugmod.
///
/// libdebugmod is free software: you can redistribute it and/or modify
/// it under the terms of the GNU Lesser General Public License as
/// published by the Free Software Foundation, either version 3 of the
/// License, or (at your option) any later version.
///
/// libdebugmod is distributed in the hope that it will be useful, but
/// WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
/// Lesser General Public License for more details.
///
/// You should have received a copy of the GNU Lesser General Public
/// License along with this program.  If not, see
/// <http://www.gnu.org/licenses/>.
///
///@author	Andre Colomb <src@andre.colomb.de>


// Uncomment this to always prevent debug builds of this module
//#undef DEBUG_MOD_ENABLE
#include <debug_mod.h>



// Lazy initialization using source file name as identifier
DEBUG_MOD_INIT(__FILE__)



/// Test some debug output in this translation unit
void
test_extern(void)
{
    // Use default (or preconfigured) config
    DEBUGL(fputs, "default\n");

    DEBUGF(fprintf, "foo %d\n", 1);

    DEBUGF(fprintf, "bar %d\n", 2);

    // Disable output for this module
#ifdef debug_mod_disable_self
    debug_mod_disable_self();
#endif

    DEBUGF(fprintf, "baz %d\n", 3);
}
