///@file
///@brief	Basic API for debug output
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


#ifndef DEBUG_MOD_H_
#define DEBUG_MOD_H_

#include <stdio.h>	//for FILE* type, NULL


/// Handle for a module's debug configuration
typedef struct debug_mod debug_mod;

///@brief Function pointer prototype to prepare output
///
/// One such function is called before every debug output.  It may
/// alter the debug configuration and do administrative tasks such as
/// prefixing each output with a timestamp.
///
///@return Zero to skip debug output, non-zero otherwise
typedef char (*debug_mod_f)(
    debug_mod* self,			///< [in] Access to the module configuration
    const char* restrict context	///< [in] Name of the calling function
);

/// Configuration for a single debug module
struct debug_mod {
    /// Setup function to decide and prepare each debug output
    debug_mod_f		func;
    /// The actual stream handle to use for output
    FILE*		stream;
    /// Module identifier to register for configuration access
    const char*		module;
};


/// Default output prepare function set at first use
extern debug_mod_f debug_mod_default_func;


///@brief Initialize and register debug module on first usage
///
/// The given configuration structure is centrally registered to allow
/// later reconfiguration.  If debug_mod_default_func is set, or a
/// configuration was already registered with the same module
/// identifier, its associated output prepare function is then called.
///
///@see debug_mod_f
char debug_mod_init(
    debug_mod* self,			///< [in] The module's debug configuration
    const char* restrict context	///< [in] Name of the calling function
);


///@name Setup functions
///@{

#ifdef DEBUG_MOD_ENABLE

#undef DEBUG_MOD_ENABLE
/// Compile time switch to enable debug output
#define DEBUG_MOD_ENABLE 1	//dummy value for true condition

///@brief Set up debugging for the current module
///
/// Calling this macro once per module is a prerequisite to use any
/// debug_mod functionality.
///
///@param modulestring Module identifier to register
#define DEBUG_MOD_INIT(modulestring)		\
    static debug_mod _debug_mod = {		\
	.func	= debug_mod_init,		\
	.stream	= NULL,				\
	.module	= (modulestring),		\
    };

#else //DEBUG_MOD_ENABLE not defined

/// Compile time switch to enable debug output
#define DEBUG_MOD_ENABLE 0	//dummy value for false condition

///@brief Set up debugging for the current module
///
///@param modulestring Module identifier to register
#define DEBUG_MOD_INIT(modulestring)		\
    static debug_mod _debug_mod = {		\
	.func	= NULL,				\
	.stream	= NULL,				\
	.module	= (modulestring),		\
    };

#endif //DEBUG_MOD_ENABLE


/// Directly access module's own debugging stream
#define debug_mod_get_stream()			\
    (_debug_mod.stream)
/// Reconfigure module's own debugging stream
#define debug_mod_set_stream(s)			\
    { _debug_mod.stream = (s); }
/// Reconfigure module's own debugging function
#define debug_mod_set_func(f)			\
    { _debug_mod.func = (f); }
/// Disable debugging in current module during runtime
#define debug_mod_disable_self()		\
    { debug_mod_set_func(NULL); }

///@}


///@name Basic API for debug output
///@{

/// Condition statement to check if debugging is enabled and call
/// output prepare function
#define DEBUG_CONDITION				\
    if (DEBUG_MOD_ENABLE && _debug_mod.func &&	\
	_debug_mod.func(&_debug_mod, __func__))

///@brief Call function with configured stream as first argument.
///
/// The DEBUG_CONDITION macro is evaluated first, calling any output
/// prepare function if set.  On success, a call to the given function
/// is generated with the configured stream passed as its first
/// parameter.
///
///@param f	Debug output function
///@param ...	Additional trailing arguments passed to function
#define DEBUGF(f, ...) {				\
	DEBUG_CONDITION					\
	    f(debug_mod_get_stream(), __VA_ARGS__); }

///@brief Call function with configured stream as last argument.
///
/// The DEBUG_CONDITION macro is evaluated first, calling any output
/// prepare function if set.  On success, a call to the given function
/// is generated with the configured stream passed as its last
/// parameter.
///
///@param f	Debug output function
///@param ...	Additional leading arguments passed to function
#define DEBUGL(f, ...) {				\
	DEBUG_CONDITION					\
	    f(__VA_ARGS__, debug_mod_get_stream()); }

///@}


#endif //DEBUG_MOD_H_
