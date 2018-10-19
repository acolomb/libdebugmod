///@file
///@brief	Runtime management API implementation
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


#ifndef DEBUG_MOD_MAX
/// Size of the array to track module configurations
#define DEBUG_MOD_MAX 4
#endif



/// List of tracked module configuration structures
static debug_mod *mods[DEBUG_MOD_MAX] = { NULL };



const debug_mod_index_t debug_mod_max = sizeof(mods) / sizeof(*mods);
debug_mod_f debug_mod_default_func = NULL;



/// Copy relevant fields from one config structure to another
static inline void
debug_mod_copy_config(
    debug_mod* restrict dst,		///< [out] Destination config structure
    const debug_mod* restrict src)	///< [in] Source configuration
{
    dst->func = src->func;
    dst->stream = src->stream;
}



char
debug_mod_register(debug_mod* restrict dm)
{
    if (dm && dm->module) {
	for (debug_mod_index_t i = 0; i < debug_mod_max; ++i) {
	    if (mods[i] == NULL) {	//first empty slot
		// Record configuration pointer
		mods[i] = dm;
		return -1;
	    } else if (mods[i]->module &&
		       0 == strcmp(dm->module, mods[i]->module)) {	//already registered
		// Apply previously stored config
		debug_mod_copy_config(dm, mods[i]);
		mods[i] = dm;
		return 1;
	    }
	}
    }
    // No suitable slot found or parameter error
    dm->func = NULL;	//avoid recursive function call
    return 0;
}



inline void
debug_mod_preinit(debug_mod* restrict self)
{
    char r = debug_mod_register(self);

    if (r < 0) {	//new entry registered
	self->func = debug_mod_default_func;
	self->stream = stderr;
    }
}



char
debug_mod_init(debug_mod* restrict self,
	       const char* restrict context)
{
    debug_mod_preinit(self);
    if (self->func) return self->func(self, context);
    return 0;
}



#ifdef DEBUG_MOD_DYNAMIC
///@brief Update configuration for one or all known modules
///
/// Registered modules are matched by identifier string.  An unknown
/// identifier is silently ignored.  Identifier NULL matches all
/// registered modules.
static inline void
debug_mod_update_config(
    const debug_mod* restrict dm)	///< [in] Reference configuration
{
    if (! dm) return;

    for (debug_mod_index_t i = 0; i < debug_mod_max; ++i) {
	if (mods[i] == NULL) {	//first empty slot
	    break;
	} else if (! dm->module) {	//no module specified, do all
	    debug_mod_copy_config(mods[i], dm);
	} else if (mods[i]->module
		   && 0 == strcmp(mods[i]->module, dm->module)) {
	    debug_mod_copy_config(mods[i], dm);
	    break;
	}
    }
}



void
debug_mod_update(const char* restrict module,
		 debug_mod_f func,
		 FILE* restrict stream)
{
    debug_mod update = {
	.func	= func,
	.stream	= stream,
	.module	= module,
    };

    if (! func) return;

    debug_mod_update_config(&update);
}



void
debug_mod_disable(const char* module)
{
    debug_mod disable = {
	.func	= NULL,
	.stream	= NULL,
	.module	= module,
    };

    debug_mod_update_config(&disable);
}
#endif //DEBUG_MOD_DYNAMIC



#ifdef DEBUG_MOD_SAVE
const debug_mod *const *
debug_mod_list(debug_mod_index_t *size)
{
    if (! size) return NULL;

    *size = debug_mod_max;
    return (const debug_mod *const *) mods;
}



debug_mod_index_t
debug_mod_save(debug_mod saved[],
	       debug_mod_index_t size)
{
    debug_mod_index_t i;
    debug_mod **dm;

    // Loop through module list
    for (i = 0, dm = mods; dm < mods + sizeof(mods); ++dm) {
	if (*dm == NULL) {	//first empty slot
	    break;
	} else {
	    saved[i] = **dm;
	    if (++i >= size) break;
	}
    }
    return i;
}



debug_mod_index_t
debug_mod_restore(debug_mod saved[],
		  debug_mod_index_t size)
{
    debug_mod_index_t i;
    debug_mod **dm;

    // Loop through module list
    for (i = 0, dm = mods; dm < mods + sizeof(mods); ++dm) {
	if (*dm == NULL) {		//empty slot
	    // Point slot to the saved stream configuration
	    *dm = saved + i;
	} else if ((*dm)->module && saved[i].module &&
		   0 == strcmp((*dm)->module, saved[i].module)) {
	    debug_mod_copy_config(*dm, saved + i);
	} else continue;
	if (++i >= size) break;	//end of saved entries
    }
    return i;
}
#endif //DEBUG_MOD_SAVE
