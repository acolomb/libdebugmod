///@file
///@brief	Runtime management API
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


#ifndef DEBUG_MOD_CONTROL_H_
#define DEBUG_MOD_CONTROL_H_

#include "debug_mod.h"


/// Numeric index in module lists
typedef unsigned char debug_mod_index_t;


/// Maximum number of debug modules that can be tracked
extern const debug_mod_index_t debug_mod_max;


///@brief Pre-record configuration for a debug module
///
/// This function can be called for a module identifier which has
/// already been registered.  In that case, the stored configuration
/// settings are copied to the new structure provided and its address
/// is recorded for later reconfiguration.
///
///@return - Negative for newly registered module
///        - Positive if a previous configuration was overwritten
///        - Zero on failure (list full)
char debug_mod_register(
    debug_mod* restrict dm		///< [in] Configuration structure
    );


#ifdef DEBUG_MOD_DYNAMIC
///@name Dynamic reconfiguration of debug streams and output prepare functions
///
/// Must be enabled at compile time by defining the macro
/// DEBUG_MOD_DYNAMIC before including this header file.
///
///@{

///@brief Update configuration for one or all known modules
///
/// If a registered module with the given identifier is found, its
/// configuration structure is updated.  Otherwise, it is silently
/// ignored.  Set the parameter to NULL to match all registered
/// modules.
void debug_mod_update(
    const char* restrict module,	///< [in] Module to configure or NULL for all known
    debug_mod_f func,			///< [in] Override output prepare function
    FILE* restrict stream		///< [in] Override output stream
    );

///@brief Disable debugging for one or all known modules
///
/// If a registered module with the given identifier is found, its
/// debugging is disabled.  Otherwise, it is silently ignored.  Set
/// the parameter to NULL to match all registered modules.
void debug_mod_disable(
    const char* module			///< [in] Module to disable or NULL for all known
    );

///@}
#endif //DEBUG_MOD_DYNAMIC


#ifdef DEBUG_MOD_SAVE
///@name Save and restore current debug module configuration
///
/// Must be enabled at compile time by defining the macro
/// DEBUG_MOD_SAVE before including this header file.
///
///@{

///@brief Access the list of known debug modules
///
/// The returned pointer provides read-only access to the
/// configuration for all known modules through a list of addresses.
/// The number of elements is returned in the size parameter.  The
/// list may contain NULL values.
///
///@return Start of the address list or NULL on error (wrong argument)
const debug_mod *const * debug_mod_list(
    debug_mod_index_t *size		///< [out] Where to write the list size
    );

///@brief Save current module configurations to the provided array
///
/// The provided array will be overwritten with the current module
/// configurations.
///
///@return Number of entries overwritten
debug_mod_index_t debug_mod_save(
    debug_mod saved[],			///< [out] Configuration array to overwrite
    debug_mod_index_t size		///< [in] Number of elements in the array
    );

///@brief Restore saved module configurations
///
/// Configuration for modules with an unknown identifier is recorded
/// and will be used once a matching module is registered.
///
///@return Number of entries restored
debug_mod_index_t debug_mod_restore(
    debug_mod saved[],			///< [in] Configuration array to read
    debug_mod_index_t size		///< [in] Number of elements in the array
    );

///@}
#endif //DEBUG_MOD_SAVE

#endif //DEBUG_MOD_CONTROL_H_
