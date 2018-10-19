///@file
///@brief	Test program to demonstrate API
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


#include <debug_mod_control.h>

#include <string.h>



// Lazy initialization using source file name as identifier
DEBUG_MOD_INIT(__FILE__)



///@brief Dump all available context before debug output
///@see debug_mod_f
char
verbose(debug_mod* restrict self,
	const char* restrict context)
{
    if (! self->stream) return 0;

    fprintf(self->stream, "%s\t%s()\t%s()\n\t",
	    self->module ? self->module : "no module",
	    context ? context : "no context",
	    __func__);

    return 1;
}



///@brief Insert a tab for indentation before debug output
///@see debug_mod_f
char
indent(debug_mod* restrict self,
       const char* restrict context __attribute__((unused)))
{
    fputc('\t', self->stream);
    return 1;
}



///@brief Prefix debug output with function context
///@see debug_mod_f
char
context(debug_mod* restrict self,
	const char* restrict context)
{
    fputs(context, self->stream);
    fputs("()\n", self->stream);
    return indent(self, context);
}



// Declare function from other module
void test_extern(void);



/// Test some debug output in this translation unit
void
test_local(void)
{
    // Use default config
    DEBUGF(fprintf, "default\n");

    // Update config for this module
#ifdef DEBUG_MOD_DYNAMIC
    debug_mod_update(__FILE__, verbose, stderr);
#endif
    DEBUGF(fprintf, "have %d module slots\n", debug_mod_max);

    // Update config for all modules
#ifdef DEBUG_MOD_DYNAMIC
    debug_mod_update(NULL, indent, stderr);
#endif
    DEBUGF(fprintf, "update to indent, stderr\n");

    // Test configuration saving
#ifdef DEBUG_MOD_SAVE
    debug_mod_index_t size, i;
    debug_mod foo[debug_mod_max];
    debug_mod *const *bars;

    debug_mod_save(foo, sizeof(foo) / sizeof(*foo));
    bars = debug_mod_list(&size);
    for (i = 0; i < size && i < sizeof(foo) / sizeof(*foo); ++i) {
	if (bars[i] && memcmp(bars[i], foo + i, sizeof(debug_mod)) == 0) {
	    DEBUGF(fprintf, "match %d\n", i);
	}
    }
    debug_mod_restore(foo, sizeof(foo) / sizeof(*foo));
#endif

    // Disable output for this module
#ifdef DEBUG_MOD_DYNAMIC
    debug_mod_disable(__FILE__);
#endif
    DEBUGF(fprintf, "should be disabled!\n");
}


/// Test program for libdebugmod control API
int
main(void)
{
#ifdef DEBUG_MOD_DYNAMIC
    debug_mod dm = { verbose, stderr, "test_ext_module.c" };
    debug_mod_register(&dm);
#endif

    test_extern();

    // Update default function before local initializations
    debug_mod_default_func = context;
    test_local();

    return 0;
}
